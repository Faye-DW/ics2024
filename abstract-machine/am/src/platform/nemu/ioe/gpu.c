#include <am.h>
#include <nemu.h>
#include <stdint.h>
#include <sys/types.h>
#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
  
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t screen_wh = inl(VGACTL_ADDR);
  uint32_t h = screen_wh & 0xffff;
  uint32_t w = screen_wh >> 16;
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = w, .height = h,
    .vmemsz = 0
  };

}


/*void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) 
{
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  if (!ctl->sync && (w == 0 || h == 0)) return;
  uint32_t *pixels = ctl->pixels;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  uint32_t screen_w = inl(VGACTL_ADDR) >> 16;
  for (int i = y; i < y+h; i++) 
  {
    for (int j = x; j < x+w; j++) 
    {
      fb[screen_w*i+j] = pixels[w*(i-y)+(j-x)];
    }
  }
  if (ctl->sync) 
  {
    outl(SYNC_ADDR, 1);
  }
}*/


/*void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  
  // 如果宽高为0，则不进行渲染
  if (!ctl->sync && (w == 0 || h == 0)) return;

  uint32_t *pixels = ctl->pixels;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  uint32_t screen_w = inl(VGACTL_ADDR) >> 16;  // 获取屏幕的宽度
  uint32_t screen_h = inl(VGACTL_ADDR) & 0xFFFF;  // 获取屏幕的高度

  // 防止越界：确保渲染区域不会超出屏幕范围
  if (x >= screen_w || y >= screen_h) return;  // 如果起始位置就已经越界，则不渲染
  if (x + w > screen_w) w = screen_w - x;  // 如果渲染区域超出屏幕右侧，调整宽度
  if (y + h > screen_h) h = screen_h - y;  // 如果渲染区域超出屏幕底部，调整高度

  // 渲染每个像素点
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      int fb_x = x + j;  // 计算 framebuffer 中的列位置
      int fb_y = y + i;  // 计算 framebuffer 中的行位置

      // 计算 framebuffer 中的线性索引
      int fb_index = fb_y * screen_w + fb_x;
      // 将像素数据写入 framebuffer
      fb[fb_index] = pixels[i * w + j];
    }
  }

  // 如果需要同步，发出同步信号
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}*/

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) 
{
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  if (!ctl->sync && (w == 0 || h == 0)) return;
  uint32_t *pixels = ctl->pixels;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  uint32_t screen_w = inl(VGACTL_ADDR) >> 16;
  for (int i = y; i < y+h; i++) 
  {
    for (int j = x; j < x+w; j++) 
    {
      fb[screen_w*i+j] = pixels[w*(i-y)+(j-x)];
    }
  }
  if (ctl->sync) 
  {
    outl(SYNC_ADDR, 1);
  }
 
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}

