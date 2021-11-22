# boot.S - start point for the kernel after GRUB gives us control
# vim:ts=4 noexpandtab

#define ASM     1
#include "x86_desc.h"
#include "keyboard.h"
#include "filesystem.h"
#include "scheduling.h"

# HANDLER OF THE HANDER TO SEND INTERRUPTS TO CORRECT FUNCTION

/**
 * rtc_interrupted
 * DESCRIPTION: handler of the rtc 
 * INPUT: none
 * RETURN VALUE: none
 * SIDE EFFECTS: none
 *
**/
.globl rtc_interrupted
rtc_interrupted:
	# save registers and flags
	pushal
	pushfl
	# call rtc interrupt function located in rtc.c
	call rtc_interrupt
	# restore registers and flags
	popfl
	popal
	# return from interrupt
	iret

/**
 * keyboard_interrupted
 * DESCRIPTION: handler of the keyboard
 * INPUT: none
 * RETURN VALUE: none
 * SIDE EFFECTS: none
 *
**/
.globl keyboard_interrupted
keyboard_interrupted:
	# save registers and flags
	pushal
	pushfl
	# call keyboard_interrupt from keyboard.c
	call keyboard_interrupt
	# pop registers and flags
	popfl
	popal
	# return from interrupt
	iret

/**
 * pit_interrupted
 * DESCRIPTION: handler of the pit
 * INPUT: none
 * RETURN VALUE: none
 * SIDE EFFECTS: none
 *
**/
.globl pit_interrupted
pit_interrupted:
	# save registers and flags
	pushal
	pushfl
	# call pit_interrupt from scheduling.c
	call pit_interrupt
	# pop registers and flags
	popfl
	popal
	# return from interrupt
	iret


/**
 * syscalls_interrupted
 * DESCRIPTION: handler of the syscalls
 * INPUT: none
 * RETURN VALUE: none
 * SIDE EFFECTS: none
 *
**/
.globl syscalls_interrupted
syscalls_interrupted:
	#save registers and flags
	#pushal
	pushl %esi
	pushl %edi
	pushl %ebx
	pushl %ebp
	pushfl

	#imul $4, eax

	#err checking
	pushl %edx
	pushl %ecx
	pushl %ebx
	decl  %eax
	cmpl $0, %eax
	jl syscall_error
	cmpl $13, %eax
	jge syscall_error
	call *syscall_jumptable(, %eax, 4)

	#tear down stack
	addl $12, %esp
	popfl
	popl %ebp
	popl %ebx
	popl %edi
	popl %esi
	jmp syscall_handler
syscall_error:
	addl $12, %esp
	popfl
	popl %ebp
	popl %ebx
	popl %edi
	popl %esi
	movl $-1, %eax
syscall_handler:
	iret
#jumptable for syscall functions
syscall_jumptable:
.long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn, create, remove, valgrind

	
