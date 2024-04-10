/* s826.c
 * Sensoray Linux 826 Driver
 * Author(s): Dean Anderson
 *
 *  Copyright (C) 2012-2014 Sensoray Co., Inc.  This program is free software;
 *  you can redistribute it and/or modify it under the terms of the GNU
 *  General Public License as published by the Free Software Foundation;
 *  either version 2 of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 18)
#include <linux/config.h>
#endif
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/pci.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/pagemap.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 17)
#include <linux/uaccess.h>
#include <linux/io.h>
#endif
#include <linux/irq.h>
#include <linux/spinlock.h>
#include <linux/moduleparam.h>
#include <linux/kthread.h>
#include "s826ioctl.h"
#include "826const.h"
#include "826api.h"

MODULE_LICENSE("GPL");

#define S826_MAJOR_VERSION	1
#define S826_MINOR_VERSION	0
#define S826_RELEASE		6
#define S826_VERSION		KERNEL_VERSION(S826_MAJOR_VERSION,	\
					       S826_MINOR_VERSION,	\
					       S826_RELEASE)
#define S826_VERSION_TEXT "Sensoray 826 driver, version 1.0.6"

/* for backward compatibility */
#define KERNEL_VERSION_IOCTL_REMOVED       KERNEL_VERSION(2, 6, 36)
#define KERNEL_VERSION_MUTEX_ADDED         KERNEL_VERSION(2, 6, 16)
#if LINUX_VERSION_CODE >= KERNEL_VERSION_MUTEX_ADDED
#define s826_mutex_init     mutex_init
#define s826_mutex_lock     mutex_lock
#define s826_mutex_lock_interruptible mutex_lock_interruptible
#define s826_mutex_unlock   mutex_unlock
#else
#define s826_mutex_init     init_MUTEX
#define s826_mutex_lock     down
#define s826_mutex_lock_interruptible     down_interruptible
#define s826_mutex_unlock   up
#endif

static int debug;
static int *s826_debug = &debug;
static unsigned int s826_major = -1;
static struct class *s826_class;
static const struct file_operations s826_fops;

module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "Debug level(0-100) default 0");

