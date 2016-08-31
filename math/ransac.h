#ifndef __MIO_RANSAC_H__
#define __MIO_RANSAC_H__

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

#include <random>
#include <vector>
#include <functional> //bind()
#include <algorithm> //generate()
#include "mio/altro/types.h"


template <typename DATA_T, typename MODEL_T>
class CRansac{
  public:
    //the type of the 'fit' function passed to CRansac<DATA_T, MODEL_T>::execute
    typedef void (*TRansacFitFunctor)(const std::vector<DATA_T> &all_data,
	                                    const std::vector<uint32_t> &use_indices,
	                                    std::vector<MODEL_T> &fit_model);

    //the type of the 'distance' function passed to CRansac<DATA_T, MODEL_T>::execute
    typedef void (*TRansacDistanceFunctor)(const std::vector<DATA_T> &all_data,
	                                         const std::vector<MODEL_T> &test_model,
	                                         const double dist_thresh,
                                           uint32_t &best_model_idx,
	                                         std::vector<uint32_t> &inlier_indices); //out

    //the type of the 'degenerate' function passed to CRansac<DATA_T, MODEL_T>::execute
    typedef bool (*TRansacDegenerateFunctor)(const std::vector<DATA_T> &all_data,
	                                           const std::vector<uint32_t> &use_indices);

    static bool execute(const std::vector<DATA_T> &data,
                        TRansacFitFunctor fit_func,
                        TRansacDistanceFunctor dist_func,
                        TRansacDegenerateFunctor degen_func,
                        const double dist_thresh,
                        const uint32_t min_samples_thresh,
                        std::vector<uint32_t> &best_inliers, //out
                        MODEL_T &best_model, //out
                        bool verbose,
                        const double good_sample_prob = 0.999,
                        const uint32_t max_iter = 2000);
};

namespace sm{

//RansacDetect...() are defined here. They are implemented in their respective headers, with explicit instantiations.
//Explicit instantiations needed when a templated function is implemented in the source file instead of the header.
//TODO: redundant information - detected_<obj_type>[0].first = number best inliers = line_inliers[0].size()
template <typename PNT_T>
void RansacDetectLines(const std::vector<PNT_T> &pnt,
                       std::vector< std::pair<uint32_t, line3d_t> > &detected_lines,
                       const double dist_thresh,
                       const uint32_t min_inliers,
                       std::vector< std::vector<uint32_t> > &line_inliers);

template <typename PNT_T>
void RansacDetectPlanes(const std::vector<PNT_T> &pnt,
                        std::vector< std::pair<uint32_t, plane4d_t> > &detected_planes,
                        const double dist_thresh,
                        const uint32_t min_inliers,
                        std::vector< std::vector<uint32_t> > &plane_inliers);

}

#endif //__MIO_RANSAC_H__

