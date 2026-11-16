# Lab 11: The Race and the Semaphore

## Part 1: See the race

1. What are the three steps of counter++?
2. How can two threads lose an addition?
3. Why does the result differ between runs?

## Part 2: Fix it with a mutex and semaphore

1. Are the mutex and semaphore results always 2,000,000?
2. What happens when the semaphore starts at 2?
3. Why are the locked versions slower?

## Part 3: A semaphore as a signal

1. What happens when the semaphore starts at 1?
2. What does a semaphore starting at 0 mean?

## Part 4: A semaphore as a counter

1. What is the largest value of inside? Which line guarantees it?
2. What changes when PLACES is set to 1?
