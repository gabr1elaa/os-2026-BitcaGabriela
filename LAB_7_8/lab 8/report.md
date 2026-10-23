# Operating Systems Lab 7: Three Small Programs

**Student:** Gabriela Bîtca, FAF-242  

# Purpose of the work

The purpose of this laboratory work is to answer, by running real programs, three questions from the lecture: how a process is born, where `printf` writes, and how `ls | wc -l` works. I compiled and ran three short C programs that do by hand what the shell does for every command: `run.c` creates a process with `fork`, `exec` and `wait`; `redirect.c` sends the output of `ls` into a file; and `pipeline.c` connects `ls` and `wc` with a pipe. A fourth program, `zombie.c`, creates a zombie process on purpose.

The work was done in Ubuntu running in a Parallels virtual machine on my MacBook Air M4, in a new folder `~/os-lab7`. Before starting I checked the compiler:

```text
$ mkdir -p ~/os-lab7 && cd ~/os-lab7
$ gcc --version | head -1
gcc (Ubuntu 15.2.0-16ubuntu1) 15.2.0
```

*Listing 0.1 - The working folder and the compiler version*

Every program was compiled with `gcc -Wall -Wextra`, and none of them produced a warning. The deliverables are listed in Table 0.1; I did not do the optional `pipeline3.c`, which is not graded.

*Table 0.1 - Deliverables*

| File | Content |
|---|---|
| `run.c` | fork + exec + wait, the three steps of the shell (Part 1) |
| `zombie.c` | `run.c` with `sleep(20)` before `waitpid`, which leaves a zombie (Part 2) |
| `redirect.c` | redirection of standard output into `out.txt`, like `ls -l > out.txt` (Part 3) |
| `pipeline.c` | a pipe between two children, like `ls /etc \| wc -l` (Part 4) |
| `report.md` | the answers to all "Observe" boxes |
| `Lab7-8_GabrielaBitca_FAF-242.docx` | the full report for Labs 7 and 8: what I did, the answers with the commands and outputs that support them, and a conclusion |

# 1 PART 1: FORK + EXEC + WAIT

## 1.1 Commands and output

