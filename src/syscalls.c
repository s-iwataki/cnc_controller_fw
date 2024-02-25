/* Includes */
#include <FreeRTOS.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

#include "bsp_api.h"
#include "task.h"
#include "tty.h"

/**
 * @brief debug info(fault stack trace)
 *
 */
volatile uint32_t r0 BSP_PLACE_IN_SECTION(BSP_SECTION_NOINIT);
volatile uint32_t r1 BSP_PLACE_IN_SECTION(BSP_SECTION_NOINIT);
volatile uint32_t r2 BSP_PLACE_IN_SECTION(BSP_SECTION_NOINIT);
volatile uint32_t r3 BSP_PLACE_IN_SECTION(BSP_SECTION_NOINIT);
volatile uint32_t r12 BSP_PLACE_IN_SECTION(BSP_SECTION_NOINIT);
volatile uint32_t lr BSP_PLACE_IN_SECTION(BSP_SECTION_NOINIT);    /* Link register. */
volatile uint32_t pc BSP_PLACE_IN_SECTION(BSP_SECTION_NOINIT);    /* Program counter. */
volatile uint32_t psr BSP_PLACE_IN_SECTION(BSP_SECTION_NOINIT);   /* Program status register. */
volatile uint32_t cfsr BSP_PLACE_IN_SECTION(BSP_SECTION_NOINIT);  /* Configurable fault status register. */
volatile uint32_t hfsr BSP_PLACE_IN_SECTION(BSP_SECTION_NOINIT);  /* Hard fault status register. */
volatile uint32_t mmfar BSP_PLACE_IN_SECTION(BSP_SECTION_NOINIT); /* MemManage fault address register. */
volatile uint32_t bfar BSP_PLACE_IN_SECTION(BSP_SECTION_NOINIT);  /* Busfault address register*/
volatile bsp_grp_irq_t error_irq BSP_PLACE_IN_SECTION(BSP_SECTION_NOINIT);

/* Variables */
#ifdef errno
#undef errno
#endif
extern int errno;

char* __env[1] = {0};
char** environ = __env;

/* Functions */

void syscall_print_debug_dump(void) {
    printf("r0: %x r1:%x r2:%x r3:%x \r\n", r0, r1, r2, r3);
    printf("r12: %x lr:%x pc:%x psr:%x \r\n", r12, lr, pc, psr);
    printf("cfsr: %x hfsr:%x mmfar:%x bfar:%x \r\n", cfsr, hfsr, mmfar, bfar);
    printf("nmi_irq:%d\r\n",error_irq);
}

__attribute__((used)) void initialise_monitor_handles() {
}

__attribute__((used)) int _getpid(void) {
    return 1;
}

__attribute__((used)) int _kill(int pid, int sig) {
    errno = EINVAL;
    return -1;
}

__attribute__((used)) void _exit(int status) {
    _kill(status, -1);
    while (1) {
    } /* Make sure we hang here */
}

__attribute__((used)) int _read(int file, char* ptr, int len) {
    int stdio_fd_min, stdio_fd_max;
    tty_get_stdio_fd_range(&stdio_fd_min, &stdio_fd_max);
    if ((file >= stdio_fd_min) && (file <= stdio_fd_max)) {
        return tty_stdio_read(file, ptr, len);
    }
    return 0;
}

__attribute__((used)) int _write(int file, char* ptr, int len) {
    int stdio_fd_min, stdio_fd_max;
    tty_get_stdio_fd_range(&stdio_fd_min, &stdio_fd_max);
    if ((file >= stdio_fd_min) && (file <= stdio_fd_max)) {
        return tty_stdio_write(file, ptr, len);
    }
    return 0;
}

/* _sbrk()は bsp_sbrk.cで実装されている */

__attribute__((used)) int _close(int file) {
    return -1;
}

__attribute__((used)) int _fstat(int file, struct stat* st) {
    st->st_mode = S_IFCHR;
    return 0;
}

__attribute__((used)) int _isatty(int file) {
    return 1;
}

__attribute__((used)) int _lseek(int file, int ptr, int dir) {
    return 0;
}

__attribute__((used)) int _open(char* path, int flags, ...) {
    return 0;
}

__attribute__((used)) int _wait(int* status) {
    errno = ECHILD;
    return -1;
}

__attribute__((used)) int _unlink(char* name) {
    errno = ENOENT;
    return -1;
}

__attribute__((used)) int _times(struct tms* buf) {
    return -1;
}

__attribute__((used)) int _stat(char* file, struct stat* st) {
    st->st_mode = S_IFCHR;
    return 0;
}

__attribute__((used)) int _link(char* old, char* new) {
    errno = EMLINK;
    return -1;
}

__attribute__((used)) int _fork(void) {
    errno = EAGAIN;
    return -1;
}

__attribute__((used)) int _execve(char* name, char** argv, char** env) {
    errno = ENOMEM;
    return -1;
}

__attribute__((used)) void __malloc_lock(struct _reent* REENT) {
    vTaskSuspendAll();
}
__attribute__((used)) void __malloc_unlock(struct _reent* REENT) {
    xTaskResumeAll();
}

void prvGetRegistersFromStack(uint32_t* pulFaultStackAddress) {
    cfsr = SCB->CFSR;
    hfsr = SCB->HFSR;
    mmfar = SCB->MMFAR;
    bfar = SCB->BFAR;
    r0 = pulFaultStackAddress[0];
    r1 = pulFaultStackAddress[1];
    r2 = pulFaultStackAddress[2];
    r3 = pulFaultStackAddress[3];

    r12 = pulFaultStackAddress[4];
    lr = pulFaultStackAddress[5];
    pc = pulFaultStackAddress[6];
    psr = pulFaultStackAddress[7];
    NVIC_SystemReset();
}

void HardFault_Handler(void) {
    __asm volatile(
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " ldr r2, hf_handler2_address_const                            \n"
        " bx r2                                                     \n"
        " hf_handler2_address_const: .word prvGetRegistersFromStack    \n");
}
void MemManage_Handler(void) {
    __asm volatile(
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " ldr r2, mm_handler2_address_const                            \n"
        " bx r2                                                     \n"
        " mm_handler2_address_const: .word prvGetRegistersFromStack    \n");
}
void BusFault_Handler(void) {
    __asm volatile(
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " ldr r2, bf_handler2_address_const                            \n"
        " bx r2                                                     \n"
        " bf_handler2_address_const: .word prvGetRegistersFromStack    \n");
}
void UsageFault_Handler(void) {
    __asm volatile(
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " ldr r2, uf_handler2_address_const                            \n"
        " bx r2                                                     \n"
        " uf_handler2_address_const: .word prvGetRegistersFromStack    \n");
}
void SecureFault_Handler(void) {
    __asm volatile(
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " ldr r2, sf_handler2_address_const                            \n"
        " bx r2                                                     \n"
        " sf_handler2_address_const: .word prvGetRegistersFromStack    \n");
}
void stack_protector_error_handler(bsp_grp_irq_t irq) {
    error_irq =irq;
    NVIC_SystemReset();
}
