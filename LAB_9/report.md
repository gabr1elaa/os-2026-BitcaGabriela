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

## Part 3: Count one kind of call

**(1) Which system call does `ls -l` make most often, and how many calls does it make in total?**

`close` and `getdents64` are made most often, 34 times each, followed by `openat` (31) and `fstat` (29). In total `ls -l` made 273 system calls of 33 kinds. The four kinds I traced in Part 1 (85 calls) are less than a third of them.

**(2) Count only `openat`. Is the number the same as in Part 1?**

Yes, `strace -c -e trace=openat` counted 31 `openat` calls, the same as the 31 lines of `part1.txt`, and one of them failed (the missing `en-US.ftl`). The filter `-e trace=` only chooses which calls `strace` shows or counts: the program makes the same calls either way.

**(3) How many times does `syscalls` call `write` when its output goes to a file, and how many when it goes to the terminal? Why is it different?**

It calls `write` 2 times with the output in a file and 4 times with the output on the terminal. One `write` is always the one in my code, the 14 bytes into `note.txt`. The others come from `printf`: when the output is a file (or a pipe, as in Part 2), the C library keeps all the text in its buffer and sends it with one `write` at the end, so 1 + 1 = 2. When the output is a terminal, the buffer is emptied after every line, and my three `printf` lines become three `write` calls, so 1 + 3 = 4.