I created `run.c` with the content from the handout. It does what the shell does for `ls -l`: the parent copies itself with `fork()`, the copy turns itself into `ls` with `execlp()`, and the parent waits for it with `waitpid()` and prints its exit status.

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/* the three steps of the shell for the command: ls -l */
int main(void)
{
    printf("parent: my PID is %d\n", getpid());
    fflush(stdout);

    pid_t pid = fork();  /* step 1: copy myself */
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {  /* only the child runs this */
        printf("child:  my PID is %d, my parent is %d\n",
               getpid(), getppid());
        fflush(stdout);

        execlp("ls", "ls", "-l", (char *)NULL);  /* step 2 */
        perror("exec");  /* runs only if exec failed */
        exit(127);
    }

    int status;
    waitpid(pid, &status, 0);  /* step 3 */
    printf("parent: child %d ended with status %d\n",
           pid, WEXITSTATUS(status));

    return 0;
}
```

*Listing 1.1 - run.c*

I compiled and ran it:

```text
$ gcc -Wall -Wextra -o run run.c && ./run
parent: my PID is 85994
child:  my PID is 85995, my parent is 85994
total 24
-rw-rw-r-- 1 gabi gabi   282 Oct 13 01:22 report.md
-rwxrwxr-x 1 gabi gabi 70928 Oct 13 01:22 run
-rw-rw-r-- 1 gabi gabi   830 Oct 13 01:22 run.c
parent: child 85995 ended with status 0
```

*Listing 1.2 - run.c with ls -l*

Then I made the child run `date` instead of `ls`, by replacing the `execlp` line with `execlp("date", "date", (char *)NULL);`:

```text
$ sed -i 's/execlp("ls", "ls", "-l", (char \*)NULL);/execlp("date", "date", (char *)NULL);/' run.c
$ gcc -Wall -Wextra -o run run.c && ./run
parent: my PID is 86039
child:  my PID is 86040, my parent is 86039
Tue Oct 13 01:23:16 AM EEST 2026
parent: child 86040 ended with status 0
```

*Listing 1.3 - The child runs date*

Finally, I asked for a program that does not exist, `nosuchprog`:

```text
$ sed -i 's/execlp("date", "date", (char \*)NULL);/execlp("nosuchprog", "nosuchprog", (char *)NULL);/' run.c
$ gcc -Wall -Wextra -o run run.c && ./run
parent: my PID is 86075
child:  my PID is 86076, my parent is 86075
exec: No such file or directory
parent: child 86076 ended with status 127
```

*Listing 1.4 - The child asks for a program that does not exist*

After the experiment I changed the line back to `ls -l`, so the submitted `run.c` is the original version.

## 1.2 Observations

The order of the lines was always the same as in the handout: the parent speaks first, then the child, then the output of the new program, and the parent's last line comes only after the child has finished, because `waitpid()` blocks the parent until then. In all three runs the child's PID was exactly one more than the parent's (85994 → 85995, 86039 → 86040, 86075 → 86076), because Linux gives new processes increasing PIDs and nothing else was started in between.

The child's own `printf` line appeared even when it later became `ls` or `date`, because the child printed it before calling `execlp()`. The `fflush(stdout)` calls matter here: without them the text could still be waiting in the `stdio` buffer when `exec` replaces the program, and it would be lost.

## 1.3 Answers

**(1) The line `perror("exec")` is in the child, right after `execlp`. Why is it printed only with `nosuchprog`?** Because a successful `execlp()` never returns. It replaces the whole program of the child (its code, data and stack) with `ls` or `date`, so the lines after it no longer exist in that process. `execlp()` returns only when it fails, and then the next line, `perror("exec")`, runs and prints the reason: "No such file or directory", because no program called `nosuchprog` was found in any folder of `PATH`.

**(2) Which number does the child print as "my parent", and whose PID is it?** In the first run the child printed `my parent is 85994`, which is exactly the number the parent printed as its own PID on the first line. So it is the PID of the process that called `fork()`, the original `./run`. The same holds for the other runs (86039 and 86075).

**(3) What status does the parent see with `ls`, and what status with `nosuchprog`?** With `ls` (and with `date`) the parent saw status 0: the program ran and finished successfully. With `nosuchprog` it saw status 127: `execlp()` failed, so the child ran `exit(127)`, the same code a shell uses for "command not found". `waitpid()` stores this exit code in `status`, and `WEXITSTATUS(status)` extracts it.

# 2 PART 2: A ZOMBIE, ON PURPOSE

## 2.1 Commands and output

I copied the original `run.c` (the version with `ls`) to `zombie.c` and added one line, `sleep(20);`, just before `waitpid`:

```text
$ cp run.c zombie.c
$ sed -i '/waitpid(pid, &status, 0);/i\    sleep(20);' zombie.c
```

After this change, the end of `zombie.c` looks like this:

```c
    int status;
    sleep(20);
    waitpid(pid, &status, 0);  /* step 3 */
```

*Listing 2.1 - The changed part of zombie.c*

I compiled it, started it in the background with its output thrown away, and looked at the process table while the parent was sleeping:

```text
$ gcc -Wall -Wextra -o zombie zombie.c
$ ./zombie > /dev/null &
[2] 86477
$ sleep 2
$ ps -o pid,ppid,stat,cmd
    PID    PPID STAT CMD
  29163   29139 Ss   /usr/bin/bash
  81527   29163 T    nano user/find.c
  86477   29163 S    ./zombie
  86479   86477 Z    [ls] <defunct>
  86532   29163 R+   ps -o pid,ppid,stat,cmd
```

*Listing 2.2 - The process table while the parent sleeps*

After more than 20 seconds I ran `ps` again:

```text
$ ps -o pid,ppid,stat,cmd
    PID    PPID STAT CMD
  29163   29139 Ss   /usr/bin/bash
  81527   29163 T    nano user/find.c
  86642   29163 R+   ps -o pid,ppid,stat,cmd
