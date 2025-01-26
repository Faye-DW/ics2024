#include <proc.h>
#include <common.h>
extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;
#define RAMDISK_SIZE ((&ramdisk_end) - (&ramdisk_start))
typedef unsigned long uintptr;

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1)
  {
    //Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    Log("Hello World from Nanos-lite with arg %s for the %dth time!", (char *)arg, j);
    //for (int i=0;i<10000000;i++)
    //{

    //}
    j ++;
    yield();
  }
}

void init_proc() {
  /*char* stra = "fuck you\n";
  context_kload(&pcb[0], hello_fun, (void *)stra);*/ //kernel context
  
  char* strb = "fuck nju\n";
  context_kload(&pcb[0], hello_fun, (void *)strb); //kernel context

  char *argv[] = {"/bin/pal", "--skip", NULL};
  char *envp[] = {NULL};
  context_uload(&pcb[1], "/bin/pal", argv, envp);

  /*char *argv[] = {"/bin/menu", NULL, NULL};
  char *envp[] = {NULL};
  context_uload(&pcb[1], "/bin/menu", argv, envp);*/
  
  switch_boot_pcb();
  Log("Initializing processes...");
  if (RAMDISK_SIZE!=0)
  {
    printf("The ramdisk isn't empty\n");
    //naive_uload(NULL, "/bin/bmp-test");
    //naive_uload(NULL, "/bin/pal");
    //naive_uload(NULL, "/bin/event-test");
    //naive_uload(NULL, "/bin/file-test");
    //naive_uload(NULL, "/bin/nslider");
    //naive_uload(NULL, "/bin/menu");
    //naive_uload(NULL, "/bin/hello");
    //naive_uload(NULL, "/bin/nterm");
    //naive_uload(NULL, "/bin/bird");
  }
  else
  {
    printf("Ramdisk is empty\n");
  }
}

Context* schedule(Context *prev) {
  current->cp = prev;
  current = (current == &pcb[0])?(&pcb[1]):(&pcb[0]);
  return current->cp;
  //return NULL;
}

void context_kload(PCB* pc, void (*entry)(void *), void *arg)
{
  pc->cp = kcontext((Area){pc->stack, pc+1}, hello_fun, arg);
}

/*void context_uload(PCB *p, const char *filename)
{
  uintptr entry = loader(p, filename);
  p->cp = ucontext(&(p->as), (Area) {p->stack, p+1}, (void *)entry);
  p->cp->GPRx = (uintptr)heap.end;
}*/

void context_uload(PCB *p, const char *filename, char *const argv[], char *const envp[]) {
  uintptr_t entry = loader(p, filename);

  int argc = 0, envc = 0;
  while (argv[argc] != NULL) argc++;
  while (envp[envc] != NULL) envc++;

  //char* us1 = (char*)heap.end;
  char* us1 = (char*)new_page(8);
  char* stack_end = us1;
  // clone argv
  for (int i = 0; i < argc; i++) {
    size_t len = strlen(argv[i]) + 1; // include null character
    us1 -= len;
    strncpy(us1, argv[i], len);
  }
  // clone envp
  for (int i = 0; i < envc; i++) {
    size_t len = strlen(envp[i]) + 1; // include null character
    us1 -= len;
    strncpy(us1, envp[i], len);
  }

  uintptr_t* us2 = (uintptr_t *)us1;
  us2 -= (argc + envc + 3);

  us2[0] = argc;
  //char* us_tmp = (char*)heap.end;
  char* us_tmp = (char*)stack_end;
  for (int i = 0; i < argc; i++) {
    size_t len = strlen(argv[i]) + 1; 
    us_tmp -= len;
    us2[i + 1] = (uintptr_t)us_tmp;
  }
  us2[argc + 1] = 0;
  for (int i = 0; i < envc; i++) {
    size_t len = strlen(envp[i]) + 1; // include null character
    us_tmp -= len;
    us2[argc + i + 2] = (uintptr_t)us_tmp;
  }
  us2[argc + 2 + envc] = 0;

  p->cp = ucontext(&p->as, (Area) { p->stack, p + 1 }, (void *)entry);
  p->cp->GPRx = (uintptr_t)us2;
}
