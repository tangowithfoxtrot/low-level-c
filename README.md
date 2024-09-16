# Low-level C

If you come from a sysadmin or adjacent background, you probably feel very comfortable using the command-line. You can get very far making computers do what you want by calling some binaries with the correct arguments. If you've ever been particularly ambitious, you may have
even pasted a bunch of those commands in a text file and executed them as a script. But if you've ever wanted to understand how those binaries
you're calling are actually working, you have to dig a little deeper.

If you're curious about the inner workings of your favorite CLI utilities, this project is here to help you bridge the gap. Much of what I
write will make comparisons between C and POSIX shell. While there are many differences between them, these comparisons will serve as a way
for us to use our familiarity with the shell to understand some of the lower-level functions that common programs call when they are executed.

## Understanding what binaries are doing

`strace` is a handy utility that will print out some of the lower-level calls that a binary makes to your system's libc implementation. To demonstrate, let's write a simple "Hello, World!" program:

```c
/* hello.c */

#include <stdio.h>

int main() {
    printf("Hello, World!\n");
    return 0;
}
```

We can compile and run it with `gcc -o hello hello.c && ./hello`. But what if we want to see what's happening under the hood? Let's try
doing `strace ./hello`. You should see output similar to the following:

```c
execve("./a.out", ["./a.out"], 0xffffcc3142d0 /* 8 vars */) = 0
brk(NULL)                               = 0xaaaafc2bd000
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0xffff879cd000
faccessat(AT_FDCWD, "/etc/ld.so.preload", R_OK) = -1 ENOENT (No such file or directory)
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
fstat(3, {st_mode=S_IFREG|0644, st_size=17879, ...}) = 0
mmap(NULL, 17879, PROT_READ, MAP_PRIVATE, 3, 0) = 0xffff879c8000
close(3)                                = 0
openat(AT_FDCWD, "/lib/aarch64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0\267\0\1\0\0\0\360\206\2\0\0\0\0\0"..., 832) = 832
fstat(3, {st_mode=S_IFREG|0755, st_size=1722920, ...}) = 0
mmap(NULL, 1892240, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_DENYWRITE, -1, 0) = 0xffff877c6000
mmap(0xffff877d0000, 1826704, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0) = 0xffff877d0000
munmap(0xffff877c6000, 40960)           = 0
munmap(0xffff8798e000, 24464)           = 0
mprotect(0xffff8796a000, 77824, PROT_NONE) = 0
mmap(0xffff8797d000, 20480, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x19d000) = 0xffff8797d000
mmap(0xffff87982000, 49040, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0xffff87982000
close(3)                                = 0
set_tid_address(0xffff879cdf90)         = 19257
set_robust_list(0xffff879cdfa0, 24)     = 0
rseq(0xffff879ce5e0, 0x20, 0, 0xd428bc00) = 0
mprotect(0xffff8797d000, 12288, PROT_READ) = 0
mprotect(0xaaaac609f000, 4096, PROT_READ) = 0
mprotect(0xffff879d2000, 8192, PROT_READ) = 0
prlimit64(0, RLIMIT_STACK, NULL, {rlim_cur=8192*1024, rlim_max=RLIM64_INFINITY}) = 0
munmap(0xffff879c8000, 17879)           = 0
fstat(1, {st_mode=S_IFCHR|0620, st_rdev=makedev(0x88, 0x1), ...}) = 0
getrandom("\x91\xc4\x64\x61\xbc\x39\x91\x95", 8, GRND_NONBLOCK) = 8
brk(NULL)                               = 0xaaaafc2bd000
brk(0xaaaafc2de000)                     = 0xaaaafc2de000
write(1, "Hello, World!\n", 14Hello, World!
)         = 14
exit_group(0)                           = ?
+++ exited with 0 +++
```

This looks overwhelming at first. However, with some digging in the `man` pages, we can learn how these lower-level functions work.

## execve()

If we run `man execve`, we should be able to see how our system's libc implements this function and see what it does:

> execve() executes the program referred to by pathname. This causes the program that is currently being run by the calling process
> to be replaced with a new program, with newly initialized stack, heap, and (initialized and uninitialized) data segments.

Let's look at its function signature see what parameters it takes:

```c
int execve(const char *pathname, char *const _Nullable argv[],
                  char *const _Nullable envp[]);
```

Okay, so it takes a pathname, some optional (`_Nullable`) arguments (`argv[]`), and `envp[]`. The meaning of
`envp[]` wasn't immediately clear to me, but if we read further:

> envp is an array of pointers to strings, conventionally of the form key=value

Ah, okay! So `envp[]` is essentially any number of environment variables.

We could think of `execve()` resembling something like this in POSIX shell:

```sh
#!/bin/sh

# envp[],          execve, pathname,  argv[]
MYVAR1='somevalue' /bin/sh /bin/echo '$MYVAR1'
```

The number of arguments and how they are passed don't map-out in the same way between C and POSIX shell, but if we think of `/bin/sh`
in the above example as a stand-in for `execve()`, `MYVAR1='somevalue'` as our `envp[]`, `/usr/bin/printenv` as our `pathname`, and `'$MYVAR1'`
as our `argv[]`, we can use this higher-level implementation to better visualize what `execve()` is doing.
