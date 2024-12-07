#ifndef __mapreduce_h__
#define __mapreduce_h__

// Different function pointer types used by MR
typedef char *(*Getter)(char *key, int partition_number);
typedef void (*Mapper)(char *file_name);
typedef void (*Reducer)(char *key, Getter get_func, int partition_number);
typedef unsigned long (*Partitioner)(char *key, int num_partitions);

// External functions: these are what you must define
void MR_Emit(char *key, char *value);

unsigned long MR_DefaultHashPartition(char *key, int num_partitions);

void MR_Run(int argc, char *argv[], 
	    Mapper map, int num_mappers, 
	    Reducer reduce, int num_reducers, 
	    Partitioner partition);


typedef struct MapThreadArgs {
  char** argv;
  int start_index;
  int end_index;
} mapperArgs;

typedef struct KeyAndValue {
  char *key;
  char **values;
  int size;
  int capacity;
  int index;
} KeyAndValue;

typedef struct KV_array {
  KeyAndValue *key_values_arr;
  int size;
  int capacity;
  pthread_mutex_t mutex;
} KV_array;

#endif // __mapreduce_h__
