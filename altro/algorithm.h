#ifndef __MIO_ALGORITHM_H__
#define __MIO_ALGORITHM_H__

#include <algorithm>
#include <vector>
#include "mio/altro/error.h"

namespace mio{

template <typename T>
inline void FillRange(std::vector<T> &vec, const T range_start, const T range_stop){
  EXP_CHK_EM(false, return, "this is deprecated, use std::iota")
  STD_INVALID_ARG_E(vec.size() > 0)
  const T step = (range_stop-range_start) / static_cast<T>( vec.size()-1 );
  T val = range_start;
  for(T &elem : vec){
    elem = val;
    val += step;
  }
}


template<typename DATA_T, typename INDEX_T>
inline std::vector<DATA_T> EraseIndices(const std::vector<DATA_T> &data, std::vector<INDEX_T> &indices_to_delete){
  if( indices_to_delete.empty() )
    return data;

  std::vector<DATA_T> data_out;
  data_out.reserve( data.size() - indices_to_delete.size() );

  std::sort( indices_to_delete.begin(), indices_to_delete.end() );

  // now we can assume there is at least 1 element to delete. copy blocks at a time.
  typename std::vector<DATA_T>::const_iterator block_begin_it = data.begin();
  for(typename std::vector<INDEX_T>::const_iterator it = indices_to_delete.begin(); it != indices_to_delete.end(); ++ it){
    typename std::vector<DATA_T>::const_iterator block_end_it = data.begin() + *it;
    if(block_begin_it != block_end_it)
      std::copy( block_begin_it, block_end_it, std::back_inserter(data_out) );
    block_begin_it = block_end_it + 1;
  }

  // copy last block.
  if( block_begin_it != data.end() )
    std::copy( block_begin_it, data.end(), std::back_inserter(data_out) );

  return data_out;
}


template <typename T>
void UniqueValues( const std::vector<T> &src_vec, std::vector<T> &unique_vec){
  if(src_vec.size() <= 0)
    return;
  unique_vec.clear();
  std::vector<T> src_vec_ = src_vec; //make a local copy of src_vec
  std::sort( src_vec_.begin(), src_vec_.end() );

  T unique_value = src_vec_.front();
  unique_vec.push_back(unique_value); 
  for(auto &elem : src_vec_)
    if(elem != unique_value){
      unique_value = elem;
      unique_vec.push_back(elem);
    }
}


template <typename TObject, typename TMember>
void UniqueValues( const std::vector<TObject> &src_vec, std::vector<TObject> &unique_vec, TMember member){
  if(src_vec.size() <= 0)
    return;
  unique_vec.clear();
  std::vector<TObject> src_vec_ = src_vec; //make a local copy of src_vec

  auto compare_func = [&](const TObject &a, const TObject &b) { return a.*member < b.*member; };
  std::sort(src_vec_.begin(), src_vec_.end(), compare_func);

  auto unique_value = src_vec_.front().*member;
  unique_vec.push_back( src_vec_.front() ); 
  for(auto &elem : src_vec_)
    if(elem.*member != unique_value){
      unique_value = elem.*member;
      unique_vec.push_back(elem);
   }
}


//removes indices in array where func returns a true a value
template<typename T>
void FilterArray(std::vector<T> &array, bool (*func)(T, float), float func_param){
  int step_back = 0, array_size = array.size();
  for(int i = 0; i < array_size; i++){
    if( (*func)(array[i], func_param) )
      step_back++;
    else
      array[i - step_back] = array[i];
  }
  array_size -= step_back;
  array.resize(array_size);
}


//bounds 'value' to go no less than 'lower' and no greater than 'upper'
//returns true if number was within bounds, false otherwise
template <typename T> 
inline bool SetBound(T &value, const T &lower, const T &upper){ //FIXME: change to [lower, upper)
  if(value < lower){
    value = lower;
    return false;
  }
  else if(value > upper){
    value = upper;
    return false;
  }
  return true;
}


//returned value belongs in set [lower, upper)
template <typename T> 
inline T GetBounded(T value, const T lower, const T upper){
  if(value < lower)
    value = lower;
  else if(value >= upper)
    value = upper;
  return value;
}


//checks if value is in the range [low, high]
template <typename T>
bool IsInBounds(const T &value, const T &low, const T &high){
  return( !(value < low) && !(high < value) );
}


/*
  Function to insert array 'arr_a' into array 'arr_b' 

  arr_a.............array to be inserted into arr_b
  arr_a_first_idx...index of first element in arr_a being added to arr_b
  arr_b_idx_l.......index of last element in arr_a being added to arr_b
  arr_b.............integer array that arr_a will be inserted into
  arr_b_len.........number of elements present in arr_b
  arr_b_idx.........the index location (in arr_b) that arr_a's arr_a_first_idx element will be inserted
  arr_b_size........the maximum number of elements that arr_b can hold
*/
template <typename T>
inline unsigned int ArrayInsert(const T *arr_a, const size_t arr_a_first_idx, const size_t arr_a_last_idx, T *arr_b, 
                                const size_t arr_b_len, const size_t arr_b_idx, const size_t arr_b_size){
  if(arr_a_first_idx > arr_a_last_idx)
    return -1;
  int i, j = 0, temp, nElemA = (arr_a_last_idx - arr_a_first_idx + 1);
  //check that the array will fit
  if( (nElemA + arr_b_len) > arr_b_size )
    return -1;

  //moving from and include arr_b's index (arr_b_len - 1) to arr_b_idx, shift
  //elements forward ElemA amount, then insert arr_a element.
  temp = arr_b_len - 1;
  for(i = temp; i >= arr_b_idx; i--)
    arr_b[ (i + nElemA) ] = arr_b[i];

  temp = arr_b_idx + nElemA;
  for(i = arr_b_idx; i < temp; i++){
    arr_b[i] = arr_a[j];
    j++;
  }

  return (nElemA + arr_b_len);
}

} //namespace mio

#endif //__MIO_ALGORITHM_H__

