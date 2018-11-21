#ifndef __SELF_TYPE_H__
#define __SELF_TYPE_H__
#include "stdio.h"
#include  <os.h>

#define M_TRUE       1
#define M_FALSE      0

#define DEBUG_INFO
#define DEBUG_ERROR

#ifdef DEBUG_INFO
#define DBG_INFO(FORMAT,...)  \
        do{ \
          OS_CRITICAL_ENTER(); \
          printf(FORMAT,##__VA_ARGS__); \
          OS_CRITICAL_EXIT(); \
        }while(0)
#else
#define DBG_INFO(FORMAT,...)
#endif

#ifdef DEBUG_ERROR
#define DBG_ERROR(FORMAT,...)  \
        do{ \
          OS_CRITICAL_ENTER(); \
          printf(FORMAT,##__VA_ARGS__); \
          OS_CRITICAL_EXIT(); \
        }while(0)
#else
#define DBG_ERROR(FORMAT,...)
#endif

#endif
