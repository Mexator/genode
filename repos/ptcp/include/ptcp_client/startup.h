#ifndef _PTCP_STARTUP_H_
#define _PTCP_STARTUP_H_

#include <base/env.h>
#include <base/allocator.h>

/**
 * PTCP startup function. Should be called within libc.
 */
void startup_callback(Genode::Env &env, Genode::Allocator &alloc);

#endif //_PTCP_STARTUP_H_
