#pragma once

#if !defined(EUREKA_DEBUG_TRAP_H)
#define EUREKA_DEBUG_TRAP_H

#if !defined(EUREKA_NDEBUG) && defined(NDEBUG) && !defined(EUREKA_DEBUG)
#  define EUREKA_NDEBUG 1
#endif

#if defined(__has_builtin) && !defined(__ibmxl__)
#  if __has_builtin(__builtin_debugtrap)
#    define trigger_debugger_breakpoint() __builtin_debugtrap()
#  elif __has_builtin(__debugbreak)
#    define trigger_debugger_breakpoint() __debugbreak()
#  endif
#endif
#if !defined(trigger_debugger_breakpoint)
#  if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#    define trigger_debugger_breakpoint() __debugbreak()
#  elif defined(__ARMCC_VERSION)
#    define trigger_debugger_breakpoint() __breakpoint(42)
#  elif defined(__ibmxl__) || defined(__xlC__)
#    include <builtins.h>
#    define trigger_debugger_breakpoint() __trap(42)
#  elif defined(__DMC__) && defined(_M_IX86)
static inline void trigger_debugger_breakpoint(void) { __asm int 3h; }
#  elif defined(__i386__) || defined(__x86_64__)
static inline void trigger_debugger_breakpoint(void) { __asm__ __volatile__("int3"); }
#  elif defined(__thumb__)
static inline void trigger_debugger_breakpoint(void) { __asm__ __volatile__(".inst 0xde01"); }
#  elif defined(__aarch64__)
static inline void trigger_debugger_breakpoint(void) { __asm__ __volatile__(".inst 0xd4200000"); }
#  elif defined(__arm__)
static inline void trigger_debugger_breakpoint(void) { __asm__ __volatile__(".inst 0xe7f001f0"); }
#  elif defined (__alpha__) && !defined(__osf__)
static inline void trigger_debugger_breakpoint(void) { __asm__ __volatile__("bpt"); }
#  elif defined(_54_)
static inline void trigger_debugger_breakpoint(void) { __asm__ __volatile__("ESTOP"); }
#  elif defined(_55_)
static inline void trigger_debugger_breakpoint(void) { __asm__ __volatile__(";\n .if (.MNEMONIC)\n ESTOP_1\n .else\n ESTOP_1()\n .endif\n NOP"); }
#  elif defined(_64P_)
static inline void trigger_debugger_breakpoint(void) { __asm__ __volatile__("SWBP 0"); }
#  elif defined(_6x_)
static inline void trigger_debugger_breakpoint(void) { __asm__ __volatile__("NOP\n .word 0x10000000"); }
#  elif defined(__STDC_HOSTED__) && (__STDC_HOSTED__ == 0) && defined(__GNUC__)
#    define trigger_debugger_breakpoint() __builtin_trap()
#  else
#    include <signal.h>
#    if defined(SIGTRAP)
#      define trigger_debugger_breakpoint() raise(SIGTRAP)
#    else
#      define trigger_debugger_breakpoint() raise(SIGABRT)
#    endif
#  endif
#endif

#if defined(HEDLEY_LIKELY)
#  define EUREKA_DBG_LIKELY(expr) HEDLEY_LIKELY(expr)
#elif defined(__GNUC__) && (__GNUC__ >= 3)
#  define EUREKA_DBG_LIKELY(expr) __builtin_expect(!!(expr), 1)
#else
#  define EUREKA_DBG_LIKELY(expr) (!!(expr))
#endif

#if !defined(EUREKA_NDEBUG) || (EUREKA_NDEBUG == 0)
#  define psnip_dbg_assert(expr) do { \
    if (!EUREKA_DBG_LIKELY(expr)) { \
      trigger_debugger_breakpoint(); \
    } \
  } while (0)
#else
#  define psnip_dbg_assert(expr)
#endif

#endif /* !defined(EUREKA_DEBUG_TRAP_H) */