[2]-  Done                       ./zombie > /dev/null
```

*Listing 2.3 - The process table after the parent woke up*

## 2.2 Observations

In the first table, `./zombie` (PID 86477) is in state `S`, sleeping in `sleep(20)`, and its child `ls` (PID 86479, PPID 86477) is in state `Z` and shown as `[ls] <defunct>`. The name is in square brackets because the zombie has already given back its memory and its command line; only its entry in the process table is left.

In the handout the parent of `./zombie` is PID 1, while on my system it is 29163, my `bash`, because I started it from my own shell. The other lines are my shell (`Ss`), `ps` itself (`R+`, running in the foreground) and an old `nano user/find.c` from Lab 6 in state `T`, stopped, which I had suspended with Ctrl-Z and which has nothing to do with this experiment.

In the second table both `./zombie` and the zombie are gone, and `bash` reported `[2]- Done ./zombie > /dev/null`: the background job had finished.

## 2.3 Answers

**(1) Which letter does `ls` have in the STAT column, and what does it mean?** `ls` has the letter `Z`, a zombie (`ps` also writes `<defunct>`). `ls` had already finished its work and exited, but its parent had not yet called `waitpid()` to collect its exit status. The kernel therefore keeps a small entry for it in the process table, with its PID and its exit status, until the parent asks for it.

**(2) After 20 seconds, is the `[ls] <defunct>` line still there? Which line of `zombie.c` made it disappear?** No, it is gone (Listing 2.3). The line that made it disappear is `waitpid(pid, &status, 0);`: when `sleep(20)` ended, the parent called `waitpid()`, which read the child's exit status and let the kernel remove the zombie's entry from the process table. Then the parent printed its last line and exited too.

# 3 PART 3: REDIRECTION: LS -L > OUT.TXT

## 3.1 Commands and output

This program does what the shell does for `ls -l > out.txt`: it closes descriptor 1, opens the file, which gets the smallest free number, and then runs `ls`. My version of `redirect.c` is shown in Listing 3.1.

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

/* redirects stdout to out.txt, then runs ls -l */
int main(void)
{
    close(1);  /* close stdout */

    int fd = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    fprintf(stderr, "out.txt opened with file descriptor %d\n", fd);

    execlp("ls", "ls", "-l", (char *)NULL);
    perror("exec");
    return 127;
}
```

*Listing 3.1 - redirect.c*

I compiled and ran it, then looked inside the file:

```text
$ gcc -Wall -Wextra -o redirect redirect.c && ./redirect
out.txt opened with file descriptor 1
$ cat out.txt
total 64
-rw-r--r-- 1 gabi gabi     0 Oct 13 01:35 out.txt
-rwxrwxr-x 1 gabi gabi 70664 Oct 13 01:35 redirect
-rw-rw-r-- 1 gabi gabi   467 Oct 13 01:35 redirect.c
-rw-rw-r-- 1 gabi gabi   282 Oct 13 01:22 report.md
-rwxrwxr-x 1 gabi gabi 70928 Oct 13 01:23 run
-rw-rw-r-- 1 gabi gabi   830 Oct 13 01:30 run.c
-rwxrwxr-x 1 gabi gabi 70976 Oct 13 01:31 zombie
-rw-rw-r-- 1 gabi gabi   845 Oct 13 01:31 zombie.c
```

*Listing 3.2 - Running redirect and the content of out.txt*

## 3.2 Observations

Only one line appeared on the screen, the message about the descriptor, because I printed it to `stderr` (descriptor 2), which was still connected to the terminal. The whole listing of `ls` went into `out.txt`. In the listing, `out.txt` itself has size 0: `open()` had already created and emptied the file, and `ls` read the folder before any of its own output was written into the file.

My program does the redirection in the process itself, without `fork()`. This shows why the real shell needs `fork()` here: after `execlp()` my program no longer exists, it has become `ls`, so nothing is left to continue afterwards. The shell does `close(1)`, `open()` and `exec` only in a child, so its own descriptor 1 stays connected to the terminal and the shell itself survives the command.

## 3.3 Answers

**(1) Which number did `open` give, and why exactly that one?** `open` gave descriptor 1 (`out.txt opened with file descriptor 1`). The kernel always gives a new file the smallest free descriptor number. Descriptor 0 (standard input) was still in use, and descriptor 1 had just been freed by `close(1)`, so 1 was the smallest free number.

**(2) `ls` was not changed at all. Why does its listing not appear on the screen?** `ls` simply writes to descriptor 1, as every program does; it does not know or care what descriptor 1 is. The open descriptors are kept across `exec`, so when `ls` started, its descriptor 1 was no longer the terminal but `out.txt`, and the listing went into the file.

**(3) To redirect the errors instead (like `2> err.txt`), which descriptor would you close before `open`?** Descriptor 2, standard error. After `close(2)` the smallest free number is 2, so `open("err.txt", ...)` would return 2, and the error messages of the program would go into `err.txt` while normal output would still go to the terminal.

# 4 PART 4: A PIPE: LS /ETC | WC -L

## 4.1 Commands and output

