#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>
word_t expr(char *e, bool *success);

typedef struct watchpoint 
{
  int NO;
  struct watchpoint *next;
  char* expr;
  word_t ans;
} WP;
void init_wp_pool();
void new_up(char *cal);
void free_wp(int num);
void check_wp(bool* good);
void scan_wp();
//void cc();
//bool free_all();
#endif
