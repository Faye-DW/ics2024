#include "sdb.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() 
{
  int i;
  for (i = 0; i < NR_WP; i ++) 
  {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }
  head = NULL;
  free_ = wp_pool;
}

void new_up(char *cal)
{
  if (head==NULL&&free_==NULL)
  {
    init_wp_pool();
  }
  if (free_==NULL)
  {
    //assert(0);
    printf("NO free watch point\n");
    return;
  }
  else
  {
    WP *node = free_;
    free_ = free_->next;
    node->next = NULL;

    node->expr = (char *)malloc(strlen(cal)+1);
    strcpy(node->expr, cal);
    bool if_su;
    node->ans = expr(cal, &if_su);
    if (head==NULL)
    {
      head = node;
    }
    else
    {
      WP *flag = head;
      while (flag->next!=NULL)
      {
        flag = flag->next;
      }
      flag->next = node;
    }
  }
  return;
}

void scan_wp()
{
  if (head==NULL)
  {
    printf("No watchpoints\n");
    return;
  }
  else
  {
    WP *help = head;
    while (help!=NULL)
    {
      printf("It is the %dst watchpoint ", (help->NO)+1);
      printf("watching the expression %s\n", help->expr);
      help = help->next;
    }
  }
  return;
}

/*void cc()
{
  WP* help = head;
  if (help==NULL)
  {
    printf("No wp\n");
  }
  else
  {
    while (help!=NULL)
    {
      printf("%s = %u\n", help->expr, help->ans);
      help=help->next;
    }
  }
  return;
}*/

void check_wp(bool *good)
{
  if (head==NULL)
  {
    *good = true;
    return;
  }
  WP *scan = head;
  while (scan!=NULL)
  {
    bool if_succ;
    word_t now_ans = expr(scan->expr, &if_succ);
    if (now_ans != scan->ans)
    {
      *good = false;
      printf("The %dst watchpoint %s changed from 0x%08x to 0x%08x\n", (scan->NO)+1, scan->expr, scan->ans, now_ans);
      scan->ans = now_ans;
    }
    scan = scan->next;
  }
  *good = true;
  return;
}

/*bool free_all()
{
    if (head != NULL)
    {
        WP *help = head;
        while (help != NULL)
        {
            WP *next_wp = help->next;
            free_wp(help->NO);
            help = next_wp;
        }
        head = NULL;
        return true;
    }
    return true;
}*/


void free_wp(int num)
{
  WP *fa = NULL;
  WP *find = head;
  while (find->NO!=num&&find!=NULL)
  {
    fa = find;
    find = find->next;
  }
  if (find==NULL)
  {
    printf("The %dst watchpoint is not in the watchpool\n", num);
    return;
  }
  if (fa==NULL)
  {
    find = find->next;
  }
  else
  {
    fa->next = find->next;
  }
  find->next = NULL;
  if (free_==NULL)
  {
    free_ = find;
  }
  else
  {
    WP *help = free_;
    while (help->next!=NULL)
    {
      help = help->next;
    }
    help->next = find;
  }
  return;
}

