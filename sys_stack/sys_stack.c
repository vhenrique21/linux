#include<linux/linkage.h>

asmlinkage long sys_insert(int value)
{
    return value + 10;
}

asmlinkage long sys_remove(void)
{
    return 0;
}
