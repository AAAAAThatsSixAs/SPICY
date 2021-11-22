/* Setup IDT */

#include "x86_desc.h"
#include "exception.h"
#include "types.h"
#include "idt_setup.h"
/**
 * idt_setup
 * DESCRIPTION: sets up the idt table
 * INPUT: none
 * RETURN VALUE: none
 * SIDE EFFECTS: sets the irq table
 *
**/
int idtsetup()
{

	int i;
	//set all 256 vectors to off
	for(i = 0; i < 256; i++)
	{
		//set correct bits
		idt[i].offset_15_00 = 0;
		idt[i].seg_selector = KERNEL_CS;
		idt[i].reserved4 = 0;

		if(i < 32)
			idt[i].reserved3 = 0;
		else
			idt[i].reserved3 = 1;

		idt[i].reserved2 = 1;
		idt[i].reserved1 = 1;
		idt[i].reserved0 = 0;
		idt[i].size = 1;
		idt[i].dpl = 0;
		//set to NOT PRESENT
		idt[i].present = 0;
		idt[i].offset_31_16 = 0;

		//turn on first 18 entries in the irq table
		if(i <= 18)
			idt[i].present = 1;

	}


	//DECLARE ALL EXCEPTIONS USING MACRO DEFINED IN exception.h

	//divide error
	MAKE_EXCEPTION_FUNCTION(exception_de, "divided by zero exception!");
	//debug 
	MAKE_EXCEPTION_FUNCTION(exception_db, "debug exception!");
	//nmi interrupt
	MAKE_EXCEPTION_FUNCTION(exception_nmi, "nonmaskable external interrupt exception!");
	//breakpoint
	MAKE_EXCEPTION_FUNCTION(exception_bp, "breakpoint exception!");
	//overflow
	MAKE_EXCEPTION_FUNCTION(exception_of, "overflow exception!");
	//bound range exceeded
	MAKE_EXCEPTION_FUNCTION(exception_br, "bound range exceeded exception!");
	//invalid opcode
	MAKE_EXCEPTION_FUNCTION(exception_ud, "invalid opcode exception!");
	//device not available
	MAKE_EXCEPTION_FUNCTION(exception_nm, "device not available exception!");
	//double fault
	MAKE_EXCEPTION_FUNCTION(exception_df, "double fault exception!");
	//coprocessor segment ovverrun
	MAKE_EXCEPTION_FUNCTION(exception_cso, "coprocessor segment overrun exception!");
	//invalid tss
	MAKE_EXCEPTION_FUNCTION(exception_ts, "invalid tss exception!");
	//segment not present
	MAKE_EXCEPTION_FUNCTION(exception_np, "segment not present exception!");
	//stack segment fault 
	MAKE_EXCEPTION_FUNCTION(exception_ss, "stack segment fault exception!");
	//general protection
	MAKE_EXCEPTION_FUNCTION(exception_gp, "general protection exception!");
	//page fault
	MAKE_EXCEPTION_FUNCTION(exception_pf, "page fault exception!");
	//floating point error
	MAKE_EXCEPTION_FUNCTION(exception_mf, "floating point errer exception!");
	//alignment check
	MAKE_EXCEPTION_FUNCTION(exception_ac, "alignment check exception!");
	//machine check
	MAKE_EXCEPTION_FUNCTION(exception_mc, "machine check exception!");
	//floating point
	MAKE_EXCEPTION_FUNCTION(exception_xf, "simd floating point exception!");


	//set IDT entries with exception addresses according to intel manual
	SET_IDT_ENTRY(idt[0], exception_de);
	SET_IDT_ENTRY(idt[1], exception_db);
	SET_IDT_ENTRY(idt[2], exception_nmi);
	SET_IDT_ENTRY(idt[3], exception_bp);
	SET_IDT_ENTRY(idt[4], exception_of);
	SET_IDT_ENTRY(idt[5], exception_br);
	SET_IDT_ENTRY(idt[6], exception_ud);
	SET_IDT_ENTRY(idt[7], exception_nm);
	SET_IDT_ENTRY(idt[8], exception_df);
	SET_IDT_ENTRY(idt[9], exception_cso);
	SET_IDT_ENTRY(idt[10], exception_ts);
	SET_IDT_ENTRY(idt[11], exception_np);
	SET_IDT_ENTRY(idt[12], exception_ss);
	SET_IDT_ENTRY(idt[13], exception_gp);
	SET_IDT_ENTRY(idt[14], exception_pf);
	//entry 15 is not used
	SET_IDT_ENTRY(idt[16], exception_mf);
	SET_IDT_ENTRY(idt[17], exception_ac);
	SET_IDT_ENTRY(idt[18], exception_mc);


	//handler of interrupt handlers located in interrupted.S ----------
	//setup IDT entry for keyboard
	idt[KEYBOARD_PIC].present = 1;
	SET_IDT_ENTRY(idt[KEYBOARD_PIC], keyboard_interrupted);

	//setup IDT entry for RTC
	idt[RTC_PIC].present = 1;
	SET_IDT_ENTRY(idt[RTC_PIC], rtc_interrupted);

	//setup IDT entry for system call
	idt[SYSCALL_PIC].present = 1;
	//set privilege level to 3
	idt[SYSCALL_PIC].dpl = 3;
	SET_IDT_ENTRY(idt[SYSCALL_PIC], syscalls_interrupted);

	//setup IDT entry for PIT
	idt[PIT_PIC].present = 1;
	SET_IDT_ENTRY(idt[PIT_PIC], pit_interrupted);

	//-----------------------------------------------------------------

	//copy pointer location to idt register
	lidt(idt_desc_ptr);

	return 0;
}

