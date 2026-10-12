# Lab 2: Meet the OS You Will Build

**Name:** Bîtca Gabriela\
**Group:** FAF-242\
**Environment:** macOS. I installed the RISC-V compiler (`riscv64-elf-gcc`) and QEMU with Homebrew. I used the xv6 version where the `sleep()` system call still exists (commit 9374395), because in the newest version it was renamed to `pause()`. My compiler (GCC 16) is newer and stricter than the one xv6 was written for, so I added `-Wno-error` to CFLAGS in the Makefile, so that warnings do not stop the build.

---

## Setup

```
$ brew install riscv64-elf-gcc qemu
$ git clone https://github.com/mit-pdos/xv6-riscv ~/xv6-riscv
$ cd ~/xv6-riscv
$ git checkout 9374395
$ ls
LICENSE		README		mkfs		user
Makefile	kernel		test-xv6.py
```

---

## Part 1: Build and boot

### Commands and output

```
$ make qemu
...
qemu-system-riscv64 -machine virt -bios none -kernel kernel/kernel -m 128M -smp 3 -nographic ...

xv6 kernel is booting

hart 1 starting
hart 2 starting
init: starting sh
$
```

### Observations

xv6 compiled from source and booted inside QEMU. QEMU starts a virtual RISC-V machine with 128 MB of memory and 3 CPU cores (`-m 128M -smp 3`). "hart 1" and "hart 2" are the other two CPU cores starting. `init` is the first process, and it starts the shell (`sh`), which gives me the `$` prompt.

---

## Part 2: Using xv6

### Commands and output

```
$ ls
.              1 1 1024
..             1 1 1024
README         2 2 2425
cat            2 3 37024
echo           2 4 35832
forktest       2 5 17520
grep           2 6 40544
init           2 7 36176
kill           2 8 35744
ln             2 9 35536
ls             2 10 39064
mkdir          2 11 35800
rm             2 12 35784
sh             2 13 59360
stressfs       2 14 36624
usertests      2 15 187912
grind          2 16 51648
wc             2 17 38016
zombie         2 18 35072
logstress      2 19 37792
forphan        2 20 36536
dorphan        2 21 36000
console        3 22 0
$ echo hello xv6
hello xv6
$ ls | grep c
cat            2 3 37024
echo           2 4 35832
wc             2 17 38016
console        3 22 0
$ wc README
48 334 2425 README
```

### Observations

The `ls` in xv6 shows 4 columns: the name, the type, the inode number and the size in bytes. The type is 1 for a directory, 2 for a normal file and 3 for a device. `console` has type 3, so even in xv6 the console is a device that looks like a file, just like `/dev` in Lab 1. `ls | grep c` only kept the lines that contain the letter "c", and `wc README` counted 48 lines, 334 words and 2425 bytes.

### Answers

**(1)** Three programs that come with xv6 are `ls` (lists files), `cat` (prints files) and `grep` (searches for text). Others are `echo`, `wc`, `mkdir`, `rm`, `kill` and `sh`.

**(2)** For a pipe to work, the OS needs:

- **Processes:** the OS must be able to create two processes (with `fork` and `exec`) and run them at the same time, one for `ls` and one for `grep`.
- **Communication between processes:** the OS must be able to connect the two processes with a pipe (the `pipe` system call). The pipe is a buffer inside the kernel: `ls` writes its output into it and `grep` reads its input from it, using file descriptors.

**(3)** The xv6 shell works like the Linux shell from Lab 1 for simple things (running programs, pipes, redirection), but it is much more basic: it has no tab completion, no command history, no variables and no job control like `jobs`.

---

## Part 3: Reading the source

### Commands and output

```
$ sed -n "1,30p" user/cat.c   (the main part)
char buf[512];

void
cat(int fd)
{
  int n;

  while((n = read(fd, buf, sizeof(buf))) > 0) {
    if (write(1, buf, n) != n) {
      fprintf(2, "cat: write error\n");
      exit(1);
    }
  }
  ...
}
$ grep -n "read\|write\|open\|close\|exit" user/cat.c
12:  while((n = read(fd, buf, sizeof(buf))) > 0) {
13:    if (write(1, buf, n) != n) {
15:      exit(1);
35:    if((fd = open(argv[i], O_RDONLY)) < 0){
40:    close(fd);
42:  exit(0);
$ grep -n "sys_read\|sys_write" kernel/sysfile.c
69:sys_read(void)
83:sys_write(void)
```

### Observations

`cat.c` is very short. It reads a file in pieces of 512 bytes into `buf` and writes each piece to file descriptor 1 (the screen), until `read` returns 0 (end of file). All the real work (reading from the disk, printing to the console) is done by the kernel through system calls. The list of all system calls a program can use is in `user/user.h`.

### Answers

**(1)** `user/cat.c` uses these system calls:

| System call | What it asks the kernel to do                                                                                                     |
| ----------- | --------------------------------------------------------------------------------------------------------------------------------- |
| `open`      | open a file and give back a file descriptor (a number for that file)                                                              |
| `read`      | copy bytes from the file into the program's buffer                                                                                |
| `write`     | write bytes to file descriptor 1, which is the console (`fprintf(2, ...)` also uses `write` to print errors to file descriptor 2) |
| `close`     | close the file and free its file descriptor                                                                                       |
| `exit`      | end the process and give back its resources                                                                                       |

**(2)** `sys_read` is implemented in `kernel/sysfile.c`, starting on line 69. `sys_write` is right after it, on line 83.

**(3)** The code in `kernel/` is the operating system itself and runs in privileged mode with full access to the hardware, while the code in `user/` is normal programs that run without privileges and have to ask the kernel for everything through system calls.

---

## Part 4: My first xv6 program

### Code (`user/sleep.c`)

```c
#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc != 2){
    fprintf(2, "usage: sleep <ticks>\n");
    exit(1);
  }
  sleep(atoi(argv[1])); // system call into the kernel
  exit(0);
}
```

I also added `$U/_sleep\` to the UPROGS list in the Makefile, right after `$U/_echo\`.

### Commands and output

```
$ make qemu
...
riscv64-elf-gcc ... -c -o user/sleep.o user/sleep.c
riscv64-elf-ld ... -o user/_sleep user/sleep.o ...
mkfs/mkfs fs.img README user/_cat user/_echo user/_sleep ...
...
xv6 kernel is booting

hart 1 starting
hart 2 starting
init: starting sh
$ sleep 10
$ sleep
usage: sleep <ticks>
```

### Observations

My program compiled without errors and was added to the xv6 file system image (`user/_sleep` appears in the `mkfs` line). `sleep 10` paused for a moment and then returned to the prompt, and `sleep` without a number printed my usage message. My program only checks the arguments and then calls `sleep()`. The actual waiting is done by the kernel, which puts my process to sleep and wakes it up after 10 clock ticks.

---

## Conclusion

In this lab I built a real operating system from source and booted it on an emulated RISC-V machine. Its shell and programs work almost like the ones in Linux and macOS, only much simpler. By reading the source I saw both sides of a system call: the user program that asks (`user/cat.c`) and the kernel code that answers (`kernel/sysfile.c`). Finally, I wrote my own program, `sleep`, which makes a system call into the kernel I compiled myself.
