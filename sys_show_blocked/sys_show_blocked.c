#include <linux/linkage.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/wait.h>
#include <linux/sched.h>

wait_queue_head_t wq = __WAIT_QUEUE_HEAD_INITIALIZER(wq);
EXPORT_SYMBOL(wq);

static struct kobject *show_blocked_kobject;

void state_format(char *output)
{
	struct list_head *head = &wq.task_list;
		struct list_head *list;
		wait_queue_t *wait;
		struct task_struct *task;

		char task_structure[960] = "";
		list_for_each(list, head)
		{
			wait = list_entry(list, wait_queue_t, task_list);
			task = wait->private;

			char task_info[64] = "";
			sprintf(task_info, "%s[%d : %ld], ", task->comm, task->pid, task->state);
			strcat(task_structure, task_info);
		}
	
		sprintf(output, "--> Actual thread: %s[%d] - %ld -> Blocked: %s", current->comm, current->pid, current->state, task_structure);
}

static ssize_t blocked_show(struct kobject *kobj, struct kobj_attribute *attr,
							char *buf)
{

	char task_structure[1024] = "";
	state_format(task_structure);

	return sprintf(buf, "%s\n", task_structure);
}

static struct kobj_attribute blocked_attribute = __ATTR_RO(blocked);

static struct attribute *attrs[] = {
	&blocked_attribute.attr,
	NULL,
};

static int __init show_blocked_init(void)
{
	int retval;

	show_blocked_kobject = kobject_create_and_add("show_blocked", kernel_kobj);
	if (!show_blocked_kobject)
		pr_debug("show_blocked_dev: can't create kobject");

	retval = sysfs_create_file(show_blocked_kobject, *attrs);
	if (retval)
	{
		pr_debug("show_blocked_dev: can't create sysfs group");
		kobject_put(show_blocked_kobject);
		return retval;
	}
	return 0;
}

static void __exit show_blocked_exit(void)
{
	kobject_put(show_blocked_kobject);
}

module_init(show_blocked_init);
module_exit(show_blocked_exit);