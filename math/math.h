#ifndef __MIO_MATH_H__
#define __MIO_MATH_H__

#include <cmath>
#include <stdio.h>
#include <cfloat>

namespace sm{
  #define PI 3.14159265

  //wavenumbers (also called reciprocal centimeters, inverse centimeters or  Kaisers), cm^{-1}
  template<typename T> inline T WavenumberToMicrometer(const T &wavenumber){ return(10000/wavenumber); }
  template<> inline float WavenumberToMicrometer(const float &wavenumber){ return(10000.0f/wavenumber); }
  template<> inline double WavenumberToMicrometer(const double &wavenumber){ return(10000.0/wavenumber); }

  template<typename T> inline T WavenumberToNanometer(const T &wavenumber){ return(10000000/wavenumber); }
  template<> inline float WavenumberToNanometer(const float &wavenumber){ return(10000000.0f/wavenumber); }
  template<> inline double WavenumberToNanometer(const double &wavenumber){ return(10000000.0/wavenumber); }

  //a == b for float point numbers. eg. epsilon = 0.0001
  inline bool FloatComp(const float &a, const float &b, const float &epsilon){
      return( std::fabs(a - b) < epsilon );
  }
  
  inline bool FloatComp(const double &a, const double &b, const double &epsilon){
      return( std::fabs(a - b) < epsilon );
  }

  template <typename T> 
  inline float VerL2Norm2(const T &a){ //L2-norm
    return std::sqrt(a.x * a.x + a.y * a.y);
  }

  template <typename T> 
  inline float VerL2Norm3(const T &a){ //L2-norm
    return std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
  }

  //returns floating vertex value of the Euclidean distance between vertices 'a' and 'b'
  template <typename T> 
  inline float VerDist2(const T &a, const T &b){
    return std::sqrt( (b.x - a.x) * (b.x - a.x) +
                      (b.y - a.y) * (b.y - a.y) );
  }

  //returns floating vertex value of the Euclidean distance between 3D vertices 'a' and 'b'
  template <typename T> 
  inline float VerDist3(const T a, const T b){
    return std::sqrt( (b.x - a.x) * (b.x - a.x) + 
                      (b.y - a.y) * (b.y - a.y) + 
                      (b.z - a.z) * (b.z - a.z) );
  }

  //returns squared floating vertex value of the Euclidean distance between vertices 'a' and 'b'
  template <typename T> 
  inline float SqrVerDist2(const T &a, const T &b){
    return( (b.x - a.x) * (b.x - a.x) + 
            (b.y - a.y) * (b.y - a.y) );
  }

  //returns squared floating vertex value of the Euclidean distance 3D between vertices 'a' and 'b'
  template <typename T> 
  inline float SqrVerDist3(const T &a, const T &b){
    return( (b.x - a.x) * (b.x - a.x) + 
            (b.y - a.y) * (b.y - a.y) + 
            (b.z - a.z) * (b.z - a.z) );
  }

  template <typename T> 
  inline void VerNormalize2(const T &src, T &dst){
    const float mag_inv = 1.0f / sm::VerL2Norm2<T>(src);
    dst.x = src.x * mag_inv;
    dst.y = src.y * mag_inv;
  }

  template <typename T> 
  inline void VerNormalize3(const T &src, T &dst){
    const float mag_inv = 1.0f / sm::VerL2Norm3<T>(src);
    dst.x = src.x * mag_inv;
    dst.y = src.y * mag_inv;
    dst.z = src.z * mag_inv;
  }

  //rotate a vertex about the origin by theta radians (postive theta rotates clockwise)
  template <typename T> 
  inline T RotatePnt2(const T &a, const float &theta){
    T result( a.x * std::cos(theta) - a.y * std::sin(theta), 
              a.x * std::sin(theta) + a.y * std::cos(theta) );
    return result;
  }

  //rotate vertex 'a' about vertex 'origin' by theta radians (positive theta rotates clockwise)
  template <typename T> 
  inline T RotatePnt2(const T &origin, const T &a, const float &theta){
    T result;
    T ver = sm::RotatePnt2<T>( T( a.x - origin.x, a.y - origin.y ), theta );
    result.x = ver.x + origin.x;
    result.y = ver.y + origin.y;
    return result;
  }


