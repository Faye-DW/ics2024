#include <memory.h>

static void *pf = NULL;
#define PGSIZE 4096
void* new_page(size_t nr_page) {
  pf += (nr_page * PGSIZE);
  return pf;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
  //return NULL;
  int page_num = n/PGSIZE;
  void *end = new_page(page_num);
  void *start = end-n;
  memset(start, 0, n);
  return start;
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  //printf("here\n");
  vme_init(pg_alloc, free_page);
  //printf("here2\n");
#endif
}
