
#pragma once

// Old C headers
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdlib>

#include <algorithm>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <string>
#include <vector>
#include <stack>
#include <functional>

namespace wangziqi2013 {
namespace cfront {

static void dummy(const char*, ...) {}

#define DEBUG_PRINT

#ifdef DEBUG_PRINT

#define dbg_printf(fmt, ...)                              \
  do {                                                    \
    fprintf(stderr, "%-24s: " fmt, __FUNCTION__, ##__VA_ARGS__); \
    fflush(stdout);                                       \
  } while (0);

#else

#define dbg_printf(fmt, ...)   \
  do {                         \
    dummy(fmt, ##__VA_ARGS__); \
  } while (0);

#endif

}
}
