#pragma once
#include <inttypes.h>

typedef struct {
  void *device_instance;
  int (*tty_d_read)(void *device_instance, char *ptr, int len);
  int (*tty_d_write)(void *device_instance, char *ptr, int len);
} tty_device_t;

int tty_stdio_read(int file, char *ptr, int len);
int tty_stdio_write(int file, char*ptr, int len);
//ttyとして有効なfdの範囲を返す
void tty_get_stdio_fd_range(int*min,int*max);
void tty_register_stdio_device(tty_device_t*d);

