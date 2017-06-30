#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>

static ssize_t blocking_dev_read(struct file *filp, char __user *buffer,
	size_t length, loff_t *ppos)
{
	return simple_read_from_buffer(buffer, length, ppos, "a", 1);
}

static ssize_t blocking_dev_write(struct file *filp, const char __user *buffer,
	size_t length, loff_t *ppos)
{
	char module_buffer;
	int retval;

	if (length != 1)
		return -EINVAL;

	retval = simple_write_to_buffer(&module_buffer, 1, ppos, buffer, 1);
	if (!retval || module_buffer != 'a')
		return -EINVAL;
	return retval;
}

static const struct file_operations blocking_dev_fops = {
	.owner = THIS_MODULE,
	.read  = blocking_dev_read,
	.write = blocking_dev_write
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
