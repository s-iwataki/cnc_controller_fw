/* Includes */
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/fcntl.h>
#include <FreeRTOS.h>
#include "task.h"
#include "tty.h"

/* Variables */
#ifdef errno
#undef errno
#endif
extern int errno;


char *__env[1] = { 0 };
char **environ = __env;


/* Functions */
__attribute__ ((used))
void initialise_monitor_handles()
{
}

__attribute__ ((used))
int _getpid(void)
{
	return 1;
}

__attribute__ ((used))
int _kill(int pid, int sig)
{
	errno = EINVAL;
	return -1;
}

__attribute__ ((used))
void _exit (int status)
{
	_kill(status, -1);
	while (1) {}		/* Make sure we hang here */
}

__attribute__ ((used))
int _read (int file, char *ptr, int len)
{
    int stdio_fd_min,stdio_fd_max;
    tty_get_stdio_fd_range(&stdio_fd_min, &stdio_fd_max);
    if((file>=stdio_fd_min)&&(file<=stdio_fd_max)){
        return tty_stdio_read(file, ptr, len);
    }
	return 0;
}

__attribute__ ((used))
int _write(int file, char *ptr, int len)
{
	int stdio_fd_min,stdio_fd_max;
    tty_get_stdio_fd_range(&stdio_fd_min, &stdio_fd_max);
    if((file>=stdio_fd_min)&&(file<=stdio_fd_max)){
        return tty_stdio_write(file, ptr, len);
    }
	return 0;
}

/* _sbrk()は bsp_sbrk.cで実装されている */

__attribute__ ((used))
int _close(int file)
{
	return -1;
}

__attribute__ ((used))
int _fstat(int file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}


__attribute__ ((used))
int _isatty(int file)
{
	return 1;
}

__attribute__ ((used))
int _lseek(int file, int ptr, int dir)
{
	return 0;
}

__attribute__ ((used))
int _open(char *path, int flags, ...)
{
	return 0;
}

__attribute__ ((used))
int _wait(int *status)
{
	errno = ECHILD;
	return -1;
}

__attribute__ ((used))
int _unlink(char *name)
{
	errno = ENOENT;
	return -1;
}

__attribute__ ((used))
int _times(struct tms *buf)
{
	return -1;
}

__attribute__ ((used))
int _stat(char *file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

__attribute__ ((used))
int _link(char *old, char *new)
{
	errno = EMLINK;
	return -1;
}

__attribute__ ((used))
int _fork(void)
{
	errno = EAGAIN;
	return -1;
}

__attribute__ ((used))
int _execve(char *name, char **argv, char **env)
{
	errno = ENOMEM;
	return -1;
}

__attribute__ ((used))
void __malloc_lock (struct _reent *REENT){
    vTaskSuspendAll();
}
__attribute__ ((used))
void __malloc_unlock (struct _reent *REENT){
    xTaskResumeAll();

}
