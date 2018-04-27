#ifndef MAPPING_INC
#define MAPPING_INC

#include <yarp/sig/Image.h>
#include <string>

class Mapping {
public:
  yarp::sig::ImageOf<yarp::sig::PixelBgra> map1, map2, light, dark, neutral;
  std::string map1_name, map2_name, light_name, dark_name, neutral_name;

  // scale factor for interpreting map
  int scale;

  Mapping() {
    scale = 1;
  }
};

#endif
