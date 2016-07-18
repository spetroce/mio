#ifndef __MIO_RAND_H__
#define __MIO_RAND_H__

#include <sys/time.h>
#include <stdlib.h>
#include <random>
#include <functional> //bind()
#include <algorithm> //generate()


namespace mio{

template <typename T>
class CRandNum{
  std::mt19937 gen;
  std::normal_distribution<T> dist;
  T mean, std_dev;

  public:
    CRandNum(const T mean_ = 0.0, const T std_dev_ = 0.01) : mean(mean_), std_dev(std_dev_){
      std::random_device rand_dev;
      const T seed = rand_dev(); //rand_dev() returns a non-deterministic uniformly-distributed random value
      gen = std::mt19937(seed);
      dist = std::normal_distribution<T>(mean, std_dev);
    }

    T GetRandNum(){
      return dist(gen);
    }
};


inline void srand(){
  timeval t;
  gettimeofday(&t, NULL);
  std::srand( static_cast<unsigned int>(t.tv_usec) );
}


//low and high and inclusive
template<typename T>
inline T rand(T low, T high){
  return( low + (high - low + 1) * std::rand() / (RAND_MAX + .5f) ); 
}


//min and max are inclusive
template<typename T>
inline void RandomIntVec(std::vector<T> &vec, const T min, const T max){ 
  std::random_device rand_dev;
  double seed = rand_dev(); //rand_dev() returns a non-deterministic uniformly-distributed random value
  std::mt19937 engine(seed);
  std::uniform_int_distribution<T> uniform_dist(min, max);
  auto generator = std::bind(uniform_dist, engine);
  std::generate(vec.begin(), vec.end(), generator);
}


//min and max are inclusive
template<typename T>
inline void RandomFloatVec(std::vector<T> &vec, const T min, const T max){
  std::random_device rand_dev;
  double seed = rand_dev(); //rand_dev() returns a non-deterministic uniformly-distributed random value
  std::mt19937 engine(seed);
  std::uniform_real_distribution<T> uniform_dist(min, max);
  auto generator = std::bind(uniform_dist, engine);
  std::generate(vec.begin(), vec.end(), generator);
}

} //namespace mio

#endif //__MIO_RAND_H__

