# Lab 5: The File System with Your Own Hands

Gabriela Bîtca, FAF-242

## Part 1: The tree and the file types

**(1) For each path in the second line, write its type from the first letter of `ls -l`.**

| Path | First letter | Type |
|---|---|---|
| `/etc` | `d` | directory |
| `/etc/passwd` | `-` | regular file (text file with the user accounts) |
| `/bin` | `l` | symbolic link |
| `/dev/null` | `c` | character device |
| `/tmp` | `d` | directory (its permissions end in `t`, the sticky bit, explained in Part 3) |

**(2) What is `/bin` on your system: a directory or a link, and to where?**

`/bin` is a symbolic link to `usr/bin`, that is, to `/usr/bin`. Both `ls -ld` (`/bin -> usr/bin`) and `file` (`symbolic link to usr/bin`) confirm it. Modern Ubuntu merged `/bin` into `/usr/bin` and keeps `/bin` only as a link so that old paths like `/bin/sh` still work.

**(3) Find one character device and one block device in `/dev`.**

A character device is `/dev/null` (type `c`, major 1, minor 3): it discards everything written to it and returns end-of-file when read. A block device is `/dev/sda3` (found with `find /dev -type b`), the third partition of my virtual disk. Block devices are accessed in fixed-size blocks and are used for storage, while character devices are accessed as a stream of bytes.

**(4) Which is bigger on your laptop, `/var/log` or `/usr`, and by roughly how much?**

`/usr` is much bigger: about 6.9 GB against 21 MB for `/var/log`, so roughly 6.9 GB more, or about 330 times bigger (6.9 × 1024 / 21 ≈ 336). This makes sense: `/usr` holds all installed programs and libraries, while `/var/log` only holds text log files.

## Part 2: Inodes and links

**(1) Which number in `stat` is the inode, and which tells how many names the file has?**

The inode number is `Inode: 3151262` (the same number `ls -li` prints in its first column). The number of names is `Links: 1`, the link count: how many directory entries point to this inode.

**(2) After `rm notes.txt`, why could you still read `hard.txt`? What does `rm` really remove?**

Because `hard.txt` is a directory entry that points to the same inode 3151262, and the inode and its data were not touched. `rm` only removes a directory entry (it calls the `unlink` system call) and decreases the link count of the inode, here from 2 to 1. The file system frees the inode and its data blocks only when the link count reaches 0 and no process still has the file open.

**(3) Why did `cat soft.txt` fail at the end, although `soft.txt` still exists? How big is `soft.txt` and why exactly that size?**

A symbolic link does not point to an inode; it only stores a path, here the text `hard.txt`. When `cat` opens `soft.txt`, the kernel follows that path, and since `hard.txt` was deleted, there is nothing to find: "No such file or directory". The link itself still exists as a dangling (broken) link. `soft.txt` is 8 bytes, exactly the length of the stored path `hard.txt` (8 characters, without a terminating zero).

**(4) A new directory has 2 links, and 4 after you create two subdirectories. Explain where each link comes from.**

- **Link 1:** the entry `d` in the parent directory (`~/os-lab5`).
- **Link 2:** the entry `.` inside `d`, which points to `d` itself.
- **Links 3 and 4:** the entries `..` inside `d/x` and inside `d/y`; each subdirectory's `..` points back to its parent `d`.

So the link count of a directory is always 2 plus the number of its immediate subdirectories.

**(5) Why does a 2-byte file use 4.0K, and a 1 GB file use 0?**

`ls -l` shows the logical size of a file, while `du` shows the disk space actually allocated. The file system gives space in whole blocks of 4 KiB (`IO Block: 4096` in `stat`), so even the 2-byte file `one.txt` (`x` plus a newline) occupies one full 4 KiB block. `truncate -s 1G` only changes the size recorded in the inode without writing any data, which creates a sparse file: the unwritten regions have no data blocks at all (reading them returns zeros), so `big.img` has a logical size of 1 GB but uses 0 bytes on disk.

## Part 3: Permissions

**(1) Write 600, 644, 755 and 700 as rwx strings, and give one real use for each.**

Each digit is the sum r = 4, w = 2, x = 1, for the owner, the group and others, in this order.

