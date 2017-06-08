#include <linux/linkage.h>
#include <linux/wait.h>
#include <linux/pid.h>
#include <linux/module.h>
#include <linux/slab.h>

static DECLARE_WAIT_QUEUE_HEAD(queue);

static unsigned long count = 0;
static struct kobject *sys_stack_kobject;

asmlinkage long sys_wait_lock(int process)
{
	struct pid* process_pid;
	struct task_struct* process_task;
	DEFINE_WAIT(process_wait);

	process_pid = find_get_pid(process);
	process_task = pid_task(process_pid, PIDTYPE_PID);
	init_waitqueue_entry(&process_wait, process_task);

	prepare_to_wait_exclusive(queue, &process_wait, TASK_INTERRUPTIBLE);
	count++;

	return 0;
}

asmlinkage long sys_wait_unlock(void)
{
	if (count) {
		count--;
		wake_up(queue);

		return 0;
	}
	return 1;
}

static ssize_t count_show(struct kobject *kobj, struct kobj_attribute *attr,
			    char *buf)
{
	return sprintf(buf, "%lu\n", count);
}

static struct kobj_attribute count_attribute = __ATTR_RO(count);

static struct attribute *attrs[] = {
	&count_attribute.attr,
	NULL,
};

static int __init sys_wait_init(void)
{
	int retval;

	sys_wait_kobject = kobject_create_and_add("sys_wait", kernel_kobj);
	if (!sys_wait_kobject)
		pr_debug("wait_dev: can't create kobject");

	retval = sysfs_create_file(sys_wait_kobject, *attrs);
	if (retval) {
		pr_debug("wait_dev: can't create sysfs group");
		kobject_put(sys_wait_kobject);
		return retval;
	}

	return 0;
}

static void __exit sys_wait_exit(void)
{
	kobject_put(sys_wait_kobject);
}

module_init(sys_wait_init);
module_exit(sys_wait_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Process locker and unlocker module");
