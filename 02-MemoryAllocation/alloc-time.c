#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define UP (8ULL * 1024ULL * 1024ULL)
#define STEP (1024)

int main()
{
  unsigned long long i = 1;
  int fd = open("/dev/alloc-time-cdev", O_RDWR, 0);
  clock_t t0 = clock() / (CLOCKS_PER_SEC / 1000);

  for (i = 1; i <= UP; i+= STEP)
    read(fd, NULL, i);
  clock_t t1 = clock() / (CLOCKS_PER_SEC / 1000);

  for (i = 1; i <= UP; i+= STEP)
    write(fd, NULL, i);
  clock_t t2 = clock() / (CLOCKS_PER_SEC / 1000);

  for (i = 1; i <= UP; i+= STEP)
    lseek(fd, i, SEEK_SET);
  clock_t t3 = clock() / (CLOCKS_PER_SEC / 1000);

  printf("GPF_KERNEL: %li ms\nGPF_ATOMIC: %li ms\nvmalloc: %li ms\n", t1 - t0, t2 - t1, t3 - t2);
  close(fd);
  return 0;
}
