#include <stdio.h>

void func(int a)
{
    printf("0x%x\n",a);
}

int fib(int n)
{
    if(n == 1 || n == 0)
    {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

int main()
{
    func(fib(10));
}