  //returns false if distance from vertices 'a' to 'b' is greater than 'limit'
  template <typename T> 
  inline bool CheckDist2(const T &a, const T &b, const float &limit){
    if(sm::VerDist2(a, b) > limit)
      return false;
    else
      return true;
  }

  //returns false if distance from vertices 'a' to 'b' is greater than 'limit'
  template <typename T> 
  inline bool CheckDist3(const T &a, const T &b, const float &limit){
    if(sm::VerDist3(a, b) > limit)
      return false;
    else
      return true;
  }


  template <typename T>
  inline T RadToDeg(const T &radian){
    return (radian * 57.2957795131);
  }

  template <typename T>
  inline T DegToRad(const T &degree){
    return (degree * 0.01745329251);
  }

  //compte the halfway vector. vectors a and b have arbitrary length.
  template <typename T>
  inline T HalfwayVector2(const T &a, const T &b){
    const T c(a.x+b.x, a.y+b.y);
    const double l_inv = 1.0/VerL2Norm2(c);
    return T( (a.x+b.x)*l_inv, (a.y+b.y)*l_inv );
  }

  //compte the halfway vector. vectors a and b have arbitrary length
  template <typename T>
  inline T HalfwayVector3(const T &a, const T &b){
    const T c(a.x+b.x, a.y+b.y, a.z+b.z);
    const double l_inv = 1.0/VerL2Norm3(c);
    return T( (a.x+b.x)*l_inv, (a.y+b.y)*l_inv, (a.z+b.z)*l_inv );
  }

  //compte the halfway vector. vectors a and b have unit length (are normalized).
  template <typename T>
  inline T HalfwayVectorNorm2(const T &a, const T &b){
    return T( (a.x+b.x)*0.5, (a.y+b.y)*0.5 );
  }

  //compte the halfway vector. vectors a and b have unit length (are normalized).
  template <typename T>
  inline T HalfwayVectorNorm3(const T &a, const T &b){
    return T( (a.x+b.x)*0.5, (a.y+b.y)*0.5, (a.z+b.z)*0.5 );
  }


  /*
    returns an angle translated from a rotated-inversed system to the standard right-handed cartesian system
    angle - the angle in secondary system to be converted
    phase - angle from the right-handed cartesian frames positive x-axis
            to the secondary systems zero-degrees angle
    inverse - true if secondary cooridinate system rotates clockwise from 0 to 360 degrees
  */
  template <typename T> 
  inline T SysRotDeg(T angle, T phase, const bool inverse){
    while( angle < 0 ) angle += 360;
    while( angle > 360 ) angle -= 360;

    while( phase < 0 ) phase += 360;
    while( phase > 360 ) phase -= 360;

    if(inverse)
      return( ( (360 + phase - angle) < 360 ) ? (360 + phase - angle) : (phase - angle) );
    return( ( (phase + angle) > 360 ) ? (phase + angle - 360) : (phase + angle) );
  }

  /*
    same as above, but in radians
  */
  template <typename T> 
  inline T SysRotRad(T angle, T phase, const bool inverse){
    float temp = 2.0f * PI;
    while( angle < 0 ) angle += temp;
    while( angle > temp ) angle -= temp;

    while( phase < 0 ) phase += temp;
    while( phase > temp ) phase -= temp;

    if(inverse)
      return( ( (temp + phase - angle) < temp ) ? (temp + phase - angle) : (phase - angle) );
    return( ( (phase + angle) > temp ) ? (phase + angle - temp) : (phase + angle) );
  }

  /*
    returns angle in radians of vector drawn from point a to point b.
    from PI to 2PI values returned by atan2 actually run from -PI to 0.
    this is changed to standard PI to 2PI (personnal preference)
  */
  template <typename T> 
  inline T VerAngleRad2(const T &a, const T &b){
    float theta = std::atan2( b.y - a.y, b.x - a.x );
    return( (theta < 0) ? ( (2.0f * PI) + theta ) : theta );
  }

