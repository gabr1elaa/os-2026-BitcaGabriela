# Lab 12: Producer, Consumer and the Other Tools

## Part 1: Producer and Consumer

1. Which call stops the producer when the buffer is full?
2. Who waits at the start, and which call makes it wait?
3. What happens if the mutex is locked before waiting for an empty slot?

>1. `sem_wait(&empty)` stops the producer when the buffer is full because the semaphore reaches zero and the producer must wait for an available slot.

2. The consumer waits at the beginning because `full` starts at 0. The call `sem_wait(&full)` blocks until the producer inserts an item and calls `sem_post(&full)`.

3. If the producer locks the mutex before waiting for an empty slot, it can hold the mutex while the buffer is full. The consumer cannot acquire the mutex to remove an item, resulting in deadlock.


## Part 2: The Condition Variable

1. The variable `message_ready` remembers whether the message is ready. It is set to 1 by the producer.

2. In this simple example, replacing `while` with `if` still works because the consumer receives the signal after the producer makes the message ready.

3. We should use `while` because condition variables can wake threads even when the condition is not satisfied. Checking the condition again prevents a thread from continuing incorrectly.

## Part 3: Readers and Writers

1. The first reader locks `wlock` when `readers == 1`, and the last reader unlocks it when `readers == 0`. This allows multiple readers while excluding writers.

2. The `readers` counter needs `rlock` to prevent multiple threads from modifying it simultaneously and causing a race condition.

3. If readers keep arriving, writers may have to wait indefinitely. This is called writer starvation.

## Part 4: Deadlock

1. Thread 1 holds `m1` and waits for `m2`, while Thread 2 holds `m2` and waits for `m1`.

2. No. Removing `sleep(1)` does not guarantee that deadlock will disappear. It only changes the timing and makes the deadlock less predictable.

3. Taking locks in the same order prevents circular waiting. Both threads request the same first mutex before requesting the second one, so they cannot hold one mutex each while waiting for the other.
