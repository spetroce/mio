#include <string>
#include <sstream>
#include <fstream>

#include "mio/altro/types.h"
#include "mio/math/ransac.h"


// Will read in a single line of numbers separated by spaces from a file. The x values are incremented for each number.
// ransac lines is run on these points.
int main(int argc, char *argv[]){
  std::fstream in_file(argv[1], std::ios_base::in);
  float x = 0, y;
  std::vector<vertex2f_t> pnt_vec;
  while(in_file >> y)
    pnt_vec.push_back(vertex2f_t(x++, y));
  std::cout << "read " << pnt_vec.size() << " points\n";

  std::vector< std::pair<uint32_t, line3f_t> > detected_lines;
  const float kMinInlierDist = 15;
  const uint32_t kMinNumInlier = 400;
  std::vector< std::vector<uint32_t> > inlier_index_vvec;

  sm::RansacDetectLines2D(pnt_vec, detected_lines, kMinInlierDist, kMinNumInlier, inlier_index_vvec);

  for(auto line_pair : detected_lines){
    std::cout << line_pair.first << ", " << line_pair.second << std::endl;
    std::cout << sm::SolveLineEqn2<float, vertex2f_t, line3f_t>(line_pair.second, pnt_vec.front().x) << std::endl <<
                 sm::SolveLineEqn2<float, vertex2f_t, line3f_t>(line_pair.second, pnt_vec.back().x) << std::endl;
    std::cout << "=(" << -line_pair.second.c << " - " << line_pair.second.a << "*A1) / " << line_pair.second.b << std::endl;
  }

  if(detected_lines.size() < 2)
    return 0;

  auto a = sm::SolveLineEqn2<float, vertex2f_t, line3f_t>(detected_lines[0].second, pnt_vec.front().x);
  auto b = sm::SolveLineEqn2<float, vertex2f_t, line3f_t>(detected_lines[0].second, pnt_vec.back().x);
  auto c = sm::SolveLineEqn2<float, vertex2f_t, line3f_t>(detected_lines[1].second, pnt_vec.front().x);
  auto d = sm::SolveLineEqn2<float, vertex2f_t, line3f_t>(detected_lines[1].second, pnt_vec.back().x);

  auto intersect = sm::SegmentIntersection2(a, b, c, d);
  std::cout << "intersect: " << intersect << std::endl;

  const size_t start_idx = intersect.x;
  if(start_idx < 0 || start_idx > pnt_vec.size()-1-150)
    return 0;
  std::vector<vertex2f_t> sub_pnt_vec(pnt_vec.begin()+start_idx, pnt_vec.begin()+start_idx+150);

  detected_lines.clear();
  inlier_index_vvec.clear();
  sm::RansacDetectLines2D(sub_pnt_vec, detected_lines, kMinInlierDist, 40, inlier_index_vvec);
  for(auto line_pair : detected_lines){
    std::cout << line_pair.first << ", " << line_pair.second << std::endl;
    std::cout << sm::SolveLineEqn2<float, vertex2f_t, line3f_t>(line_pair.second, pnt_vec.front().x) << std::endl <<
                 sm::SolveLineEqn2<float, vertex2f_t, line3f_t>(line_pair.second, pnt_vec.back().x) << std::endl;
    std::cout << "=(" << -line_pair.second.c << " - " << line_pair.second.a << "*A1) / " << line_pair.second.b << std::endl;
  }

  c = sm::SolveLineEqn2<float, vertex2f_t, line3f_t>(detected_lines[0].second, pnt_vec.front().x);
  d = sm::SolveLineEqn2<float, vertex2f_t, line3f_t>(detected_lines[0].second, pnt_vec.back().x);

  intersect = sm::SegmentIntersection2(a, b, c, d);
  std::cout << "intersect: " << intersect << std::endl;

  return 0;
}