#define dprintk(level, fmt, arg...)                 \
	do {						    \
		if (*s826_debug >= (level)) {			\
			printk(KERN_DEBUG "s826: " fmt, ##arg);	\
		}						\
	} while (0)


static const struct pci_device_id s826_tbl[] = {
	{ 0x10b5, 0x9056, 0x6000, 0x0826},
	{ 0, }
};

/* The 826s BARs we care about are 0(PCI) and 2 (FPGA) */
/* we have an array of 2 so BAR2 is at index 1 */

#define MAX_BARS 2

/*
 * Index in mmio array.  FPGA is physically the 3rd BAR in the system.
 * (The 2nd BAR is a port, not a memory address.)
 */
#define IDX_BAR_PCICFG   0
#define IDX_BAR_FPGA     1

struct s826_dev {
	struct pci_dev *pdev;
	struct cdev *char_device;
	struct device *device;
	__u64 bar[MAX_BARS];
	int bar_size[MAX_BARS];
	void *mmio[MAX_BARS];
	int mmio_size[MAX_BARS];
	int devid;
	int users;
	int remove;
#if LINUX_VERSION_CODE >= KERNEL_VERSION_MUTEX_ADDED
	struct mutex open_lock;
#else
	struct semaphore open_lock;
#endif
	int last_int_val;

	wait_queue_head_t wq_wd; /* watchdog (WD) interrupt wait queue */
	wait_queue_head_t wq_ct; /* count interrupt wait queue */
	wait_queue_head_t wq_adc; /* adc wait queue */
	wait_queue_head_t wq_dio; /* dio wait queue */

	int int_ready;

	int val_wd;     /* wd status value (from interrupt) */
	int val_ct;     /* counter status value (from interrupt) */
	int val_adc;    /* adc value (from interrupt) */
	int val_dio[2]; /* dio value (from interrupt) */

	int cancel_wd;     /* cancel watchdog (WD) */
	int cancel_ct;     /* cancel counter */
	int cancel_adc;    /* cancel adc */
	int cancel_dio[2]; /* cancel dio */

	spinlock_t        slock;
};

struct s826_fh {
	struct s826_dev *dev;
};

#define MAX_826_DEVICES 256
struct s826_dev *s826_device[MAX_826_DEVICES];

MODULE_DEVICE_TABLE(pci, s826_tbl);
static int numdev;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 20)
irqreturn_t s826_interrupt(int irq, void *param, struct pt_regs *regs)
#else
irqreturn_t s826_interrupt(int irq, void *param)
#endif
{
	struct s826_dev *dev = (struct s826_dev *) param;
	int handled = 0;
	unsigned int sts;
	unsigned int ie;
	unsigned int val_ct;
	unsigned int val_wd;
	unsigned int sts_adc;
	unsigned int wake_adc = 0;
	unsigned int wake_dio = 0;
	unsigned int wake_ct = 0;
	unsigned int wake_wd = 0;
	int i;
	unsigned long flags;
	if (dev->remove)
		return IRQ_RETVAL(handled);
	ie = readl((unsigned char *)dev->mmio[1] + S826_ADR_INTEN*4);
	sts = readl(((unsigned char *)dev->mmio[1] + S826_ADR_INTSTS*4));
	val_ct = ie & sts & S826_MASK_INTEN_CT;
	val_wd = ie & sts & S826_MASK_INTEN_WD;

	if (!sts)
		return IRQ_RETVAL(handled);

	if (val_wd) {
		writel((0x40000000UL | val_wd),
		       (unsigned char *)dev->mmio[1] + S826_ADR_INTEN*4);
		spin_lock_irqsave(&dev->slock, flags);
		dev->val_wd |= val_wd;
		spin_unlock_irqrestore(&dev->slock, flags);
		wake_wd = 1;
		dprintk(8, "counter interrupt\n");
	}
	if (val_ct) {
		/* counter interrupt, clear it */
		writel((0x40000000UL | val_ct),
		       (unsigned char *)dev->mmio[1] + S826_ADR_INTEN*4);
		spin_lock_irqsave(&dev->slock, flags);
		dev->val_ct |= val_ct;
		spin_unlock_irqrestore(&dev->slock, flags);
		wake_ct = 1;
		dprintk(8, "counter interrupt\n");
	}
	/* check for DIO interrupt. */
	if (sts & S826_MASK_INTSTS_DIO) {
		int j;
		unsigned int sts_dio[2];
		unsigned int val_dio[2];
		unsigned int ie_dio[2];
		/* leave DIO interrupt enabled */
		/* capture status */
		sts_dio[0] = readl((unsigned char *) dev->mmio[1] +
				   S826_ADR_GPSTSL*4);
		sts_dio[1] = readl((unsigned char *) dev->mmio[1] +
				   S826_ADR_GPSTSH*4);
		ie_dio[0] = readl((unsigned char *) dev->mmio[1] +
				  S826_ADR_CAPIENL*4);
		ie_dio[1] = readl((unsigned char *) dev->mmio[1] +
				  S826_ADR_CAPIENH*4);
		val_dio[0] = sts_dio[0] & ie_dio[0];
		val_dio[1] = sts_dio[1] & ie_dio[1];

		dprintk(8, "cap STS dio[0]: %x\n", sts_dio[0]);
		dprintk(8, "cap STS dio[1]: %x\n", sts_dio[1]);
		dprintk(8, "ie STS dio[0]: %x\n", ie_dio[0]);
		dprintk(8, "ie STS dio[1]: %x\n", ie_dio[1]);

		/* disable the interrupt */
		writel((val_dio[0] | 0x40000000UL),
		       ((unsigned char *) dev->mmio[1] + S826_ADR_CAPIENL*4));
		writel((val_dio[0] | 0x40000000UL),
		       ((unsigned char *) dev->mmio[1] + S826_ADR_CAPEL*4));
		writel((val_dio[1] | 0x40000000UL),
		       ((unsigned char *) dev->mmio[1] + S826_ADR_CAPIENH*4));
		writel((val_dio[1] | 0x40000000UL),
		       ((unsigned char *) dev->mmio[1] + S826_ADR_CAPEH*4));
		/* re-enable it */
		writel((val_dio[0] | 0x80000000UL),
		       ((unsigned char *) dev->mmio[1] + S826_ADR_CAPEL*4));
		writel((val_dio[1] | 0x80000000UL),
		       ((unsigned char *) dev->mmio[1] + S826_ADR_CAPEH*4));

		spin_lock_irqsave(&dev->slock, flags);
		dev->val_dio[0] |= val_dio[0];
		dev->val_dio[1] |= val_dio[1];
		spin_unlock_irqrestore(&dev->slock, flags);
		wake_dio = 1;
	}
	if (sts & S826_MASK_INTSTS_ADC) {
		int j;
		unsigned int val_adc;
		unsigned int ie_adc;
		/* capture status */
		sts_adc = readl((unsigned char *) dev->mmio[1] +
				S826_ADR_ADSTS*4);
		ie_adc = readl((unsigned char *) dev->mmio[1] +
			       S826_ADR_ADINTEN*4);
		val_adc = sts_adc & ie_adc;
		dprintk(8, "ADC STS: %x\n", sts_adc);
		dprintk(8, "ADC IE: %x\n", ie_adc);
		wake_adc = 1;
		/* clear the interrupt */
		writel((val_adc | 0x40000000UL),
		       ((unsigned char *) dev->mmio[1] + S826_ADR_ADINTEN*4));

		spin_lock_irqsave(&dev->slock, flags);
		dev->val_adc |= val_adc;
		spin_unlock_irqrestore(&dev->slock, flags);
	}
	if (wake_wd)
		wake_up(&dev->wq_wd);
	if (wake_ct)
		wake_up(&dev->wq_ct);
	if (wake_dio)
		wake_up(&dev->wq_dio);
	if (wake_adc)
		wake_up(&dev->wq_adc);

	handled = 1;
	return IRQ_RETVAL(handled);
}



