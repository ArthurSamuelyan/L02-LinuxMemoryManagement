#ifndef CUSTOM_CDEV_SIZE
#define CUSTOM_CDEV_SIZE (2 * 1024 * 1024)
#endif

#ifndef CUSTOM_CDEV_MAJOR
#define CUSTOM_CDEV_MAJOR 243
#endif

#ifndef CUSTOM_CDEV_MINOR
#define CUSTOM_CDEV_MINOR 1
#endif

struct alloc_time_cdev {
  struct cdev cdev; /* Char device structure */
  unsigned long size; /* amount of data stored */
  int virt;
  struct cha_qset *data; /* First qset in list */
};
