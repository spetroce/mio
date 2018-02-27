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
#include <functional> // bind()
#include <algorithm> // generate()
#include "mio/altro/types.h"


template <typename DATA_T, typename MODEL_DATA_T, typename MODEL_T>
class CRansac{
  public:
    // The type of the 'fit' function passed to CRansac<DATA_T, MODEL_DATA_T, MODEL_T>::execute
    typedef void (*TRansacFitFunctor)(const std::vector<DATA_T> &all_data,
	                                    const std::vector<uint32_t> &use_indices,
	                                    std::vector<MODEL_T> &fit_model);

    // The type of the 'distance' function passed to CRansac<DATA_T, MODEL_DATA_T, MODEL_T>::execute
    typedef void (*TRansacDistanceFunctor)(const std::vector<DATA_T> &all_data,
	                                         const std::vector<MODEL_T> &test_model,
	                                         const DATA_T dist_thresh,
                                           uint32_t &best_model_idx,
	                                         std::vector<uint32_t> &inlier_indices); // out

    // The type of the 'degenerate' function passed to CRansac<DATA_T, MODEL_DATA_T, MODEL_T>::execute
    typedef bool (*TRansacDegenerateFunctor)(const std::vector<DATA_T> &all_data,
	                                           const std::vector<uint32_t> &use_indices);

    static bool execute(const std::vector<DATA_T> &data,
                        TRansacFitFunctor fit_func,
                        TRansacDistanceFunctor dist_func,
                        TRansacDegenerateFunctor degen_func,
                        const DATA_T dist_thresh,
                        const uint32_t min_samples_thresh,
                        std::vector<uint32_t> &best_inliers, // out
                        MODEL_T &best_model, // out
                        bool verbose,
                        const DATA_T good_sample_prob = 0.999,
                        const uint32_t max_iter = 2000);
};

namespace sm{

// RansacDetect...() are defined here. They are implemented in their respective headers, with explicit instantiations.
// Explicit instantiations needed when a templated function is implemented in the source file instead of the header.
// TODO: redundant information - detected_<obj_type>[0].first = number best inliers = line_inliers[0].size()
template <typename DATA_T, typename PNT_T, typename LINE_T>
void RansacDetectLines(const std::vector<PNT_T> &pnt,
                       std::vector< std::pair<uint32_t, LINE_T> > &detected_lines,
                       const DATA_T dist_thresh,
                       const uint32_t min_inliers,
                       std::vector< std::vector<uint32_t> > &line_inliers);

template <typename DATA_T, typename PNT_T, typename PLANE_T>
void RansacDetectPlanes(const std::vector<PNT_T> &pnt,
                        std::vector< std::pair<uint32_t, PLANE_T> > &detected_planes,
                        const DATA_T dist_thresh,
                        const uint32_t min_inliers,
                        std::vector< std::vector<uint32_t> > &plane_inliers);

}

#endif // __MIO_RANSAC_H__

