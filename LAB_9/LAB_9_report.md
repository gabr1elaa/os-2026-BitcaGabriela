# Lab 9: Watching System Calls with strace

Gabriela Bîtca, FAF-242

## Part 1: See the calls of a real command

**(1) Count, roughly, how many system calls one simple `ls` makes.**

In my run `ls -l` made 85 calls of the four kinds I traced: 31 `openat`, 19 `read`, 1 `write` and 34 `close`. Most of them are not about my folder at all, but about loading libraries, finding user names and the time zone. With all kinds of calls shown, the number is much larger.

**(2) Which system call does `ls` use to print to the screen? To which descriptor does it write?**

`ls` prints with the `write` system call, to descriptor 1, standard output: `write(1, "total 4\n-rw-rw-r-- 1 gabi gabi 2"..., 59) = 59`. It printed the whole listing in a single call of 59 bytes, and the return value 59 shows that the kernel accepted all of them.

## Part 2: A program that calls the kernel directly

**(1) The descriptor returned by the first `openat` is 3, not 0, 1 or 2. Why are those three already taken?**

Every process starts with three descriptors already open: 0 is standard input, 1 is standard output and 2 is standard error. The shell opens them, connected to my terminal or to a pipe, and the program inherits them through `fork` and `exec`. The kernel always gives a new file the smallest free number, and 0, 1 and 2 were in use, so `note.txt` got 3. The loader had also used 3 for `ld.so.cache` and for `libc`, but it closed it each time, so 3 was free again.
