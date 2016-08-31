#ifndef __MIO_RANSAC_PLANES_H__
#define __MIO_RANSAC_PLANES_H__

//Detect Planes in 3D space via RANSAC

#include "mio/math/math.h"
#include "mio/altro/algorithm.h"
#include "opencv2/core.hpp"

template <typename PNT_T, typename MODEL_T>
void PlaneFit(const std::vector<PNT_T> &all_data,
              const std::vector<uint32_t> &use_indices,
              std::vector<MODEL_T> &fit_model){
  fit_model.resize(1);
  sm::PlaneCoefFromPnt<PNT_T, MODEL_T>(all_data[ use_indices[0] ], all_data[ use_indices[1] ], 
                                       all_data[ use_indices[2] ], fit_model[0]);
}


template <typename PNT_T, typename MODEL_T>
void PlaneDistance(const std::vector<PNT_T> &all_data,
                   const std::vector<MODEL_T> &test_model,
                   const double dist_thresh,
                   uint32_t &best_model_idx,
                   std::vector<uint32_t> &inlier_indices){ //out
  best_model_idx = 0;
  inlier_indices.clear();
  if( test_model.empty() || all_data.empty() )
    return;
  inlier_indices.reserve(100); //TODO - 100 is arbitrary
  for(size_t i = 0, allDataSize = all_data.size(); i < allDataSize; ++i){
    const double d = sm::DistToPlane<MODEL_T, PNT_T>(test_model[0], all_data[i]);
    if(d < dist_thresh)
      inlier_indices.push_back(i);
  }
}


//return "true" if the selected points are a degenerate (invalid) case.
template <typename PNT_T>
bool PlaneDegenerate(const std::vector<PNT_T> &all_data,
                     const std::vector<uint32_t> &use_indices){
  return false;
}


namespace sm{

template <typename PNT_T>
void RansacDetectPlanes(const std::vector<PNT_T> &pnt,
                        std::vector< std::pair<uint32_t, plane4d_t> > &detected_planes, //out
                        const double dist_thresh,
                        const uint32_t min_inliers,
                        std::vector< std::vector<uint32_t> > &plane_inliers){ //out
  detected_planes.clear();
  if( pnt.empty() )
    return;
  //the running lists of remaining points after each plane, as a matrix
  std::vector<PNT_T> remaining_pnts = pnt;
  plane_inliers.clear();
  std::vector<uint32_t> indices( pnt.size() );
  for(size_t i = 0, indicesSize = indices.size(); i < indicesSize; ++i)
    indices[i] = i;
  bool first_iter_flag = true;

  while(remaining_pnts.size() >= 3){
    std::vector<uint32_t>	best_inliers;
    plane4d_t best_model;

    CRansac<PNT_T, plane4d_t>::execute(remaining_pnts,
                                       PlaneFit,
                                       PlaneDistance,
                                       PlaneDegenerate,
                                       dist_thresh,
                                       3,  //minimum set of points
                                       best_inliers,
                                       best_model,
                                       false, //verbose
                                       0.999); //prob. of good result
    //check fit robustness
    if(best_inliers.size() >= min_inliers){
      //add this plane to the output list
      detected_planes.push_back( std::make_pair<uint32_t, plane4d_t>( best_inliers.size(), plane4d_t(best_model) ) );
      if(first_iter_flag){
        plane_inliers.push_back(best_inliers);
        first_iter_flag = false;
      }
      else{
        std::vector<uint32_t> temp( best_inliers.size() );
        const size_t kTempSize = temp.size();
        for(size_t i = 0; i < kTempSize; ++i)
          temp[i] = indices[ best_inliers[i] ];
        plane_inliers.push_back(temp);
      }
      //normalize coefifients
      plane4d_t &plane = detected_planes.rbegin()->second;
      double inv_l2_norm = 1.0f / std::sqrt(plane.a*plane.a + plane.b*plane.b + plane.c*plane.c + plane.d*plane.d);
      plane.a *= inv_l2_norm;
      plane.b *= inv_l2_norm;
      plane.c *= inv_l2_norm;
      plane.d *= inv_l2_norm;
      //discard the selected points so they are not used again for finding subsequent planes
      remaining_pnts = mio::EraseIndices<PNT_T>(remaining_pnts, best_inliers);
      indices = mio::EraseIndices<uint32_t>(indices, best_inliers);
    }
    else
      break; //do not search for more planes
  }
}


//template explicit instantiations:
#define RansacDetectPlanes_EXPLICIT_INST(PNT_TYPE) \
template void RansacDetectPlanes<PNT_TYPE>(const std::vector<PNT_TYPE> &pnt,\
                                           std::vector< std::pair<uint32_t, plane4d_t> > &detected_planes,\
                                           const double dist_thresh,\
                                           const uint32_t min_inliers,\
                                         std::vector< std::vector<uint32_t> > &plane_inliers);
RansacDetectPlanes_EXPLICIT_INST(vertex3f_t)
RansacDetectPlanes_EXPLICIT_INST(cv::Point3f)

}

#endif //__MIO_RANSAC_PLANES_H__

