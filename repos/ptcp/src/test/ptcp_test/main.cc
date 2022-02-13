#include <libc/component.h>
#include <stdio.h>

void Libc::Component::construct(Libc::Env &env)
{
    with_libc([&]()
              { puts("hello world"); });
}