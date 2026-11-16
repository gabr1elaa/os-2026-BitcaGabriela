# Lab 10: Add Your Own System Call: trace

Gabriela Bîtca, FAF-242

## Part 3: Play with the mask

**(1) In `trace 32 grep hello README` why do you see only `read`, and not `open` or `close`?**

Because 32 = 2^5 has only bit 5 on, and 5 is `SYS_read`. The kernel prints a call only if `(p->trace_mask >> num) & 1` is 1. For `open` (15) and `close` (21) the bits 15 and 21 of 32 are 0, so those calls run normally but are not printed.

**(2) In forkforkfork, three different pids print lines. Which line of code from Part 2 makes that possible?**

The line `np->trace_mask = p->trace_mask;` that I added to `fork()` in `kernel/proc.c`. Only the first process received the mask from the `trace` call; every child created by `fork` copies the mask of its parent, so the children and the children of the children are traced too. Without that line the children would start with mask 0 and print nothing.

**(3) In `trace 65536 echo hi` the trace line is printed between "hi" and the new line. Why? (How many times does `echo` call `write`?)**

`echo` calls `write` twice: first for the word `hi` (2 bytes, so `write -> 2`), then for the new line (1 byte, `write -> 1`). The kernel prints the trace line right after each call returns, on the same console. So the order on the screen is: `hi`, the trace line of the first write, the new line written by the second write, and the trace line of the second write.

**(4) Which mask traces both `open` (15) and `close` (21), and nothing else?**

The mask with only bits 15 and 21 on: 2^15 + 2^21 = 32768 + 2097152 = **2129920**, in hexadecimal `0x208000`.
