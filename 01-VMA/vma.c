

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define FILENAME "test_file.txt"
#define PAGESIZE 0x2000



/* Returns rough checksum */
unsigned eatmem(void** ptr, size_t mem_size)
{
  *ptr = malloc(mem_size);

  size_t iter;
  unsigned result = 0;
  for (iter = 0; iter < mem_size - sizeof(unsigned); iter += PAGESIZE)
  {
    result += *((unsigned *)(*ptr + iter));
  }
  return result;
}


int main(int argc, char** argv)
{
  char a;

  printf("PID: %d", getpid());
  scanf("%c", &a);

  void *ptr_alloc;
  unsigned chk_sum;

  chk_sum = eatmem(&ptr_alloc, 0x100000);

  printf("malloc(1MB): %p [%x]", ptr_alloc, chk_sum);
  scanf("%c", &a);

  free(ptr_alloc);
  chk_sum = eatmem(&ptr_alloc, 0x400000);

  printf("malloc(4MB): %p [%x]", ptr_alloc, chk_sum);
  scanf("%c", &a);

  free(ptr_alloc);
  chk_sum = eatmem(&ptr_alloc, 0x1000000);

  printf("malloc(16MB): %p [%x]", ptr_alloc, chk_sum);
  scanf("%c", &a);

  free(ptr_alloc);
  chk_sum = eatmem(&ptr_alloc, 0x4000000);

  printf("malloc(64MB): %p [%x]", ptr_alloc, chk_sum);
  scanf("%c", &a);

  free(ptr_alloc);
  chk_sum = eatmem(&ptr_alloc, 0x10000000);

  printf("malloc(256MB): %p [%x]", ptr_alloc, chk_sum);
  scanf("%c", &a);

  free(ptr_alloc);

  /* 1GB */
  chk_sum = eatmem(&ptr_alloc, 0x40000000);

  printf("malloc(1GB): %p [%x]", ptr_alloc, chk_sum);
  scanf("%c", &a);

  free(ptr_alloc);


  int i;
  void *ptr_allocs[4];
  for (i = 0; i < 4; i++) {
    chk_sum += eatmem(&(ptr_allocs[i]), 0x100000);
  }

  printf("4xmalloc(1MB): %p %p %p %p [%x]",
      ptr_allocs[0], ptr_allocs[1], ptr_allocs[2], ptr_allocs[3],
      chk_sum);
  scanf("%c", &a);

  for (i = 0; i < 4; i++) {
    free(ptr_allocs[i]);
  }


  int fd = open(FILENAME, O_RDONLY);

  printf ("fopen(%s, O_RDONLY): %d", FILENAME, fd);
  scanf("%c", &a);

  void *ptr_map = mmap(NULL, 128, PROT_READ, MAP_PRIVATE, fd, 0);
  
  printf ("mmap(.., %d, 0): %p %s", fd, ptr_map, (char *)ptr_map);
  scanf("%c", &a);

  munmap(ptr_map, 128);
  close(fd);
  

  return 0;
}
