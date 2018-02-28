#ifndef __MIO_RANSAC_PLANES_INL_H__
#define __MIO_RANSAC_PLANES_INL_H__

// Detect Planes in 3D space via RANSAC


template <typename PNT_T, typename PLANE_T>
void PlaneFit(const std::vector<PNT_T> &kDataVec,
              const std::vector<uint32_t> &kUseIndexVec,
              std::vector<PLANE_T> &model_vec){
  model_vec.resize(1);
  sm::PlaneCoefFromPnt<PNT_T, PLANE_T>(kDataVec[kUseIndexVec[0]], kDataVec[kUseIndexVec[1]], 
                                       kDataVec[kUseIndexVec[2]], model_vec[0]);
}


template <typename DATA_T, typename PNT_T, typename PLANE_T>
void PlaneDistance(const std::vector<PNT_T> &kDataVec,
                   const std::vector<PLANE_T> &kTestModelVec,
                   const DATA_T kMinInlierDist,
                   uint32_t &best_model_idx,
                   std::vector<uint32_t> &inlier_index_vec){ // out
  best_model_idx = 0;
  inlier_index_vec.clear();
  if( kTestModelVec.empty() || kDataVec.empty() )
    return;
  inlier_index_vec.reserve(100); // TODO - 100 is arbitrary
  for(size_t i = 0, allDataSize = kDataVec.size(); i < allDataSize; ++i){
    const DATA_T d = sm::DistToPlane<PLANE_T, PNT_T>(kTestModelVec[0], kDataVec[i]);
    if(d < kMinInlierDist)
      inlier_index_vec.push_back(i);
  }
}


// Return "true" if the selected points are a degenerate (invalid) case.
template <typename PNT_T>
bool PlaneDegenerate(const std::vector<PNT_T> &kDataVec,
                     const std::vector<uint32_t> &kUseIndexVec){
  return false;
}


template <typename DATA_T, typename PNT_T, typename PLANE_T>
void RansacDetectPlanes(const std::vector<PNT_T> &kPntVec,
                        std::vector< std::pair<uint32_t, PLANE_T> > &detected_planes, // out
                        const DATA_T kMinInlierDist,
                        const uint32_t kMinNumInlier,
                        std::vector< std::vector<uint32_t> > &inlier_index_vvec){ // out
  detected_planes.clear();
  if(kPntVec.empty())
    return;
  // The running lists of remaining points after each plane, as a matrix
  std::vector<PNT_T> remaining_pnt_vec = kPntVec;
  inlier_index_vvec.clear();
  const size_t kPntVecSize = kPntVec.size();
  std::vector<uint32_t> index_vec(kPntVecSize);
  for(size_t i = 0; i < kPntVecSize; ++i)
    index_vec[i] = i;
  bool is_first_iter = true;

  while(remaining_pnt_vec.size() >= 3){
    std::vector<uint32_t>	best_inlier_vec;
    PLANE_T best_model;

    CRansac<DATA_T, PNT_T, PLANE_T>::execute(remaining_pnt_vec,
                                             PlaneFit,
                                             PlaneDistance,
                                             PlaneDegenerate,
                                             kMinInlierDist,
                                             3, // Minimum set of points
                                             best_inlier_vec,
                                             best_model);
    // Check fit robustness
    if(best_inlier_vec.size() >= kMinNumInlier){
      // Add this plane to the output list
      detected_planes.push_back(std::make_pair<uint32_t, PLANE_T>(best_inlier_vec.size(), PLANE_T(best_model)));
      if(is_first_iter){
        inlier_index_vvec.push_back(best_inlier_vec);
        is_first_iter = false;
      }
      else{
        std::vector<uint32_t> temp(best_inlier_vec.size());
        const size_t kTempSize = temp.size();
        for(size_t i = 0; i < kTempSize; ++i)
          temp[i] = index_vec[best_inlier_vec[i]];
        inlier_index_vvec.push_back(temp);
      }
      // Normalize coefifients
      PLANE_T &plane = detected_planes.rbegin()->second;
      DATA_T inv_l2_norm = 1.0f / std::sqrt(plane.a*plane.a + plane.b*plane.b + plane.c*plane.c + plane.d*plane.d);
      plane.a *= inv_l2_norm;
      plane.b *= inv_l2_norm;
      plane.c *= inv_l2_norm;
      plane.d *= inv_l2_norm;
      // Discard the selected points so they are not used again for finding subsequent planes
      remaining_pnt_vec = mio::EraseIndices<PNT_T>(remaining_pnt_vec, best_inlier_vec);
      index_vec = mio::EraseIndices<uint32_t>(index_vec, best_inlier_vec);
    }
    else
      break; // Do not search for more planes
  }
}

#endif // __MIO_RANSAC_PLANES_INL_H__

