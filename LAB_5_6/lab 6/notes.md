# Lab 6: Inside the xv6 File System

Gabriela Bîtca, FAF-242

## Part 1: Read what xv6 shows you

**(1) What do types 1, 2 and 3 mean (`kernel/stat.h`)?**

Type 1 is `T_DIR`, a directory; type 2 is `T_FILE`, a regular file; type 3 is `T_DEVICE`, a device file. In my listing `.` and `..` are directories, all the programs and `README` are files, and `console` is the only device (size 0, because its data comes from the console driver, not from disk blocks).

**(2) Which inode number does the root directory have, and why are `.` and `..` the same there?**

The root directory has inode 1 (`ROOTINO` in `kernel/fs.h`). Normally `.` points to the directory itself and `..` points to its parent, but the root has no parent, so `mkfs` makes its `..` point back to the root itself. Both entries therefore show inode 1, and `cd ..` in `/` stays in `/`.

**(3) An empty directory `a` has size 32. How big is one entry, and which two entries does `a` contain?**

An xv6 directory entry is `struct dirent` in `kernel/fs.h`:

```c
struct dirent {
  ushort inum;          // 2 bytes
  char name[DIRSIZ];    // 14 bytes
};
```

One entry has 2 + 14 = 16 bytes, so 32 bytes means exactly 2 entries: `.` (pointing to `a` itself) and `..` (pointing to the root, inode 1). A directory in xv6 is just a file whose content is a table of these 16-byte entries.

## Part 2: Write mystat

**(1) Paste the outputs after `ln` and after `rm`. What changed and what did not?**

```text
$ mystat README . console
README: file, inode 2, links 1, size 2441 bytes
.: directory, inode 1, links 1, size 1024 bytes
console: device, inode 26, links 1, size 0 bytes
```

```text
$ ln README r2
$ ls
$ mystat README
$ rm README
$ mystat README
$ mystat r2
$ cat r2 | wc
48 336 2441
```

After `ln README r2`, `README` and `r2` referred to the same inode 2, and the link count was 2: they are two names for the same inode. After `rm README`, the name `README` no longer existed, but `r2` still showed inode 2 and size 2441, with links 1, and `cat r2 | wc` printed `48 336 2441`, the full content. What changed was only the list of directory entries and the link count (1 → 2 → 1). The inode number, the size and the data blocks did not change at all.

**(2) Find the line in `kernel/sysfile.c` (`sys_unlink`) that decrements the link count. When are the data blocks freed?**

In `sys_unlink`, after clearing the directory entry (writing a zeroed `struct dirent` over it), the kernel runs these two lines:

```c
  ip->nlink--;
  iupdate(ip);
```

`iupdate()` writes the new link count to the inode on disk. The data blocks are not freed here. They are freed later in `iput()` in `kernel/fs.c`: when the last in-memory reference to the inode is released (`ip->ref == 1`) and its link count is 0, `iput()` calls `itrunc()`, which frees all data blocks, and then sets the inode type to 0 so the inode itself becomes free. So the blocks are freed only when the file has no names left **and** no process still has it open, the same rule as on Linux.

**(3) xv6 has no symbolic links and no permissions. Look at `struct dinode` in `kernel/fs.h`: which fields does an xv6 inode have?**

```c
struct dinode {
  short type;               // File type (0 = free, T_DIR, T_FILE, T_DEVICE)
  short major;              // Major device number (T_DEVICE only)
  short minor;              // Minor device number (T_DEVICE only)
  short nlink;              // Number of links to inode in file system
  uint size;                // Size of file (bytes)
  uint addrs[NDIRECT+1];    // Data block addresses
};
```

An xv6 inode has only six fields: the type, the major and minor device numbers, the link count, the size and the block addresses (12 direct plus 1 indirect). Compared with ext4 there is no owner, no group, no permission bits and no timestamps, which is why xv6 has no permissions, and there is no symbolic link type, which is why `ln` can only make hard links. One on-disk inode takes 2 + 2 + 2 + 2 + 4 + 13 × 4 = 64 bytes.

## Part 3: Measure the limits

**(1) What does `ls` show for a long directory name, and which constant explains it?**

I used the name `averyverylongname` (17 characters), and `ls` showed `averyverylongn`, its first 14 characters. The constant is `DIRSIZ = 14`: the name field of `struct dirent` has a fixed size of 14 bytes, so longer names are truncated. A name of exactly 14 characters also has no terminating zero, which is why `find` and `ls` copy it into a buffer and add `'\0'` themselves.

**(2) Compute the largest file xv6 can hold.**

- Direct blocks: `NDIRECT` = 12 blocks.
- The indirect block is one block of `BSIZE` = 1024 bytes filled with 4-byte block numbers, so it points to `NINDIRECT` = 1024 / 4 = 256 more blocks.
- Total: `MAXFILE` = 12 + 256 = 268 blocks.
- Largest file: 268 × 1024 = **274,432 bytes = 268 KiB**.

This is tiny compared to ext4, which uses double and triple indirect blocks (or extents) to reach files of many terabytes.

**(3) From the `nmeta` line: how many blocks does the whole disk have, and how many are for the log, the inodes, and the data?**

| Region | Blocks |
|---|---|
| Boot block and superblock | 2 |
| Log | 31 |
| Inodes | 13 |
| Free-block bitmap | 1 |
| Total metadata (`nmeta`) | 47 |
| Data | 1953 |
| **Whole disk (`total`, `FSSIZE` in `kernel/param.h`)** | **2000** |

The whole disk has 2000 blocks of 1024 bytes, about 2 MB. The metadata takes 47 blocks: 2 for the boot block and the superblock, 31 for the log (the journal), 13 for the inodes and 1 for the free-block bitmap (2 + 31 + 13 + 1 = 47). The remaining 2000 − 47 = 1953 blocks are for data. Each inode block holds 1024 / 64 = 16 on-disk inodes, so the 13 inode blocks have room for 13 × 16 = 208 inodes, and a single bitmap block is enough, because its 1024 × 8 = 8192 bits can describe far more than 2000 blocks.
