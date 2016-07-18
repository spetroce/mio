//thread.hpp

#ifndef __MIO_THREAD_H__
#define __MIO_THREAD_H__

#include <stdio.h>
#include <pthread.h>
#include "mio/altro/error.h"


namespace mio{

  //devide an index range for parallel processing
  inline void index_range_partition(int range_lower_idx, int range_upper_idx, int num_partition,
                                    int partition_idx, int &idx_low, int &idx_high){
    idx_low = idx_high = -1;
    STD_INVALID_ARG_E(range_lower_idx >= 0 && range_lower_idx <= range_upper_idx)
    STD_INVALID_ARG_E(num_partition > 0 && partition_idx >= 0 && partition_idx < num_partition)

    if(num_partition == 1){
      idx_low = range_lower_idx;
      idx_high = range_upper_idx;
      return;
    }

    const int num_indice = range_upper_idx - range_lower_idx + 1;
    if(num_indice < num_partition){
      //some partitions will not have an index range
      if(range_lower_idx + partition_idx <= range_upper_idx)
        idx_low = idx_high = range_lower_idx + partition_idx;
      return;
    }

    const int part_size = num_indice / num_partition;
    const bool last_partition = (partition_idx + 1 == num_partition);
    idx_low = range_lower_idx + partition_idx*part_size;
    idx_high = last_partition ? range_upper_idx : idx_low + part_size - 1;

    //handle remainder situations. disperse the remainder among the partitions
    const int remainder = num_indice % num_partition;
    if(remainder > 0)
      if(partition_idx == 0)
        idx_high++;
      else if(last_partition)
        idx_low += remainder;
      else if(partition_idx < remainder){
        idx_low += partition_idx;
        idx_high += partition_idx + 1;
      }
      else{
        idx_low += remainder;
        idx_high += remainder;
      }
  }

  template <typename OBJ_T>
  class LockableType{
    public:
      typedef OBJ_T ObjectType_;
      OBJ_T obj_;
      std::mutex mtx_;

      void lock(){
        mtx_.lock();
      }

      void unlock(){
        mtx_.unlock();
      }

      OBJ_T get(){
        mtx_.lock();
        OBJ_T obj = obj_;
        mtx_.unlock();
        return obj;
      }

      void set(OBJ_T obj){
        mtx_.lock();
        obj_ = obj;
        mtx_.unlock();
      }
  };

} //namespace mio

#endif //__MIO_THREAD_H__

