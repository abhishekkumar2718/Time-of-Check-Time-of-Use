#include <stdio.h>
#include <unistd.h>
#include <errno.h>

/*
 * An insecure way to check if the current user has write access to a file,
 * demonstrating Time-of-Check Time-of-Use vulnerability.
 *
 * Time-of-Check Time-of-Use is a race condition in which the state of a
 * resource (typically a file) is changed after the check, invalidating the
 * check itself.
 *
 * In our demonstration, we have two linux users - abhishek and sachin and
 * two linux files - temporary-file, is owned by abhishek and
 * privileged-file, owned by sachin. abhishek does not have read, write or
 * execute permissions to the privileged-file.
 *
 * However, the program is run as root and uses access() to verify whether the
 * current user has write permission to temporary-file. However, after the check,
 * there is a gap before the file is used - during which the attack deletes
 * temporary-file and creates a symlink to privileged-file. This incorrectly
 * updates the privileged-file instead of temporary-file.
 */
int main() {
  FILE *file;
  char file_name[] = "temporary-file";

  // Check whether the current user has write access to the file.
  if (!access(file_name, W_OK)) {
    /*
     * The attacker requires a window of time between the check and use.
     * Even simple, single-threaded applications will have such windows
     * as the OS can evict the process at any point as part of its job
     * scheduling.
     *
     * However, let's explicitly add a window to make the demonstration
     * simple and reliable.
     */
    for (int i = 0; i < 2; i++) {
      sleep(5);
      printf("Five seconds have passed...\n");
    }

    /*
     * During the gap between check and use, the attacker will create a
     * symlink from temporary-file to privileged-file.
     */
    file = fopen(file_name, "w+");  // Symlink resolves to privileged-file
    fprintf(file, "Hello, World!\n"); // Updates privileged-file!
  } else
    printf("Unable to open file %s\n", file_name);

  return 0;
}
