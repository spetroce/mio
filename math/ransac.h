#ifndef __MIO_RANSAC_INL_H__
#define __MIO_RANSAC_INL_H__

/* 
  Samuel Petrocelli
    -derived from MRPT (http://www.mrpt.org/) RANSAC class , which is based on http://www.csse.uwa.edu.au/~pk/
    -cleaned up and refactored code, modified to use std objects
*/

#include <random>
#include <vector>
#include <functional> // bind()
#include <algorithm> // generate()
#include <utility>

#include "mio/math/math.h"
#include "mio/altro/error.h"
#include "mio/altro/algorithm.h" // EraseIndices()
#include "mio/altro/rand.h"

namespace sm {

template <typename DATA_T, typename MODEL_DATA_T, typename MODEL_T>
class CRansac{
  public:
    // The type of the 'fit' function passed to CRansac<DATA_T, MODEL_DATA_T, MODEL_T>::execute
    typedef void (*TRansacFitFunctor)(const std::vector<MODEL_DATA_T> &kDataVec,
	                                    const std::vector<uint32_t> &kUseIndexVec,
	                                    std::vector<MODEL_T> &model_vec);

    // The type of the 'distance' function passed to CRansac<DATA_T, MODEL_DATA_T, MODEL_T>::execute
    typedef void (*TRansacDistanceFunctor)(const std::vector<MODEL_DATA_T> &kDataVec,
	                                         const std::vector<MODEL_T> &kTestModelVec,
	                                         const DATA_T kMinInlierDist,
                                           uint32_t &best_model_idx,
	                                         std::vector<uint32_t> &inlier_index_vec); // out

    // The type of the 'degenerate' function passed to CRansac<DATA_T, MODEL_DATA_T, MODEL_T>::execute
    typedef bool (*TRansacDegenerateFunctor)(const std::vector<MODEL_DATA_T> &kDataVec,
	                                           const std::vector<uint32_t> &kUseIndexVec);

