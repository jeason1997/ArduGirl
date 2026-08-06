#include "crash.hpp"

#include <execinfo.h>
#include <signal.h>
#include <unistd.h>

#include <cstddef>
#include <iterator>

namespace ardugirl::platform::linux_crash {
namespace {

void write_text(const char* text, std::size_t size) noexcept {
    while (size != 0) {
        const auto written = ::write(STDERR_FILENO, text, size);
        if (written <= 0) return;
        text += written;
        size -= static_cast<std::size_t>(written);
    }
}

void handle_signal(int signal_number) noexcept {
    static constexpr char prefix[] = "\nArduGirl 捕获到致命信号，调用栈如下：\n";
    write_text(prefix, sizeof(prefix) - 1);

    void* frames[64]{};
    const auto count = ::backtrace(frames, static_cast<int>(std::size(frames)));
    ::backtrace_symbols_fd(frames, count, STDERR_FILENO);

    // 恢复默认处理后重新触发信号，让 shell 仍能获得正确的失败状态和 core dump。
    ::signal(signal_number, SIG_DFL);
    ::raise(signal_number);
}

} // 匿名命名空间

void install() noexcept {
    ::signal(SIGSEGV, handle_signal);
    ::signal(SIGABRT, handle_signal);
    ::signal(SIGBUS, handle_signal);
    ::signal(SIGFPE, handle_signal);
    ::signal(SIGILL, handle_signal);
}

} // 命名空间 ardugirl::platform::linux_crash
