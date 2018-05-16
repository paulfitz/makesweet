#ifndef VIDANIM_INC
#define VIDANIM_INC

#include "Renders.h"

#include <vector>

// #include <yarp/os/Bytes.h>

class VidAnim {
private:
  Renders *renders;
  double period;
  double hold;
  int first_frame;

public:

  VidAnim() {
    renders = 0 /*NULL*/;
    period = 0.1;
    hold = 5;
    first_frame = -1;
  }

  ~VidAnim();

  void attach_renders(Renders *renders) {
    this->renders = renders;
  }

  void check();

  void set_first_frame(int index) {
    first_frame = index;
  }

  void apply(const char *fname);

  void set_timing(double period, double hold) {
    this->period = period;
    this->hold = hold;
  }
};

#endif
