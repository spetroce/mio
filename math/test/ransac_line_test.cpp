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
  const float kMinInlierDist = 10;
  const uint32_t kMinNumInlier = 75;
  std::vector< std::vector<uint32_t> > inlier_index_vvec;

  sm::RansacDetectLines2D(pnt_vec, detected_lines, kMinInlierDist, kMinNumInlier, inlier_index_vvec);

  for(auto line_pair : detected_lines)
    std::cout << line_pair.first << ", " << line_pair.second << std::endl;

  return 0;
}

