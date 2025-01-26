#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <stdint.h>
#include <memory/paddr.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}
static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}
static int cmd_q(char *args) 
{
  if (nemu_state.state == NEMU_STOP)/*the nemu has not been running*/
  {
    nemu_state.state = NEMU_QUIT;
  }
  return -1;
}
static int cmd_si(char *args)
{
  if (args==NULL)
  {
    cpu_exec(1);
    return 0;
  }
  else
  {
    unsigned long long steps;
    //steps = strtoull(args,NULL,10);
    steps = atoi(args);
    cpu_exec(steps);
    return 0;
  }
}
static int cmd_info(char *args)
{
  if (strcmp(args,"r")==0)
  {
    isa_reg_display();
  }
  else if (strcmp(args, "w")==0)
  {
    scan_wp();
  }
  return 0;
}

static int cmd_x(char* args)
{
  char* num;
  char* start;
  char *find_space = strchr(args, ' ');
  *find_space = '\0';
  num = args;
  start = find_space+1;
  unsigned long long time = strtoull(num,NULL,10);
  time = (int)time;
  bool if_ex;
  paddr_t exp = expr(start, &if_ex);
  unsigned char* begin = guest_to_host(exp);
  for (int i = 0; i < time; i++) 
  {
    unsigned int val = begin[0] | (begin[1] << 8) | (begin[2] << 16) | (begin[3] << 24);
    begin += 4;
    printf("0x%08x\n", val);
  }
  return 0;
}

static int cmd_help(char *args);

static int cmd_p(char *args)
{
  bool if_ = true;
  word_t ans = expr(args,&if_);
  printf("%u\n",ans);
  return 0;
}

static int cmd_w(char *args)
{
  new_up(args);
  return 0;
}

static int cmd_d(char *args)
{
  int num = strtoul(args, NULL, 10);
  free_wp(num);
  return 0;
}

/*int cmd_t(char *args)
{
  FILE* flag = fopen("/home/shaoyuqiao/ics2024/nemu/tools/gen-expr/input", "r");
  char help[500];
  if (flag==NULL)
  {
    printf("Error\n");
    return 0;
  }
  while (fgets(help, sizeof(help), flag) != NULL)
  {
    int len = strlen(help);
    help[len-1]='\0';
    char* find_space = strchr(help, ' ');
    *find_space = '\0';
    char* exp = find_space+1;
    printf("%s %s\n", help, exp);
    bool if_ = true;
    word_t ans = expr(exp, &if_);
    printf("%s %u\n", help, ans);
  }
  return 0;
}
*/
int cmd_it()
{
  return iringbuf_();
}

int cmd_mt()
{
  return 0;
}
static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Single step execution", cmd_si},
  { "info", "Print the register and the watchpoint", cmd_info},
  { "x", "Scan the memory", cmd_x},
  { "p", "Calculate the regex", cmd_p},
  { "w", "Set a watch point", cmd_w},
  { "d", "Delete a watch point", cmd_d},
  { "it", "Run the instruction trace", cmd_it},
  { "mt", "Run the memory trace", cmd_mt},
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() 
{
  /*#ifdef BATCH_MODE
    sdb_set_batch_mode();
  #endif*/
  if (is_batch_mode)
  {
    cmd_c(NULL);
  }
  for (char *str; (str = rl_gets()) != NULL; ) 
  {
    char *str_end = str + strlen(str);
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) 
    {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) 
    {
      if (strcmp(cmd, cmd_table[i].name) == 0)
      {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
