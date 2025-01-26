#include <isa.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <regex.h>
#include <limits.h>
#include <memory/host.h>
#include <memory/paddr.h>
#include <device/mmio.h>
#define MAX_LENGTH_ 65535
enum {
  TK_NOTYPE = 256, 
  TK_EQ,
  TK_NUMBER,
  DEREF,
  TK_HEXA_NUMBER,
  TK_REGISTER,
  TK_AND,
  TK_M,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {
  {" +", TK_NOTYPE},
  {"==", TK_EQ},
  {"\\b0x[0-9A-Fa-f]+\\b", TK_HEXA_NUMBER},
  {"[0-9]+",TK_NUMBER},
  {"\\$0|\\$ra|\\$sp|\\$gp|\\$tp|\\$t[0-6]|\\$s[0-9]|\\$s10|\\$s11|\\$a[0-7]", TK_REGISTER},
  {"\\+", '+'},
  {"\\-",'-'},
  {"\\*",'*'},
  {"\\/",'/'},
  {"\\(",'('},
  {"\\)",')'},
  {"&&", TK_AND},
};

#define NR_REGEX ARRLEN(rules)
static regex_t re[NR_REGEX] = {};
void init_regex() 
{
  int i;
  char error_msg[128];
  int ret;
  for (i = 0; i < NR_REGEX; i ++) 
  {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0)
    {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}
typedef struct token 
{
  int type;
  char *str;
} Token;
static Token tokens[MAX_LENGTH_] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;
static bool make_token(char *e)
{
  int position = 0;
  int i;
  regmatch_t pmatch;
  nr_token = 0;
  while (e[position] != '\0') 
  {
    for (i = 0; i < NR_REGEX; i ++) 
    {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
      {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;
        /*Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);*/
        position += substr_len;
        if (i!=0)
        {
          tokens[nr_token].type = rules[i].token_type;
          tokens[nr_token].str = (char*)malloc((substr_len+1) * sizeof(char));
          strncpy(tokens[nr_token].str,substr_start,substr_len);
          tokens[nr_token].str[substr_len] = '\0';
          nr_token++;
        }
        /*switch (rules[i].token_type) 
        {
          case TK_NUMBER:
            printf("You match number\n");
            break;
          case TK_NOTYPE:
            printf("You match space\n");
            break;
          case TK_HEXA_NUMBER:
            printf("You match hexanumber\n");
            break;
          case TK_REGISTER:
            printf("You match registers\n");
            break;
          case '+':
            printf("You match +\n");
            break;
          case '-':
            printf("You match -\n");
            break;
          case '*':
            printf("You match *\n");
            break;
          case '/':
            printf("You match /\n");
            break;
          case '(':
            printf("You match (\n");
            break;
          case ')':
            printf("You match )\n");
            break;
          case TK_EQ:
            printf("You match ==\n");
            break;
          case TK_AND:
            printf("You match &&\n");
            break;
          default: printf("false\n");
          //TODO();
            break;
        }*/
        break;
      }
    }
    if (i == NR_REGEX) 
    {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  return true;
}

bool check(int left, int right)
{
    int balance = 0;
    for (int j = left; j <= right; j++) 
    {
        if (tokens[j].type == '(')
            balance++;
        else if (tokens[j].type == ')') 
        {
            if (balance == 0)
                return false;
            balance--;
        }
    }
    return balance == 0;
}

int findop(int left, int right) 
{
    int in_br = 0;
    for (int i = right; i >= left; i--) 
    {
        if (tokens[i].type == ')')
        {
          in_br++;
        }
        else if (tokens[i].type == '(')
        {
          in_br--;
        }
        else if (in_br==0 && (tokens[i].type==TK_EQ||tokens[i].type==TK_AND))
        {
          return i;
        }
        else if (in_br == 0 && (tokens[i].type == '+' || tokens[i].type == '-'))
        {
          return i;
        }
    }
    for (int i = right; i >= left; i--) 
    {
        if (tokens[i].type ==')')
        {
          in_br++;
        }
        if (tokens[i].type == '(')
        {
          in_br--;
        }
        if (in_br == 0 && (tokens[i].type == '*' || tokens[i].type == '/'))
        {
          return i;
        }
    }
    for (int i=left;i<=right;i++)
    {
      if (tokens[i].type==TK_M)
      {
        return i;
      }
    }
    for (int i=left;i<=right;i++)
    {
      if (tokens[i].type==DEREF)
      {
        return i;
      }
    }
    return 0;
}

word_t eval(int left, int right)
{
    if (left > right) 
    {
      return 0;
    } 
    else if (left == right) 
    {
        if (tokens[left].type==TK_REGISTER)
        {
          bool if_get_reg = true;
          char* help = tokens[left].str + 1;
          return isa_reg_str2val(help, &if_get_reg);
        }
        return strtoul(tokens[left].str, NULL, 0);
    } 
    else if (check(left+1, right-1)&&(tokens[left].type=='('&&tokens[right].type==')')) 
    {
      return eval(left+1,right-1);
    }
    else
    {
        int op = findop(left, right);
        if (tokens[op].type==DEREF)
        {
          int ans2 = eval(op+1, right);
          ans2 = paddr_read(ans2, 4);
          return ans2;
        }
        else if (tokens[op].type==TK_M)
        {
          word_t ans1 = -eval(op+1, right);
          return ans1;
        }
        else
        {
          word_t ans1 = eval(left, op - 1);
          word_t ans2 = eval(op + 1, right);
          switch (tokens[op].type) 
          {
            case '+': return ans1 + ans2;
            case '-': return ans1 - ans2;
            case '*': return ans1 * ans2;
            case '/': if (ans2 == 0) 
                      {
                        return 0;
                      } 
                      else
                      {
                        return ans1 / ans2;
                      }
            case TK_EQ:
                if (ans1==ans2)
                {
                  return 1;
                }
                else
                {
                  return 0;
                }
                
            case TK_AND:
                if (ans1&&ans2)
                {
                  return 1;
                }
                else
                {
                  return 0;
                }
                
          }
        }
    }
    return 0;
}
word_t expr(char *e, bool *success) 
{
  if (!make_token(e))
  {
    *success = false;
    return 0;
  }
  for (int i=0;i<nr_token;i++)
  {
    if (tokens[i].type=='*'&&
    (i==0||
    tokens[i-1].type=='('||tokens[i-1].type=='+'||tokens[i-1].type=='-'
    ||tokens[i-1].type=='*'||tokens[i-1].type=='/'))
    {
      tokens[i].type = DEREF;
    }
    if (tokens[i].type=='-'&&
    (i==0||
    tokens[i-1].type=='('||tokens[i-1].type=='+'||tokens[i-1].type=='-'
    ||tokens[i-1].type=='*'||tokens[i-1].type=='/'))
    {
      tokens[i].type=TK_M;
    }
  }
  word_t ans = eval(0,nr_token-1);
  return ans;
}