static int s826_probe(struct pci_dev *pdev,
				const struct pci_device_id *ent)
{
	int rc;
	struct s826_dev *dev;
	int major, minor;
	int i;
	u32 d;

	printk("%s", __func__);
	dev = kzalloc(sizeof(struct s826_dev), GFP_KERNEL);
	if (dev == NULL) {
		printk(KERN_ERR "[s826]: out of memory!\n");
		rc = -ENOMEM;
		goto err_free;
	}
	init_waitqueue_head(&dev->wq_wd);
	init_waitqueue_head(&dev->wq_ct);
	init_waitqueue_head(&dev->wq_adc);
	init_waitqueue_head(&dev->wq_dio);
	spin_lock_init(&dev->slock);
	s826_mutex_init(&dev->open_lock);
	rc = pci_enable_device(pdev);
	if (rc) {
		printk(KERN_ERR "[s826]: pci enable error!\n");
		goto err_free;
	}

	pci_set_master(pdev);
	pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
	/* Interested only in BAR0 and BAR2 */
	for (i = 0; i < MAX_BARS; i++) {
		dev->bar[i] = pci_resource_start(pdev, i ? 2 : 0);
		dev->bar_size[i] = pci_resource_len(pdev, i ? 2 : 0);
		if (!request_mem_region(dev->bar[i], dev->bar_size[i], "s826")) {
			rc = -EBUSY;
			printk(KERN_ERR "[s826] BAR 0x%llx size %d %d busy\n",
			       dev->bar[i], dev->bar_size[i], i);
			dev->bar[i] = 0;
			dev->bar_size[i] = 0;
			goto err_free_bars;
		}
		dev->mmio[i] = ioremap(dev->bar[i], dev->bar_size[i]);
		if (dev->mmio[i] == NULL) {
			rc = -EIO;
			printk(KERN_ERR "[s826] could not map BAR\n");
			goto err_free_bars;
		}
		dev->mmio_size[i] = dev->bar_size[i];
	}
	/* enable local address space 0 */
	d = readl((unsigned char *)dev->mmio[0] + 4);
	d |= 1;
	writel(d, (unsigned char *)dev->mmio[0] + 4);

	/* enable interrupts */
	d = readl((unsigned char *)dev->mmio[0] + 0x68);
	d |= 0x0900;
	writel(d, (unsigned char *)dev->mmio[0] + 0x68);
	d = readl((unsigned char *)dev->mmio[0] + 0x68);
	writel(0, ((unsigned char *) dev->mmio[1] + S826_ADR_CAPIENL * 4));
	writel(0, ((unsigned char *) dev->mmio[1] + S826_ADR_CAPIENH * 4));
	/* enable FPGA board interrupt */
	writel(0x80000000UL | S826_MASK_INTEN_BD,
	       ((unsigned char *) dev->mmio[1] + S826_ADR_INTEN * 4));
#ifdef S826_DEBUG_VERBOSE
	printk(KERN_DEBUG "device ID: %x\n", readl(dev->mmio[1]));
#endif
	dev->pdev = pdev;
	minor = numdev;
	dev->devid = minor;
	pci_set_drvdata(pdev, dev);
	s826_device[minor] = dev;
	dev->char_device = cdev_alloc();
	if (!dev->char_device) {
		s826_device[minor] = NULL;
		printk(KERN_ERR "[s826]: cdev alloc error!\n");
		goto err_disable_dev;
	}
	cdev_init(dev->char_device, &s826_fops);
	major = s826_major;
	dev->char_device->dev = MKDEV(major, minor);
	dev->char_device->owner = s826_fops.owner;
	rc = cdev_add(dev->char_device, dev->char_device->dev, 1);
	if (rc) {
		s826_device[minor] = NULL;
		printk(KERN_ERR "[s826]: cdev add error!\n");
		goto err_free_cdev;
	}
	dev->device = device_create(s826_class, NULL, dev->char_device->dev,
				    NULL, "%s%d", "s826_", numdev);
	if (IS_ERR(dev->device)) {
		rc = PTR_ERR(dev->device);
		s826_device[minor] = NULL;
		dprintk(4, "[s826] failed to create device\n");
		goto err_free_cdev;
	}
	/* register interrupt handler */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 18)
	if (request_irq(pdev->irq, s826_interrupt, SA_INTERRUPT | SA_SHIRQ,
			"s826", dev)) {
		/* Report the successful installation.*/
		printk(KERN_INFO "s826 interrupt %d assignment failed\n",
		       pdev->irq);
	} else {
		printk(KERN_INFO "s826 interrupt %d installed\n",
		       pdev->irq);
	}
