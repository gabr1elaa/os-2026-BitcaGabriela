# Lab 1: The OS as a Resource Manager

**Name:** Bîtca Gabriela\
**Group:** FAF-242\
**Environment:** macOS on a MacBook Air M4. The teacher allowed macOS, so I replaced the Linux-only commands (`free`, `lsblk`, `/proc`) with macOS commands that show the same information.

---

## Setup

```
$ whoami
gabrielabitca
$ uname -a
Darwin Bimbimbim.local 25.6.0 Darwin Kernel Version 25.6.0: Fri Jul 31 19:11:03 PDT 2026; root:xnu-12377.161.14~5/RELEASE_ARM64_T8132 arm64
$ uptime
17:22  up 26 days, 15:37, 2 users, load averages: 2.48 2.38 2.15
```

I am logged in as `gabrielabitca`. The kernel that manages my laptop is Darwin (XNU) version 25.6.0 running on an ARM64 processor (Apple M4). My Mac has been running for 26 days without a restart.

---

## Part 1: Files

### Commands and output

Looking at the root of the file tree:

```
$ ls -la /   (shortened)
drwxrwxr-x  44 root  admin  1408 Sep 28 15:41 Applications
drwxr-xr-x   6 root  admin   192 Sep  2 01:45 Users
dr-xr-xr-x   4 root  wheel  5016 Sep  2 01:45 dev
lrwxr-xr-x@  1 root  wheel    11 Aug 13 05:51 etc -> private/etc
drwxr-xr-x@ 11 root  wheel   352 Aug 13 05:51 usr
```

Creating files and changing permissions:

```
$ ls -l
-rw-r--r--  1 gabrielabitca  staff  24 Sep 28 17:34 note.txt
-rw-r--r--  1 gabrielabitca  staff  24 Sep 28 17:34 renamed.txt
$ chmod 600 note.txt
$ ls -l note.txt
-rw-------  1 gabrielabitca  staff  24 Sep 28 17:34 note.txt
$ chmod 644 note.txt
$ ls -l note.txt
-rw-r--r--  1 gabrielabitca  staff  24 Sep 28 17:34 note.txt
```

### Observations

On macOS, `/etc` is not a real folder. The line starts with `l`, which means it is a symbolic link (a shortcut) that points to `/private/etc`. Also, almost everything directly under `/` belongs to `root`, while the files I created belong to me.

### Answers

**(1)** The files I created are owned by my user `gabrielabitca` and belong to the group `staff`. On macOS, `staff` is the default group for normal users.

**(2)** After `chmod 600`, `ls -l` shows `-rw-------` for note.txt:

| Characters | Meaning                                                    |
| ---------- | ---------------------------------------------------------- |
| `-`        | it is a normal file (`d` would be a directory, `l` a link) |
| `rw-`      | the owner (me) can read and write, but not execute         |
| `---`      | the group `staff` has no rights                            |
| `---`      | other users have no rights                                 |

So after `chmod 600`, only I can read and change the file. After `chmod 644`, everyone can read it again.

**(3)** Two directories under `/`:

- `/Users` holds the home folders of all users (mine is `/Users/gabrielabitca`).
- `/dev` holds device files, which represent disks, terminals and other hardware.

---

## Part 2: Processes

### Commands and output

Listing processes:

```
$ ps aux | head -5
USER               PID  %CPU %MEM      VSZ    RSS   TT  STAT STARTED      TIME COMMAND
_windowserver      606  10.7  1.2 436618576 314032   ??  Ss    2Sep26 2110:55.86 /System/Library/PrivateFrameworks/SkyLight.framework/Resources/WindowServer -daemon
gabrielabitca    21370   7.1  0.3 435440576  63248   ??  S    16Sep26  51:01.83 /usr/libexec/duetexpertd
gabrielabitca    38911   7.0  1.3 437212560 323152   ??  S     4:00PM   2:24.87 /Applications/Microsoft Word.app/Contents/MacOS/Microsoft Word
root               566   6.8  0.0 435369536  11936   ??  Ss    2Sep26  15:43.07 /usr/libexec/opendirectoryd
$ ps aux | wc -l
     789
$ ps -p 1
  PID TTY           TIME CMD
    1 ??       196:26.16 /sbin/launchd
$ top   (first lines)
Processes: 785 total, 2 running, 783 sleeping, 3873 threads
Load Avg: 1.11, 1.20, 1.47  CPU usage: 3.45% user, 2.36% sys, 94.17% idle
```

