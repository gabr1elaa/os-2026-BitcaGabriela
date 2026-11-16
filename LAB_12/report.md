# Lab 12: Producer, Consumer and the Other Tools

Gabriela Bîtca, FAF-242

## Part 1: Producer and consumer

**(1) The producer is fast, yet "in buffer" never goes above 4. Which call stops it?**

The call `sem_wait(&empty)` stops it. The semaphore `empty` counts the free slots and starts at 4; the producer takes one unit before every `put`, and when the buffer is full `empty` is 0, so the producer sleeps in `sem_wait(&empty)` until the consumer calls `sem_post(&empty)`.

**(2) Who waits at the start: the consumer or the producer? Which call makes it wait?**

The consumer waits at the start. It is created first, and it calls `sem_wait(&full)`; the semaphore `full` starts at 0 because there is nothing in the buffer, so the consumer sleeps there until the producer puts the first item and calls `sem_post(&full)`.

**(3) In producer(), swap the first two lines: lock the mutex BEFORE sem_wait(&empty). Run it. What happens when the buffer is full, and why?**

The program hangs (a deadlock). When the buffer is full the producer holds the mutex and waits in `sem_wait(&empty)` for a free slot. The only one who can make a slot free is the consumer, but to take an item it needs the same mutex, which the producer holds. So the producer waits for the consumer and the consumer waits for the producer. In my run the output stopped after `put 5 (in buffer: 4)`.

## Part 2: The condition variable

**(1) In order.c (Lab 11) the semaphore itself remembered that the message was ready. Here, which variable remembers it?**

The normal variable `ready` remembers it: A sets it to 1 under the mutex. The condition variable `changed` remembers nothing; it is only the place where B sleeps and from where A wakes it up.

**(2) Replace "while (!ready)" with "if (!ready)", compile, run. Does it still work here?**

Yes, it still works here: the output is the same as with `while`. There is only one waiting thread and A signals only once, after `ready` is already 1, so when B wakes up the condition is true.

**(3) Then why do we write while? Give one reason why a thread can wake up and still find ready == 0.**

Because a woken thread must check the condition again: it is not guaranteed to be true. One reason is that POSIX allows spurious wakeups, a thread can return from `pthread_cond_wait` even when nobody signalled. Another one is that with several waiting threads, or with `pthread_cond_broadcast`, another thread can take the mutex first, use the thing and set the condition back to 0 before the woken thread gets the mutex. With `if` the thread would go on with a false condition; with `while` it goes back to sleep.

## Part 3: Readers and writers

**(1) Three readers are inside at the same time, but a writer is always alone. Which two lines of reader() make that possible?**

The two lines are `if (++readers == 1) sem_wait(&room);` and `if (--readers == 0) sem_post(&room);`. The first reader that comes in takes the room, so no writer can enter, and the other readers pass freely because they do not wait on `room`. The last reader that goes out gives the room back, and then a writer can enter.

**(2) Why does the counter readers need its own lock (rlock)? What could go wrong without it?**

`++readers` and `--readers` are like `counter++` of Lab 11: three steps, so two readers can lose an update. Then `readers` could be wrong. For example, it could become 0 while a reader is still inside, and the last-out reader would give the room to a writer who writes while someone reads; or it could never get back to 0, and then `room` would never be released and the writers would wait forever. The lock `rlock` makes the check and the change of the counter one indivisible step.

**(3) Imagine readers that keep arriving without a pause. What happens to writer 2?**

The counter `readers` never gets back to 0, so no reader is the "last one out" and nobody calls `sem_post(&room)`. Writer 2 waits in `sem_wait(&room)` for ever: this is starvation. The program is not blocked, the readers go on working, but this one writer never gets its turn.

## Part 4: Build a deadlock, then remove it

**(1) In the stuck run, which thread holds which mutex, and which one is each thread waiting for? Draw the circle.**

Thread `one` holds mutex A and waits for mutex B. Thread `two` holds mutex B and waits for mutex A. The circle is: `one` holds A, A is wanted by `two`; `two` holds B, B is wanted by `one`. So: `one` -> waits for B -> held by `two` -> waits for A -> held by `one`. Nobody can move, because every one waits for something that only the other can free.

**(2) Remove both sleep(1) lines from the ORIGINAL version and run it ten times. Does it still always hang? Why is that worse, not better?**

No, it does not always hang: in my ten runs it hung 5 times and finished 5 times. It is worse, because the sleeps made the bad timing happen every time, so the bug was easy to see. Without them, `one` often takes both mutexes before `two` has started, and then everything is fine, so the program passes many tests and the deadlock comes out only now and then, at a random moment, which is much harder to find and to repeat. The bug is still there; it is only hidden.

**(3) In one sentence: why can taking the locks in the same order never deadlock?**

If all threads take the locks in the same order, a thread that holds a lock can only wait for a lock that comes later in that order, so the waiting can never go back to the start and make a circle, and without a circle somebody can always go on.
