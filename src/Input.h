#ifndef INPUT_INC
#define INPUT_INC

#include <math.h>

#include <yarp/sig/Image.h>

#include "Filer.h"

class Input {
private:
  yarp::sig::ImageOf<yarp::sig::PixelBgra> src;
  double theta;

public:

  const yarp::sig::ImageOf<yarp::sig::PixelBgra>& get() const { return src; }
  
  // scale, angle, and offset in source coordinates
  double xs;
  double ys;
  double xo;
  double yo;

  int layer;

  int in_scale;
  double in_x0;
  double in_y0;

  double xa;
  double ya;

  Input() {
    xs = ys = 1;
    theta = 0;
    xo = yo = 0;
    layer = 1;
    in_scale = 0;
    in_x0 = in_y0 = 0;
    xa = 1;
    ya = 0;
  }

  bool load(const char *fname);
};


#endif

