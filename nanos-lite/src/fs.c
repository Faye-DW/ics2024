#include <fs.h>
#include <common.h>
#define FILE_NUM sizeof(file_table)/sizeof(Finfo)
typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;
#define RAMDISK_SIZE ((&ramdisk_end) - (&ramdisk_start))


typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  size_t open_offset;//start from the file
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_EVE, FD_FB, FD_DIS};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, 0, invalid_read, invalid_write},//0
  [FD_STDOUT] = {"stdout", 0, 0, 0, invalid_read, serial_write},//1
  [FD_STDERR] = {"stderr", 0, 0, 0, invalid_read, serial_write},//2
  {"/dev/events", 0, 0, 0, events_read, invalid_write},//3
  {"/dev/fb",0,0,0, invalid_read, fb_write},//4
  {"/proc/dispinfo",0,0,0, dispinfo_read, invalid_write},//5
#include "files.h"
};

void init_fs() 
{
  //look_file();
  // TODO: initialize the size of /dev/fb
  AM_GPU_CONFIG_T config = io_read(AM_GPU_CONFIG);
  int width = config.width;
  int height = config.height;
  file_table[4].size = width * height * sizeof(uint32_t);
}

//API to operate files using ramdisk function
void look_file()
{
  printf("ALL %d files\n",FILE_NUM);
  for (int i=0;i<FILE_NUM;i++)
  {
    printf("%s\n",file_table[i].name);
  }
}

int fs_close(int fd)
{
  return 0;
}

int fs_open(const char *pathname, int flags, int mode){
  for (int i = 0; i < FILE_NUM; i++) 
  {
        if (strcmp(pathname, file_table[i].name) == 0) 
        {
            file_table[i].open_offset = 0;
            return i;
        }
  }
  panic("file %s not found", pathname);
  return -1;
}


size_t fs_read(int fd, void *buf, size_t len){
  if (fd <= 2) 
  {
    printf("ignore read %s", file_table[fd].name);
    return 0;
  }
  else if(fd==FD_EVE)
  {
    return file_table[3].read(buf, 0, len);
  }
  else if (fd==FD_DIS)
  {
    return file_table[5].read(buf, 0, len);
  }
  size_t read_len = len;
  size_t open_offset = file_table[fd].open_offset;
  size_t size = file_table[fd].size;
  size_t disk_offset = file_table[fd].disk_offset;
  if (open_offset > size) return 0;
  if (open_offset + len > size) read_len = size - open_offset;
  ramdisk_read(buf, disk_offset + open_offset, read_len);
  file_table[fd].open_offset += read_len;
  return read_len;
 
}



size_t fs_write(int fd, const void *buf, size_t len)
{
  if (fd==0)//stdin
  {
    printf("Write failed!\n");
    return 0;
  }
  else if (fd==1||fd==2)//stdout stderr
  {
    printf("In the sfs, we abstarct the stdout into a file\n");
    return file_table[fd].write(buf, 0, len);
  }
  else if (fd>=FILE_NUM)
  {
    printf("Write failed!\n");
    return 0;
  }
  else if (fd==FD_FB)
  {
    size_t write_len = len;
    if (file_table[fd].open_offset+write_len>file_table[fd].size)
    {
      //printf("Write bad!\n");
      write_len=file_table[fd].size-file_table[fd].open_offset;
    }
    else
    {
      //printf("Write good!\n");
    }
    file_table[4].write(buf, file_table[fd].disk_offset+file_table[fd].open_offset, len);
    file_table[fd].open_offset+=write_len;
    return write_len;
  }
  else
  {
    size_t write_len = len;
    if (file_table[fd].open_offset+write_len>file_table[fd].size)
    {
      //printf("Write bad!\n");
      write_len=file_table[fd].size-file_table[fd].open_offset;
    }
    else
    {
      //printf("Write good!\n");
    }
    ramdisk_write(buf, file_table[fd].disk_offset+file_table[fd].open_offset, write_len);
    file_table[fd].open_offset+=write_len;
    return write_len;
  }
}

size_t fs_lseek(int fd, size_t offset, int whence)
{
  if (fd<=2)
  {
    printf("Lseek failed and the file is %s\n", file_table[fd].name);
    return -1;
  }
  else
  {
    switch (whence)
    {
    case SEEK_SET:
      file_table[fd].open_offset=offset;
      break;
    case SEEK_CUR:
      file_table[fd].open_offset+=offset;
      break;
    case SEEK_END:
      file_table[fd].open_offset=file_table[fd].size+offset;
      break;
    default:
      printf("Bad whence value %d\n", whence);
      return -1;
      break;
    }
    if (file_table[fd].open_offset>file_table[fd].size)
    {
      printf("Bad lseek!Over the size!\n");
      return -1;
    }
    return file_table[fd].open_offset;
  }
}