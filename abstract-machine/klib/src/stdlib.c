#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed) {
  next = seed;
}

int abs(int x) {
  return (x < 0 ? -x : x);
}

int atoi(const char* nptr) {
  int x = 0;
  while (*nptr == ' ') { nptr ++; }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr ++;
  }
  return x;
}

#define HEAP_SIZE 1000000
uint8_t Heap_Arr[HEAP_SIZE];
typedef struct heap
{
  uint8_t *heap_start;
  uint8_t *heap_end;
  int  heap_size;
  uint8_t *arr;
  uint8_t *last_malloc_start;
}Heap;

static Heap AM_Heap = {
  .heap_start = Heap_Arr,
  .heap_end = Heap_Arr + HEAP_SIZE - 1,
  .heap_size = HEAP_SIZE,
  .arr = Heap_Arr,
  .last_malloc_start = Heap_Arr,
};

void *malloc(size_t size) {
  // On native, malloc() will be called during initializaion of C runtime.
  // Therefore do not call panic() here, else it will yield a dead recursion:
  //   panic() -> putchar() -> (glibc) -> malloc() -> panic()
#if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
  //panic("Not implemented");
  uint8_t *new_malloc = AM_Heap.last_malloc_start;
  AM_Heap.last_malloc_start += size;
#endif
  
  return (void*)new_malloc;
}

void free(void *ptr) {
}

#endif
