#ifndef __MIO_RANSAC_LINES_H__
#define __MIO_RANSAC_LINES_H__

//Detect Lines in 3D space via RANSAC

template <typename PNT_T, typename MODEL_T>
void LineFit(const std::vector<PNT_T> &all_data,
             const std::vector<uint32_t> &use_indices,
             std::vector<MODEL_T> &fit_model){
  fit_model.resize(1);
  sm::LineCoefFromPnt2<PNT_T, MODEL_T>(all_data[ use_indices[0] ], all_data[ use_indices[1] ], fit_model[0]);
}


template <typename PNT_T, typename MODEL_T>
void LineDistance(const std::vector<PNT_T> &all_data,
                  const std::vector<MODEL_T> &test_model,
                  const double dist_thresh,
                  uint32_t &best_model_idx,
                  std::vector<uint32_t> &inlier_indices){ //out
  best_model_idx = 0;
  inlier_indices.clear();
  if( test_model.empty() || all_data.empty() )
    return;
  inlier_indices.reserve(100);
  for(uint32_t i = 0, allDataSize = all_data.size(); i < allDataSize; ++i){
    const double d = sm::DistToLine2<MODEL_T, PNT_T>(test_model[0], all_data[i]);
    if(d < dist_thresh)
      inlier_indices.push_back(i);
  }
}


//return "true" if the selected points are a degenerate (invalid) case.
template <typename PNT_T>
bool LineDegenerate(const std::vector<PNT_T> &all_data,
                    const std::vector<uint32_t> &use_indices){
  return false;
}


namespace sm{

template <typename PNT_T>
void RansacDetectLines(const std::vector<PNT_T> &pnt,
                       std::vector< std::pair<uint32_t, line3d_t> > &detected_lines,
                       const double threshold,
                       const uint32_t min_inliers,
                       std::vector< std::vector<uint32_t> > &line_inliers){
  detected_lines.clear();
  if( pnt.empty() )
    return;
  //the running lists of remaining points after each plane, as a matrix
  std::vector<PNT_T> remaining_pnts = pnt;
  line_inliers.clear();
  std::vector<uint32_t> indices( pnt.size() );
  for(uint32_t i = 0, indicesSize = indices.size(); i < indicesSize; ++i)
    indices[i] = i;
  bool first_iter_flag = true;

  //for each line
  while(remaining_pnts.size() >= 2){
    std::vector<uint32_t>	best_inliers;
    line3d_t best_model;

    CRansac<PNT_T, line3d_t>::execute(remaining_pnts,
                                      LineFit,
                                      LineDistance,
                                      LineDegenerate,
                                      threshold,
                                      2, //minimum set of points
                                      best_inliers,
                                      best_model,
                                      false, //verbose
                                      0.99999); //prob. of good result

    //is this plane good enough?
    if(best_inliers.size() >= min_inliers){
      //Add this plane to the output list
      detected_lines.push_back( std::make_pair<uint32_t, line3d_t>( best_inliers.size(), line3d_t(best_model) ) );
      if(first_iter_flag){
        line_inliers.push_back(best_inliers);
        first_iter_flag = false;
      }
      else{
        std::vector<uint32_t> temp( best_inliers.size() );
        for(uint32_t i = 0, tempSize = temp.size(); i < tempSize; ++i)
          temp[i] = indices[ best_inliers[i] ];
        line_inliers.push_back(temp);
      }
      //normalize coefifients
      line3d_t &line = detected_lines.rbegin()->second;
      double inv_l2_norm = 1.0f / std::sqrt(line.a*line.a + line.b*line.b + line.c*line.c);
      line.a *= inv_l2_norm;
      line.b *= inv_l2_norm;
      line.c *= inv_l2_norm;
      //discard the selected points so they are not used again for finding subsequent planes
      remaining_pnts = mio::EraseIndices<PNT_T>(remaining_pnts, best_inliers);
      indices = mio::EraseIndices<uint32_t>(indices, best_inliers);
    }
    else
      break; //do not search for more planes
  }
}


//template explicit instantiations:
#define RansacDetectLines_EXPLICIT_INST(_TYPE_) \
template void RansacDetectLines<_TYPE_>(const std::vector<_TYPE_> &pnt, \
                                        std::vector< std::pair<uint32_t, line3d_t> > &detected_lines, \
                                        const double threshold, \
                                        const uint32_t min_inliers, \
                                        std::vector< std::vector<uint32_t> > &line_inliers);
RansacDetectLines_EXPLICIT_INST(vertex2f_t)
RansacDetectLines_EXPLICIT_INST(cv::Point2f)

}

#endif //__MIO_RANSAC_LINES_H__

