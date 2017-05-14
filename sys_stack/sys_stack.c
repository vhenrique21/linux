#include <linux/linkage.h>

asmlinkage long sys_stack_push(int value)
{
    return value + 10;
}

asmlinkage long sys_stack_pop(void)
{
    return 0;
}
