#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#define CHUNK_SIZE 1048576	/* 1 MB per chunk */
#define MAX_CHUNK 1000	

typedef struct
{
	char* ChunkPtr;
	int ChunkSize;
	int ChunkID;
	int end;

} ChunksData;

typedef struct
{
	char *file_contents;
	int fd;
	size_t size;
} FileMapping;

/* Global array to hold file mappings to clean them later */
FileMapping *file_mappings = NULL;
int file_mappings_count = 0;

/* mutex and condition variables */
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond_fill  = PTHREAD_COND_INITIALIZER;
pthread_cond_t queue_cond_empty = PTHREAD_COND_INITIALIZER;

/* the Bounded Buffer for the threads interaction */
ChunksData *chunks[MAX_CHUNK];

/* array to store the zipping result of each chunck */
char* results[MAX_CHUNK];

/* global variables needed */
int fill = 0, use = 0, count = 0, idx = 0;
int files_num;
int end = 0;
int mx = 0;

/* queue interfaces */
void enqueue(ChunksData *c);
ChunksData *dequeue(void);	

/* Function to append a number and a character to a string */
char* append_number_and_char(char *str, int num, char c);

/* consumer threads function */
void* compress(void* arg);

/* producer thread function */
void* producer(void* arg);

/* function to merge all the outputs the threads */
void Merge();

int main(int argc, char *argv[])
{
	/* check for crrect usage */
	if (argc < 2) {
		printf("wzip: file1 [file2 ...]\n");
		exit(1);
	}

	/* get number of files and properly needed threads */
	files_num = argc;
	int threads_num = get_nprocs() - 1;

	/* Create producer thread */
	pthread_t producer_thread;
	pthread_create(&producer_thread, NULL, producer, (void*)argv);

	/* create consumer threads */
	pthread_t consumers[threads_num];
	for(int i = 0; i < threads_num; ++i)
	{
		pthread_create(&consumers[i], NULL, compress, NULL);
	}

	/* Wait for the producer to finish */
	pthread_join(producer_thread, NULL);

	/* Wait for consumer threads to finish */
	for (int i = 0; i < threads_num; i++) {
		pthread_join(consumers[i], NULL);
	}

	/* merge the output */
	Merge();

	/* Clean everything up */
	for(int i = 0; i <= mx; i++) {
		if (results[i] != NULL) {
			free(results[i]); 
		}
	}
	for (int i = 0; i < file_mappings_count; i++) {
		if (file_mappings[i].file_contents != NULL) {
			munmap(file_mappings[i].file_contents, file_mappings[i].size);
			close(file_mappings[i].fd); 
		}
	}

	exit(0);
}

void* producer(void* arg)
{
	const char** filenames = (const char**)arg;

	/* Number of files to be processed */
	file_mappings_count = files_num - 1;

	/* Allocate memory for mappings */
	file_mappings = malloc(file_mappings_count * sizeof(FileMapping));

	for (int i = 1; i < files_num; i++) {

		int fd = open(filenames[i], O_RDONLY);
		if (fd < 0) {
			printf("wzip: cannot open file\n");
			exit(1);
		}

		/* Get the file size */
		struct stat sb;
		if (fstat(fd, &sb) == -1) {
			perror("fstat");
			close(fd);
			exit(1);
		}

		/* Memory-map the file */
		char *file_contents = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
		if (file_contents == MAP_FAILED) {
			perror("mmap");
			close(fd);
			exit(1);
		}

		/* Store the mapping in the array to clean them later */
		file_mappings[i - 1].file_contents = file_contents;
		file_mappings[i - 1].fd = fd;
		file_mappings[i - 1].size = sb.st_size;

		/* Divide the file into chunks and produce work */
		for(size_t offset = 0; offset < sb.st_size; offset += CHUNK_SIZE)
		{
			int chunk_size = (offset + CHUNK_SIZE > sb.st_size) ? sb.st_size - offset : CHUNK_SIZE;
			ChunksData *data = malloc(sizeof(ChunksData));
			if (data == NULL) {
				fprintf(stderr, "Memory allocation failed\n");
				exit(1);
			}
			data->ChunkPtr = file_contents + offset;
			data->ChunkSize = chunk_size;
			data->ChunkID = i + offset/CHUNK_SIZE;
			data->end = (i == files_num - 1 && offset + CHUNK_SIZE >= sb.st_size) ? 1 : 0;

			pthread_mutex_lock(&queue_mutex);
			while(count == MAX_CHUNK) {
				pthread_cond_wait(&queue_cond_empty, &queue_mutex);
			}

			enqueue(data);
			pthread_cond_signal(&queue_cond_fill);
			pthread_mutex_unlock(&queue_mutex);
		}

	}
	return NULL;
}

