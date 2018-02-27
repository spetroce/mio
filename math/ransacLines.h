#ifndef __MIO_RANSAC_LINES_H__
#define __MIO_RANSAC_LINES_H__


template <typename PNT_T, typename MODEL_T>
void LineFit2D(const std::vector<PNT_T> &all_data,
               const std::vector<uint32_t> &use_indices,
               std::vector<MODEL_T> &fit_model){
  fit_model.resize(1);
  sm::LineCoefFromPnt2<PNT_T, MODEL_T>(all_data[use_indices[0]], all_data[use_indices[1]], fit_model[0]);
}


template <typename DATA_T, typename PNT_T, typename MODEL_T>
void LineDistance2D(const std::vector<PNT_T> &all_data,
                    const std::vector<MODEL_T> &test_model,
                    const DATA_T dist_thresh,
                    uint32_t &best_model_idx,
                    std::vector<uint32_t> &inlier_indices){ // out
  best_model_idx = 0;
  inlier_indices.clear();
  if(test_model.empty() || all_data.empty())
    return;
  inlier_indices.reserve(100);
  for(size_t i = 0, allDataSize = all_data.size(); i < allDataSize; ++i){
    const DATA_T d = sm::DistToLine2<MODEL_T, PNT_T>(test_model[0], all_data[i]);
    if(d < dist_thresh)
      inlier_indices.push_back(i);
  }
}


// Return "true" if the selected points are a degenerate (invalid) case
template <typename PNT_T>
bool LineDegenerate(const std::vector<PNT_T> &all_data,
                    const std::vector<uint32_t> &use_indices){
  return false;
}


namespace sm {

template <typename DATA_T, typename PNT_T, typename LINE_T>
void RansacDetectLines2D(const std::vector<PNT_T> &pnt,
                         std::vector< std::pair<uint32_t, LINE_T> > &detected_lines,
                         const DATA_T dist_thresh,
                         const uint32_t min_inliers,
                         std::vector< std::vector<uint32_t> > &line_inliers){
  detected_lines.clear();
  if(pnt.empty())
    return;
  // The running lists of remaining points after each plane, as a matrix
  std::vector<PNT_T> remaining_pnts = pnt;
  line_inliers.clear();
  std::vector<uint32_t> indices( pnt.size() );
  for(size_t i = 0, indicesSize = indices.size(); i < indicesSize; ++i)
    indices[i] = i;
  bool first_iter_flag = true;

  // For each line
  while(remaining_pnts.size() >= 2){
    std::vector<uint32_t>	best_inliers;
    LINE_T best_model;

    CRansac<DATA_T, PNT_T, LINE_T>::execute(remaining_pnts,
                                            LineFit2D,
                                            LineDistance2D,
                                            LineDegenerate2D,
                                            dist_thresh,
                                            2, // Minimum set of points
                                            best_inliers,
                                            best_model,
                                            false, // Verbose
                                            0.99999); // Prob. of good result

    // Is this line good enough?
    if(best_inliers.size() >= min_inliers){
      // Add this line to the output list
      detected_lines.push_back(std::make_pair<uint32_t, LINE_T>(best_inliers.size(), LINE_T(best_model)));
      if(first_iter_flag){
        line_inliers.push_back(best_inliers);
        first_iter_flag = false;
      }
      else{
        std::vector<uint32_t> temp( best_inliers.size() );
        const size_t kTempSize = temp.size();
        for(size_t i = 0; i < kTempSize; ++i)
          temp[i] = indices[best_inliers[i]];
        line_inliers.push_back(temp);
      }
      // Normalize coefifients
      LINE_T &line = detected_lines.rbegin()->second;
      DATA_T inv_l2_norm = 1.0f / std::sqrt(line.a*line.a + line.b*line.b + line.c*line.c);
      line.a *= inv_l2_norm;
      line.b *= inv_l2_norm;
      line.c *= inv_l2_norm;
      // Discard the selected points so they are not used again for finding subsequent planes
      remaining_pnts = mio::EraseIndices<PNT_T>(remaining_pnts, best_inliers);
      indices = mio::EraseIndices<uint32_t>(indices, best_inliers);
    }
    else
      break; // Do not search for more planes
  }
}


// Template explicit instantiations:
#define RansacDetectLines2D_EXPLICIT_INST(DATA_T, PNT_T, LINE_T) \
template void RansacDetectLines2D<DATA_T, PNT_T, LINE_T>( \
  const std::vector<PNT_T> &pnt, \
  std::vector< std::pair<uint32_t, LINE_T> > &detected_lines, \
  const DATA_T dist_thresh, \
  const uint32_t min_inliers, \
  std::vector< std::vector<uint32_t> > &line_inliers);
RansacDetectLines2D_EXPLICIT_INST(float, vertex2f_t, line3f_t)

} // namespace sm

#endif // __MIO_RANSAC_LINES_H__

