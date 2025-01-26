#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) 
{
  size_t len = 0;
  while (*s!='\0')
  {
    s++;
    len++;
  }
  return len;
  //panic("Not implemented");
}

char *strcpy(char *dst, const char *src) 
{
  char *base = dst;  // Preserve the base of dst to return it
  while ((*dst++ = *src++) != '\0');  // Copy string and null terminator
  return base;
  //panic("Not implemented");
}

char *strncpy(char *dst, const char *src, size_t n) 
{
  char *base = dst;
  while (n-- && (*dst++ = *src++) != '\0');  // Copy string or until n characters
  if (*(dst-1) != '\0') {  // Ensure null termination if n was reached
    *dst = '\0';
  }
  return base;
  //panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
  char *base = dst;
  while (*dst != '\0') {  // Find end of dst
    dst++;
  }
  while ((*dst++ = *src++) != '\0');  // Concatenate src after dst
  return base;
  //panic("Not implemented");
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  return (unsigned char)*s1 - (unsigned char)*s2;
  //panic("Not implemented");
}

int strncmp(const char *s1, const char *s2, size_t n) {
  while (n-- > 0 && *s1 && *s2 && (*s1==*s2))
  {
    s1++;
    s2++;
  }
  return (unsigned char)*s1 - (unsigned char)*s2;
  //panic("Not implemented");
}


void *memset(void *s, int c, size_t n) 
{
  for (size_t i = 0; i < n; i++) 
  {
    ((unsigned char *)s)[i] = (unsigned char)c;
  }
  //panic("Not implemented");
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  if (dst < src) {
    for (size_t i = 0; i < n; i++) {
      ((unsigned char *)dst)[i] = ((const unsigned char *)src)[i];
    }
  } else if (dst > src) {
    const unsigned char *s = (const unsigned char *)src + n;
    unsigned char *d = (unsigned char *)dst + n;
    while (n--) {
      *(--d) = *(--s);
    }
  }
  return dst;
  //panic("Not implemented");
}


void *memcpy(void *out, const void *in, size_t n) {
  unsigned char *a = out;
  const unsigned char *b = in;
  while (n--)
  {
    *a = *b;
    a++;
    b++;
  }
  return out;
  //panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *a = s1;
  const unsigned char *b = s2;
  while (n--)
  {
    if (*a != *b)
    {
      return (unsigned char)*a - (unsigned char)*b;
    }
    a++;
    b++;
  }
  return 0;
  //panic("Not implemented");
}

#endif
