#include <isa.h>
#include <cpu/cpu.h>
#define IRQ_TIMER 0x80000007
#define MIE_OFFSET 0x00000003
#define MPIE_OFFSET 0x00000007
word_t isa_raise_intr(word_t NO, vaddr_t epc) {

  cpu.csr.mepc = epc;
  cpu.csr.mcause = NO;
  
  cpu.csr.mstatus = (cpu.csr.mstatus & ~(1 << MPIE_OFFSET)) | ((cpu.csr.mstatus >> MIE_OFFSET) & 0x1) << MPIE_OFFSET ;
  cpu.csr.mstatus &= ~(1 << MIE_OFFSET);

  return cpu.csr.mtvec;
}



word_t isa_query_intr() {
  //return INTR_EMPTY;
  if ((cpu.csr.mstatus & (1 << MIE_OFFSET)) && cpu.INTR) {
    cpu.INTR = false;
    return IRQ_TIMER;
  }
  return INTR_EMPTY;
}

void restore_interrupt() { 
  // 还原之前保存的 MPIE 到 MIE
  cpu.csr.mstatus = (cpu.csr.mstatus & ~(1 << MIE_OFFSET)) | ((cpu.csr.mstatus >> MPIE_OFFSET) & 0x1)<< MIE_OFFSET;
  // 将 MPIE 位置为 1
  cpu.csr.mstatus |= (1 << MPIE_OFFSET);
 
}

