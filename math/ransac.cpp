/* 
  Samuel Petrocelli
    -derived from MRPT RANSAC class
    -cleaned up and refactored code, modified to use std objects
   
   MRPT licnese and copyright notice:

   +---------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)               |
   |                          http://www.mrpt.org/                             |
   |                                                                           |
   | Copyright (c) 2005-2015, Individual contributors, see AUTHORS file        |
   | See: http://www.mrpt.org/Authors - All rights reserved.                   |
   | Released under BSD License. See details in http://www.mrpt.org/License    |
   +---------------------------------------------------------------------------+
*/

#include "mio/math/ransac.h"
#include "mio/math/ransacPlanes.h"
#include "mio/math/ransacLines.h"

#include <utility>

#include "opencv2/core.hpp"
#include "mio/math/math.h"
#include "mio/altro/error.h"
#include "mio/altro/algorithm.h" //EraseIndices()
#include "mio/altro/rand.h"


template <typename DATA_T, typename MODEL_T>
bool CRansac<DATA_T, MODEL_T>::execute(const std::vector<DATA_T> &data,
                                       TRansacFitFunctor fit_func,
                                       TRansacDistanceFunctor dist_func,
                                       TRansacDegenerateFunctor degen_func,
                                       const double dist_thresh,
                                       const uint32_t	min_samples_thresh,
                                       std::vector<uint32_t> &best_inliers, //out
                                       MODEL_T &best_model, //out
                                       bool	verbose,
                                       const double good_sample_prob,
                                       const uint32_t	max_iter){
  STD_INVALID_ARG_E(min_samples_thresh > 1);
  if(data.size() < 1)
    return false;
  const uint32_t data_size = data.size();
  const uint32_t max_data_trials = 100; //max attempts to select a non-degenerate data set
  best_inliers.clear();
  bool best_model_is_set = false;

  uint32_t trial_count = 0;
  size_t best_score = std::string::npos; //npos will mean "none"
  uint32_t N = 1; //dummy initialisation for number of trials.

  std::vector<uint32_t> ind(min_samples_thresh);

  //select random datapoints to form a trial model, M.  
  //check that data points are not in a degenerate configuration.
  while(N > trial_count){
    bool degenerate = true;
    uint32_t count = 1;
    std::vector<MODEL_T> models;

    while(degenerate){
      //generate random indicies
      ind.resize(min_samples_thresh);
      mio::RandomIntVec<uint32_t>(ind, 0, data_size-1);

      //test that these points are not a degenerate configuration.
      degenerate = degen_func(data, ind);

      if(!degenerate){
        //fit model to this random selection of data points. Note that M may represent a set of models that fit the data
        fit_func(data, ind, models);
        //depending on your problem it might be that the only way you can determine whether a data set is 
        //degenerate or not is to try to fit a model and see if it succeeds.  If it fails we reset degenerate to true.
        degenerate = models.empty();
      }

      //Safeguard against being stuck in this loop forever
      if(++count > max_data_trials){
        if(verbose) 
          printf("%s - unable to select a nondegenerate data set\n", CURRENT_FUNC);
        break;
      }
    }

    //evaluate distances between points and model returning the indices of elements in x that are inliers.  
    //additionally, if M is a cell array of possible models 'distfn' will return the model that has the most inliers.  
    //after this call M will be a non-cell object representing only one model.
    uint32_t best_model_idx = models.size() + 1;
    std::vector<uint32_t> inliers;
    if(!degenerate){
      dist_func(data, models, dist_thresh, best_model_idx, inliers);
      EXP_CHK_M(best_model_idx < models.size(), return(false), "invalid model index")
    }

    //find the number of inliers to this model
    const uint32_t num_inliers = inliers.size();
    bool update_estim_num_iters = trial_count == 0; //always update on the first iteration, regardless
                                                    //of the result (even for num_inliers=0)

    if( num_inliers > best_score || (best_score == std::string::npos && num_inliers != 0) ){
      //record data for this model
      best_score = num_inliers;  
      best_model = models[best_model_idx];
      best_model_is_set = true;
      best_inliers = inliers;
      update_estim_num_iters = true;
    }

    if(update_estim_num_iters){
      //update estimate of N, the number of trials to ensure we pick,
      //with probability p, a data set with no outliers.
      double fracinliers =  num_inliers / static_cast<double>(data_size);
      double prob_no_outliers = 1 - std::pow( fracinliers, static_cast<double>(min_samples_thresh) );

      prob_no_outliers = std::max(std::numeric_limits<double>::epsilon(), prob_no_outliers); //avoid division by -Inf
      prob_no_outliers = std::min(1.0 - std::numeric_limits<double>::epsilon(), prob_no_outliers); //avoid division by 0
      N = std::log(1-good_sample_prob) / std::log(prob_no_outliers);
      if(verbose)
        printf("%s - iter #%u Estimated number of iters: %u  prob_no_outliers = %f  #inliers: %u\n", 
               CURRENT_FUNC, static_cast<uint32_t>(trial_count), static_cast<uint32_t>(N),
               prob_no_outliers, static_cast<uint32_t>(num_inliers) );
    }

    ++trial_count;

    if(verbose)
      printf("%s - trial %u out of %u \r", CURRENT_FUNC, static_cast<uint32_t>(trial_count),
             static_cast<uint32_t>( std::ceil(static_cast<double>(N) ) ) );

    //safeguard against being stuck in this loop forever
    if(trial_count > max_iter){
      printf("%s - warning: maximum number of trials (%u) reached\n", CURRENT_FUNC, static_cast<uint32_t>(max_iter) );
      break;
    }
  }

  if(best_model_is_set){
    if(verbose)
      printf("%s - finished in %u iterations.\n", CURRENT_FUNC, static_cast<uint32_t>(trial_count) );
    return true;
  }
  else{
    if(verbose)
      printf("%s - warning: finished without any proper solution.\n", CURRENT_FUNC);
    return false;
  }
}

