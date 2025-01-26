#include <common.h>
#include "syscall.h"
#include <fs.h>
#include <debug.h>
#include <proc.h>
#include <elf.h>
void do_sys_exit();
void do_sys_yield(Context *c);
void do_sys_write(Context *c);
void do_sys_brk(Context *c);
void do_sys_read(Context *c);
void do_sys_open(Context *c);
void do_sys_lseek(Context *c);
void do_sys_close(Context *c);
void do_sys_gettimeofday(Context *c);
//void do_sys_execve(const char* fname);
int do_sys_execve(const char *filename, char *const argv[], char *const envp[]);
void do_sys_read(Context *c)
{
  c->GPRx = fs_read((int)c->GPR2, (void *)c->GPR3, (size_t)c->GPR4);
}

void do_sys_open(Context *c)
{
  c->GPRx = fs_open((char *)c->GPR2, (int)c->GPR3, (int)c->GPR4);
}

void do_sys_lseek(Context *c)
{
  c->GPRx = fs_lseek((int)c->GPR2, (size_t)c->GPR3, (int)c->GPR4);
}

void do_sys_exit()
{
  //halt(0);
  naive_uload(NULL, "/bin/nterm");
}

void do_sys_yield(Context *c)
{
  yield();
  c->GPRx = 0;
}

void do_sys_write(Context *c)
{
  int fd = c->GPR2;
  intptr_t buf = c->GPR3;
  size_t count = c->GPR4;
  c->GPRx = fs_write((int)fd, (void *)buf, (size_t)count);
}

void do_sys_brk(Context *c)
{
  c->GPRx = 0;
}

void do_sys_close(Context *c)
{
  c->GPRx = fs_close((int)c->GPR2);
}

void do_sys_gettimeofday(Context *c)
{
  uint64_t us = io_read(AM_TIMER_UPTIME).us;
  ((struct timeval *)(c->GPR2))->tv_sec = us / 1000000;
  ((struct timeval *)(c->GPR2))->tv_usec = us - us / 1000000 * 1000000;
  //redefine in "common.h"
}

/*void do_sys_execve(const char* fname)
{
  naive_uload(NULL, fname);
}*/

int do_sys_execve(const char *filename, char *const argv[], char *const envp[]) {
  context_uload(current, filename, argv, envp);
  switch_boot_pcb();
  yield();
  return 0;
}

void do_syscall(Context *c) 
{
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  //printf("Syscall %d\n",a[0]);
  switch (a[0]) 
  {
    case SYS_exit :  do_sys_exit()  ; break;
    case SYS_yield:  do_sys_yield(c); break;
    case SYS_write:  do_sys_write(c); break;
    case SYS_brk  :  do_sys_brk(c)  ; break;
    case SYS_read :  do_sys_read(c) ; break;
    case SYS_open :  do_sys_open(c) ; break;
    case SYS_lseek:  do_sys_lseek(c); break;
    case SYS_close:  do_sys_close(c); break;
    //case SYS_execve: do_sys_execve((char*)a[1]); break;
    case SYS_execve: do_sys_execve((char*)a[1], (char**)a[2], (char**)a[3]); break;
    case SYS_gettimeofday: do_sys_gettimeofday(c); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
