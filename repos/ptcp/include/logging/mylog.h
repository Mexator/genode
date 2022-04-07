#ifndef _PTCP_MYLOG_H_
#define _PTCP_MYLOG_H_

#define debug_log(flag, args ...)       \
    do {                                \
        if(flag) Genode::log(args);     \
    } while(0)

#endif //_PTCP_MYLOG_H_
