#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#define BYTES_TO_KILOBYTES 8192

off_t diskusage(char* fname) {
  int fd, err;
  DIR* dirp;
  struct stat statbuf;
  struct dirent* entry;
  off_t total_size, entry_size;

  fd = open(fname, O_RDONLY);
  if (fd == -1) return 0; // can't open (didnt print error cuz lazy)

  err = fstat(fd, &statbuf);
  if (err == -1) return 0; // still recoverable

  if (!S_ISDIR(statbuf.st_mode)) { // its not a directory
    return statbuf.st_blksize * statbuf.st_blocks; // so we can just return
  }

  dirp = opendir(fname);
  if (!dirp) return 0; // its ok if it fails here, still recoverable

  total_size = 0;

  err = chdir(fname);
  if (err == -1) return 0; // its ok if it fails here, we just leave hah

  while ((entry = readdir(dirp))) {
    if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0)) {
      continue;
    } // we skip cwd and upper wd.

    entry_size = diskusage(entry->d_name);

    // keeping this line -> might be used in a particular option
    // printf("%lu\t%luK\t%s\n", entry_size, entry_size / 1024 / 8, entry->d_name);

    total_size += entry_size;
  }

  if (strcmp(fname, "./") != 0) {
    err = chdir("../"); // go back up!
    if (err == -1) exit(errno); // very bad if we can't get back up
  }

  return total_size;
}

int main(int argc, char** argv) {
  off_t total_size;
  
  if (argc > 1) {

    for (size_t i = 0; i < argc - 1; ++i) {
      total_size = diskusage(argv[i + 1]);

      printf("%lu\t%luK\t%s\n", total_size, total_size / BYTES_TO_KILOBYTES, argv[i + 1]);   
    }    

    return 0;
  }

  total_size = diskusage("./");

  printf("%lu\t%luK\t./\n", total_size, total_size / BYTES_TO_KILOBYTES);

  return 0;
}