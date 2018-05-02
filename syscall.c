#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"

// User code makes a system call with INT T_SYSCALL.
// System call number in %eax.
// Arguments on the stack, from the user call to the C
// library system call function. The saved user %esp points
// to a saved program counter, and then the first argument.

// Fetch the int at addr from the current process.
int
fetchint(uint addr, int *ip)
{
  struct proc *curproc = myproc();

  // check if the address valid
  if(addr >= curproc->sz || addr+4 > curproc->sz)
    // fail -> return -1
    return -1;
  // direct fetch the int from addr
  *ip = *(int*)(addr);
  // success -> return 0
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Doesn't actually copy the string - just sets *pp to point at it.
// Returns length of string, not including nul.
int
fetchstr(uint addr, char **pp)
{
  // s is the current readable data's pointer
  // ep is the current process's data bound's pointer
  char *s, *ep;
  struct proc *curproc = myproc();

  // check if addr valid
  if(addr >= curproc->sz)
    // the addr not valid return -1
    return -1;

  *pp = (char*)addr;
  ep = (char*)curproc->sz;
  // count for the length of the string
  for(s = *pp; s < ep; s++){
    // check if s point to NULL
    if(*s == 0)
      // if s point to NULL, it is the end of the string
      return s - *pp;
  }
  // cannot find the end of the string; error -> return -1
  return -1;
}

// Fetch the nth 32-bit system call argument.
int
argint(int n, int *ip)
{
  // fetch the int arg and store in ip
  // return value tells whether the fetch success or fail
  return fetchint((myproc()->tf->esp) + 4 + 4*n, ip);
}

// Fetch the nth word-sized system call argument as a pointer
// to a block of memory of size bytes.  Check that the pointer
// lies within the process address space.
int
argptr(int n, char **pp, int size)
{
  int i;
  struct proc *curproc = myproc();
 
  // fetch the nth 32bits data as pointer to a string
  if(argint(n, &i) < 0)
    return -1;
  // check if the pointer i is a valid address
  if(size < 0 || (uint)i >= curproc->sz || (uint)i+size > curproc->sz)
    return -1;
  *pp = (char*)i;
  return 0;
}

// Fetch the nth word-sized system call argument as a string pointer.
// Check that the pointer is valid and the string is nul-terminated.
// (There is no shared writable memory, so the string can't change
// between this check and being used by the kernel.)
int
argstr(int n, char **pp)
{
  // the address of the string arg
  int addr;
  // check if the address is valid
  if(argint(n, &addr) < 0)
    return -1;
  // fetch the string arg and store in pp
  // return the length of string; -1 means fail to fetch
  return fetchstr(addr, pp);
}

extern int sys_chdir(void);
extern int sys_close(void);
extern int sys_dup(void);
extern int sys_exec(void);
extern int sys_exit(void);
extern int sys_fork(void);
extern int sys_fstat(void);
extern int sys_getpid(void);
extern int sys_kill(void);
extern int sys_link(void);
extern int sys_mkdir(void);
extern int sys_mknod(void);
extern int sys_open(void);
extern int sys_pipe(void);
extern int sys_read(void);
extern int sys_sbrk(void);
extern int sys_sleep(void);
extern int sys_unlink(void);
extern int sys_wait(void);
extern int sys_write(void);
extern int sys_uptime(void);

static int (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
[SYS_exit]    sys_exit,
[SYS_wait]    sys_wait,
[SYS_pipe]    sys_pipe,
[SYS_read]    sys_read,
[SYS_kill]    sys_kill,
[SYS_exec]    sys_exec,
[SYS_fstat]   sys_fstat,
[SYS_chdir]   sys_chdir,
[SYS_dup]     sys_dup,
[SYS_getpid]  sys_getpid,
[SYS_sbrk]    sys_sbrk,
[SYS_sleep]   sys_sleep,
[SYS_uptime]  sys_uptime,
[SYS_open]    sys_open,
[SYS_write]   sys_write,
[SYS_mknod]   sys_mknod,
[SYS_unlink]  sys_unlink,
[SYS_link]    sys_link,
[SYS_mkdir]   sys_mkdir,
[SYS_close]   sys_close,
};

void
syscall(void)
{
  // current trap number
  int num;
  // current process state
  struct proc *curproc = myproc();

  // read trap number from %eax which can be accessed from trap frame
  num = curproc->tf->eax;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    // save the return value of syscall to tf->eax
    curproc->tf->eax = syscalls[num]();
  } else {
    // if it's unknown syscall, print some error message
    cprintf("%d %s: unknown sys call %d\n",
            curproc->pid, curproc->name, num);
    // if error happens, set tf->eax to -1
    curproc->tf->eax = -1;
  }
}
