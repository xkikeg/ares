#pragma once

#define ENABLE_DEBUG_PRINTING 0

#if ENABLE_DEBUG_PRINTING
#define DEBUG_PRT std::cerr
#else
struct NullOutputStream : public std::streambuf
{
};

namespace {
  NullOutputStream DEBUG_PRT;
}

template <class T>
inline NullOutputStream & operator<<(NullOutputStream & stream,
                              const T)
{
  return stream;
}
#endif
