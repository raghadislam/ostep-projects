#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mapreduce.h"

#define DEFAULT_CAPACITY (128)
bool DEBUG = false;

/* global variables */
KV_array *KV_arr;
Mapper global_mapper;
Reducer global_reducer;
Partitioner global_partition;
int num_partitions;

unsigned long MR_DefaultHashPartition(char *key, int num_partitions) {
  unsigned long hash = 5381;
  int c;
  while ((c = *key++) != '\0')
    hash = hash * 33 + c;
  return hash % num_partitions;
}

int compare_by_key(const void *a, const void *b) {
  KeyAndValue *kva_p = (KeyAndValue *) a;
  KeyAndValue *kvb_p = (KeyAndValue *) b;
  return strcmp(kva_p->key, kvb_p->key);
}

int qsort_strcmp(const void* a, const void* b) {
    const char* aa = *(const char**) a;
    const char* bb = *(const char**) b;
    return strcmp(aa,bb);
}

void init_kv_array() {
  KV_arr = (KV_array *) malloc(num_partitions * sizeof(KV_array));
  assert(KV_arr != NULL);
  for (int i = 0; i < num_partitions; i++) {
    KV_arr[i].key_values_arr = (KeyAndValue *) malloc(DEFAULT_CAPACITY * sizeof(KeyAndValue));
    assert(KV_arr[i].key_values_arr != NULL);
    KV_arr[i].size = 0;
    KV_arr[i].capacity = DEFAULT_CAPACITY;
    pthread_mutex_init(&(KV_arr[i].mutex), NULL);
  }
}

void free_kv_array() {
  for (int i = 0; i < num_partitions; i++) {
    KV_array *kv_p = &(KV_arr[i]);
    for (int j = 0; j < kv_p->size; j++) {
      KeyAndValue *kav_p = &(kv_p->key_values_arr[j]);
      free(kav_p->key);
      for (int k = 0; k < kav_p->size; k++) {
        free(kav_p->values[k]);
      }
      free(kav_p->values);
    }
    free(kv_p->key_values_arr);
  }
  free(KV_arr);
}

void MR_Emit(char *key, char *value) {
  int partition_num = global_partition(key, num_partitions);
  KV_array *kv_p = &(KV_arr[partition_num]);

  pthread_mutex_lock(&(kv_p->mutex));

  int key_index = -1;
  for (int i = 0; i < kv_p->size; i++) {
    if (strcmp(key, kv_p->key_values_arr[i].key) == 0) {
      key_index = i;
      break;
    }
  }

  /* check if not found */
  if (key_index == -1) {
    /* add a new Key */
    if (kv_p->size == kv_p->capacity) {
      kv_p->key_values_arr = (KeyAndValue *) realloc(kv_p->key_values_arr, kv_p->capacity * 2 * sizeof(KeyAndValue));
      assert(kv_p->key_values_arr != NULL);
      kv_p->capacity *= 2;
    }
    KeyAndValue *kav_p = &(kv_p->key_values_arr[kv_p->size]);
    kav_p->key = strdup(key);
    assert(kav_p->key != NULL);
    kav_p->values = (char **) malloc(DEFAULT_CAPACITY * sizeof(char *)),
    assert(kav_p->values != NULL);
    kav_p->values[0] = strdup(value);
    assert(kav_p->values[0] != NULL);
    kav_p->size = 1;
    kav_p->capacity = DEFAULT_CAPACITY;
    kav_p->index = 0;
    kv_p->size++;
  } else {
    /* add this value to an existing values array */
    KeyAndValue *kav_p = &(kv_p->key_values_arr[key_index]);
    if (kav_p->size == kav_p->capacity) {
      kav_p->values = (char **) realloc(kav_p->values, kav_p->capacity * 2 * sizeof(char *));
      assert(kav_p->values != NULL);
      kav_p->capacity *= 2;
    }
    kav_p->values[kav_p->size] = strdup(value);
    kav_p->size++;
  }
  pthread_mutex_unlock(&(kv_p->mutex));
}

