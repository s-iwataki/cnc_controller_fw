#include "tty.h"
#include "stddef.h"
#include "stdint.h"


tty_device_t *device_table[3] = {0};

int tty_stdio_read(int file, char *ptr, int len) {
  if (file < sizeof(device_table) / sizeof(device_table[0])) {
    if (device_table[file]) {
      return device_table[file]->tty_d_read(device_table[file]->device_instance,
                                            ptr, len);
    }
  }
  return -1;
}
int tty_stdio_write(int file, char *ptr, int len) {
  if (file < sizeof(device_table) / sizeof(device_table[0])) {
    if (device_table[file]) {
      return device_table[file]->tty_d_write(
          device_table[file]->device_instance, ptr, len);
    }
  }
  return -1;
}
// ttyとして有効なfdの範囲を返す
void tty_get_stdio_fd_range(int *min, int *max) {
  *min = 0;
  *max = 2;
}
void tty_register_stdio_device(tty_device_t *d) {
  if (device_table[0] == NULL && device_table[1] == NULL &&
      device_table[2] == NULL) {
    device_table[0] = d; // stdin
    device_table[1] = d; // stdout
    device_table[2] = d; // stderr
  }
}