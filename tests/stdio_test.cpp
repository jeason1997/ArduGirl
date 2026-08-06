#include <cassert>
#include <cstdio>
#include <cstring>

int main() {
    char buffer[48]{};

    assert(std::sprintf(buffer, "%d", -27) == 3);
    assert(std::strcmp(buffer, "-27") == 0);

    assert(std::sprintf(buffer, "%02d %u %x %X", 5, 42U, 0x2aU, 0x2aU) == 11);
    assert(std::strcmp(buffer, "05 42 2a 2A") == 0);

    assert(std::sprintf(buffer, "%c%s%%", 'A', "rdu") == 5);
    assert(std::strcmp(buffer, "Ardu%") == 0);
}
