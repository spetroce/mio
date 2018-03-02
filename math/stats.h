#ifndef __MIO_STATS_H__
#define __MIO_STATS_H__

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <math.h>
#include <set>

namespace sm{

// if kReadOnlySrc is defined a copy of the src date will be made for computing the median and mode
template <typename DATA_T>
inline void ComputeStatistics(std::vector<DATA_T> &_src_vec, DATA_T &_min, DATA_T &_max, DATA_T &_variance,
                              DATA_T &_std_dev, DATA_T &_mean, DATA_T &_median, std::set<DATA_T> &_set_data,
                              const bool kReadOnlySrc = true){
  std::vector<DATA_T> src_vec_copy;
  if(kReadOnlySrc)
    src_vec_copy = _src_vec;
  std::vector<DATA_T> &src_vec = kReadOnlySrc ? src_vec_copy : _src_vec;
  _set_data.clear();
	std::sort(src_vec.begin(), src_vec.end());
	_min = src_vec.front();
	_max = src_vec.back();

	DATA_T sum = 0;
  const size_t kSrcVecSize = src_vec.size();
	std::vector<unsigned int> occur_vec(kSrcVecSize);
  std::fill(occur_vec.begin(), occur_vec.end(), 0);
  for(const auto &kSrcVal : src_vec){
		sum += kSrcVal; // used for mean
		// occur_vec will be used later to find the mode
    for(size_t i = 0; i < kSrcVecSize; ++i)
			if(kSrcVal == src_vec[i])
				++occur_vec[i];
	}

	// arithmetic mean
	_mean = sum / src_vec.size();

	// statistical median
	if(src_vec.size() % 2 == 1)
	  _median = src_vec[src_vec.size()/2];
	else if(src_vec.size() % 2 == 0){
		DATA_T median_elem_a = src_vec[(src_vec.size() / 2) - 1];
		DATA_T median_elem_b = src_vec[src_vec.size() / 2];
		_median = (median_elem_a + median_elem_b)/2;
	} // end statistical median

	//variance
	_std_dev = 0;
	_variance = 0;
  for(auto &data_in_val : src_vec)
		_variance = std::pow( (data_in_val - _mean), 2 ) + _variance;
	_variance = _variance / src_vec.size();

	// standard deviation
	_std_dev = sqrt(_variance);

	// mode
	DATA_T max_occur_val = 0;
	int occur_check = 0;
  for(auto &occur_val : occur_vec){
		if(occur_val == 1) // if the vector is filled with 1's it means each number has occurred only once, therefore no mode.
			occur_check = occur_check + 1;
		else if(occur_val > max_occur_val)
			max_occur_val = occur_val;
	}

	for(size_t i = 0; i < kSrcVecSize; ++i)
		if(occur_vec[i] == max_occur_val)
			_set_data.insert(src_vec[i]);
}


// the std_dev_coef parameter needs to be tuned
// filt_data_idx contains the index of the data filt_data value in src_data
template <typename DATA_T>
DATA_T StatisticalOutlierRemoval(const std::vector<DATA_T> &src_data, std::vector<DATA_T> &filt_data,
                                 std::vector<size_t> &filt_data_idx, const DATA_T std_dev_coef){
  filt_data.resize(0);
  filt_data.reserve(src_data.size());
  filt_data_idx.resize(0);
  filt_data_idx.reserve(src_data.size());
  DATA_T sum = 0.0, mean, std_dev = 0.0;
  const unsigned int kDataLen = src_data.size();
  for(unsigned int i = 0; i < kDataLen; ++i){
    sum += src_data[i];
  }
  mean = sum/static_cast<DATA_T>(kDataLen);
  for(unsigned int i = 0; i < kDataLen; ++i){
    DATA_T data_minus_mean = src_data[i] - mean;
    std_dev += data_minus_mean*data_minus_mean;
  }

  std_dev = std::sqrt(std_dev / kDataLen);

  // a distance that is bigger than this signals an outlier
  const DATA_T thresh_upper = mean + std_dev_coef*std_dev,
               thresh_lower = mean - std_dev_coef*std_dev;

  unsigned int count = 0;
  for(unsigned int i = 0; i < kDataLen; ++i){
    if(src_data[i] <= thresh_upper && src_data[i] >= thresh_lower){
      sum += src_data[i];
      ++count;
      filt_data.push_back(src_data[i]);
      filt_data_idx.push_back(i);
    }
  }

  return sum/static_cast<DATA_T>(count);
}

} // namespace sm

#endif //__MIO_STATS_H__

