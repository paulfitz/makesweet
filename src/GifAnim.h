#ifndef GIFANIM_INC
#define GIFANIM_INC

#include "Renders.h"

#include <vector>

#include <yarp/os/Bytes.h>

class GifAnim {
private:
  Renders *renders;
  std::vector<int> palette_frames;
  std::vector<yarp::os::Bytes> blocks;
  double period;
  double hold;
  bool early_release;
  int first_frame;

public:

  GifAnim() {
    renders = 0 /*NULL*/;
    period = 0.1;
    hold = 5;
    early_release = true;
    first_frame = -1;
  }

  ~GifAnim();

  void attach_renders(Renders *renders) {
    this->renders = renders;
  }

  void check();

  void set_palette(int index) {
    palette_frames.clear();
    palette_frames.push_back(index);
  }

  void set_palette(const std::vector<int>& palette) {
    palette_frames = palette;
  }

  void set_first_frame(int index) {
    first_frame = index;
  }

  void apply();

  void save(const char *fname);

  void set_timing(double period, double hold) {
    this->period = period;
    this->hold = hold;
  }
};

#endif
