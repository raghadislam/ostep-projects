9300 // Boot loader.
9301 //
9302 // Part of the boot block, along with bootasm.S, which calls bootmain().
9303 // bootasm.S has put the processor into protected 32-bit mode.
9304 // bootmain() loads an ELF kernel image from the disk starting at
9305 // sector 1 and then jumps to the kernel entry routine.
9306 
9307 #include "types.h"
9308 #include "elf.h"
9309 #include "x86.h"
9310 #include "memlayout.h"
9311 
9312 #define SECTSIZE  512
9313 
9314 void readseg(uchar*, uint, uint);
9315 
9316 void
9317 bootmain(void)
9318 {
9319   struct elfhdr *elf;
9320   struct proghdr *ph, *eph;
9321   void (*entry)(void);
9322   uchar* pa;
9323 
9324   elf = (struct elfhdr*)0x10000;  // scratch space
9325 
9326   // Read 1st page off disk
9327   readseg((uchar*)elf, 4096, 0);
9328 
9329   // Is this an ELF executable?
9330   if(elf->magic != ELF_MAGIC)
9331     return;  // let bootasm.S handle error
9332 
9333   // Load each program segment (ignores ph flags).
9334   ph = (struct proghdr*)((uchar*)elf + elf->phoff);
9335   eph = ph + elf->phnum;
9336   for(; ph < eph; ph++){
9337     pa = (uchar*)ph->paddr;
9338     readseg(pa, ph->filesz, ph->off);
9339     if(ph->memsz > ph->filesz)
9340       stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz);
9341   }
9342 
9343   // Call the entry point from the ELF header.
9344   // Does not return!
9345   entry = (void(*)(void))(elf->entry);
9346   entry();
9347 }
9348 
9349 
9350 void
9351 waitdisk(void)
9352 {
9353   // Wait for disk ready.
9354   while((inb(0x1F7) & 0xC0) != 0x40)
9355     ;
9356 }
9357 
9358 // Read a single sector at offset into dst.
9359 void
9360 readsect(void *dst, uint offset)
9361 {
9362   // Issue command.
9363   waitdisk();
9364   outb(0x1F2, 1);   // count = 1
9365   outb(0x1F3, offset);
9366   outb(0x1F4, offset >> 8);
9367   outb(0x1F5, offset >> 16);
9368   outb(0x1F6, (offset >> 24) | 0xE0);
9369   outb(0x1F7, 0x20);  // cmd 0x20 - read sectors
9370 
9371   // Read data.
9372   waitdisk();
9373   insl(0x1F0, dst, SECTSIZE/4);
9374 }
9375 
9376 // Read 'count' bytes at 'offset' from kernel into physical address 'pa'.
9377 // Might copy more than asked.
9378 void
9379 readseg(uchar* pa, uint count, uint offset)
9380 {
9381   uchar* epa;
9382 
9383   epa = pa + count;
9384 
9385   // Round down to sector boundary.
9386   pa -= offset % SECTSIZE;
9387 
9388   // Translate from bytes to sectors; kernel starts at sector 1.
9389   offset = (offset / SECTSIZE) + 1;
9390 
9391   // If this is too slow, we could read lots of sectors at a time.
9392   // We'd write more to memory than asked, but it doesn't matter --
9393   // we load in increasing order.
9394   for(; pa < epa; pa += SECTSIZE, offset++)
9395     readsect(pa, offset);
9396 }
9397 
9398 
9399 