  /*
    returns angle in radians of vector drawn from point a to point b.
    from PI to 2PI values returned by atan2 actually run from -PI to 0.
    this is changed to standard PI to 2PI (personnal preference)
  */
  template <typename T> 
  inline T VerAngleRad2(const float &x1, const float &y1, const float &x2, const float &y2){
    float theta = std::atan2( y2 - y1, x2 - x1 );
    return( (theta < 0) ? ( (2.0f * PI) + theta ) : theta );
  }

  /*
    returns angle in degrees of vector drawn from point a to b point
  */
  template <typename T> 
  inline T VerAngleDeg2(const T &a, const T &b){
    return cvFastArctan( b.y - a.y, b.x - a.x );
  }


  /*
    returns difference in moving from angle_1 to angle_2, CLOCKWISE rotations are NEGATIVE valued
  */
  template <typename T>
  inline T AngleDiff(const T angle_1, const T angle_2){
    float diff = angle_2 - angle_1;
    while(diff < -180) diff += 360;
    while(diff > 180) diff -= 360;

    return diff;
  }

  /*
    returns difference in moving from angle1 to angle2, CLOCKWISE rotations are NEGATIVE valued
  */
//  inline int angleDiff(const int nAngle1, const int nAngle2){
//    return( ( (nAngle2 - nAngle1 + 180) % 360 ) - 180 );
//  }

  /*
    given two vectors drawn from the vertex origin to there respective ends (vertices 'a' and 'b'),
    returns NEGATIVE value if rotation from vector 'a' to vector 'b' is CLOCKWISE
  */
  template <typename T> 
  inline T VerDir2(const T &origin, const T &a, const T &b){
    return( ( (a.x - origin.x) * (b.y - origin.y) ) - ( (a.y - origin.y) * (b.x - origin.x) ) );
  }

  /*
    given two vectors drawn from the vertex origin to there respective ends (vertices 'a' and 'b'),
    returns magnitude of the angle (in radians) between two vectors
  */
  template <typename T> 
  inline T VerAngle2(const T &origin, const T &a, const T &b){
    a.x -= origin.x;
    a.y -= origin.y;
    b.x -= origin.x;
    b.y -= origin.y;
    return std::acos( (a.x*b.x + a.y*b.y) /
                      ( std::sqrt(a.x*a.x + a.y*a.y) * std::sqrt(b.x*b.x + b.y*b.y) ) );
  }


  /*
    given three vertices a, b, and c, the functions returns the radius of a circle that lies on the vertices.
    Compute length of each side of the triangle. Find the circumscribed circle around the triangle.
    if turn from a -> b -> c is to LEFT the returned value is NEGATIVE
  */
  template <typename PointType> 
  inline PointType RadiusFromPnt2(const PointType &a, const PointType &b, const PointType &c){
    double A, B, C, S, K;

    A = sm::VerDist2(a, b);
    B = sm::VerDist2(b, c);
    C = sm::VerDist2(a, c);

    S = 0.5f * (A + B + C);
    K = std::sqrt(S * (S - A) * (S - B) * (S - C));

    if( sm::VerDir2(a, b, c) < 0.0 )
      return( (A * B * C) / (4.0 * K) );
    return( (A * B * C) / (-4.0 * K) ); //TODO - inf radius?
  }

  /*
    Given three vertices that lie on the perimeter of a circle, the function returns the center of the circle
  */
  template <typename PointType> 
  inline PointType CenterFromPnt2(const PointType a, const PointType b, const PointType c){
    PointType result;
    double s = 0.5 * ( (b.x - c.x)*(a.x - c.x) - (b.y - c.y)*(c.y - a.y) );
    double su = (a.x - b.x)*(c.y - a.y) - (b.y - a.y)*(a.x - c.x);

    if(su != 0){
      s /= su;
      result.x = 0.5 * (a.x + b.x) + s * (b.y - a.y);
      result.y = 0.5 * (a.y + b.y) + s * (a.x - b.x);
      return result;
    }
    result.x = result.y = NULL;
    return result;
  }

