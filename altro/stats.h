#ifndef __MIO_STATS_H__
#define __MIO_STATS_H__

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <math.h>
#include <set>


inline void VectorStats(std::vector<double> &_src_vec, double &_min, double &_max, double &_variance,
                        double &_std_dev, double &_mean, double &_median, std::set<double> &_set_data){
  _set_data.clear();
	std::sort(_src_vec.begin(), _src_vec.end());
	_min = _src_vec.front();
	_max = _src_vec.back();

	double sum = 0;
  const size_t kSrcVecSize = _src_vec.size();
	std::vector<unsigned int> occur_vec(kSrcVecSize);
  std::fill(occur_vec.begin(), occur_vec.end(), 0);
  for(const auto &kSrcVal : _src_vec){
		sum += kSrcVal; //used for mean
		//occur_vec will be used later to find the mode
    for(size_t i = 0; i < kSrcVecSize; ++i)
			if(kSrcVal == _src_vec[i])
				++occur_vec[i];
	}

	//arithmetic mean
	_mean = sum / _src_vec.size();

	//statistical median
	if(_src_vec.size() % 2 == 1)
	  _median = _src_vec[_src_vec.size()/2];
	else if(_src_vec.size() % 2 == 0){
		double median_elem_a = _src_vec[(_src_vec.size() / 2) - 1];
		double median_elem_b = _src_vec[_src_vec.size() / 2];
		_median = (median_elem_a + median_elem_b)/2;
	}//end statistical median

	//variance
	_std_dev = 0;
	_variance = 0;
  for(auto &data_in_val : _src_vec)
		_variance = std::pow( (data_in_val - _mean), 2 ) + _variance;
	_variance = _variance / _src_vec.size();

	//standard deviation
	_std_dev = sqrt(_variance);

	//mode
	double max_occur_val = 0;
	int occur_check = 0;
  for(auto &occur_val : occur_vec){
		if(occur_val == 1)//if the vector is filled with 1's it means each number has occurred only once, therefore no mode.
			occur_check = occur_check + 1;
		else if(occur_val > max_occur_val)
			max_occur_val = occur_val;
	}

	for(size_t i = 0; i < kSrcVecSize; ++i)
		if(occur_vec[i] == max_occur_val)
			_set_data.insert(_src_vec[i]);
}


#endif //__MIO_STATS_H__

