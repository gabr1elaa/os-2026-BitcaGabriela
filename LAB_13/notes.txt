# Lab 13: Semaphores in Your Own Kernel

Gabriela Bîtca, FAF-242

## Part 3: A semaphore as a lock: mix

**(1) Without the lock, why do the letters mix? (How many system calls does one line cost?)**

One line costs 31 system calls, because `mix` writes every letter with its own `write(1, p, 1)`: 30 letters and the new line. The two processes run at the same time on two CPUs, and the kernel does not know that the 31 writes belong together, so the writes of the two processes are interleaved and the letters mix.

**(2) With the lock every line is whole. Which two lines of mix.c make one line a critical section?**

The line `sem_down(3);` before the loop that writes the letters, and the line `sem_up(3);` after it. Semaphore 3 starts at 1 (`sem_init(3, 1)`), so it is a lock: only one process at a time can be between these two lines, which is the whole line of letters.

**(3) In sem_down the test is "while (s->value == 0)", not "if". Why can a process wake up and still find 0?**

`sem_up` calls `wakeup(s)`, which wakes ALL the processes that sleep on that semaphore, but it adds only one unit. The first process that gets the spinlock takes the unit and makes the value 0 again; the others wake up and find 0. They must test again and go back to sleep, and `while` does exactly that. (It is also how `sleep()` can return at once if `sem_up` came in between `sleep_prepare` and `sleep`, with no sleeping at all.) With `if` a process would take a unit that is not there, and the value would become -1.

**(4) sem_down calls release(&s->lock) BEFORE sleep(). What would go wrong if it slept holding the spinlock?**

`sem_up` needs the same spinlock to add 1 to the value. If the sleeping process kept it, nobody could ever call `sem_up` successfully: the process that would wake it up would spin on the lock for ever, and the sleeper would never be woken, so it would be a deadlock. A spinlock is also not meant to be held while a process sleeps: the CPU would go on to another process with the lock still taken, and everyone who needs that lock would spin and waste CPU.