void* compress(void* arg)
{
	while (1) {
		pthread_mutex_lock(&queue_mutex);
		while (count == 0 && end == 0) {
			pthread_cond_wait(&queue_cond_fill, &queue_mutex);
		}
		if (count == 0 && end) {
			pthread_mutex_unlock(&queue_mutex);
			return NULL;
		}
		ChunksData* data = dequeue();
		pthread_cond_signal(&queue_cond_empty); 
		pthread_mutex_unlock(&queue_mutex);

		/* do the real work */
		char prev_char = EOF;
		char current_char;
		int char_count = 0;
		char* current_compress = NULL;
		/* Process the file contents */
		for (size_t j = 0; j < data->ChunkSize; j++) {
			current_char = data->ChunkPtr[j];
			if (current_char == prev_char) {
				char_count++;
			} else {
				if (prev_char != EOF) {
					current_compress = append_number_and_char(current_compress, char_count, prev_char);
				}
				prev_char = current_char;
				char_count = 1;
			}
		}

		/* Write the last character */
		if (prev_char != EOF && prev_char != '\0') {

			current_compress = append_number_and_char(current_compress, char_count, prev_char);
		}

		if(data->ChunkID > mx) mx = data->ChunkID;
		results[data->ChunkID] = current_compress;

		if (data->end == 1) {
			end = 1;
			/* Wake all the consumer threads */
			pthread_cond_broadcast(&queue_cond_fill);
			pthread_mutex_unlock(&queue_mutex);
			free(data);
			return NULL;
		}

		free(data);

	}
	return NULL;
}

void Merge()
{

	char last_chr = '\0';
	int last_n = 0;

	for (int i = 1; i <= mx; i++) {
		int idx = 0;
		int num = 0;

		/* Parse the first number in the current string */
		while (results[i][idx] >= '0' && results[i][idx] <= '9') {
			num *= 10;
			num += results[i][idx] - '0';
			idx++;
		}

		char current_chr = results[i][idx];

		/* If the last character matches the current character, merge them */
		if (last_chr == current_chr) {
			last_n += num;
		} else {
			/* If there's a previous character, print it */
			if (last_chr != '\0') {
				fwrite(&last_n, sizeof(int), 1, stdout);
				putchar(last_chr);
			}

			last_chr = current_chr;
			last_n = num;
		}

		/* Process the rest of the string for additional characters */
		idx++;
		while (results[i][idx] != '\0') {
			if (results[i][idx] >= '0' && results[i][idx] <= '9') {
				int additional_num = 0;
				while (results[i][idx] >= '0' && results[i][idx] <= '9') {
					additional_num *= 10;
					additional_num += results[i][idx] - '0';
					idx++;
				}

				char next_chr = results[i][idx];
				if (next_chr == last_chr) {
					last_n += additional_num;
				} else {
					fwrite(&last_n, sizeof(int), 1, stdout);
					putchar(last_chr);
					last_chr = next_chr;
					last_n = additional_num; 
				}
			} else {
				idx++;
			}
		}
	}

	/* Print the last processed character */
	if (last_chr != '\0') {
		fwrite(&last_n, sizeof(int), 1, stdout);
		putchar(last_chr);
	}

}

void enqueue(ChunksData *c)
{
	chunks[fill] = c ;
	fill = (fill + 1) % MAX_CHUNK ;
	count++ ;
}

ChunksData *dequeue(void)
{
	ChunksData *c = chunks[use];
	use = (use + 1) % MAX_CHUNK ;
	count-- ;
	return c;
}

char* append_number_and_char(char *str, int num, char c) {

	int num_length = snprintf(NULL, 0, "%d", num); 
	int current_length = str ? strlen(str) : 0;    
	int new_length = current_length + num_length + 1;
	char *new_str = realloc(str, new_length + 1);

	if (new_str == NULL) {
		fprintf(stderr, "Memory allocation failed\n");
		exit(1);
	}

	sprintf(new_str + current_length, "%d", num);
	new_str[current_length + num_length] = c;
	new_str[new_length] = '\0';                     

	return new_str;
}