#else
	if (request_irq(pdev->irq, s826_interrupt, IRQF_SHARED, "s826", dev)) {
		/* Report the successful installation.*/
		printk(KERN_INFO "s826 interrupt %d assignment failed\n",
		       pdev->irq);
	} else {
		printk(KERN_INFO "s826 interrupt %d installed\n",
		       pdev->irq);
	}
#endif
	numdev++;
	printk(KERN_INFO S826_VERSION_TEXT " successfully loaded\n");
	return 0;
err_free_cdev:
	cdev_del(dev->char_device);
err_disable_dev:
	pci_disable_device(pdev);
err_free_bars:
	for (i = 0; i < MAX_BARS; i++) {
		if (dev->mmio[i]) {
			iounmap((void *)dev->mmio[i]);
			dev->mmio[i] = NULL;
		}
		if (dev->bar[i]) {
			release_mem_region(dev->bar[i], dev->bar_size[i]);
			dev->bar[i] = 0;
		}
	}
err_free:
	printk(KERN_ERR "[s826]: driver load failed\n");
	kfree(dev);
	return rc;
}

static void s826_remove(struct pci_dev *pdev)
{
	struct s826_dev *dev = pci_get_drvdata(pdev);
	int i;
	int d;
	dev->remove = 1;
	if (dev) {
		/* disable interrupts */
		d = readl((unsigned char *)dev->mmio[0] + 0x68);
		d &= ~0x0900;
		writel(d, (unsigned char *)dev->mmio[0] + 0x68);
		free_irq(pdev->irq, dev);
		for (i = 0; i < MAX_BARS; i++) {
			if (dev->mmio[i]) {
				iounmap((void *)dev->mmio[i]);
				dev->mmio[i] = NULL;
			}
			if (dev->bar[i]) {
				release_mem_region(dev->bar[i],
						   dev->bar_size[i]);
				dev->bar[i] = 0;
			}
		}
		if (dev->char_device) {
			device_destroy(s826_class, dev->char_device->dev);
			cdev_del(dev->char_device);
		}

		kfree(dev);
	}
	pci_disable_device(pdev);
	numdev--;
}

static struct pci_driver s826_driver = {
	.name = "Sensoray 826",
	.id_table = s826_tbl,
	.probe = s826_probe,
	.remove = s826_remove,
};

struct device *s826_dev;

static int __init s826_init(void)
{
	int rc;
	printk(KERN_INFO "s826 init driver\n");
	s826_major = register_chrdev(0, "s826", &s826_fops);
	if (s826_major < 0)
		return s826_major;
	s826_class = class_create(THIS_MODULE, "s826");
	if (IS_ERR(s826_class)) {
		rc = PTR_ERR(s826_class);
		goto fail_unregister_chrdev;
	}
	rc = pci_register_driver(&s826_driver);
	if (rc)
		goto fail_class_destroy;
	return 0;
fail_unregister_driver:
	pci_unregister_driver(&s826_driver);
fail_class_destroy:
	class_destroy(s826_class);
fail_unregister_chrdev:
	unregister_chrdev(s826_major, "s826");
	return rc;
}

static void __exit s826_exit(void)
{
	unregister_chrdev(s826_major, "s826");
	pci_unregister_driver(&s826_driver);
	class_destroy(s826_class);
	printk(KERN_INFO "[s826] driver unloaded\n");
}

struct s826_dev *s826_devdata(struct file *file)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 20)
	return s826_device[iminor(file->f_dentry->d_inode)];
