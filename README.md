# Time-of-Check Time-of-Us e

Time-of-Check Time-of-Use is a race condition in which the state of a
resource (typically a file) is changed after the check, invalidating the
check itself.

TODO: Add a point about how attacker needs a window of attack

## Demonstration

In my demonstration, I have two linux users - `abhishek` and `sachin`
and two files - `temporary-file`, owned by abhishek and
`privileged-file`, owned by sachin.

Abhishek does not have read, write or execute permissions to the
`privileged-file`.

However, the program is run as root and uses
`[access()](https://man7.org/linux/man-pages/man2/access.2.html)` to
verify whether the current user has write permission to
`temporary-file`. However, after the check, there is a gap before the
file is used - during which the attacker deletes `temporary-file` and
creates a symlink to `privileged-file`. This incorrectly updates the
`privileged-file` instead of `temporary-file`.

## Secure Implementation

`safe_open_wplus()` works as follows:

1. `[lstat()](https://man7.org/linux/man-pages/man2/lstat.2.html)` the
   file before opening.
2. `[open()](https://man7.org/linux/man-pages/man2/open.2.html) the
   file, returning a file descriptor.
3. `[fstat()]()` the file descriptor returned in second step.
4. Compare the file type and mode, inode numbmer and ID of device
   containing file between stat structures returned in first and third
   steps.
5. If the stat structures are same, return FILE pointer by opening the
   file descriptor.

`safe_open_wplus()` is _safer_ because:
- Uses `lstat()` instead of `stat()` or `access()` and does not resolve
  symbolic links.
- Compares the stat structures before and after opening the file, and
  verifies if they are same.
- Relies on file descriptor and inode numbers which are immutable
  instead of file names, which can point to different files.

## References

1. [Time-of-Check, Time-of-Use | Building Secure Software: Race Conditions | InformIT](https://www.informit.com/articles/article.aspx?p=23947&seqNum=30)
2. [TOCTTOU Vulnerabilities in UNIX-Style File Systems: An Anatomical Study](https://www.usenix.org/conference/fast-05/tocttou-vulnerabilities-unix-style-file-systems-anatomical-study)
