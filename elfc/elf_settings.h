#ifndef __ELFC_ELF_SETTINGS_H__
#define __ELFC_ELF_SETTINGS_H__

/**************************************************************
 * Basic Types
 *
 * DO NOT CHANGE UNLESS YOU KNOW WAHT ARE YOU DOING!
 */

#if (__STDC_VERSION__ >= 199901L)

#include <stdint.h>

typedef int8_t ELF_i8;
typedef uint8_t ELF_u8;
typedef int16_t ELF_i16;
typedef uint16_t ELF_u16;
typedef int32_t ELF_i32;
typedef uint32_t ELF_u32;

#else

/* stm32 */
typedef char ELF_i8;
typedef unsigned char ELF_u8;
typedef short ELF_i16;
typedef unsigned short ELF_u16;
typedef int ELF_i32;
typedef unsigned int ELF_u32;

#endif

typedef ELF_u16 ELF_size;

/**************************************************************
 * Application Types
 */

typedef ELF_u16 ELF_Tick;
typedef ELF_i32 ELF_Event;

/**************************************************************
 * Max Limits
 */

#define ELF_MAX_TIMERS 10
#define ELF_MAX_LISTENERS 10
#define ELF_MAX_TASKS (ELF_MAX_TIMERS + ELF_MAX_LISTENERS)

#endif