#else
	return s826_device[iminor(file->f_path.dentry->d_inode)];
#endif
}

static int s826_open(struct inode *inode, struct file *file)
{
	struct s826_dev *dev = s826_devdata(file);
	struct s826_fh *fh = NULL;

	dprintk(2, "device %p\n", dev);
	s826_mutex_lock(&dev->open_lock);
	fh = kmalloc(sizeof(struct s826_fh), GFP_KERNEL);
	if (fh == NULL) {
		s826_mutex_unlock(&dev->open_lock);
		return -ENOMEM;
	}
	fh->dev = dev;
	file->private_data = fh;
	dev->users++;
	s826_mutex_unlock(&dev->open_lock);
	dprintk(4, "open done\n");
	return 0;
}

static int s826_release(struct inode *inode, struct file *file)
{
	struct s826_dev *dev;
	struct s826_fh *fh = file->private_data;

	dev = fh->dev;
	s826_mutex_lock(&dev->open_lock);
	kfree(fh);
	dev->users--;
	s826_mutex_unlock(&dev->open_lock);
	return 0;
}

static int s826_ioctl(struct inode *inode, struct file *file,
		      unsigned int cmd, unsigned long arg);

static long s826_ioctl_unlocked(struct file *file,
				unsigned int cmd, unsigned long arg)
{
	int rc;

	dprintk(4, "unlocked ioctl cmd 0x%x, arg 0x%lx\n", cmd, arg);
	rc = s826_ioctl(NULL, file, cmd, arg);
	return rc;
}

static long s826_ioctl_compat(struct file *file,
			       unsigned int cmd, unsigned long arg)
{
	long rc;

	dprintk(4, "compat ioctl cmd 0x%x, arg 0x%lx\n", cmd, arg);
	rc = s826_ioctl(NULL, file, cmd, arg);
	return rc;
}