    static bool execute(const std::vector<MODEL_DATA_T> &kDataVec,
                        TRansacFitFunctor fit_func,
                        TRansacDistanceFunctor dist_func,
                        TRansacDegenerateFunctor degen_func,
                        const DATA_T kMinInlierDist, // minimum distance for data pnt to be considered an inlier
                        const uint32_t kMinNumRandomSample,
                        std::vector<uint32_t> &best_inlier_vec, // out
                        MODEL_T &best_model, // out
                        const bool kVerbosePrint = false,
                        const DATA_T kGoodSampleProb = 0.999,
                        const uint32_t kMaxIter = 20000);
};


template <typename DATA_T, typename MODEL_DATA_T, typename MODEL_T>
bool CRansac<DATA_T, MODEL_DATA_T, MODEL_T>::execute(
    const std::vector<MODEL_DATA_T> &kDataVec,
    TRansacFitFunctor fit_func,
    TRansacDistanceFunctor dist_func,
    TRansacDegenerateFunctor degen_func,
    const DATA_T kMinInlierDist, // minimum distance for data pnt to be considered an inlier
    const uint32_t kMinNumRandomSample,
    std::vector<uint32_t> &best_inlier_vec, // out
    MODEL_T &best_model, // out
    const bool kVerbosePrint,
    const DATA_T kGoodSampleProb,
    const uint32_t kMaxIter){
  EXP_CHK(kMinNumRandomSample > 1, return(false))
  EXP_CHK(kDataVec.size() > 1, return(false))

  const uint32_t kDataVecSize = kDataVec.size(),
                 kMaxDataTrials = 100; // Max attempts to find a non-degenerate data set
  best_inlier_vec.clear();
  bool best_model_is_set = false;
  uint32_t num_iter = 0,
           estimated_num_iter = 1;
  size_t best_score = std::string::npos; // npos will mean "none"
  std::vector<uint32_t> rand_idx_vec(kMinNumRandomSample);

  while(estimated_num_iter > num_iter){
    // We start in the degenerate state so we can form a trial model with a random set of points
    bool degenerate = true;
    std::vector<MODEL_T> model_vec;

    uint32_t count = 0;
    while(degenerate){
      // Generate random indicies
      rand_idx_vec.resize(kMinNumRandomSample);
      mio::RandomIntVec<uint32_t>(rand_idx_vec, 0, kDataVecSize-1);
      // Test that these points are not a degenerate configuration
      degenerate = degen_func(kDataVec, rand_idx_vec);
      if(!degenerate){
        // Fit model to random selection of data points
        fit_func(kDataVec, rand_idx_vec, model_vec);
        // Depending on the problem, checking whether we have a model or not might be the
        // only way to determine a data set is degenerate. If it is, we try a new data set
        degenerate = model_vec.empty();
      }
      if(++count > kMaxDataTrials){
        if(kVerbosePrint) 
          printf("%s - unable to find a nondegenerate data set\n", CURRENT_FUNC);
        break;
      }
    }

    uint32_t best_model_idx = model_vec.size() + 1; //TODO - why is this set here? also, it's out of bounds.
    std::vector<uint32_t> inlier_index_vec;
    if(!degenerate){
      // The distance function will determine which model has the most number of inliers and
      // will populate best_model_idx and inlier_index_vec accordingly
      dist_func(kDataVec, model_vec, kMinInlierDist, best_model_idx, inlier_index_vec);
      EXP_CHK_M(best_model_idx < model_vec.size(), return(false), "invalid model index")
    }

    const uint32_t kNumInliers = inlier_index_vec.size();
    bool update_estimated_num_iter = num_iter == 0; // Always update estimated_num_iter on the first iteration

    if(kNumInliers > best_score || (best_score == std::string::npos && kNumInliers != 0)){
      // Record best model and inlier vec
      best_score = kNumInliers;  
      best_model = model_vec[best_model_idx];
      best_model_is_set = true;
      best_inlier_vec = inlier_index_vec;
      update_estimated_num_iter = true;
    }

    if(update_estimated_num_iter){
      // Update estimate of estimated_num_iter, the number of trials to ensure we pick,
      // with probability p, a data set with no outliers.
      DATA_T fracinliers =  kNumInliers / static_cast<DATA_T>(kDataVecSize);
      DATA_T prob_no_outliers = static_cast<DATA_T>(1) -
                                std::pow(fracinliers, static_cast<DATA_T>(kMinNumRandomSample));
      prob_no_outliers = std::max(std::numeric_limits<DATA_T>::epsilon(), prob_no_outliers); // Avoid division by -Inf
      prob_no_outliers = std::min(static_cast<DATA_T>(1.0 - std::numeric_limits<DATA_T>::epsilon()),
                                  prob_no_outliers); // Avoid division by 0
      estimated_num_iter = std::log(static_cast<DATA_T>(1)-kGoodSampleProb) / std::log(prob_no_outliers);
      if(kVerbosePrint)
        printf("%s - Number of iterations: %u, Estimated number of iterations: %u, "
"Probability of no outliers: %f, Number of inliers: %u\n", 
               CURRENT_FUNC, num_iter, estimated_num_iter, prob_no_outliers, kNumInliers);
    }

    ++num_iter;

    if(kVerbosePrint)
      printf("%s - Number of iterations: %u, Estimated number of iterations: %u\n",
             CURRENT_FUNC, num_iter, estimated_num_iter);

    if(num_iter > kMaxIter){
      if(kVerbosePrint)
        printf("%s - warning: maximum number of iterations (%u) reached\n", CURRENT_FUNC, kMaxIter);
      break;
    }
  }

  if(best_model_is_set){
    if(kVerbosePrint)
      printf("%s - finished in %u iterations\n", CURRENT_FUNC, static_cast<uint32_t>(num_iter));
    return true;
  }
  else{
    if(kVerbosePrint)
      printf("%s - warning: finished without finding solution\n", CURRENT_FUNC);
    return false;
  }
}


#include "mio/math/ransac_planes.h"
#include "mio/math/ransac_lines.h"

}

#endif // __MIO_RANSAC_INL_H__