Starting and stopping a background process:

```
$ sleep 300 &
[1] 41027
$ jobs
[1]  + running    sleep 300
$ ps -ef | grep sleep
  501 41027 40659   0  5:40PM ttys000    0:00.00 sleep 300
  501 41030 40659   0  5:40PM ttys000    0:00.00 grep sleep
$ kill 41027
[1]  + terminated  sleep 300
```

Looking at one process. macOS has no `/proc`, so I used `ps` and `lsof` instead:

```
$ sleep 300 &
$ PID=$!
$ ps -o pid,stat,rss,command -p $PID
  PID STAT    RSS COMMAND
41051 SN     1168 sleep 300
$ lsof -p $PID | head -5
COMMAND   PID          USER   FD   TYPE DEVICE SIZE/OFF                NODE NAME
sleep   41051 gabrielabitca  cwd    DIR   1,15       96            39451311 /Users/gabrielabitca/os-lab1/demo
sleep   41051 gabrielabitca  txt    REG   1,15   100976 1152921500312571411 /bin/sleep
sleep   41051 gabrielabitca  txt    REG   1,15  2562000 1152921500312573255 /usr/lib/dyld
sleep   41051 gabrielabitca    0u   CHR   16,0  0t97832                 989 /dev/ttys000
$ kill $PID
```

### Observations

Out of 785 processes, only 2 were actually running and the other 783 were sleeping. The CPU was 94% idle. This shows that the OS keeps many processes alive, but it only gives CPU time to the ones that have work to do.

When I started `sleep 300 &`, the OS gave it the PID 41027, and `kill` stopped it. With `lsof` I could see what the OS keeps track of for a process: its current folder, its program file (`/bin/sleep`) and the terminal it is connected to (`/dev/ttys000`).

### Answers

**(1)** The process with PID 1 is `/sbin/launchd`. It is the first process that the macOS kernel starts when the computer boots. It then starts and manages all the other system services and apps. On Linux, the same job is done by `systemd`.

**(2)** About 788 processes were running (789 lines minus the header line). `top` showed 785, which is almost the same number.

**(3)** The `STAT` column for my `sleep` process shows `SN`. `S` means sleeping, which is the same as `State: S (sleeping)` in Linux's `/proc/<pid>/status`. The process is just waiting for its 300 second timer, so it does not use the CPU. `N` means it has a lower priority ("nice"), because the shell lowers the priority of background jobs.

---

## Part 3: Memory

### Commands and output

macOS has no `free` or `/proc/meminfo`, so I used `sysctl`, `top`, `vm_stat` and `ps` instead:

```
$ sysctl hw.memsize
hw.memsize: 25769803776
$ top -l 1 | grep PhysMem
PhysMem: 18G used (2573M wired, 3160M compressor), 5289M unused.
$ sysctl vm.swapusage
vm.swapusage: total = 3072.00M  used = 1452.06M  free = 1619.94M  (encrypted)
$ vm_stat | head -6
Mach Virtual Memory Statistics: (page size of 16384 bytes)
Pages free:                                   319392.
Pages active:                                 408263.
Pages inactive:                               418137.
Pages speculative:                             19774.
Pages throttled:                                   0.
$ sleep 300 & PID=$!
$ ps -o pid,rss,command -p $PID
  PID    RSS COMMAND
41104   1168 sleep 300
$ kill $PID
```

### Observations

