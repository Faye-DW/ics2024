#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>

static int evtdev = -1;
static int fbdev = -1;
int event_fd;
int fb_fd;
int dispinfo_fd;


static int screen_w = 0, screen_h = 0;
static int canvas_w=0,canvas_h=0;
static int canvas_x=0,canvas_y=0;
uint32_t NDL_GetTicks() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}


int NDL_PollEvent(char *buf, int len) 
{
  return read(event_fd, buf, len);
}



void NDL_OpenCanvas(int *w, int *h) 
{
  if (getenv("NWM_APP")) 
  {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) 
    {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
  if (*w == 0 && *h == 0) 
  {
    *w = screen_w;
    *h = screen_h;
  }
  canvas_w = *w;
  canvas_h = *h;
  canvas_x=(screen_w - canvas_w) / 2;
  canvas_y=(screen_h - canvas_h) / 2;
}


void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  int fd = open("/dev/fb", 0, 0);
  for(int i = 0; i < h; i++) {
    lseek(fd, (screen_w * (i + y) + x) * 4, SEEK_SET);
    write(fd, pixels + w * i, w * 4);
  }
  close(fd);
}


void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) 
{
  if (getenv("NWM_APP")) 
  {
    evtdev = 3;
  }
  event_fd = open("/dev/events", 0);
  fb_fd = open("/dev/fb", 0);
  dispinfo_fd = open("/proc/dispinfo", 0);
  char buf[64];
  assert(read(dispinfo_fd, buf, sizeof(buf)));
  sscanf(buf, "WIDTH:%d\nHEIGHT:%d", &screen_w, &screen_h);
  return 0;
}

void NDL_Quit() {
}