static int s826_ioctl(struct inode *inode, struct file *file,
		      unsigned int cmd, unsigned long arg)
{
	struct s826_fh *fh = (struct s826_fh *) file->private_data;
	struct s826_dev *dev = (struct s826_dev *) fh->dev;
	void __user *argp = (void __user *)arg;
	int rc;

	dprintk(4, "ioctl cmd 0x%x, arg 0x%lx\n", cmd, arg);
	switch (cmd) {
	case S826_IOC_VERSION:
	{
		struct s826_version p;

		p.version = S826_MAJOR_VERSION << 24;
		p.version |= S826_MINOR_VERSION << 16;
		p.version |= S826_RELEASE;
		rc = copy_to_user(argp, &p, sizeof(struct s826_version));
		break;
	}
	case S826_IOC_GETMEM:
	{
		struct s826_getmem p;

		if (copy_from_user(&p, (void __user *) arg,
				   sizeof(struct s826_getmem))) {
			printk(KERN_DEBUG "invalid copy\n");
			return -EINVAL;
		}
		/* check if offset in value range */
		if (p.idx < 0 || p.idx > 1) {
			printk(KERN_DEBUG "invalid index %d\n", p.idx);
			return -EINVAL;
		}
		p.bar = dev->bar[p.idx];
		dprintk(4, "GETMEM 0x%x : 0x%llx\n", p.idx, p.bar);
		rc = copy_to_user(argp, &p, sizeof(struct s826_getmem));
	}
	break;
	case S826_IOC_READREG:
	{
		struct s826_regparm p;

		if (copy_from_user(&p, (void __user *) arg,
				   sizeof(struct s826_regparm))) {
			printk(KERN_DEBUG "invalid copy\n");
			return -EINVAL;
		}
		/* check if offset in value range */
		if (p.off < 0 || p.off > 4096) {
			printk(KERN_DEBUG "invalid offset\n");
			return -EINVAL;
		}
		p.val = readl((unsigned int *)dev->mmio[IDX_BAR_FPGA] + p.off);
		dprintk(4, "READREG 0x%x : 0x%x\n", p.off, p.val);
		rc = copy_to_user(argp, &p, sizeof(struct s826_regparm));
	}
	break;
	case S826_IOC_WRITEREG:
	{
		struct s826_regparm p;

		if (copy_from_user(&p, (void __user *) arg,
				   sizeof(struct s826_regparm)))
			return -EINVAL;

		/* check if offset in value range */
		if (p.off < 0 || p.off > 4096) {
			printk(KERN_DEBUG "invalid offset\n");
			return -EINVAL;
		}
		dprintk(4, "WRITEREG 0x%x : 0x%x\n", p.off, p.val);
		writel(p.val, (unsigned int *)dev->mmio[IDX_BAR_FPGA] + p.off);
	}
	return 0;
	case S826_IOC_WAITWD:
	{
		struct s826_waitwd p;
		int rc;
		int int_ready;
		int val;
		unsigned long flags;
		u32 jiffies;

		if (copy_from_user(&p, (void __user *) arg,
				   sizeof(struct s826_waitwd)))
			return -EINVAL;

		dprintk(4, "waitwd %d usecs\n", p.usecs);
		dev->cancel_wd = 0;
		writel((0x80000000UL | 0x00000100UL),
		       ((unsigned char *) dev->mmio[1] + S826_ADR_INTEN*4));

#define WD_WAIT (dev->val_wd || dev->cancel_wd)
		if (p.usecs > 0) {
			jiffies = usecs_to_jiffies(p.usecs);
			/* miminum wait time */
			if (!jiffies)
				jiffies++;
			rc = wait_event_interruptible_timeout(dev->wq_wd,
							      WD_WAIT,
							      jiffies);
		} else {
			rc = wait_event_interruptible(dev->wq_wd,
						      WD_WAIT);
		}
		/* disable interrupt */
		writel((0x40000000UL | 0x00000100UL),
		       ((unsigned char *) dev->mmio[1] + S826_ADR_INTEN*4));

		spin_lock_irqsave(&dev->slock, flags);
		/* clear events */
		val = dev->val_wd;
		dev->val_wd = 0;
		spin_unlock_irqrestore(&dev->slock, flags);

		if (dev->cancel_wd) {
			dprintk(4, "S826_IOC_WAITWD cancelled\n");
			/* was cancelled */
			p.rc = S826_ERR_CANCELLED;
			/* clear cancel */
		} else if (val) {
			p.rc = 0;
		} else {
			/* timeout */
			p.rc = S826_ERR_NOTREADY;
		}
		dev->cancel_wd = 0;
		return copy_to_user(argp, &p, sizeof(struct s826_waitwd));
	}
	case S826_IOC_WAITCOUNT:
	{
		struct s826_waitcount p;
		int rc;
		int int_ready;
		int mask_orig;
		unsigned long flags;
		u32 jiffies;

		if (copy_from_user(&p, (void __user *) arg,
				   sizeof(struct s826_waitcount)))
			return -EINVAL;
		dprintk(4, "waitcount %d usecs\n", p.usecs);
		/* set interrupt enable mask */
		mask_orig = p.mask;
		dev->cancel_ct &= ~mask_orig;
		writel((0x80000000UL | mask_orig),
		       ((unsigned char *) dev->mmio[1] + S826_ADR_INTEN*4));

#define COUNT_WAITANY ((p.mask & dev->val_ct) || (p.mask & dev->cancel_ct))
#define COUNT_WAITALL (((p.mask & dev->val_ct) == p.mask) ||	\
		       (p.mask & dev->cancel_ct))

		if (p.usecs > 0) {
			jiffies = usecs_to_jiffies(p.usecs);
			/* miminum wait time */
			if (!jiffies)
				jiffies++;
			rc = wait_event_interruptible_timeout(dev->wq_ct,
							      p.bWaitAny ?
							      COUNT_WAITANY :
							      COUNT_WAITALL,
							      jiffies);
		} else {
			rc = wait_event_interruptible(dev->wq_ct,
						      p.bWaitAny ?
						      COUNT_WAITANY :
						      COUNT_WAITALL);
		}

		/* disable interrupt */
		writel((0x40000000UL | mask_orig),
		       ((unsigned char *) dev->mmio[1] + S826_ADR_INTEN*4));

		spin_lock_irqsave(&dev->slock, flags);
		/* save result */
		p.mask &= dev->val_ct;
		/* clear events */
		dev->val_ct &= ~mask_orig;
		spin_unlock_irqrestore(&dev->slock, flags);

		if (mask_orig & dev->cancel_ct) {
			dprintk(4, "S826_IOC_WAITCOUNT cancelled\n");
			/* was cancelled */
			p.rc = S826_ERR_CANCELLED;
			/* clear cancel */
		} else if (p.mask) {
			p.rc = 0;
		} else {
			/* timeout */
			p.rc = S826_ERR_NOTREADY;
		}
		dev->cancel_ct &= ~mask_orig;
		return copy_to_user(argp, &p, sizeof(struct s826_waitcount));
	}
	case S826_IOC_WAITDIO:
	{
		struct s826_waitdio p;
		int rc;
		int int_ready;
		int mask_orig[2];
		unsigned long flags;
		u32 jiffies;

		if (copy_from_user(&p, (void __user *) arg,
				   sizeof(struct s826_waitdio)))
			return -EINVAL;

		dprintk(4, "waitdio %d usecs\n", p.usecs);
		/* set interrupt enable mask */
		mask_orig[0] = p.mask[0];
		mask_orig[1] = p.mask[1];
		dev->cancel_dio[0] &= ~mask_orig[0];
		dev->cancel_dio[1] &= ~mask_orig[1];
		writel((0x80000000UL | mask_orig[0]),
		       ((unsigned char *) dev->mmio[1] + S826_ADR_CAPIENL*4));
		writel((0x80000000UL | mask_orig[1]),
		       ((unsigned char *) dev->mmio[1] + S826_ADR_CAPIENH*4));

#define DIO_WAITANY ((p.mask[0] & dev->val_dio[0]) ||       \
		     (p.mask[1] & dev->val_dio[1]) ||       \
		     (p.mask[0] & dev->cancel_dio[0]) ||    \
		     (p.mask[1] & dev->cancel_dio[1]))

#define DIO_WAITALL ((((p.mask[0] & dev->val_dio[0]) == p.mask[0]) &&  \
		      ((p.mask[1] & dev->val_dio[1]) == p.mask[1])) || \
		     (p.mask[0] & dev->cancel_dio[0]) ||               \
		     (p.mask[1] & dev->cancel_dio[1]))

		if (p.usecs > 0) {
			jiffies = usecs_to_jiffies(p.usecs);
			if (!jiffies)
				jiffies++;
			rc = wait_event_interruptible_timeout(dev->wq_dio,
							      p.bWaitAny ?
							      DIO_WAITANY :
							      DIO_WAITALL,
							      jiffies);
		} else {
			rc = wait_event_interruptible(dev->wq_dio,
						      p.bWaitAny ?
						      DIO_WAITANY :
						      DIO_WAITALL);
		}
		/* disable interrupt */
		writel((0x40000000UL | mask_orig[0]),
		       ((unsigned char *) dev->mmio[1] + S826_ADR_CAPIENL*4));
		writel((0x40000000UL | mask_orig[1]),
		       ((unsigned char *) dev->mmio[1] + S826_ADR_CAPIENH*4));
		/* save result */
		spin_lock_irqsave(&dev->slock, flags);
		p.mask[0] &= dev->val_dio[0];
		p.mask[1] &= dev->val_dio[1];
		/* clear events */
		dev->val_dio[0] &= ~mask_orig[0];
		dev->val_dio[1] &= ~mask_orig[1];
		spin_unlock_irqrestore(&dev->slock, flags);

		if ((mask_orig[0] & dev->cancel_dio[0]) ||
		    (mask_orig[1] & dev->cancel_dio[1])) {
			dprintk(4, "S826_IOC_WAITDIO cancelled\n");
			/* was cancelled */
			p.rc = S826_ERR_CANCELLED;
		} else if (p.mask[0] || p.mask[1]) {
			p.rc = 0;
		} else {
			p.rc = S826_ERR_NOTREADY;
		}
		dev->cancel_dio[0] &= ~mask_orig[0];
		dev->cancel_dio[1] &= ~mask_orig[1];
		return copy_to_user(argp, &p, sizeof(struct s826_waitdio));
	}
	case S826_IOC_WAITADC:
	{
		struct s826_waitadc p;
		int rc;
		int int_ready;
		int mask_orig;
		u32 jiffies;
		unsigned long flags;

		if (copy_from_user(&p, (void __user *) arg,
				   sizeof(struct s826_waitadc)))
			return -EINVAL;

		dprintk(4, "waitadc %d usecs\n", p.usecs);
		/* set interrupt enable mask */
		mask_orig = p.mask;
		dev->cancel_adc &= ~mask_orig;
		writel((0x80000000UL | mask_orig),
		       ((unsigned char *) dev->mmio[1] + S826_ADR_ADINTEN*4));

#define ADC_WAITANY ((p.mask & dev->val_adc) || (p.mask & dev->cancel_adc))
#define ADC_WAITALL (((p.mask & dev->val_adc) == p.mask) ||	\
		     (p.mask & dev->cancel_adc))

		if (p.usecs > 0) {
			jiffies = usecs_to_jiffies(p.usecs);
			if (!jiffies)
				jiffies++;
			rc = wait_event_interruptible_timeout(dev->wq_adc,
							      p.bWaitAny ?
							      ADC_WAITANY :
							      ADC_WAITALL,
							      jiffies);
		} else
			rc = wait_event_interruptible(dev->wq_adc,
						      p.bWaitAny ?
						      ADC_WAITANY :
						      ADC_WAITALL);

		/* disable interrupt */
		writel((0x40000000UL | mask_orig),
		       ((unsigned char *) dev->mmio[1] + S826_ADR_ADINTEN*4));
		/* save result */
		spin_lock_irqsave(&dev->slock, flags);
		p.mask &= dev->val_adc;
		/* clear events */
		dev->val_adc &= ~mask_orig;
		spin_unlock_irqrestore(&dev->slock, flags);

		if (mask_orig & dev->cancel_adc) {
			dprintk(4, "S826_IOC_WAITADC cancelled\n");
			/* was cancelled */
			p.rc = S826_ERR_CANCELLED;
		} else if (p.mask) {
			p.rc = 0;
		} else {
			p.rc = S826_ERR_NOTREADY;
		}
		dev->cancel_adc &= ~mask_orig;
		return copy_to_user(argp, &p, sizeof(struct s826_waitadc));
	}
	case S826_IOC_CANCEL_WAITWD:
	{
		struct s826_cancel_waitwd p;

		if (copy_from_user(&p, (void __user *) arg,
				   sizeof(struct s826_cancel_waitwd)))
			return -EINVAL;
		dprintk(4, "cancel_waitwd\n");
		dev->cancel_wd = 1;
		wake_up(&dev->wq_wd);
		return 0;
	}
	case S826_IOC_CANCEL_WAITCOUNT:
	{
		struct s826_cancel_waitcount p;

		if (copy_from_user(&p, (void __user *) arg,
				   sizeof(struct s826_cancel_waitcount)))
			return -EINVAL;
		dprintk(4, "cancel_waitcount mask %x\n", p.mask);
		dev->cancel_ct |= p.mask;
		wake_up(&dev->wq_ct);
		return 0;
	}
	case S826_IOC_CANCEL_WAITDIO:
	{
		struct s826_cancel_waitdio p;

		if (copy_from_user(&p, (void __user *) arg,
				   sizeof(struct s826_cancel_waitdio)))
			return -EINVAL;
		dprintk(4, "cancel_waitdio\n");
		dev->cancel_dio[0] |= p.mask[0];
		dev->cancel_dio[1] |= p.mask[1];
		wake_up(&dev->wq_dio);
		return 0;
	}
	case S826_IOC_CANCEL_WAITADC:
	{
		struct s826_cancel_waitadc p;

		if (copy_from_user(&p, (void __user *) arg,
				   sizeof(struct s826_cancel_waitadc)))
			return -EINVAL;
		dprintk(4, "cancel_waitadc mask %x\n", p.mask);
		dev->cancel_adc |= p.mask;
		wake_up(&dev->wq_adc);
		return 0;
	}
	default:
		printk(KERN_DEBUG "s826 invalid ioctl\n");
		return -EINVAL;
	}
	return 0;
}


