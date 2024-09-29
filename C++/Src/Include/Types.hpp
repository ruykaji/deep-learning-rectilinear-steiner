#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#include <cstdint>

namespace types
{

constexpr uint8_t PATH_CELL             = 1;
constexpr uint8_t TRACE_CELL            = 5;
constexpr uint8_t TERMINAL_CELL         = 4;
constexpr uint8_t INTERSECTION_CELL     = 3;
constexpr uint8_t INTERSECTION_VIA_CELL = 2;

} // namespace types

#endif