/* No need for locking here because each key is used by only one reducer */
char *get_next(char *key, int partition_number) {
  KV_array *kv_p = &(KV_arr[partition_number]);
  KeyAndValue *kav_p = NULL;
  for (int i = 0; i < kv_p->size; i++) {
    if (strcmp(key, kv_p->key_values_arr[i].key) == 0) {
      kav_p = &(kv_p->key_values_arr[i]);
    }
  }
  assert(kav_p != NULL);
  if (kav_p->index == kav_p->size) {
    return NULL;
  }
  return kav_p->values[kav_p->index++];
}

void *map_thread_func(void *map_thread_args) {
  mapperArgs *mapperArgsArr = (mapperArgs *) map_thread_args;
  if (DEBUG) {
    printf("mapper thread arguments %d %d\n", mapperArgsArr->start_index, mapperArgsArr->end_index);
  }
  for (int i = mapperArgsArr->start_index; i < mapperArgsArr->end_index; i++) {
    global_mapper(mapperArgsArr->argv[i]);
  }
  return NULL;
}

void *reduce_thread_func(void *partition_num_void) {
  int *partition_num_p = (int *) partition_num_void;
  KV_array *kvs_p = &(KV_arr[*partition_num_p]);
  for (int i = 0; i < kvs_p->size; i++) {
    global_reducer(kvs_p->key_values_arr[i].key, get_next, *partition_num_p);
  }
  return NULL;
}

void MR_Run(int argc, char *argv[],
	    Mapper map, int num_mappers,
	    Reducer reduce, int num_reducers,
	    Partitioner partition) {

  /* initialize global state */
  global_mapper = map;
  global_reducer = reduce;
  global_partition = partition;
  num_partitions = num_reducers;


  /* initialize the key value array */
  init_kv_array();

  /* array for mapper threads */
  pthread_t *mappers = (pthread_t *) malloc(num_mappers * sizeof(pthread_t));
  assert(mappers != NULL);

  /* array for mapper threads arguments */
  mapperArgs *mappersArgsArr = (mapperArgs *) malloc(num_mappers * sizeof(mapperArgs));

  /* assign a number of arguments for each mapper thread */
  int idx = 0;
  int *numOfArgs = (int *) calloc(num_mappers, sizeof(int));
  assert(numOfArgs != NULL);
  while (idx < argc - 1) {
    numOfArgs[idx % num_mappers]++;
    idx++;
  }

  /* initialize thread args */
  for (int i = 0; i < num_mappers; i++) {
    if (i == 0) {
      mappersArgsArr[i].start_index = 1;
    } else {
      mappersArgsArr[i].start_index = mappersArgsArr[i-1].end_index;
    }
    mappersArgsArr[i].end_index = mappersArgsArr[i].start_index + numOfArgs[i];
    mappersArgsArr[i].argv = argv;

    assert(pthread_create(&(mappers[i]), NULL, map_thread_func, &(mappersArgsArr[i])) == 0);
  }
  free(numOfArgs);

  /* Join mapper threads */
  for (int i = 0; i < num_mappers; i++) {
    assert(pthread_join(mappers[i], NULL) == 0);
  }

  /* clean up */
  free(mappers);
  free(mappersArgsArr);

  /* Sort kv_stores */
  if (DEBUG) {
    printf("Sorting keys\n");
  }
  for (int i = 0; i < num_partitions; i++) {
    qsort(KV_arr[i].key_values_arr, KV_arr[i].size, sizeof(KeyAndValue), &compare_by_key);
    for (int j = 0; j < KV_arr[j].size; j++) {
      KeyAndValue *kav_p = &(KV_arr[j].key_values_arr[j]);
      qsort(kav_p->values, kav_p->size, sizeof(char *), qsort_strcmp);
    }
  }

  /* Create reducer threads */
  if (DEBUG) {
    printf("Creating reducer threads\n");
  }
  pthread_t *reducers = (pthread_t *) malloc(num_reducers * sizeof(pthread_t));
  assert(reducers != NULL);
  int *reducerArgsArr = (int *) malloc(num_reducers * sizeof(int));
  for (int i = 0; i < num_reducers; i++) {
    reducerArgsArr[i] = i;
    assert(pthread_create(&(reducers[i]), NULL, reduce_thread_func, &(reducerArgsArr[i])) == 0);
  }

  /* Join reducer threads */
  for (int i = 0; i < num_reducers; i++) {
    assert(pthread_join(reducers[i], NULL) == 0);
  }
  free(reducerArgsArr);
  free(reducers);
  free_kv_array();
}

