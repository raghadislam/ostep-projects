9400 /* Simple linker script for the JOS kernel.
9401    See the GNU ld 'info' manual ("info ld") to learn the syntax. */
9402 
9403 OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
9404 OUTPUT_ARCH(i386)
9405 ENTRY(_start)
9406 
9407 SECTIONS
9408 {
9409 	/* Link the kernel at this address: "." means the current address */
9410         /* Must be equal to KERNLINK */
9411 	. = 0x80100000;
9412 
9413 	.text : AT(0x100000) {
9414 		*(.text .stub .text.* .gnu.linkonce.t.*)
9415 	}
9416 
9417 	PROVIDE(etext = .);	/* Define the 'etext' symbol to this value */
9418 
9419 	.rodata : {
9420 		*(.rodata .rodata.* .gnu.linkonce.r.*)
9421 	}
9422 
9423 	/* Include debugging information in kernel memory */
9424 	.stab : {
9425 		PROVIDE(__STAB_BEGIN__ = .);
9426 		*(.stab);
9427 		PROVIDE(__STAB_END__ = .);
9428 	}
9429 
9430 	.stabstr : {
9431 		PROVIDE(__STABSTR_BEGIN__ = .);
9432 		*(.stabstr);
9433 		PROVIDE(__STABSTR_END__ = .);
9434 	}
9435 
9436 	/* Adjust the address for the data segment to the next page */
9437 	. = ALIGN(0x1000);
9438 
9439 	/* Conventionally, Unix linkers provide pseudo-symbols
9440 	 * etext, edata, and end, at the end of the text, data, and bss.
9441 	 * For the kernel mapping, we need the address at the beginning
9442 	 * of the data section, but that's not one of the conventional
9443 	 * symbols, because the convention started before there was a
9444 	 * read-only rodata section between text and data. */
9445 	PROVIDE(data = .);
9446 
9447 
9448 
9449 
9450 	/* The data segment */
9451 	.data : {
9452 		*(.data)
9453 	}
9454 
9455 	PROVIDE(edata = .);
9456 
9457 	.bss : {
9458 		*(.bss)
9459 	}
9460 
9461 	PROVIDE(end = .);
9462 
9463 	/DISCARD/ : {
9464 		*(.eh_frame .note.GNU-stack)
9465 	}
9466 }
9467 
9468 
9469 
9470 
9471 
9472 
9473 
9474 
9475 
9476 
9477 
9478 
9479 
9480 
9481 
9482 
9483 
9484 
9485 
9486 
9487 
9488 
9489 
9490 
9491 
9492 
9493 
9494 
9495 
9496 
9497 
9498 
9499 