The OS divides memory into pages of 16 KB. I calculated 319392 free pages x 16 KB, which is about 5.2 GB. This matches the "unused" value from `top`. I also noticed that macOS compresses memory (3160M compressor) to save space before it uses swap.

### Answers

**(1)** My Mac has 24 GB of RAM in total (25769803776 bytes). About 18 GB is used and about 5.2 GB is free.

**(2)** Swap is space on the disk that the OS uses as extra memory when the RAM is full. The OS moves memory pages that are not being used right now from RAM to the disk. It is much slower than RAM. My Mac has 3 GB of swap configured, and about 1.45 GB of it is used right now.

**(3)** A bare `sleep` process uses 1168 KB (about 1.1 MB) of resident memory. On macOS I checked this with the RSS column of `ps`, which is the same thing as VmRSS on Linux. It surprised me a bit, because `sleep` does nothing. But even a very simple program needs its own code, the system libraries and the loader (`/usr/lib/dyld`) in memory. A lot of this memory is shared with other processes, so it is not really 1 MB extra for every process.

---

## Part 4: Devices

### Commands and output

macOS has no `lsblk`, so I used `diskutil list` instead:

```
$ df -h   (first lines)
Filesystem        Size    Used   Avail Capacity iused ifree %iused  Mounted on
/dev/disk3s1s1   460Gi    12Gi    91Gi    12%    459k  954M    0%   /
devfs            210Ki   210Ki     0Bi   100%     726     0  100%   /dev
/dev/disk3s5     460Gi   345Gi    91Gi    80%    1.6M  954M    0%   /System/Volumes/Data
$ diskutil list   (first disk)
/dev/disk0 (internal, physical):
   #:                       TYPE NAME                    SIZE       IDENTIFIER
   0:      GUID_partition_scheme                        *500.3 GB   disk0
   1:             Apple_APFS_ISC Container disk2         524.3 MB   disk0s1
   2:                 Apple_APFS Container disk3         494.4 GB   disk0s2
   3:        Apple_APFS_Recovery Container disk1         5.4 GB     disk0s3
$ du -sh ~/os-lab1
 12K	/Users/gabrielabitca/os-lab1
$ ls -l /dev | head -4
crw-rw-rw-  1 root           wheel     0x1e000000 Sep  2 01:45 aes_0
crw-------  1 root           wheel     0x16000001 Sep 28 16:59 afsc_type5
brw-------  1 root           operator   0x2000000 Sep 28 15:40 apfs-raw-device.2.0
$ mount | head -3
/dev/disk3s1s1 on / (apfs, sealed, local, read-only, journaled)
devfs on /dev (devfs, local, nobrowse)
/dev/disk3s6 on /System/Volumes/VM (apfs, local, noexec, journaled, noatime, nobrowse)
```

### Observations

On macOS the root filesystem `/` is sealed and read-only, so the system files cannot be changed. My own files are stored on a separate volume, `/dev/disk3s5` ("Data"). In `ls -l /dev`, the first letter shows the type of device: `c` is a character device (data is sent byte by byte, like a terminal) and `b` is a block device (data is sent in blocks, like a disk).

### Answers

**(1)** The root filesystem `/` is mounted on `/dev/disk3s1s1`. This is a read-only snapshot of the "Macintosh HD" volume, which is inside `disk3`, and `disk3` is built on the physical SSD `disk0`.

**(2)** `/dev/disk0` stands for the real internal SSD of my Mac (500 GB). Another example is `/dev/ttys000`, which is the Terminal window I was typing in. I saw it in the `lsof` output in Part 2.

**(3)** "Everything is a file" means that the OS shows real hardware like disks and terminals as files in `/dev`, so programs can work with them using the same operations as normal files (open, read, write).

---

## Conclusion

The operating system manages four resources: files, processes, memory and devices. I could see the files with `ls -l` and the processes with `ps aux`. I could see the memory with `top -l 1 | grep PhysMem` (instead of `free -h` on Linux) and the devices with `diskutil list` (instead of `lsblk` on Linux).
