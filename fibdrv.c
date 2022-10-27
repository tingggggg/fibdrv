#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "bn_kernel.h"

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("National Cheng Kung University, Taiwan");
MODULE_DESCRIPTION("Fibonacci engine driver");
MODULE_VERSION("0.1");

#define DEV_FIBONACCI_NAME "fibonacci"

/* MAX_LENGTH is set to 92 because
 * ssize_t can't fit the number > 92
 */
#define MAX_LENGTH 300

static dev_t fib_dev = 0;
static struct cdev *fib_cdev;
static struct class *fib_class;
static DEFINE_MUTEX(fib_mutex);

/* Recursion with cache */
static long long fib_sequence(long long k)
{
    if (k < 2)
        return k;

    long long f_0 = 0, f_1 = 1, f;
    for (int i = 2; i <= k; i++) {
        f = f_0 + f_1;
        f_0 = f_1;
        f_1 = f;
    }

    return f;
}

/* Calculate Fibonacci numbers by Fast Doubling */
static long long fib_sequence_fdouble(long long n)
{
    if (n < 2)
        return n;

    long long f[2];
    f[0] = 0;
    f[1] = 1;

    for (unsigned int i = 1U << (31 - __builtin_clz(n)); i; i >>= 1) {
        long long k1 =
            f[0] * (f[1] * 2 - f[0]); /* F(2k) = F(k) * [ 2 * F(k+1) – F(k) ] */
        long long k2 =
            f[0] * f[0] + f[1] * f[1]; /* F(2k+1) = F(k)^2 + F(k+1)^2 */
        if (n & i) {                   /* current binary digit == 1 */
            f[0] = k2;                 /* F(n) = F(2k+1) */
            f[1] = k1 + k2; /* F(n+1) = F(2k+2) =  F(2k) +  F(2k+1) */
        } else {
            f[0] = k1; /* F(n) = F(2k) */
            f[1] = k2; /* F(n+1) = F(2k+1) */
        }
    }
    return f[0];
}

static int fib_open(struct inode *inode, struct file *file)
{
    if (!mutex_trylock(&fib_mutex)) {
        printk(KERN_ALERT "fibdrv is in use");
        return -EBUSY;
    }
    return 0;
}

static int fib_release(struct inode *inode, struct file *file)
{
    mutex_unlock(&fib_mutex);
    return 0;
}

/* calculate the fibonacci number at given offset */
static ssize_t fib_read(struct file *file,
                        char *buf,
                        size_t size,
                        loff_t *offset)
{
    bn *fib = bn_alloc(1);
    if (1)
        bn_fib_fdoubling(fib, *offset);
    else
        bn_fib(fib, *offset);
    char *p = bn_to_string(fib);
    size_t len = strlen(p) + 1;
    size_t left = copy_to_user(buf, p, len);

    bn_free(fib);
    kfree(p);

    return left;
}

__attribute__((always_inline)) static inline void escape(void *p)
{
    __asm__ volatile("" : : "g"(p) : "memory");
}

/* write operation is skipped */
static ssize_t fib_write(struct file *file,
                         const char *buf,
                         size_t size,
                         loff_t *offset)
{
    ktime_t kt;
    long long result;
    switch (size) {
    case 0:
        kt = ktime_get();
        result = fib_sequence(*offset);
        kt = ktime_sub(ktime_get(), kt);
        escape(&result);
        break;
    case 1:
        kt = ktime_get();
        result = fib_sequence_fdouble(*offset);
        kt = ktime_sub(ktime_get(), kt);
        escape(&result);
        break;
    default:
        return 0;
    }
    return (ssize_t) ktime_to_ns(kt);
}

static loff_t fib_device_lseek(struct file *file, loff_t offset, int orig)
{
    loff_t new_pos = 0;
    switch (orig) {
    case 0: /* SEEK_SET: */
        new_pos = offset;
        break;
    case 1: /* SEEK_CUR: */
        new_pos = file->f_pos + offset;
        break;
    case 2: /* SEEK_END: */
        new_pos = MAX_LENGTH - offset;
        break;
    }

    if (new_pos > MAX_LENGTH)
        new_pos = MAX_LENGTH;  // max case
    if (new_pos < 0)
        new_pos = 0;        // min case
    file->f_pos = new_pos;  // This is what we'll use now
    return new_pos;
}

const struct file_operations fib_fops = {
    .owner = THIS_MODULE,
    .read = fib_read,
    .write = fib_write,
    .open = fib_open,
    .release = fib_release,
    .llseek = fib_device_lseek,
};

static int __init init_fib_dev(void)
{
    int rc = 0;

    mutex_init(&fib_mutex);

    // Let's register the device
    // This will dynamically allocate the major number
    rc = alloc_chrdev_region(&fib_dev, 0, 1, DEV_FIBONACCI_NAME);

    if (rc < 0) {
        printk(KERN_ALERT
               "Failed to register the fibonacci char device. rc = %i",
               rc);
        return rc;
    }

    fib_cdev = cdev_alloc();
    if (fib_cdev == NULL) {
        printk(KERN_ALERT "Failed to alloc cdev");
        rc = -1;
        goto failed_cdev;
    }
    fib_cdev->ops = &fib_fops;
    rc = cdev_add(fib_cdev, fib_dev, 1);

    if (rc < 0) {
        printk(KERN_ALERT "Failed to add cdev");
        rc = -2;
        goto failed_cdev;
    }

    fib_class = class_create(THIS_MODULE, DEV_FIBONACCI_NAME);

    if (!fib_class) {
        printk(KERN_ALERT "Failed to create device class");
        rc = -3;
        goto failed_class_create;
    }

    if (!device_create(fib_class, NULL, fib_dev, NULL, DEV_FIBONACCI_NAME)) {
        printk(KERN_ALERT "Failed to create device");
        rc = -4;
        goto failed_device_create;
    }
    return rc;
failed_device_create:
    class_destroy(fib_class);
failed_class_create:
    cdev_del(fib_cdev);
failed_cdev:
    unregister_chrdev_region(fib_dev, 1);
    return rc;
}

static void __exit exit_fib_dev(void)
{
    mutex_destroy(&fib_mutex);
    device_destroy(fib_class, fib_dev);
    class_destroy(fib_class);
    cdev_del(fib_cdev);
    unregister_chrdev_region(fib_dev, 1);
}

module_init(init_fib_dev);
module_exit(exit_fib_dev);