  /*
    Given two arc vertices, 'a' and 'b', considered to be two adjacent vertices lying on an arc,
    this function returns the location of the next vertex (vertex 'c') in the direction of a -> b -> c,
    in the arc given the radius. a negative radius means arc turns to the left, positive turns to right
  */
  template <typename PointType> 
  inline PointType NextArcVer2(const PointType a, const PointType b, const double radius){
    PointType c;
    double length = sm::VerDist2(a, b);
    if( length < (2.0 * std::fabs(radius) ) ){
      double theta = verAngle_rad(a, b) + ( 2.0 * ( (PI * 0.5) - std::acos( length / (2.0 * radius) ) ) );
      c.x = b.x + length*std::cos(theta);
      c.y = b.y + length*std::sin(theta);
    
      return c;
    }
    else{
      printf("sm::NextArcVer2() - the distance from vertex a to b is > (2 * radius)\n");
      c.x = c.y = NULL;
      return c;
    }
  }


  /*
    returns true if vertex 'Pnt' lies on the line segment that runs from 'a' to 'b'.
    returns false otherwise.
  */
  template <typename T> 
  inline bool OnSegment2(const T &a, const T &b, const T &pt){
    return ( ( std::min(a.x, b.x) <= pt.x && pt.x <= std::max(a.x, b.x) ) && 
             ( std::min(a.y, b.y) <= pt.y && pt.y <= std::max(a.y, b.y) ) );
  }

  /*
    returns true if the line segment drawn from 'a' to 'b' intersects with the line segment
    from drawn from 'c' to 'd', returns false otherwise.
  */
  template <typename T> 
  inline bool SegmentIntersect2(const T &a, const T &b, const T &c, const T &d){
    if( ( (sm::VerDir2(c, d, a) > 0 && sm::VerDir2(c, d, b) < 0) ||
          (sm::VerDir2(c, d, a) < 0 && sm::VerDir2(c, d, b) > 0) ) &&
        ( (sm::VerDir2(a, b, c) > 0 && sm::VerDir2(a, b, d) < 0) ||
          (sm::VerDir2(a, b, c) < 0 && sm::VerDir2(a, b, d) > 0) ) )
      return(true);
    else if( (sm::VerDir2(c, d, a) == 0) && sm::OnSegment2(c, d, a) )
      return(true);
    else if( (sm::VerDir2(c, d, b) == 0) && sm::OnSegment2(c, d, b) )
      return(true);
    else if( (sm::VerDir2(a, b, c) == 0) && sm::OnSegment2(a, b, c) )
      return(true);
    else if( (sm::VerDir2(a, b, d) == 0) && sm::OnSegment2(a, b, d) )
      return(true);
    else
      return(false);
  }

  /*
    adapted from Darel Rex Finley, 2006: http://alienryderflex.com/intersect/
    Determines the intersection vertex of the line segment defined by vertices A and B
    with the line segment defined by vertices C and D.
  */
  template <typename T> 
  inline T SegmentIntersection2(const T &a, const T &b, const T &c, const T &d){
    float dist_ab, cos_val, sin_val, new_x, pos_ab;

    //fail if either line segment is zero-length.
    if( (a.x == b.x && a.y == b.y) || (c.x == d.x && c.y == d.y) )
      return T(NULL, NULL);

    if( (a.x == c.x && a.y == c.y) || (b.x == c.x && b.y == c.y) )
      return c;

    if( (a.x == d.x && a.y == d.y) || (b.x == d.x && b.y == d.y) )
      return d;

    //translate the system so that vertex 'a' is on the origin.
    b.x -= a.x;  b.y -= a.y;
    c.x -= a.x;  c.y -= a.y;
    d.x -= a.x;  d.y -= a.y;

    //get length of segment ab.
    dist_ab = std::sqrt(b.x*b.x + b.y*b.y);

    //rotate the system so that vertex 'b' is on the positive X axis.
    cos_val = b.x / dist_ab;            
    sin_val = b.y / dist_ab;            
    new_x = cos_val*c.x + sin_val*c.y;            
    c.y = cos_val*c.y - sin_val*c.x; 
    c.x = new_x;
    new_x = cos_val*d.x + sin_val*d.y;
    d.y  = cos_val*d.y - sin_val*d.x; 
    d.x = new_x;

    //fail if segment cd doesn't cross line ab.
    if( (c.y < 0.0 && d.y < 0.0) || (c.y >= 0.0 && d.y >= 0.0) ) 
      return T(NULL, NULL);

    //get position of the intersection vertex along line ab.
    pos_ab = d.x + ( ( (c.x - d.x) * d.y ) / (d.y - c.y) );

    //fail if segment cd crosses line ab outside of segment ab.
    if( (pos_ab < 0.) || (pos_ab > dist_ab) )
      return T(NULL, NULL);

    //translate the discovered position to line ab in the original coordinate system.
    return T(a.x + pos_ab * cos_val, a.y + pos_ab * sin_val);
  }


