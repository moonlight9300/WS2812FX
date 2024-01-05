#ifndef _DEFINES_H_
#define _DEFINES_H_

#include <stddef.h>

#define NULL 0

#define true  1
#define false 0

typedef unsigned char   		u8, bool, BOOL;
typedef char            		s8;
typedef unsigned short  		u16;
typedef signed short    		s16;
typedef unsigned int    		u32;
typedef signed int      		s32;
typedef unsigned long long 		u64;
typedef u32						FOURCC;
typedef long long               s64;
typedef unsigned long long      u64;

#define uint8_t  u8
#define uint16_t u16
#define uint32_t u32
#define uint64_t u64
#define int8_t   s8
#define int16_t  s16
#define int32_t  s32
#define int64_t  s64

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define abs(n) (n < 0) ? -n : n

int Adafruit_NeoPixel_constrain(int value, int min, int max);
void* Adafruit_NeoPixel_memmove(void* dest, const void* src, size_t num);
void* Adafruit_NeoPixel_memset(void* dest, int value, size_t num);
void* Adafruit_NeoPixel_memchr(const void* ptr, int value, size_t num);
double Adafruit_NeoPixel_pow(double base, int exponent);

#endif