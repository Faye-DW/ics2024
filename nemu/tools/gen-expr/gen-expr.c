/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdbool.h>
#include <regex.h>


#define MAX_NUM 100
#define MAX_DEPTH 10
#define MAX_LENGTH 65536
// this should be enough

static char buf[MAX_LENGTH] = {};
static char code_buf[MAX_LENGTH + 128] = {}; // a little larger than `buf`
static char oper[4] = {'+', '-', '*', '/'};
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

unsigned int choose(unsigned int n)
{
  return (rand() % n);
}

char choose_op()
{
  return oper[choose(4)];
}

unsigned int choose_num()
{
  unsigned int number;
  while (1)
  {
    number = choose(MAX_NUM);
    if (number!=0)/*avoid the zero*/
    {
      return number;
    }
  }
}
static void gen_expr(char *expr, int depth, size_t size)
{
  if (depth>=MAX_DEPTH || size < 2)
  {
    sprintf(expr, "%d", choose_num(MAX_NUM));
    return;
  }
  int num;
  char opr;
  char left[MAX_LENGTH];
  char right[MAX_LENGTH];
  size_t len;
  switch (choose(3)) 
  {
        case 0:
            num = choose_num(MAX_NUM);
            sprintf(expr, "%d", num);
            break;
        case 1:
            gen_expr(left, depth + 1, MAX_LENGTH);
            sprintf(expr, "(%s)", left);
            break;
        case 2:
            gen_expr(left, depth + 1, MAX_LENGTH / 2);
            gen_expr(right, depth + 1, MAX_LENGTH / 2);
            opr = choose_op();
            len = sprintf(expr, "%s %c %s", left, opr, right);
            if (len >= size) 
            {
              expr[size - 1] = '\0';
            }
            break;
  }
}

static void gen_rand_expr() 
{
  buf[0] = '\0';
  gen_expr(buf, 0, MAX_LENGTH);
  return;
}
int main(int argc, char *argv[]) 
{
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) 
  {
    gen_rand_expr();
    
    sprintf(code_buf, code_format, buf);
    
    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);
    
    printf("%u %s\n", result, buf);
  }
  return 0;
}
