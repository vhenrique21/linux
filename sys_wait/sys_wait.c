#include <linux/linkage.h>
#include <linux/wait.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched/signal.h>

static unsigned long count = 0;
static struct kobject *sys_wait_kobject;

static LIST_HEAD(queue);

struct queue_value {
	int value;
	struct list_head queue;
};



static int queue_push(int value)
{
	struct queue_value *new_queue_value;

	new_queue_value = kmalloc(sizeof(*new_queue_value), GFP_KERNEL);
	new_queue_value->value = value;

	list_add_tail(&new_queue_value->queue, &queue);
	count++;

	return 0;
}

static int queue_pop(void)
{
	int value;
	struct queue_value *top_queue_value;

	if (list_empty(&queue)) {
		pr_debug("EMPTY QUEUE\n");
		return -1;
	}

	top_queue_value = list_first_entry(&queue, struct queue_value, queue);
	value = top_queue_value->value;
	list_del(&top_queue_value->queue);
	kfree(top_queue_value);
	count--;

	return value;
}

static int queue_has(int value)
{
	struct list_head *pos;
	struct queue_value *current_queue_value;
	list_for_each(pos, &queue) {
		current_queue_value = list_entry(pos, struct queue_value, queue);
		if (current_queue_value->value == value)
			return 1;
	}
	return 0;
}



asmlinkage long sys_wait_lock(int process)
{
	struct pid *process_pid;
	struct task_struct *process_task;
	wait_queue_t process_wait;
	init_wait(&process_wait);

	process_pid = find_get_pid(process);

	process_task = pid_task(process_pid, PIDTYPE_PID);
	if (process_task == NULL)
	{
		printk("No such pid\n");
		return 1;
	} else if (queue_has(process)) {
		printk("pid already locked\n");
		return 1;
	}

	kill_pid(process_pid, SIGSTOP, 1);
	queue_push(process);

	return 0;
}

asmlinkage long sys_wait_unlock(void)
{
	int process;
	struct pid *process_pid;
	if (count) {
		process = queue_pop();
		process_pid = find_get_pid(process);
		kill_pid(process_pid, SIGCONT, 1);
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