static int s826_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct s826_fh *fh = (struct s826_fh *) file->private_data;
	struct s826_dev *dev = (struct s826_dev *) fh->dev;
	size_t size;
	void *addr;
	unsigned long boff;
	int j, i;
	int bFound = 0;
	int bStat = 0;
	struct s826_mapping *pmap;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)
	lock_kernel();
#endif
	addr = (void *) (vma->vm_pgoff << PAGE_SHIFT);
	boff = (unsigned long)  (vma->vm_pgoff << PAGE_SHIFT);
	size = vma->vm_end - vma->vm_start;
	dprintk(4, "mmap virt %p %lu, 0x%lx vm_pgoff 0x%lx\n", addr,
		(unsigned long) size,
		vma->vm_pgoff, vma->vm_pgoff << PAGE_SHIFT);

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	/* VM_RESERVED, now VM_DONTEXPAND | VM_DONTDUMP set by remap */
	if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, size,
			    vma->vm_page_prot))
		return -EAGAIN;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)
	unlock_kernel();
#endif
	dprintk(4, "mmap done\n");
	return 0;
}

static const struct file_operations s826_fops = {
	.owner   = THIS_MODULE,
	.open    = s826_open,
	.release = s826_release,
	.compat_ioctl = s826_ioctl_compat,
#if LINUX_VERSION_CODE < KERNEL_VERSION_IOCTL_REMOVED
	.ioctl =    s826_ioctl,
#else
	.unlocked_ioctl = s826_ioctl_unlocked,
#endif
	.mmap    = s826_mmap,
};

module_init(s826_init);
module_exit(s826_exit);
