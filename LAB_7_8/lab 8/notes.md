# Lab 8: The Same Calls, Inside Your Own OS

Gabriela Bîtca, FAF-242

## Part 1: run, inside xv6

**(1) Write the three calls that `run.c` uses in xv6 and the three calls `run.c` used on Linux in Lab 7. What is the same?**

In xv6: `fork()`, `exec()` and `wait()`. On Linux in Lab 7: `fork()`, `execlp()` and `waitpid()`. The idea is the same in both, in the same order and with the same meaning: `fork()` makes a copy of the process and returns 0 in the child and the child's PID in the parent; `exec` replaces the child's program and returns only if it fails; and the parent's `wait` blocks until the child has ended and gives back its exit status. The differences are only in the details: `execlp()` searches the folders of `PATH` and takes the arguments as a list, while the xv6 `exec()` takes a path and an `argv` array; `waitpid()` waits for one chosen child, while the xv6 `wait()` waits for any child.

**(2) Who prints "hello from the child": the code of `run`, or `echo`?**

`echo` prints it. The code of `run` prints only the lines that start with `parent:` and `child:`. After `exec("echo", args)` succeeds, the child no longer runs the code of `run`: its memory is replaced by the program `echo`, which receives `args` as its `argv` and prints `argv[1]` to `argv[4]`, that is, `hello from the child`. The `ls` experiment confirms it: with the same `run` code, the child printed a directory listing instead, because it had become `ls`.

**(3) Why is the status 1 here and 127 on Linux?**

Because each program chooses this number itself, in the line after `exec`. In the xv6 `run.c` the line is `exit(1);`, and in the Linux `run.c` of Lab 7 it was `exit(127);`. When `exec` fails, the child continues to this line, and the number it passes to `exit()` is exactly what the parent receives from `wait()`. The kernel does not decide it; 127 is only the convention the Linux shell uses for "command not found".

## Part 2: pingpong

**(1) Why do you need TWO pipes, and not one for both directions?**

A pipe carries data in one direction only, from its writing end to its reading end, and every process that holds the reading end can read whatever is in it. With a single pipe, the parent would write its byte and then read from the same pipe, and it could read its own byte back before the child had a chance to take it; the child might then wait forever. With two pipes each direction has its own channel: the parent only writes into `p2c` and only reads from `c2p`, and the child does the opposite, so each process can only receive the other's byte.

**(2) Nothing in your code says "wait until the child has replied". Which call makes the parent wait?**

The parent's `read(c2p[0], &b, 1)`. When a pipe is empty, `read()` blocks: the kernel puts the process to sleep until data arrives (or until all writing ends are closed). So the parent stays inside this call until the child has written its byte into `c2p`, and only then prints `pong`. The later `wait(0)` waits for something else: for the child to end, so that it does not stay a zombie.
