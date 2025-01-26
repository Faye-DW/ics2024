#ifndef __ISA_RISCV_H__
#define __ISA_RISCV_H__

#include <common.h>

typedef struct
{
  vaddr_t mepc;//the pc which make the stop
  word_t mstatus;//the status of cpu
  word_t mcause;//the reason for the stopmm
  word_t mtvec;
  word_t satp;
}CSR;

typedef struct {
  word_t gpr[MUXDEF(CONFIG_RVE, 16, 32)];
  vaddr_t pc;
  CSR csr;
  bool INTR;
} MUXDEF(CONFIG_RV64, riscv64_CPU_state, riscv32_CPU_state);

// decode
typedef struct {
  uint32_t inst;
} MUXDEF(CONFIG_RV64, riscv64_ISADecodeInfo, riscv32_ISADecodeInfo);

//#define isa_mmu_check(vaddr, len, type) (MMU_DIRECT)
#define isa_mmu_check(vaddr, len, type) ((((cpu.csr.satp & 0x80000000) >> 31) == 1) ? MMU_TRANSLATE : MMU_DIRECT)

#endif