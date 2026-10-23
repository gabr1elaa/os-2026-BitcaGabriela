# Lab 7: Three Small Programs: Processes, Redirection, a Pipe

Gabriela Bîtca, FAF-242

## Part 1: fork + exec + wait

**(1) The line `perror("exec")` is in the child, right after `execlp`. Why is it printed only with `nosuchprog`?**

Because a successful `execlp()` never returns. It replaces the whole program of the child (its code, data and stack) with `ls` or `date`, so the lines after it no longer exist in that process. `execlp()` returns only when it fails, and then the next line, `perror("exec")`, runs and prints the reason: "No such file or directory", because no program called `nosuchprog` was found in any folder of `PATH`.

**(2) Which number does the child print as "my parent", and whose PID is it?**

In the first run the child printed `my parent is 85994`, which is exactly the number the parent printed as its own PID on the first line. So it is the PID of the process that called `fork()`, the original `./run`. The same holds for the other runs (86039 and 86075).

**(3) What status does the parent see with `ls`, and what status with `nosuchprog`?**

With `ls` (and with `date`) the parent saw status 0: the program ran and finished successfully. With `nosuchprog` it saw status 127: `execlp()` failed, so the child ran `exit(127)`, the same code a shell uses for "command not found". `waitpid()` stores this exit code in `status`, and `WEXITSTATUS(status)` extracts it.

## Part 2: A zombie, on purpose

**(1) Which letter does `ls` have in the STAT column, and what does it mean?**

`ls` has the letter `Z`, a zombie (`ps` also writes `<defunct>`). `ls` had already finished its work and exited, but its parent had not yet called `waitpid()` to collect its exit status. The kernel therefore keeps a small entry for it in the process table, with its PID and its exit status, until the parent asks for it.

**(2) After 20 seconds, is the `[ls] <defunct>` line still there? Which line of `zombie.c` made it disappear?**

No, it is gone. The line that made it disappear is `waitpid(pid, &status, 0);`: when `sleep(20)` ended, the parent called `waitpid()`, which read the child's exit status and let the kernel remove the zombie's entry from the process table. Then the parent printed its last line and exited too.

## Part 3: Redirection: ls -l > out.txt

**(1) Which number did `open` give, and why exactly that one?**

`open` gave descriptor 1 (`out.txt opened with file descriptor 1`). The kernel always gives a new file the smallest free descriptor number. Descriptor 0 (standard input) was still in use, and descriptor 1 had just been freed by `close(1)`, so 1 was the smallest free number.

**(2) `ls` was not changed at all. Why does its listing not appear on the screen?**

`ls` simply writes to descriptor 1, as every program does; it does not know or care what descriptor 1 is. The open descriptors are kept across `exec`, so when `ls` started, its descriptor 1 was no longer the terminal but `out.txt`, and the listing went into the file.

**(3) To redirect the errors instead (like `2> err.txt`), which descriptor would you close before `open`?**

Descriptor 2, standard error. After `close(2)` the smallest free number is 2, so `open("err.txt", ...)` would return 2, and the error messages of the program would go into `err.txt` while normal output would still go to the terminal.

## Part 4: A pipe: ls /etc | wc -l

**(1) Did `./pipeline` and `ls /etc | wc -l` give the same number?**

Yes, both printed 244, and the repaired program printed 244 again.

**(2) After the change, which of the two children never ends, `ls` or `wc`?**

`wc`. `ls` writes its 244 lines into the pipe and exits normally, but `wc` never finishes.

**(3) Why, in one sentence: what is `wc` waiting for?**

`wc` is waiting for end-of-file on the pipe, which the kernel signals only when every writing end of the pipe is closed, and the parent still kept its copy of `p[1]` open, so as far as `wc` could tell, more data might still come.
