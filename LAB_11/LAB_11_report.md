# Lab 11: The Race and the Semaphore

Gabriela Bîtca, FAF-242

## Part 1: See the race

**(1) `counter++` looks like one step. Write the three steps the CPU really does.**

The CPU does three steps: (1) load the value of `counter` from memory into a register, (2) add 1 to the value in the register, (3) store the register back into `counter` in memory.

**(2) Describe, with those steps, how two threads can lose one addition.**

Thread A loads 5, and before it stores anything, thread B also loads 5. A adds 1 and stores 6. B, which still has the old 5 in its register, adds 1 and also stores 6. Two additions were made but the counter grew only by 1, so one addition is lost.

**(3) Why is the result different at every run?**

The operating system decides when a thread is stopped or when the two threads run at the same time on two cores, and this changes from run to run. The number of bad moments, when both threads are between their load and their store, is different each time, so the number of lost additions is different too (45 423 in one run, 491 370 in another).

## Part 2: Fix it: mutex, then semaphore

**(1) Run mutex and sem three times each. Is the answer always 2 000 000?**

Yes. I ran `mutex` three times and `sem` three times and every run printed `counter = 2000000`. The lock lets only one thread at a time into the critical section, so no addition is lost.

**(2) What would happen in sem.c with sem_init(&s, 0, 2)? Try it and explain.**

The answer is wrong again: my run printed `counter = 1977898`. With the start value 2, two units are available, so both threads can pass `sem_wait` at the same time, and then both are inside the critical section together. The semaphore no longer gives mutual exclusion, so the race of Part 1 comes back, only smaller, because sometimes a thread must wait.

**(3) Why is the locked version slower?**

Every one of the 2 000 000 additions now does a lock and an unlock, and these are atomic operations on a variable that both cores use, so the cores must pass the cache line to each other all the time. When a thread finds the lock taken it has to wait (the library may even ask the kernel to put it to sleep), and while it waits, it does no work. The unprotected loop does none of this, and that is why it is fastest, and wrong.

## Part 3: A semaphore as a signal

**(1) Change the start value to 1 (sem_init(&ready, 0, 1)), compile, run. What does B read now, and why?**

B reads an empty message, `B: I read ""`, and this is printed before `A: message written`. With the start value 1, `sem_wait` finds one unit and does not block, so B does not wait for A at all. It reads `message` while it is still empty, because A writes it only after one second.

**(2) Put it back to 0. In one sentence: what does a semaphore that starts at 0 mean?**

A semaphore that starts at 0 means that nothing is available yet, so everyone who calls `sem_wait` must wait until another thread says, with `sem_post`, that the thing they wait for has happened: it is a signal.

## Part 4: A semaphore as a counter: the parking lot

**(1) What is the largest value of "inside" you see? Which line of code guarantees it?**

The largest value is 3. It is guaranteed by `sem_wait(&places);` at the start of `car()`, together with the start value in `sem_init(&places, 0, PLACES);` with `PLACES` equal to 3: a car can go on only after it has taken one of the three units, and the fourth car finds the semaphore at 0 and sleeps until a car leaves and calls `sem_post(&places)`.

**(2) Set PLACES to 1. What does the semaphore become?**

With `PLACES` equal to 1 the semaphore becomes a binary semaphore, which is a lock: only one car can be inside at a time. My run shows it: `inside` is never more than 1 and each car leaves before the next one enters, exactly as in `sem.c` of Part 2.
