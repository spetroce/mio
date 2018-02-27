#ifndef __MIO_RANSAC_PLANES_H__
#define __MIO_RANSAC_PLANES_H__

// Detect Planes in 3D space via RANSAC

#include "mio/math/math.h"
#include "mio/altro/algorithm.h"


template <typename PNT_T, typename PLANE_T>
void PlaneFit(const std::vector<PNT_T> &all_data,
              const std::vector<uint32_t> &use_indices,
              std::vector<PLANE_T> &fit_model){
  fit_model.resize(1);
  sm::PlaneCoefFromPnt<PNT_T, PLANE_T>(all_data[use_indices[0]], all_data[use_indices[1]], 
                                       all_data[use_indices[2]], fit_model[0]);
}


template <typename DATA_T, typename PNT_T, typename PLANE_T>
void PlaneDistance(const std::vector<PNT_T> &all_data,
                   const std::vector<PLANE_T> &test_model,
                   const DATA_T dist_thresh,
                   uint32_t &best_model_idx,
                   std::vector<uint32_t> &inlier_indices){ // out
  best_model_idx = 0;
  inlier_indices.clear();
  if( test_model.empty() || all_data.empty() )
    return;
  inlier_indices.reserve(100); // TODO - 100 is arbitrary
  for(size_t i = 0, allDataSize = all_data.size(); i < allDataSize; ++i){
    const DATA_T d = sm::DistToPlane<PLANE_T, PNT_T>(test_model[0], all_data[i]);
    if(d < dist_thresh)
      inlier_indices.push_back(i);
  }
}


// Return "true" if the selected points are a degenerate (invalid) case.
template <typename PNT_T>
bool PlaneDegenerate(const std::vector<PNT_T> &all_data,
                     const std::vector<uint32_t> &use_indices){
  return false;
}


namespace sm{

template <typename DATA_T, typename PNT_T, typename PLANE_T>
void RansacDetectPlanes(const std::vector<PNT_T> &pnt,
                        std::vector< std::pair<uint32_t, PLANE_T> > &detected_planes, // out
                        const DATA_T dist_thresh,
                        const uint32_t min_inliers,
                        std::vector< std::vector<uint32_t> > &plane_inliers){ // out
  detected_planes.clear();
  if(pnt.empty())
    return;
  // The running lists of remaining points after each plane, as a matrix
  std::vector<PNT_T> remaining_pnts = pnt;
  plane_inliers.clear();
  std::vector<uint32_t> indices( pnt.size() );
  for(size_t i = 0, indicesSize = indices.size(); i < indicesSize; ++i)
    indices[i] = i;
  bool first_iter_flag = true;

  while(remaining_pnts.size() >= 3){
    std::vector<uint32_t>	best_inliers;
    PLANE_T best_model;

    CRansac<DATA_T, PNT_T, PLANE_T>::execute(remaining_pnts,
                                             PlaneFit,
                                             PlaneDistance,
                                             PlaneDegenerate,
                                             dist_thresh,
                                             3, // Minimum set of points
                                             best_inliers,
                                             best_model,
                                             false, // verbose
                                             0.999); // Prob. of good result
    // Check fit robustness
    if(best_inliers.size() >= min_inliers){
      // Add this plane to the output list
      detected_planes.push_back(std::make_pair<uint32_t, PLANE_T>(best_inliers.size(), PLANE_T(best_model)));
      if(first_iter_flag){
        plane_inliers.push_back(best_inliers);
        first_iter_flag = false;
      }
      else{
        std::vector<uint32_t> temp( best_inliers.size() );
        const size_t kTempSize = temp.size();
        for(size_t i = 0; i < kTempSize; ++i)
          temp[i] = indices[best_inliers[i]];
        plane_inliers.push_back(temp);
      }
      // Normalize coefifients
      PLANE_T &plane = detected_planes.rbegin()->second;
      DATA_T inv_l2_norm = 1.0f / std::sqrt(plane.a*plane.a + plane.b*plane.b + plane.c*plane.c + plane.d*plane.d);
      plane.a *= inv_l2_norm;
      plane.b *= inv_l2_norm;
      plane.c *= inv_l2_norm;
      plane.d *= inv_l2_norm;
      // Discard the selected points so they are not used again for finding subsequent planes
      remaining_pnts = mio::EraseIndices<PNT_T>(remaining_pnts, best_inliers);
      indices = mio::EraseIndices<uint32_t>(indices, best_inliers);
    }
    else
      break; // Do not search for more planes
  }
}


// template explicit instantiations:
#define RansacDetectPlanes_EXPLICIT_INST(DATA_T, PNT_T, PLANE_T) \
template void RansacDetectPlanes<DATA_T, PNT_T, PLANE_T>( \
  const std::vector<PNT_T> &pnt, \
  std::vector< std::pair<uint32_t, PLANE_T> > &detected_planes, \
  const DATA_T dist_thresh, \
  const uint32_t min_inliers, \
  std::vector< std::vector<uint32_t> > &plane_inliers);
RansacDetectPlanes_EXPLICIT_INST(float, vertex3f_t, plane4f_t)

}

#endif // __MIO_RANSAC_PLANES_H__

