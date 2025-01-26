#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
int vsprintf(char *out, const char *fmt, va_list ap);
static void reverse(char *s, int len) {
  char *end = s + len - 1;
  char tmp;
  while (s < end) {
    tmp = *s;
    *s = *end;
    *end = tmp;
    ++s;
    --end;
  }
}

static int chuli(int n, char *s) {
  int i = 0, sign = n;
  int num;
  if (sign < 0) n = -n;
  do {
    num = n % 10;
    s[i] = '0' + num;
    i++;
  } while ((n /= 10) > 0);
  if (sign < 0) {s[i] = '-';i++;}
  s[i] = '\0';
  reverse(s, i);

  return i;
}
static int chuli_unsigned(unsigned long n, char *s, unsigned int base_size) {
    int i = 0;
    do {
        unsigned long num = n % 10;
        s[i++] = '0' + num;
    } while ((n /= 10) > 0);
    s[i] = '\0';
    reverse(s, i);
    return i;
}

int printf(const char *fmt, ...) {
  char buff[2048];
  va_list arg;
  va_start (arg, fmt);
  int lenth = vsprintf(buff, fmt, arg);
  const char *p=buff;
  for(;*p!='\0';p++){
    putch(*p);
  }
  return lenth;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
    char *start = out;
    while (*fmt != '\0') {
        if (*fmt != '%') {
            *out++ = *fmt++;
        } else {
            ++fmt;
            switch (*fmt) {
                case 'd':
                    out += chuli(va_arg(ap, int), out);
                    break;
                case 'u':
                    out += chuli_unsigned(va_arg(ap, unsigned int), out, sizeof(unsigned int));
                    break;
                case 'h': {
                    ++fmt; 
                    if (*fmt == 'u') {
                        out += chuli_unsigned(va_arg(ap, unsigned int) , out, sizeof(unsigned short));
                    } else if (*fmt == 'h' && *(fmt + 1) == 'u') {
                        ++fmt; 
                        out += chuli_unsigned(va_arg(ap, unsigned int) , out, sizeof(unsigned char));
                    } else {
                        --fmt; 
                    }
                    break;
                }
                case 's': {
                    const char *s = va_arg(ap, const char*);
                    while (*s != '\0') {
                        *out++ = *s++;
                    }
                    break;
                }
                case 'c':
                    *out++ = (char) va_arg(ap, int);  
                    break;
                default:
                    *out++ = *fmt;
                    break;
            }
            ++fmt;
        }
    }
    *out = '\0';
    return out - start;
}



#include <stdarg.h>

int sprintf(char *out, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char *start = out;

  while (*fmt != '\0') {
    if (*fmt != '%') {
      *out = *fmt;
      ++out;
      ++fmt;
    } else {
      ++fmt;
      switch (*fmt) {
        case 'd':
          out += chuli(va_arg(args, int), out);
          break;
        case 's': {
          const char *s = va_arg(args, const char*);
          while (*s != '\0') {
            *out = *s;
            ++out;
            ++s;
          }
          break;
        }
        case 'c':{
          const char *s = va_arg(args, const char*);
          *out = *s;
          ++out;
          break;
        }
        default:
          *out = *fmt;
          ++out;
          break;
      }
      ++fmt;
    }
  }
  *out = '\0';
  va_end(args);

  return out - start;
}

#include <stdarg.h>
#include <stddef.h>

int snprintf(char *out, size_t n, const char *fmt, ...) {
    if (!out || !fmt) {
        return -1;  // 参数错误
    }

    va_list args;
    va_start(args, fmt);

    size_t written = 0;
    const char *ptr = fmt;

    while (*ptr) {
        if (*ptr == '%') {
            ptr++;  // 跳过'%'
            if (!*ptr) break;  // 避免后续没有字符

            switch (*ptr) {
                case 'd': {
                    // 格式化整数
                    int value = va_arg(args, int);
                    char buffer[32];
                    int len = snprintf(buffer, sizeof(buffer), "%d", value);
                    // 确保不溢出缓冲区
                    for (int i = 0; i < len && written < n - 1; i++) {
                        out[written++] = buffer[i];
                    }
                    break;
                }
                case 's': {
                    // 格式化字符串
                    const char *str = va_arg(args, const char *);
                    if (!str) str = "(null)";
                    while (*str && written < n - 1) {
                        out[written++] = *str++;
                    }
                    break;
                }
                case 'c': {
                    // 格式化单个字符
                    char c = (char)va_arg(args, int);
                    if (written < n - 1) {
                        out[written++] = c;
                    }
                    break;
                }
                case '%': {
                    // 打印百分号
                    if (written < n - 1) {
                        out[written++] = '%';
                    }
                    break;
                }
                default: {
                    // 处理不认识的格式符，直接输出'%'和下一个字符
                    if (written < n - 1) {
                        out[written++] = '%';
                    }
                    if (written < n - 1) {
                        out[written++] = *ptr;
                    }
                    break;
                }
            }
        } else {
            // 普通字符直接输出
            if (written < n - 1) {
                out[written++] = *ptr;
            }
        }
        ptr++;
    }

    // 确保字符串正确终止
    if (n > 0) {
        out[written < n ? written : n - 1] = '\0';
    }

    va_end(args);

    return written;
}



int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
