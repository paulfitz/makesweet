#ifndef PIXER_INC
#define PIXER_INC

#include <yarp/sig/Image.h>

class Pixer {
public:
  double r;
  double g;
  double b;
  double a;

  Pixer operator+(const Pixer& alt) {
    Pixer p;
    p.r = r+alt.r;
    p.g = g+alt.g;
    p.b = b+alt.b;
    p.a = a+alt.a;
    return p;
  }

  void operator+=(const yarp::sig::PixelBgra& pix) {
    r += pix.r;
    g += pix.g;
    b += pix.b;
    a += pix.a;
  }

  Pixer operator*(double v) {
    Pixer p;
    p.r = r*v;
    p.g = g*v;
    p.b = b*v;
    p.a = a*v;
    return p;
  }

  Pixer operator/(double v) {
    Pixer p;
    p.r = r/v;
    p.g = g/v;
    p.b = b/v;
    p.a = a/v;
    return p;
  }
};

inline Pixer sampleLinear(const yarp::sig::ImageOf<yarp::sig::PixelBgra>& src, 
			  double x, double y) {
  const yarp::sig::PixelBgra& p = src.safePixel(x,y);
  Pixer v;
  v.r = p.r;
  v.g = p.g;
  v.b = p.b;
  v.a = p.a;
  return v;
}

#endif
