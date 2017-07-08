#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/slab.h>

static DECLARE_WAIT_QUEUE_HEAD(wq);
static LIST_HEAD(data_queue);

struct device_data {
	struct list_head head;
	char data;
};

static char get_data(void)
{
	struct device_data *entry;
	char c;

	entry = list_first_entry(&data_queue, struct device_data, head);
	c = entry->data;
	list_del(&entry->head);
	kfree(entry);
	return c;
}

static long put_data(char c)
{
	struct device_data *entry = kmalloc(sizeof(*entry), GFP_KERNEL);
	if (!entry) {
		pr_debug("Device queue data allocation failed\n");
		return -1;
	}
	entry->data = c;
	list_add_tail(&entry->head, &data_queue);
	return 0;
}

static ssize_t blocking_dev_read(struct file *filp, char __user *buffer,
	size_t length, loff_t *ppos)
{
	if (wait_event_interruptible(wq, !list_empty(&data_queue)))
		return -ERESTARTSYS;

	// FIXME Race condition if multiple readers
	char c = get_data();
	return simple_read_from_buffer(buffer, length, ppos, &c, 1);
}

asmlinkage long sys_write_device(int data)
{
	int rc = put_data(data);
	if (rc)
		return rc;
	wake_up_interruptible(&wq);
	return 0;
}

static const struct file_operations blocking_dev_fops = {
	.owner = THIS_MODULE,
	.read  = blocking_dev_read
};

static struct miscdevice id_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "blocking_dev",
	.fops = &blocking_dev_fops
};

static int __init blocking_dev_init(void)
{
	int retval;

	retval = misc_register(&id_misc_device);
	if (retval)
		pr_err("blocking_dev_dev: misc_register %d\n", retval);
	return retval;
}

static void __exit blocking_dev_exit(void)
{
	misc_deregister(&id_misc_device);
}

module_init(blocking_dev_init);
module_exit(blocking_dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tiago Koji Castro Shibata <tishi@linux.com>");
MODULE_DESCRIPTION("Blocking device sample");
