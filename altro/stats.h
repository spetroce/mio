#ifndef __MIO_STATS_H__
#define __MIO_STATS_H__

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <math.h>
#include <set>


inline void VectorStats(const std::vector<double> &_data_in, double &_min, double &_max, double &_variance,
                        double &_std_dev, double &_mean, double &_median, std::set<double> &_set_data){
  _set_data.clear();
	std::sort(_data_in.begin(), _data_in.end());
	_min = _data_in.front();
	_max = _data_in.back();

	double sum = 0;
	std::vector<int> occur_vec(_data_in.size());
	for(std::vector<double>::const_iterator iter = _data_in.begin(); iter != _data_in.end(); ++iter){
		sum = sum + *iter; //used for mean
		//occur_vec will be used later to find the mode
		for(size_t i = 0; i < _data_in.size(); ++ i)
			if( *iter == _data_in[i] )
				++occur_vec[i];
	}

	//arithmetic mean
	_mean = sum / _data_in.size();

	//statistical median
	if(_data_in.size() % 2 == 1)
	  _median = _data_in[_data_in.size()/2];
	else if(_data_in.size() % 2 == 0){
		double median_elem_a = _data_in[(_data_in.size() / 2) - 1];
		double median_elem_b = _data_in[_data_in.size() / 2];
		_median = (median_elem_a + median_elem_b)/2;
	}//end statistical median

	//variance
	_std_dev = 0;
	_variance = 0;
	for(std::vector<double>::const_iterator iter = _data_in.begin(); iter != _data_in.end(); ++iter)
		_variance = std::pow( (*iter - _mean), 2 ) + _variance;
	_variance = _variance / _data_in.size();

	//standard deviation
	_std_dev = sqrt(_variance);

	//mode
	double max_occur_val = 0;
	int occur_check = 0;
	for(std::vector<int>::const_iterator iter = occur_vec.begin(); iter != occur_vec.end(); ++iter){
		if(*iter == 1)//if the vector is filled with 1's it means each number has occurred only once, therefore no mode.
			occur_check = occur_check + 1;
		else if(*iter > max_occur_val)
			max_occur_val = *iter;
	}

	for(size_t i = 0; i < occur_vec.size(); ++i)
		if(occur_vec[i] == max_occur_val)
			_set_data.insert(_data_in[i]);
}


#endif //__MIO_STATS_H__

