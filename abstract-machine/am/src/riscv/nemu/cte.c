#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>
#include <debug.h>
static Context* (*user_handler)(Event, Context*) = NULL;
#define IRQ_ECALL 0x0000000b
#define IRQ_TIMER 0x80000007

void Logf(Context *c)
{
  printf("%u %u %u\n",c->mcause, c->mstatus, c->mepc);
  for (int i=0;i<32;i++)
  {
    printf("%u ", (c->gpr)[i]);
    if (i%4==3)
    {
      printf("\n");
    }
  }
}
Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
    //printf("get here the sys function\n");
    switch (c->mcause) 
    {
      case IRQ_ECALL: ev.event = ((c->GPR1)==-1)?EVENT_YIELD:EVENT_SYSCALL;break;
      case IRQ_TIMER: ev.event = EVENT_IRQ_TIMER; break;
      default: ev.event = EVENT_ERROR;break;
    }
    c = user_handler(ev, c);
    assert(c != NULL);
  }
  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

#define OFFSET 0x00000007
Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  //printf("build proc\n");
  Context* kernel_proc = (Context*)kstack.end - 1;//stack grow from high addr to low addr
  kernel_proc -> mepc = (uintptr_t)(entry-4);//mret instuction will + 4
  (kernel_proc -> gpr)[10] = (uintptr_t)arg;//$a0 reg is the arg register
  (kernel_proc -> mstatus) |= (1 << OFFSET);
  return kernel_proc;
  //return NULL;
}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");//
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
