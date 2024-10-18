3600 #include "types.h"
3601 #include "defs.h"
3602 #include "param.h"
3603 #include "memlayout.h"
3604 #include "mmu.h"
3605 #include "proc.h"
3606 #include "x86.h"
3607 #include "syscall.h"
3608 #include "spinlock.h"
3609 
3610 // User code makes a system call with INT T_SYSCALL.
3611 // System call number in %eax.
3612 // Arguments on the stack, from the user call to the C
3613 // library system call function. The saved user %esp points
3614 // to a saved program counter, and then the first argument.
3615 
3616 // Fetch the int at addr from the current process.
3617 int
3618 fetchint(uint addr, int *ip)
3619 {
3620   struct proc *curproc = myproc();
3621 
3622   if(addr >= curproc->sz || addr+4 > curproc->sz)
3623     return -1;
3624   *ip = *(int*)(addr);
3625   return 0;
3626 }
3627 
3628 // Fetch the nul-terminated string at addr from the current process.
3629 // Doesn't actually copy the string - just sets *pp to point at it.
3630 // Returns length of string, not including nul.
3631 int
3632 fetchstr(uint addr, char **pp)
3633 {
3634   char *s, *ep;
3635   struct proc *curproc = myproc();
3636 
3637   if(addr >= curproc->sz)
3638     return -1;
3639   *pp = (char*)addr;
3640   ep = (char*)curproc->sz;
3641   for(s = *pp; s < ep; s++){
3642     if(*s == 0)
3643       return s - *pp;
3644   }
3645   return -1;
3646 }
3647 
3648 
3649 
3650 // Fetch the nth 32-bit system call argument.
3651 int
3652 argint(int n, int *ip)
3653 {
3654   return fetchint((myproc()->tf->esp) + 4 + 4*n, ip);
3655 }
3656 
3657 // Fetch the nth word-sized system call argument as a pointer
3658 // to a block of memory of size bytes.  Check that the pointer
3659 // lies within the process address space.
3660 int
3661 argptr(int n, char **pp, int size)
3662 {
3663   int i;
3664   struct proc *curproc = myproc();
3665 
3666   if(argint(n, &i) < 0)
3667     return -1;
3668   if(size < 0 || (uint)i >= curproc->sz || (uint)i+size > curproc->sz)
3669     return -1;
3670   *pp = (char*)i;
3671   return 0;
3672 }
3673 
3674 // Fetch the nth word-sized system call argument as a string pointer.
3675 // Check that the pointer is valid and the string is nul-terminated.
3676 // (There is no shared writable memory, so the string can't change
3677 // between this check and being used by the kernel.)
3678 int
3679 argstr(int n, char **pp)
3680 {
3681   int addr;
3682   if(argint(n, &addr) < 0)
3683     return -1;
3684   return fetchstr(addr, pp);
3685 }
3686 
3687 
3688 
3689 
3690 
3691 
3692 
3693 
3694 
3695 
3696 
3697 
3698 
3699 
3700 extern int sys_chdir(void);
3701 extern int sys_close(void);
3702 extern int sys_dup(void);
3703 extern int sys_exec(void);
3704 extern int sys_exit(void);
3705 extern int sys_fork(void);
3706 extern int sys_fstat(void);
3707 extern int sys_getpid(void);
3708 extern int sys_kill(void);
3709 extern int sys_link(void);
3710 extern int sys_mkdir(void);
3711 extern int sys_mknod(void);
3712 extern int sys_open(void);
3713 extern int sys_pipe(void);
3714 extern int sys_read(void);
3715 extern int sys_sbrk(void);
3716 extern int sys_sleep(void);
3717 extern int sys_unlink(void);
3718 extern int sys_wait(void);
3719 extern int sys_write(void);
3720 extern int sys_uptime(void);
3721 extern int sys_getreadcount(void);
3722 
3723 extern int counter;
3724 
3725 static int (*syscalls[])(void) = {
3726 [SYS_fork]    sys_fork,
3727 [SYS_exit]    sys_exit,
3728 [SYS_wait]    sys_wait,
3729 [SYS_pipe]    sys_pipe,
3730 [SYS_read]    sys_read,
3731 [SYS_kill]    sys_kill,
3732 [SYS_exec]    sys_exec,
3733 [SYS_fstat]   sys_fstat,
3734 [SYS_chdir]   sys_chdir,
3735 [SYS_dup]     sys_dup,
3736 [SYS_getpid]  sys_getpid,
3737 [SYS_sbrk]    sys_sbrk,
3738 [SYS_sleep]   sys_sleep,
3739 [SYS_uptime]  sys_uptime,
3740 [SYS_open]    sys_open,
3741 [SYS_write]   sys_write,
3742 [SYS_mknod]   sys_mknod,
3743 [SYS_unlink]  sys_unlink,
3744 [SYS_link]    sys_link,
3745 [SYS_mkdir]   sys_mkdir,
3746 [SYS_close]   sys_close,
3747 [SYS_getreadcount]  sys_getreadcount
3748 };
3749 
3750 struct spinlock lock;
3751 void
3752 syscall(void)
3753 {
3754   int num;
3755   struct proc *curproc = myproc();
3756 
3757   num = curproc->tf->eax;
3758   if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
3759     curproc->tf->eax = syscalls[num]();
3760     if(num==5){
3761       acquire(&lock);
3762       counter++;
3763       release(&lock);
3764     }
3765 
3766   } else {
3767     cprintf("%d %s: unknown sys call %d\n",
3768             curproc->pid, curproc->name, num);
3769     curproc->tf->eax = -1;
3770   }
3771 }
3772 
3773 
3774 
3775 
3776 
3777 
3778 
3779 
3780 
3781 
3782 
3783 
3784 
3785 
3786 
3787 
3788 
3789 
3790 
3791 
3792 
3793 
3794 
3795 
3796 
3797 
3798 
3799 
