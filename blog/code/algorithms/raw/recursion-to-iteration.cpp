


#include <stack>
using namespace std;

int fac(int n)
{
    if (n == 1)
    {
        return 1;
    }
    return n * fac(n - 1);
}

enum Rigister
{
    RIP_RETURN_FROM_CALL = 0xfff0,
    RIP_ENTER_CALL,
    RIP_LEAVE_FACTORIAL,
    RBP
};

int stack_fac(int n)
{
    stack<int> stack;
    int eax, edi, rip_status;
    edi = n;
    stack.push(RIP_LEAVE_FACTORIAL);
    rip_status = RIP_ENTER_CALL;
    goto fac;
    while (1)
    {
    fac:
        if (rip_status == RIP_ENTER_CALL)
        {
            stack.push(RBP);
            stack.push(edi);
            int& _0x4 = stack.top();
            if (_0x4 != 1)
            {
                eax = _0x4;
                eax -= 1;
                edi = eax;
                stack.push(RIP_RETURN_FROM_CALL);
                rip_status = RIP_ENTER_CALL;
                goto fac;
            }
            else
            {
                eax = 1;
                goto ret;
            }
        }
        else if (rip_status == RIP_RETURN_FROM_CALL)
        {
            goto imul;
        }
        else if (rip_status == RIP_LEAVE_FACTORIAL)
        {
            goto exit_fac;
        }
    imul:
        eax = eax * stack.top();
    ret:
        while (stack.top() != RBP)
        {
            stack.pop();
        }
        stack.pop();
        rip_status = stack.top();
        stack.pop();
    }
exit_fac:
    return eax;
}

#include <cassert>
#include <iostream>
int main()
{
    assert(fac(1)  == stack_fac(1));
    assert(fac(2)  == stack_fac(2));
    assert(fac(3)  == stack_fac(3));
    assert(fac(4)  == stack_fac(4));
    assert(fac(5)  == stack_fac(5));
    assert(fac(6)  == stack_fac(6));
    assert(fac(7)  == stack_fac(7));
    assert(fac(8)  == stack_fac(8));
    assert(fac(9)  == stack_fac(9));
    assert(fac(10) == stack_fac(10));
    assert(fac(11) == stack_fac(11));
    assert(fac(12) == stack_fac(12));
    assert(fac(13) == stack_fac(13));
    assert(fac(14) == stack_fac(14));
    assert(fac(15) == stack_fac(15));
}

