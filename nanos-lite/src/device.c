#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  //yield();
  char* buff = (char *)buf;
  for (int i=0;i<len;i++)
  {
    putch(*(buff+i));
  }
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  //yield();
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if (ev.keycode == AM_KEY_NONE) {
    *(char*)buf = '\0';
    return 0;
  }
  int ret = snprintf(buf, len, "%s %s\n", ev.keydown?"kd":"ku", keyname[ev.keycode]);
  printf("%s\n", buf);
  return ret;
}

static int width, height;
size_t dispinfo_read(void *buf, size_t offset, size_t len) 
{
  char *_buf = buf;
  int ret = 0;
  AM_GPU_CONFIG_T config = io_read(AM_GPU_CONFIG);
  width = config.width;
  height = config.height;
  sprintf(_buf, "WIDTH:%d\nHEIGHT:%d", width, height);
  while (_buf[ret] != '\0')
    ret++;
  return ret + 1;
  return 0;
}


size_t fb_write(const void *buf, size_t offset, size_t len) {
  //yield();
  io_write(AM_GPU_FBDRAW, (offset % (width * 4)) / 4,
           offset / (width * 4), (uint32_t *)buf, len / 4, 1, true);
  return 1;
}




void init_device() {
  Log("Initializing devices...");
  ioe_init();
}



