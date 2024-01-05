#include "Adafruit_NeoPixel_defines.h"

int Adafruit_NeoPixel_constrain(int value, int min, int max) {
    if (value < min) {
        return min;
    } else if (value > max) {
        return max;
    } else {
        return value;
    }
}

void* Adafruit_NeoPixel_memmove(void* dest, const void* src, size_t num) {
    unsigned char* d = dest;
    const unsigned char* s = src;

    if (d < s) {
        while (num--) {
            *d++ = *s++;
        }
    } else {
        d += num;
        s += num;
        while (num--) {
            *(--d) = *(--s);
        }
    }

    return dest;
}

void* Adafruit_NeoPixel_memset(void* dest, int value, size_t num) {
    unsigned char* p = (unsigned char*)dest;
    unsigned char v = (unsigned char)value;

    for (size_t i = 0; i < num; ++i) {
        p[i] = v;
    }

    return dest;
}

void* Adafruit_NeoPixel_memchr(const void* ptr, int value, size_t num) {
    const unsigned char* p = ptr;
    unsigned char v = (unsigned char)value;

    for (size_t i = 0; i < num; ++i) {
        if (p[i] == v) {
            return (void*)(p + i);
        }
    }

    return NULL;
}

double Adafruit_NeoPixel_pow(double base, int exponent) {
    double result = 1.0;

    if (exponent >= 0) {
        for (int i = 0; i < exponent; ++i) {
            result *= base;
        }
    } else {
        for (int i = 0; i > exponent; --i) {
            result /= base;
        }
    }

    return result;
}

