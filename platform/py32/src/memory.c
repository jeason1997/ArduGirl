#include <stddef.h>

/* 这些基础符号必须由独立的 C 编译单元提供，避免链接时优化把实现再次替换成自身调用。 */
#define ARDUGIRL_RUNTIME_SYMBOL __attribute__((used, noinline, externally_visible))

ARDUGIRL_RUNTIME_SYMBOL void* memset(void* destination, int value, size_t size) {
    unsigned char* bytes = (unsigned char*)destination;
    while (size-- != 0U) {
        *bytes++ = (unsigned char)value;
    }
    return destination;
}

ARDUGIRL_RUNTIME_SYMBOL void* memcpy(void* destination, const void* source, size_t size) {
    unsigned char* out = (unsigned char*)destination;
    const unsigned char* in = (const unsigned char*)source;
    while (size-- != 0U) {
        *out++ = *in++;
    }
    return destination;
}

ARDUGIRL_RUNTIME_SYMBOL size_t strlen(const char* text) {
    size_t size = 0U;
    while (text[size] != '\0') {
        ++size;
    }
    return size;
}