  template <typename T>
  inline T MidPoint2(const T &a, const T &b){
    return T( (a.x + b.x) / 2.0, (a.y + b.y) / 2.0);
  }

  template <typename CoefType, typename PointType>
  inline double DistToLine2(const CoefType &coef, const PointType &pnt){
    return( std::fabs(coef.a*pnt.x + coef.b*pnt.y + coef.c) / std::sqrt(coef.a*coef.a + coef.b*coef.b) );
  }

  template <typename CoefType, typename PointType>
  inline double EvaluatePntPlane(const CoefType &coef, const PointType &pnt){
	  return(coef.a*pnt.x + coef.b*pnt.y + coef.c*pnt.z + coef.d);
  }

  template <typename CoefType, typename PointType>
  inline double DistToPlane(const CoefType &coef, const PointType &pnt){
	  return( std::fabs(sm::EvaluatePntPlane(coef, pnt) ) / std::sqrt(coef.a*coef.a + coef.b*coef.b + coef.c*coef.c) );
  }

  //computes plane equation from 3 3D points
  template <typename PointType, typename CoefType> 
  inline void PlaneCoefFromPnt(const PointType &p1, const PointType &p2, const PointType &p3, CoefType &coef){
	  double dx1 = p2.x - p1.x,
	         dy1 = p2.y - p1.y,
	         dz1 = p2.z - p1.z,
	         dx2 = p3.x - p1.x,
	         dy2 = p3.y - p1.y,
	         dz2 = p3.z - p1.z;
	  coef.a = dy1*dz2 - dy2*dz1;
	  coef.b = dz1*dx2 - dz2*dx1;
	  coef.c = dx1*dy2 - dx2*dy1;
	  //ERR_CHK(abs(coef[0]) < geometryEpsilon && abs(coef[1]) < geometryEpsilon && abs(coef[2]) < geometryEpsilon,
            //"getPlane() - points are linearly dependent", return)
	  coef.d =- coef.a*p1.x - coef.b*p1.y - coef.c*p1.z;
  }

  template <typename CoefType, typename PointType>
  inline void NormalVecFromPlaneCoef(const CoefType &coef, PointType &normal_vec, const bool normalize = true){
    normal_vec.x = coef.a;
    normal_vec.y = coef.b;
    normal_vec.z = coef.c;
    if(normalize)
      VerNormalize3(normal_vec);
  }

  template <typename PointType, typename CoefType>
  inline void LineCoefFromPnt2(const PointType &p1, const PointType &p2, CoefType &coef){
	  //ERR_CHK(p1 == p2, "getLine() - p1 == p2, points are equal", return);
	  coef.a = p2.y - p1.y;
	  coef.b = p1.x - p2.x;
	  coef.c = p2.x*p1.y - p2.y*p1.x;
  }

  template<typename PointType, typename LINE_T>
  inline PointType SolveLineEqn2(const LINE_T &line, const double x){
    return PointType(x, (-line.c - line.a*x) / line.b); //y = (-line.c - line.a*x) / line.b)
  }

  template<typename PointType, typename LINE_T>
  inline PointType SolveXLineEqn2(const LINE_T &line, const double y){
    return PointType((-line.c - line.b*y) / line.a, y); //x = (-line.c - line.b*x) / line.a)
  }

} //namespace sm

#endif //__MIO_MATH_H__
