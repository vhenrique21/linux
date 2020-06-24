#include <linux/linkage.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/sched/signal.h>
#include <linux/unistd.h>
#include <linux/list_sort.h>
#include <linux/string.h>

int compare(void *priv, struct list_head *a, struct list_head *b)
{
    struct task_struct *task_a = list_entry(a, struct task_struct, sibling);
    struct task_struct *task_b = list_entry(b, struct task_struct, sibling);

    u64 vruntime_a = task_a->se.vruntime;
    u64 vruntime_b = task_b->se.vruntime;

    if (vruntime_a > vruntime_b)
        return -1;
    else if (vruntime_a < vruntime_b)
        return 1;
    else
        return 0;
}

asmlinkage long sys_cfs(void)
{
    struct task_struct *current_task = current;
   
    struct list_head *head = &current_task->children;
    list_sort(NULL, head, compare);
    
    struct task_struct *task;
    struct list_head *list;

    list_for_each(list, head)
    {
        task = list_entry(list, struct task_struct, sibling);
        /* task now points to the current first child process */
        pr_info("%s[%d] - %llu\n", task->comm, task->pid, task->se.vruntime);
    }

    char task_structure[1024] = "";
    list_for_each(list, head)
    {
        task = list_entry(list, struct task_struct, sibling);
        char task_info[64]; 
        sprintf(task_info, "pid%d:%llu ", task->pid,  task->se.vruntime);
        strcat(task_structure, task_info);
    }

    pr_info("%s\n", task_structure);

    // struct task_struct *task2;

    // for_each_process(task2)
    // {
    //     /* this pointlessly prints the name and PID of each task2 */
    //     pr_info("BBB: %s[%d] - %llu\n", task2->comm, task2->pid, task2->se.vruntime);
    // }

    // struct task_struct *task;
    // struct list_head *list;
    // list = &current_task->children;

    // list_for_each(&list, &current->children) {
    //     task = list_entry(&list, struct task_struct, sibling);
    //     /* task now points to the current first child process */
    // }

        return 0;
}