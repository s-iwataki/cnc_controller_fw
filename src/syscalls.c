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
#include "r_sci_uart.h"
#include "hal_data.h"

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
	return 0;
}

__attribute__ ((used))
int _write(int file, char *ptr, int len)
{
	R_SCI_UART_Write(&g_uart9_ctrl,ptr, len);
	return len;
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
