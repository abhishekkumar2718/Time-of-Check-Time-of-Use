/*
 * A secure way to check if the current user has write access to a file.
 *
 * Time-of-Check Time-of-Use is a race condition in which the state of a
 * resource (typically a file) is changed after the check, invalidating
 * the check itself. Refer to insecure-file-access.c for more background.
 *
 * Taken from: https://www.informit.com/articles/article.aspx?p=23947&seqNum=3
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#define HAS_ATTACK_WINDOW 1

/*
 * The safer function works as follows:
 * 1. lstat() the file before opening.
 * 2. open() the file, returning a file descriptor.
 * 3. fstat() the file descriptor returned in second step.
 * 4. Compare the file type and mode, inode number and ID of device containing
 *    file between stat structures returned in first and third steps.
 * 5. If the stat structures are same, return FILE pointer by opening the
 *    file descriptor.
 *
 * This function is safer because:
 * - Uses lstat() instead of stat() or access() and does not resolve
 *   symbolic links.
 * - Compares the stat structures before and after opening the file,
 *   and verifies if they are same.
 * - Relies on file descriptor and inode numbers which are immutable instead of
 *   file names which can point to different files.
 */
FILE *safe_open_wplus(char *fname) {
  int fd;
  FILE *fp;
  char *mode = "rb+"; // We perform our own truncation.
  struct stat lstat_info, fstat_info;

  if (lstat(fname, &lstat_info) == -1) {
    /*
     * If the lstat() failed for reasons other than the file not existing,
     * return 0, specifying error.
     */
    if (errno != ENOENT)
      return 0;

    // Introduce a deliberate window of attack to demostrate.
    if (HAS_ATTACK_WINDOW) {
      for (int i = 0; i < 2; i++) {
        sleep(5);
        printf("Five seconds have passed...\n");
      }
    }

    if ((fd = open(fname, O_CREAT|O_EXCL| O_RDWR, 0600)) == -1) {
      printf("File has been created during the window - we are under attack!\n");
      return 0;
    }

    mode = "wb";
  } else {
    // Introduce a deliberate window of attack to demostrate.
    if (HAS_ATTACK_WINDOW) {
      for (int i = 0; i < 2; i++) {
        sleep(5);
        printf("Five seconds have passed...\n");
      }
    }

    // Open an existing file
    if ((fd = open(fname, O_RDWR)) == -1)
      return 0;

    if (fstat(fd, &fstat_info) == -1
        || lstat_info.st_mode != fstat_info.st_mode
        || lstat_info.st_ino != fstat_info.st_ino
        || lstat_info.st_dev != fstat_info.st_dev ) {
      printf("Stat structures do not match - we are under attack!\n");
      close(fd);
      return 0;
    }

    // Turn the file into an empty file, to mimic w+ semantics.
    ftruncate(fd, 0);
  }

  // Open a FILE pointer using the file descriptor
  fp = fdopen(fd, mode);

  if (!fp) {
    close(fd);
    unlink(fname);
    return 0;
  }

  return fp;
}

int main() {
  char file_name[] = "temporary-file";

  FILE *file = safe_open_wplus(file_name);

  if (file)
    fprintf(file, "Hello, World\n");
  else
    printf("Unable to open file %s\n", file_name);

  return 0;
}
