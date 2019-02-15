#ifndef MAPPING_INC
#define MAPPING_INC

#include <yarp/sig/Image.h>
#include <string>
#include <map>
#include <vector>
#include "Filer.h"

struct Sample {
  float fx;
  float fy;
  float factor;
};

class Summary {
public:
  std::vector<Sample> samples;
};

class Summaries {
public:
  int offset(int x, int y) const {
    return 10000 * y + x;
  }

  std::map<int, Summary> summaries;
};

class Mapping {
public:
  yarp::sig::ImageOf<yarp::sig::PixelBgra> map1, map2, light, dark, neutral;
  std::string map1_name, map2_name, light_name, dark_name, neutral_name;
  Summaries sum;
  bool have_sum;

  // scale factor for interpreting map
  int scale;

  Mapping() {
    scale = 1;
    have_sum = false;
  }

  void load_samples(const char *name, Filer& filer);
};

#endif
