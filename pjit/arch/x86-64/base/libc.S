/* Copyright 2012-2013 Peter Goodman, all rights reserved. */
/*
 * libc.S
 *
 *  Created on: 2013-12-22
 *      Author: Peter Goodman
 */



.text


    .align 16
    .globl pjit_memcpy
    .type pjit_memcpy, @function
pjit_memcpy:
    movq %rdx, %rcx;
    movq %rdi, %rdx;
    rep movsb;
    movq %rdx, %rax;
    retq;


    .align 16
    .globl pjit_memset
    .type pjit_memset, @function
pjit_memset:
    movq %rsi, %rax;
    movq %rdx, %rcx;
    movq %rdi, %rdx;
    rep stosb;
    movq %rdx, %rax;
    retq;


    .align 16
    .globl pjit_memcmp
    .type pjit_memcmp, @function
pjit_memcmp:
.Lpjit_memcmp_next_byte:
    test %rdx, %rdx;
    movq $0, %rax;
    jz .Lpjit_memcmp_done;

    movb (%rdi), %al;
    subb (%rsi), %al;
    jnz .Lpjit_memcmp_done;

    sub $1, %rdx;
    add $1, %rdi;
    add $1, %rsi;
    jmp .Lpjit_memcmp_next_byte;

.Lpjit_memcmp_done:
    retq;


    .align 16
    .globl pjit_mmap
    .type pjit_mmap, @function
pjit_mmap:
    mov %rcx, %r10;
    mov $0x9, %eax;
    syscall;
    retq;


    .align 16
    .globl pjit_mprotect
    .type pjit_mprotect, @function
pjit_mprotect:
    mov $0xa, %eax;
    syscall;
    retq;


    .align 16
    .globl pjit_munmap
    .type pjit_munmap, @function
pjit_munmap:
    mov $0xb, %eax;
    syscall;
    retq;

