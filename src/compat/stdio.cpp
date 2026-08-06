#include <cstdarg>
#include <cstddef>
#include <cstdint>

namespace {

char* append_unsigned(char* output,
                      unsigned long value,
                      unsigned base,
                      unsigned width,
                      char padding,
                      bool uppercase) noexcept {
    char digits[sizeof(unsigned long) * 8U];
    unsigned count = 0;
    const char* alphabet = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";

    do {
        digits[count++] = alphabet[value % base];
        value /= base;
    } while (value != 0);

    while (width > count) {
        *output++ = padding;
        --width;
    }
    while (count != 0) {
        *output++ = digits[--count];
    }
    return output;
}

} // namespace

// MCU 构建使用这个无堆整数子集，避免一次简单格式化拉入完整 newlib。
extern "C" int sprintf(char* output, const char* format, ...) noexcept {
    char* cursor = output;
    va_list arguments;
    va_start(arguments, format);

    while (*format != '\0') {
        if (*format++ != '%') {
            *cursor++ = format[-1];
            continue;
        }
        if (*format == '%') {
            *cursor++ = *format++;
            continue;
        }

        char padding = ' ';
        if (*format == '0') {
            padding = '0';
            ++format;
        }
        unsigned width = 0;
        while (*format >= '0' && *format <= '9') {
            width = width * 10U + static_cast<unsigned>(*format++ - '0');
        }
        bool long_value = false;
        if (*format == 'l') {
            long_value = true;
            ++format;
        }

        const char conversion = *format++;
        if (conversion == 'c') {
            *cursor++ = static_cast<char>(va_arg(arguments, int));
        } else if (conversion == 's') {
            const char* value = va_arg(arguments, const char*);
            while (*value != '\0') {
                *cursor++ = *value++;
            }
        } else if (conversion == 'd' || conversion == 'i') {
            const long value = long_value ? va_arg(arguments, long) : va_arg(arguments, int);
            unsigned long magnitude = static_cast<unsigned long>(value);
            if (value < 0) {
                *cursor++ = '-';
                magnitude = 0UL - magnitude;
                if (width != 0) {
                    --width;
                }
            }
            cursor = append_unsigned(cursor, magnitude, 10U, width, padding, false);
        } else if (conversion == 'u' || conversion == 'x' || conversion == 'X') {
            const unsigned long value = long_value ? va_arg(arguments, unsigned long)
                                                   : va_arg(arguments, unsigned int);
            const unsigned base = conversion == 'u' ? 10U : 16U;
            cursor = append_unsigned(cursor, value, base, width, padding, conversion == 'X');
        }
    }

    va_end(arguments);
    *cursor = '\0';
    return static_cast<int>(cursor - output);
}
