#include <common.h>
#include <debug.h>
#define YIELD 1
#define SYSCALL 2
#define TIMER 5
static Context* do_event(Event e, Context* c) {
  switch (e.event) {
    //case YIELD:  printf("Get Here so do exit\n");halt(0);break;
    case YIELD:    c = schedule(c); break;
    case SYSCALL:  do_syscall(c);   break;
    case TIMER:    c = schedule(c); /*printf("get here boy\n");*/ break;
    default: panic("Unhandled event ID = %d", e.event);
  }
  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
