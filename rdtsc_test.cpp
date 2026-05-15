#include <iostream>
#include <cstdint>
#include <limits>
#include <conio.h>

#define NOMINMAX
#include <windows.h>

#if defined(_MSC_VER) // MSVC building
#include <intrin.h>

static inline uint64_t rdtsc_now() {
    return __rdtsc();
}

static inline void do_cpuid() {
    int regs[4];
    __cpuid(regs, 0);
}

#else // GCC/MinGW building
#include <x86intrin.h>
#include <cpuid.h>

static inline uint64_t rdtsc_now() {
    return __rdtsc();
}

static inline void do_cpuid() {
    unsigned int a, b, c, d;
    __cpuid(0, a, b, c, d);
}
#endif

#define RESET   "\033[0m"
#define YELLOW "\033[93m"
#define GRAY    "\033[90m"
#define RED     "\033[91m"

void enable_colors() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;

    if (GetConsoleMode(hOut, &mode)) {
        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, mode);
    }
}

struct Stats {
    uint64_t min = std::numeric_limits<uint64_t>::max();
    uint64_t max = 0;
    uint64_t sum = 0;
};

void add_sample(Stats& s, uint64_t delta) {
    if (delta < s.min) s.min = delta;
    if (delta > s.max) s.max = delta;
    s.sum += delta;
}

void print_stats(const char* name, const Stats& s, int samples) {
    uint64_t avg = s.sum / samples;
    std::cout << GRAY << "-- " << name << " --" << "\n";
    std::cout << "- " << RESET << " Min     : " << s.min << GRAY << " cycles\n";
    std::cout << "- " << RESET << " Max     : " << s.max << GRAY << " cycles\n";
    std::cout << "- " << YELLOW << " Average " << RESET << ": " << YELLOW << avg << GRAY << " cycles\n\n\n";
}

int main() {

    const int SAMPLES = 10;

    enable_colors();
    std::cout << RED << "RDTSC test" << RESET << "\n\n";
    std::cout << RESET << "Samples : " << SAMPLES << "\n\n";

    Stats b2b;
    Stats cpuid_latency;

    for (int i = 0; i < SAMPLES; i++) {
        uint64_t a = rdtsc_now();
        uint64_t b = rdtsc_now();
        add_sample(b2b, b - a);
    }

    for (int i = 0; i < SAMPLES; i++) {
        uint64_t a = rdtsc_now();
        do_cpuid();
        uint64_t b = rdtsc_now();
        add_sample(cpuid_latency, b - a);
    }

    print_stats("RDTSC Delta Check", b2b, SAMPLES);
    Sleep(250);// optional pause
    print_stats("CPUID VM Exit Timing Check", cpuid_latency, SAMPLES);

    std::cout << GRAY << "https://github.com/spamByte/rdtsc-timing-check\n";
    std::cout << RESET << "Press any key to exit...";

    _getch();
    return 0;
}