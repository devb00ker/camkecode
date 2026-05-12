#ifndef STRUCT_TYPEDEF_H
#define STRUCT_TYPEDEF_H

#include <stdint.h>

/* Keil → GCC 兼容宏 */
#ifndef __packed
  #ifdef __GNUC__
    #define __packed __attribute__((packed))
  #else
    #define __packed
  #endif
#endif

/*
 * intX_t / uintX_t 由工具链的 <stdint.h> 提供
 * 此处仅保留 Keil 工程常用的别名
 */
typedef unsigned char bool_t;
typedef float fp32;
typedef double fp64;

#endif