This program builds the pipeline by hand, with one pipe and two children. In the `ls` child, descriptor 1 becomes the writing end `p[1]`; in the `wc` child, descriptor 0 becomes the reading end `p[0]`. `dup()` copies a descriptor into the smallest free number, exactly like `open()` in Part 3.

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/* does the same as the shell command: ls /etc | wc -l */
int main(void)
{
    int p[2];

    if (pipe(p) < 0) {
        perror("pipe");
        return 1;
    }

    if (fork() == 0) {
        close(1);
        dup(p[1]);
        close(p[0]);
        close(p[1]);

        execlp("ls", "ls", "/etc", (char *)NULL);
        perror("exec ls");
        exit(127);
    }

    if (fork() == 0) {
        close(0);
        dup(p[0]);
        close(p[0]);
        close(p[1]);

        execlp("wc", "wc", "-l", (char *)NULL);
        perror("exec wc");
        exit(127);
    }

    close(p[0]);
    close(p[1]);

    wait(NULL);
    wait(NULL);

    return 0;
}
```

*Listing 4.1 - pipeline.c*

I compiled it and compared it with the real shell pipeline:

```text
$ gcc -Wall -Wextra -o pipeline pipeline.c
$ ./pipeline
244
$ ls /etc | wc -l
244
```

*Listing 4.2 - pipeline.c and the shell give the same number*

Then I broke it on purpose by turning the parent's `close(p[1]);` into a comment. My `sed` command only matches a `close(p[1]);` indented by exactly four spaces, which is the parent's line; the two lines in the children are indented by eight spaces and were not changed. The program did not end, and I stopped it with Ctrl-C:

```text
$ sed -i 's/^    close(p\[1\]);/    \/\/ close(p[1]);/' pipeline.c
$ gcc -Wall -Wextra -o pipeline pipeline.c
$ ./pipeline
^C
```

*Listing 4.3 - Without the parent's close(p[1]) the program never ends*

Finally, I put the line back, and the program worked again:

```text
$ sed -i 's/^    \/\/ close(p\[1\]);/    close(p[1]);/' pipeline.c
$ gcc -Wall -Wextra -o pipeline pipeline.c
$ ./pipeline
244
```

*Listing 4.4 - The repaired program*

## 4.2 Observations

The two `dup` calls are the same trick as `open` in Part 3. In the `ls` child, `close(1)` frees descriptor 1, so `dup(p[1])` copies the writing end into 1; in the `wc` child, `close(0)` frees descriptor 0, so `dup(p[0])` copies the reading end into 0. Then every process closes the original `p[0]` and `p[1]` it no longer needs, so `ls` and `wc` run with only 0, 1 and 2, as usual, without knowing they are connected by a pipe.

When the program hung, nothing was printed at all, not even a wrong number: `wc` prints its count only at the very end, after it has read all its input. The parent was also stuck, inside the second `wait(NULL)`, waiting for `wc` to end.

## 4.3 Answers

**(1) Did `./pipeline` and `ls /etc | wc -l` give the same number?** Yes, both printed 244 (Listing 4.2), and the repaired program printed 244 again (Listing 4.4).

**(2) After the change, which of the two children never ends, `ls` or `wc`?** `wc`. `ls` writes its 244 lines into the pipe and exits normally, but `wc` never finishes.

**(3) Why, in one sentence: what is `wc` waiting for?** `wc` is waiting for end-of-file on the pipe, which the kernel signals only when every writing end of the pipe is closed, and the parent still kept its copy of `p[1]` open, so as far as `wc` could tell, more data might still come.

# Conclusion

In this laboratory work I ran, step by step, the mechanisms that the shell uses for every command I type. `run.c` showed the three steps of starting a program: `fork()` makes a copy of the process, `exec` replaces the copy with a new program and never returns when it succeeds, and `waitpid()` lets the parent wait for the child and read its exit status, 0 for `ls` and 127 for a program that does not exist.

`zombie.c` showed what happens when the parent does not wait: the finished child stays in the process table as a zombie, `Z` and `<defunct>`, until the parent's `waitpid()` collects its status and the entry disappears.

The last two programs showed that redirection and pipes are only a matter of file descriptors. Because a new descriptor always gets the smallest free number, closing descriptor 1 and then calling `open()` or `dup()` puts a file or a pipe in the place of standard output, and the program, which was not changed at all, writes there without knowing it. Finally, breaking the pipe on purpose showed why every unused pipe end must be closed: a reader gets end-of-file only when the last writing end is closed, and a single forgotten `close(p[1])` is enough to make the whole pipeline hang.