| Octal | rwx string | Meaning | Real use |
|---|---|---|---|
| 600 | `rw-------` | only the owner can read and write | an SSH private key (`~/.ssh/id_ed25519`); `ssh` refuses keys that others can read |
| 644 | `rw-r--r--` | owner reads and writes, everyone else only reads | ordinary documents and config files, such as `/etc/passwd` |
| 755 | `rwxr-xr-x` | owner does everything, others read and execute | programs such as those in `/usr/bin`, and directories that others may enter |
| 700 | `rwx------` | only the owner can do anything | a private directory such as `~/.ssh`, or a personal script |

**(2) With `chmod 000` you own the file but cannot read it. Why does ownership not help?**

The kernel checks only one class of permission bits: if I am the owner, it uses the owner bits and stops there, without looking at the group or others bits. With `000` the owner bits are `---`, so reading is refused even for the owner. Ownership gives a different right: only the owner (or root) may change the permissions, which is why `chmod u+r` succeeded and then `cat` worked again.

**(3) What is your umask, and how does it explain the permissions of `new.txt` and `newdir`?**

My umask is `0002`; the same value is confirmed by Part 2, where `notes.txt` was created with mode 0664. The umask lists the bits that are removed from every new file. Programs create files with mode 666 (`rw-rw-rw-`) and directories with mode 777 (`rwxrwxrwx`); removing the "write for others" bit gives 664 (`rw-rw-r--`) for `new.txt` and 775 (`rwxrwxr-x`) for `newdir`. New files never get `x`, because the starting mode for files is 666, not 777.

**(4) Explain the box experiment: what does `r` allow on a directory, and what does `x` allow?**

A directory is a table of names and inode numbers. `r` allows reading that table, so I can list the names (`ls box`), but without `x` I cannot use them to reach the files, so `cat box/f` is refused. `x` (search permission) allows passing through the directory to reach an entry by its name, so `cat box/f` works if I already know the name, but without `r` I cannot list what the directory contains, so `ls box` is refused. To work normally with a directory, I need both.

**(5) What do the `s` in `/usr/bin/passwd` and the `t` in `/tmp` mean, and why does `passwd` need the `s`?**

The `s` in the owner's execute position (`rwsr-xr-x`) is the setuid bit: the program runs with the permissions of the file's owner, which is root, not those of the user who started it. `passwd` needs it because changing a password means writing the new hash into `/etc/shadow`, which a normal user can neither read nor write (as `cat /etc/shadow` showed). The `t` at the end of `/tmp` (`rwxrwxrwt`) is the sticky bit: everyone may create files in `/tmp`, but only the owner of a file (or the owner of the directory, or root) may delete or rename it, so users cannot delete each other's files.

## Part 4: Disks and the kernel as files

**(1) Draw your disk: disk name, partitions, which one is mounted at `/` and with what file system type.**

My virtual disk is `sda` (64 GB). Its partition `sda3` is an LVM physical volume, and the logical volume `ubuntu--vg-ubuntu--lv` created on it holds the ext4 root file system, mounted at `/`. The other, smaller partitions of `sda` are used for booting.

```text
sda                          64G  disk
├─ ...                            part   (boot partitions)
└─sda3                            part   LVM physical volume
  └─ubuntu--vg-ubuntu--lv    60G  lvm    ext4   /     <- root file system
```

**(2) How full is your root file system?**

According to `df -h`, the file system mounted at `/` has a size of about 60 GB, of which 14 GB are used and 44 GB are available, so it is 24% full. Used and available add up to slightly less than the size, because ext4 keeps a small part of the blocks reserved for root.

**(3) How many CPU cores and how much RAM does your laptop have, according to `/proc`?**

`grep -c processor /proc/cpuinfo` counted 2 processors, and `/proc/meminfo` shows `MemTotal: 3461500 kB`, about 3.3 GiB (3461500 / 1024 / 1024 ≈ 3.30). Since Ubuntu runs in a Parallels virtual machine, these are the cores and memory given to the VM, not the full hardware of the MacBook.

**(4) `/proc/self/status` showed the name of a program. Which one, and why that one?**

It showed `Name: head`. `/proc/self` is a special link that always points to the `/proc/<PID>` folder of the process that is looking at it. The process that opened `/proc/self/status` was `head` itself, not the shell, so it read its own status. For the same reason, `ls -l /proc/self/fd` listed the open file descriptors of `ls`: 0, 1 and 2 (standard input, output and error) connected to my terminal.
