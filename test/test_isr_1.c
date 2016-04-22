
#include <kernel.h>
#include <test.h>



/*
 * This is the first test to check Interrupts. What essentially
 * happens is that an ISR (Interrupt Service Routine) is installed
 * that pokes into the video memory at a certain location. The ISR
 * is hooked with the timer interrupt so that this video update
 * happens in certain intervals. Meanwhile, the main program (or
 * rather the boot process) continues to run doing the same thing
 * (poking to the video memory; but at another location).
 */
MEM_ADDR screen_offset_for_timer_isr = 0xb8000 + 3 * 160 + 2 * 11;
int counter = 10;

void timer_isr ();
void timer_isr_wrapper()
{
    /*
     *	PUSHL	%EAX		; Save process' context
     *  PUSHL   %ECX
     *  PUSHL   %EDX
     *  PUSHL   %EBX
     *  PUSHL   %EBP
     *  PUSHL   %ESI
     *  PUSHL   %EDI
     */
    asm ("timer_isr:");
    asm ("pushl %eax;pushl %ecx;pushl %edx");
    asm ("pushl %ebx;pushl %ebp;pushl %esi;pushl %edi");
    /* Call the actual implementation of the ISR */
    asm ("call timer_isr_impl");
    /*
     *	MOVB  $0x20,%AL	; Reset interrupt controller
     *	OUTB  %AL,$0x20
     *	POPL  %EDI      ; Restore previously saved context
     *  POPL  %ESI
     *  POPL  %EBP
     *  POPL  %EBX
     *  POPL  %EDX
     *  POPL  %ECX
     *  POPL  %EAX
     *	IRET		; Return to new process
     */
    asm ("movb $0x20,%al;outb %al,$0x20");
    asm ("popl %edi;popl %esi;popl %ebp;popl %ebx");
    asm ("popl %edx;popl %ecx;popl %eax");
    asm ("iret");
}

void timer_isr_impl ()
{
    if (!counter--) {
	counter = 20;
	poke_b(screen_offset_for_timer_isr,
	       peek_b(screen_offset_for_timer_isr) + 1);
        check_sum ++;
    }
}


void test_isr_1()
{
    MEM_ADDR screen_offset_for_boot_proc = 0xb8000 + 4 * 160 + 2 * 11;
    volatile int flag;
    
    test_reset();
    check_sum = 0;
    init_interrupts();
    DISABLE_INTR(flag);
    init_idt_entry(TIMER_IRQ, timer_isr);
    kprintf("=== test_isr_1 === \n");
    kprintf("This test will take a while.\n\n");
    kprintf("Timer ISR: A\n");
    kprintf("Boot proc: Z\n");
    ENABLE_INTR(flag);

    int i;
    for (i = 1; i < 700000; i++)
	poke_b(screen_offset_for_boot_proc,
	       peek_b(screen_offset_for_boot_proc) + 1);

    if (check_sum == 0)
	test_failed(70);
}

