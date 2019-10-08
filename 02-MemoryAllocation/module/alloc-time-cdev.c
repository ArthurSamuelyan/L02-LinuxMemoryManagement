#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>

#include "alloc-time-cdev.h"

/* Initialize major/minor but let the user have an argument */
static int alloc_time_cdev_major = CUSTOM_CDEV_MAJOR;
static int alloc_time_cdev_minor = CUSTOM_CDEV_MINOR;
module_param(alloc_time_cdev_major, int, S_IRUGO);
module_param(alloc_time_cdev_minor, int, S_IRUGO);

/* Initialize alloc-time-cdev quantum and alloc-time-cdev set */
static long alloc_time_cdev_size = CUSTOM_CDEV_SIZE;
module_param(alloc_time_cdev_size, long, S_IRUGO);

static int alloc_time_cdev_nr_devs = 1;

struct alloc_time_cdev *alloc_time_cdev_devices;

/* free data area */
static int alloc_time_cdev_trim(struct alloc_time_cdev *dev)
{
  dev->data = NULL;
  dev->size=0;
  return 0;
}

/* open function */
static int alloc_time_cdev_open(struct inode *inode, struct file *filp)
{
  struct alloc_time_cdev *dev; /* device information */

  dev = container_of(inode->i_cdev, struct alloc_time_cdev, cdev);
  filp->private_data = dev; /* for other methods */
  /* trim to 0 the length of the device if open was write-only */
  if ((filp->f_flags & O_ACCMODE) == O_WRONLY) {
    alloc_time_cdev_trim(dev);
  }
  return 0;
}

/* close operation */
static int alloc_time_cdev_release(struct inode *inode, struct file *filp)
{
  return 0;
}

/* read function */
static ssize_t alloc_time_cdev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
  /* =========================
   * Payload 1
   * ========================= */

  struct alloc_time_cdev *dev = filp->private_data;
  if (dev->data) {
    if (dev->virt) {
      vfree(dev->data);
    } else {
      kfree(dev->data);
    }
  }
  dev->virt = 0;
  dev->data = kmalloc(count, GFP_KERNEL);
  if (!dev->data) {
    dev->size = 0;
    return -ENOMEM;
  }
  memset(dev->data, 0, count);
  dev->size = count;
  return count;
}

/* llseek function */
static loff_t alloc_time_cdev_llseek(struct file *filp, loff_t count, int whence)
{
  /* =========================
   * Payload 2
   * ========================= */

  struct alloc_time_cdev *dev = filp->private_data;
  if (dev->data) {
    if (dev->virt) {
      vfree(dev->data);
    } else {
      kfree(dev->data);
    }
  }

  dev->virt = 1;
  dev->data = vmalloc(count);
  if (!dev->data) {
    dev->size = 0;
    return -ENOMEM;
  }
  memset(dev->data, 0, count);
  dev->size = count;
  return count;
}

/* write function */
static ssize_t alloc_time_cdev_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
  /* =========================
   * Payload 3
   * ========================= */

  struct alloc_time_cdev *dev = filp->private_data;
  if (dev->data) {
    if (dev->virt) {
      vfree(dev->data);
    } else {
      kfree(dev->data);
    }
  }
  dev->virt = 1;
  dev->data = kmalloc(count, GFP_ATOMIC);
  if (!dev->data) {
    dev->size = 0;
    return -ENOMEM;
  }
  memset(dev->data, 0, count);
  dev->size = count;
  return count;
}

/* File operations structure */
static struct file_operations alloc_time_cdev_fops = {
  .owner = THIS_MODULE, /* used to prevent unload while operations are used */
  .open = alloc_time_cdev_open,
  .release = alloc_time_cdev_release,
  .read = alloc_time_cdev_read,
  .write = alloc_time_cdev_write,
  .llseek = alloc_time_cdev_llseek,
};

/* cdev registration function */
static void alloc_time_cdev_setup_cdev(struct alloc_time_cdev *dev, int index)
{
  int err;
  int devno = MKDEV(alloc_time_cdev_major, alloc_time_cdev_minor + index);

  cdev_init(&dev->cdev, &alloc_time_cdev_fops);
  dev->cdev.owner = THIS_MODULE;
  dev->cdev.ops = &alloc_time_cdev_fops;
  err = cdev_add(&dev->cdev, devno, 1);
  if (err) {
    printk(KERN_NOTICE "alloc-time-cdev: Error %d while adding alloc-time-cdev cde %d", 
        err, index);
  }
}

static void alloc_time_cdev_do_exit(void)
{
  int i;
  dev_t devno = MKDEV(alloc_time_cdev_major, alloc_time_cdev_minor);

  /* remove char devices */
  if (alloc_time_cdev_devices) {
    for (i = 0; i < alloc_time_cdev_nr_devs; i++) {
      alloc_time_cdev_trim(alloc_time_cdev_devices + i);
      cdev_del(&alloc_time_cdev_devices[i].cdev);
    }
    kfree(alloc_time_cdev_devices);
  }

  /* We are sure register suceeded because module fails at init if otherwise */
  unregister_chrdev_region(devno, alloc_time_cdev_nr_devs);
  printk(KERN_INFO "alloc-time-cdev: Unloaded module.\n");
}

/* exit function */
static void __exit alloc_time_cdev_exit(void)
{
  alloc_time_cdev_do_exit();
}

/* init function */
static int __init alloc_time_cdev_init(void)
{
  int result, i;
  dev_t dev = 0;

  /* Allocate major and minor */
  if (alloc_time_cdev_major) {
    dev = MKDEV(alloc_time_cdev_major, alloc_time_cdev_minor);
    result = register_chrdev_region(dev, alloc_time_cdev_nr_devs, "alloc-time-cdev");
  } else {
    result = alloc_chrdev_region(&dev, alloc_time_cdev_minor, alloc_time_cdev_nr_devs, "alloc-time-cdev");
    alloc_time_cdev_major = MAJOR(dev);
  }
  if (result < 0) {
    printk(KERN_WARNING "alloc-time-cdev: Can't get major %d.\n", alloc_time_cdev_major);
  } else {
    printk(KERN_INFO "alloc-time-cdev: Got major %d.\n", alloc_time_cdev_major);
  }

  /* allocate devices */
  alloc_time_cdev_devices = kmalloc(alloc_time_cdev_nr_devs * sizeof(struct alloc_time_cdev), GFP_KERNEL);
  if (!alloc_time_cdev_devices) {
    result = -ENOMEM;
    goto fail;
  }
  memset(alloc_time_cdev_devices, 0, alloc_time_cdev_nr_devs * sizeof(struct alloc_time_cdev));

  /* init each device */
  for (i=0; i<alloc_time_cdev_nr_devs; i++) {
    alloc_time_cdev_devices[i].size = alloc_time_cdev_size;
    alloc_time_cdev_setup_cdev(&alloc_time_cdev_devices[i], i);
  }

  printk(KERN_NOTICE "alloc-time-cdev: Initialized %d alloc-time-cdev devices.\n", alloc_time_cdev_nr_devs);
  return 0;

fail:
  alloc_time_cdev_do_exit();
  return result;
}

module_init(alloc_time_cdev_init);
module_exit(alloc_time_cdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arthur Samuelyan <artur.samuelyan@phystech.edu>");
