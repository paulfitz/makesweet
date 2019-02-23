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

  void preblend() {
    r *= a;
    g *= a;
    b *= a;
  }

  void postblend(double sc) {
    r /= sc;
    g /= sc;
    b /= sc;
  }
};

inline Pixer sampleWeakly(const yarp::sig::ImageOf<yarp::sig::PixelBgra>& src, 
			  double x, double y) {
  const yarp::sig::PixelBgra& p = src.safePixel(x,y);
  Pixer v;
  v.r = p.r;
  v.g = p.g;
  v.b = p.b;
  v.a = p.a;
  return v;
}

inline Pixer sampleLinear(const yarp::sig::ImageOf<yarp::sig::PixelBgra>& src, 
			  double x, double y) {
  int xx = (int)x;
  int yy = (int)y;
  float fx = x - xx;
  float fy = y - yy;
  const yarp::sig::PixelBgra& poo = src.safePixel(x,y);
  const yarp::sig::PixelBgra& poy = src.safePixel(x,y+1);
  const yarp::sig::PixelBgra& pxo = src.safePixel(x+1,y);
  const yarp::sig::PixelBgra& pxy = src.safePixel(x+1,y+1);
  Pixer v;
  double aa = (1-fx)*(1-fy)*poo.a + fx*(1-fy)*pxo.a + (1-fx)*fy*poy.a + fx*fy*pxy.a;
  if (aa < 0.0001) aa = 0.0001;
  v.r =
    ((1-fx)*(1-fy)*poo.r*poo.a +
     fx*(1-fy)*pxo.r*pxo.a +
     (1-fx)*fy*poy.r*poy.a +
     fx*fy*pxy.r*pxy.a)/aa;
  v.g =
    ((1-fx)*(1-fy)*poo.g*poo.a +
     fx*(1-fy)*pxo.g*pxo.a +
     (1-fx)*fy*poy.g*poy.a +
     fx*fy*pxy.g*pxy.a)/aa;
  v.b =
    ((1-fx)*(1-fy)*poo.b*poo.a +
     fx*(1-fy)*pxo.b*pxo.a +
     (1-fx)*fy*poy.b*poy.a +
     fx*fy*pxy.b*pxy.a)/aa;
  v.a =
    (1-fx)*(1-fy)*poo.a +
    fx*(1-fy)*pxo.a +
    (1-fx)*fy*poy.a +
    fx*fy*pxy.a;
  return v;
}

#endif
