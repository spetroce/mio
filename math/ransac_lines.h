#ifndef __MIO_RANSAC_LINES_INL_H__
#define __MIO_RANSAC_LINES_INL_H__


template <typename PNT_T, typename MODEL_T>
void LineFit2D(const std::vector<PNT_T> &kDataVec,
               const std::vector<uint32_t> &kUseIndexVec,
               std::vector<MODEL_T> &model_vec){
  model_vec.resize(1);
  sm::LineCoefFromPnt2<PNT_T, MODEL_T>(kDataVec[kUseIndexVec[0]], kDataVec[kUseIndexVec[1]], model_vec[0]);
}


template <typename DATA_T, typename PNT_T, typename MODEL_T>
void LineDistance2D(const std::vector<PNT_T> &kDataVec,
                    const std::vector<MODEL_T> &kTestModelVec,
                    const DATA_T kMinInlierDist,
                    uint32_t &best_model_idx,
                    std::vector<uint32_t> &inlier_index_vec){ // out
  best_model_idx = 0;
  inlier_index_vec.clear();
  if(kTestModelVec.empty() || kDataVec.empty())
    return;
  inlier_index_vec.reserve(100);
  for(size_t i = 0, allDataSize = kDataVec.size(); i < allDataSize; ++i){
    const DATA_T d = sm::DistToLine2<MODEL_T, PNT_T>(kTestModelVec[0], kDataVec[i]);
    if(d < kMinInlierDist)
      inlier_index_vec.push_back(i);
  }
}


// Return "true" if the selected points are a degenerate (invalid) case
template <typename PNT_T>
bool LineDegenerate2D(const std::vector<PNT_T> &kDataVec,
                      const std::vector<uint32_t> &kUseIndexVec){
  return false;
}


template <typename DATA_T, typename PNT_T, typename LINE_T>
void RansacDetectLines2D(const std::vector<PNT_T> &kPntVec,
                         std::vector< std::pair<uint32_t, LINE_T> > &detected_lines,
                         const DATA_T kMinInlierDist,
                         const uint32_t kMinNumInlier,
                         std::vector< std::vector<uint32_t> > &inlier_index_vvec){
  detected_lines.clear();
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

  // For each line
  while(remaining_pnt_vec.size() >= 2){
    std::vector<uint32_t>	best_inlier_vec;
    LINE_T best_model;

    CRansac<DATA_T, PNT_T, LINE_T>::execute(remaining_pnt_vec,
                                            LineFit2D,
                                            LineDistance2D,
                                            LineDegenerate2D,
                                            kMinInlierDist,
                                            2, // Minimum set of points
                                            best_inlier_vec,
                                            best_model);

    // Is this line good enough?
    if(best_inlier_vec.size() >= kMinNumInlier){
      // Add this line to the output list
      detected_lines.push_back(std::make_pair<uint32_t, LINE_T>(best_inlier_vec.size(), LINE_T(best_model)));
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
      LINE_T &line = detected_lines.rbegin()->second;
      DATA_T inv_l2_norm = 1.0f / std::sqrt(line.a*line.a + line.b*line.b + line.c*line.c);
      line.a *= inv_l2_norm;
      line.b *= inv_l2_norm;
      line.c *= inv_l2_norm;
      // Discard the selected points so they are not used again for finding subsequent planes
      remaining_pnt_vec = mio::EraseIndices<PNT_T>(remaining_pnt_vec, best_inlier_vec);
      index_vec = mio::EraseIndices<uint32_t>(index_vec, best_inlier_vec);
    }
    else
      break; // Do not search for more planes
  }
}

#endif // __MIO_RANSAC_LINES_INL_H__

