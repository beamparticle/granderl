#pragma once

#include <stdint.h>

#if defined(__ARM_ARCH)
#include <sys/time.h>
#endif

#ifdef __amd64
inline void rdtsc(uint64_t *t, uint64_t *u)
{
    asm volatile ("rdtsc" : "=a" (*t), "=d" (*u));
}
#elif defined(__ARM_ARCH)
inline void rdtsc(uint64_t *t, uint64_t *u)
{
#if (__ARM_ARCH >= 6)  // V6 is the earliest arch that has a standard cyclecount
  uint32_t pmccntr;
  uint32_t pmuseren;
  uint32_t pmcntenset;
  // Read the user mode perf monitor counter access permissions.
  asm volatile("mrc p15, 0, %0, c9, c14, 0" : "=r"(pmuseren));
  if (pmuseren & 1) {  // Allows reading perfmon counters for user mode code.
    asm volatile("mrc p15, 0, %0, c9, c12, 1" : "=r"(pmcntenset));
    if (pmcntenset & 0x80000000ul) {  // Is it counting?
      asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(pmccntr));
      // The counter is set up to count every 64th cycle
      uint64_t result = (uint64_t)(pmccntr) * 64;  // Should optimize to << 6
      *t = result >> 32;
      *u = result & ((1LLU << 32) - 1);
      return;
    }
  }
#endif
  struct timeval tv;
  gettimeofday(&tv, NULL);
  uint64_t result = (uint64_t)(tv.tv_sec) * 1000000 + tv.tv_usec;
  *t = result >> 32;
  *u = result & ((1LLU << 32) - 1);
}
#else
#error "Read your platform's perf counter here"
#endif
