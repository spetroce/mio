#ifndef __MIO_BIT_BYTE_OPS_H__
#define __MIO_BIT_BYTE_OPS_H__

#include <cstdint>

// a=target variable, b=bit number to act upon 0-n
#define BIT_SET(a,b) ((a) |= (1<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1<<(b)))
#define BIT_CHECK(a,b) ((a) & (1<<(b)))

// x=target variable, y=mask
#define BITMASK_SET(x,y) ((x) |= (y))
#define BITMASK_CLEAR(x,y) ((x) &= (~(y)))
#define BITMASK_FLIP(x,y) ((x) ^= (y))
#define BITMASK_CHECK(x,y) ((x) & (y))

#define NUM_TO_TWO_BYTE(num, arr, start_index) \
  arr[start_index] = num & 0xff;              \
  arr[start_index+1] = (num >> 8) & 0xff;

#define NUM_TO_FOUR_BYTE(num, arr, start_index) \
  arr[start_index] = num & 0xff;                \
  arr[start_index+1] = (num >> 8) & 0xff;       \
  arr[start_index+2] = (num >> 16) & 0xff;      \
  arr[start_index+3] = (num >> 24) & 0xff;

#define TWO_BYTE_TO_NUM(num_type, arr, start_index) \
  static_cast<num_type>(arr[start_index]) |         \
  static_cast<num_type>(arr[start_index+1] << 8)

#define FOUR_BYTE_TO_NUM(num_type, arr, start_index) \
  static_cast<num_type>(arr[start_index]) |          \
  static_cast<num_type>(arr[start_index+1] << 8) |   \
  static_cast<num_type>(arr[start_index+2] << 16) |  \
  static_cast<num_type>(arr[start_index+3] << 24);

inline uint32_t SetByte(const uint32_t num,
                        const uint8_t byte,
                        const uint8_t index) {
  const uint32_t masks[] = {0xffffff00, 0xffff00ff, 0xff00ffff, 0xffffff};
  if (index >= 0 && index < 4)
    return (num & masks[index]) | byte << 8*index;
  return 0;
}

inline uint32_t SetWord(const uint32_t num,
                        const uint16_t word,
                        const uint8_t index) {
  const uint32_t masks[] = {0xffff0000, 0xffff};
  if (index == 0 || index == 1)
    return (num & masks[index]) | word << 16*index;
  return 0;
}

#endif //__MIO_BIT_BYTE_OPS_H__
  
