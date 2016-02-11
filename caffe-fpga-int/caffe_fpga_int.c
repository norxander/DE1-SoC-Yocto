#include <linux/module.h> 
#include <linux/init.h> 
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/irq.h>
#include <linux/kdev_t.h> 
#include <linux/cdev.h> 
#include <linux/device.h>
#include <linux/kernel.h> 
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/spinlock_types.h>

/* Define IRQ number */
#define	ACCEL_FPGAHPS_INT_NUM	72

/* Define macros */
#define DEBUG_ENABLED   1

#if DEBUG_ENABLED
#   define DEBUG( flag, fmt, args... ) do { if ( gDebug ## flag ) printk( "%s: " fmt, __FUNCTION__ , ## args ); } while (0)
#else
#   define DEBUG( flag, fmt, args... )
#endif

#define ACCELERATOR_DEVICE_NAME "accelerator-fpga"

static int accelerator_fpga_open(struct inode * inodp, struct file * filp);
static int accelerator_fpga_fasync(int fd, struct file * filp, int mode);
static int accelerator_fpga_release(struct inode *inode, struct file *file);
static ssize_t accelerator_fpga_read(struct file *file, char *buffer, size_t spaceRemaining, loff_t *ppos);

static struct file_operations accelerator_fpga_fops = 
{
	owner:      THIS_MODULE,
	open:       accelerator_fpga_open,
	read:       accelerator_fpga_read,
	release:    accelerator_fpga_release,
	fasync:     accelerator_fpga_fasync
};

static int gDebugTrace = 0;
static int gDebugError = 1;

static dev_t accelerator_fpga_DevNum = 0;
static struct cdev *accelerator_fpga_Dev;
static struct fasync_struct * async = NULL;
static struct class *accelerator_fpga_Class = NULL;

static DEFINE_SPINLOCK(interrupt_flag_lock);

static int accelerator_fpga_open(struct inode * inodp, struct file * filp)
{ 
	return 0; 
}

static ssize_t accelerator_fpga_read(struct file *file, char *buffer, size_t spaceRemaining, loff_t *ppos)
{
	return 0;
}

static int accelerator_fpga_fasync(int fd, struct file * filp, int mode)
{ 
	return fasync_helper (fd, filp, mode, &async); 
}

static int accelerator_fpga_release(struct inode *inode, struct file *file)
{
    DEBUG( Trace, "accelerator_fpga_release called\n" );

    // remove this file from fasync notifcation queue
    accelerator_fpga_fasync(-1, file, 0);

    return 0;
}

static irqreturn_t accelerator_fpga_handler(int irq, void *dev_id)
{
    if (irq != ACCEL_FPGAHPS_INT_NUM)
		return IRQ_NONE;

    spin_lock(&interrupt_flag_lock);
    {
		if (async)
		{
			kill_fasync(&async, SIGIO, POLL_IN);
		}
    }
    spin_unlock(&interrupt_flag_lock);
	
    return IRQ_HANDLED;
}

/*
 * Initialize the module − register the IRQ handler
 */
static int __init accelerator_fpga_handler_init(void)
{
	int ret = 0;
	
	DEBUG(Trace, "called\n");
	
	if ( (ret = alloc_chrdev_region( &accelerator_fpga_DevNum, 0, 1, ACCELERATOR_DEVICE_NAME )) < 0 )
    {
        printk(KERN_WARNING "sample: Unable to allocate major, err: %d\n", ret);
        return ret;
    }
    
    DEBUG(Trace, "allocated major:%d minor:%d\n", MAJOR(accelerator_fpga_DevNum), MINOR(accelerator_fpga_DevNum));
	
	accelerator_fpga_Dev = cdev_alloc();
	
	if (!accelerator_fpga_Dev)
	{
		unregister_chrdev_region(accelerator_fpga_DevNum, 1);
		printk(KERN_WARNING "sample: Unable to allocate cdev, err: %d\n", ret);
		return -1;
	}
	
	// Register our device. The device becomes "active" as soon as cdev_add 
    // is called.
    cdev_init(accelerator_fpga_Dev, &accelerator_fpga_fops);
    accelerator_fpga_Dev->owner = THIS_MODULE;
	
	if ( (ret = cdev_add( accelerator_fpga_Dev, accelerator_fpga_DevNum, 1 )) != 0 )
    {
        printk(KERN_WARNING "sample: cdev_add failed: %d\n", ret);
        return ret;
    }
	
	// Create a class, so that udev will make the /dev entry
    accelerator_fpga_Class = class_create(THIS_MODULE, ACCELERATOR_DEVICE_NAME);
    if ( IS_ERR(accelerator_fpga_Class) )
    {
        printk(KERN_WARNING "sample: Unable to create class\n" );
        return -1;
    }

    device_create(accelerator_fpga_Class, NULL, accelerator_fpga_DevNum, NULL, ACCELERATOR_DEVICE_NAME);
	
	if ( ( ret = request_irq(ACCEL_FPGAHPS_INT_NUM, accelerator_fpga_handler, 0,
					  "accelerator_fpga_handler", (void *)(accelerator_fpga_handler)) ) != 0 )
	{
		DEBUG(Error, "Unable to register irq for Accelerator FPGA %d\n", ret);
		return ret;
	}
	
	return 0;
}

static void __exit accelerator_fpga_handler_exit(void)
{
	DEBUG( Trace, "called\n" );
	
	// Deregister our driver
    device_destroy(accelerator_fpga_Class, accelerator_fpga_DevNum);
    class_destroy(accelerator_fpga_Class);
	
	cdev_del(accelerator_fpga_Dev);
	
	unregister_chrdev_region(accelerator_fpga_DevNum, 1);
}

MODULE_LICENSE("GPL");
MODULE_VERSION("1.0"); 
MODULE_AUTHOR("Alexander Leiva <norxander@gmail.com>");
MODULE_DESCRIPTION("FPGA to HPS driver for Caffe Accelerator interrupt");

module_init(accelerator_fpga_handler_init);
module_exit(accelerator_fpga_handler_exit);
