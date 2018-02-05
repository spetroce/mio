#ifndef __MIO_BIT_OPS_H__
#define __MIO_BIT_OPS_H__

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

#define NUM_TO_2_BYTE(num, arr, start_index) \
  data[start_index] = num & 0xff;            \
  data[start_index+1] = (num >> 8)  & 0xff;

#define NUM_TO_4_BYTE(num, arr, start_index) \
  data[start_index] = num & 0xff;            \
  data[start_index+1] = (num >> 8)  & 0xff;  \
  data[start_index+2] = (num >> 16)  & 0xff; \
  data[start_index+3] = (num >> 24)  & 0xff;

#define 2_BYTE_TO_NUM(num, num_type, arr, start_index)  \
  num = static_cast<num_type>(arr[start_index]) |         \
        static_cast<num_type>(arr[start_index+1] << 8) |  \

#define 4_BYTE_TO_NUM(num, num_type, arr, start_index)    \
  num = static_cast<num_type>(arr[start_index]) |         \
        static_cast<num_type>(arr[start_index+1] << 8) |  \
        static_cast<num_type>(arr[start_index+2] << 16) | \
        static_cast<num_type>(arr[start_index+3] << 24);

#endif //__MIO_BIT_OPS_H__

