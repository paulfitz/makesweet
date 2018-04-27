#ifndef REPOSITORY_INC
#define REPOSITORY_INC

#include <yarp/os/Property.h>
#include "Mapping.h"
#include "Filer.h"

#include <map>
#include <vector>

class Repository {
private:
  yarp::os::Property inventory;
  yarp::os::Property config;
  std::map<int,Mapping> mappings;
  Filer filer;
  bool flag_animation;
  int frames;
  int peak_cache_count;

public:
  Repository() {
    flag_animation = false;
    frames = 0;
    peak_cache_count = 0;
  }

  void load_part(const char *postfix, int idx);

  void load(const char *fname);

  Mapping *get_mapping(int index = 0);

  Mapping *get_thumb_mapping() {
    return get_mapping(-2);
  }

  void remove_mapping(int index);

  bool is_animation() {
    return flag_animation;
  }
  
  int length() {
    return frames;
  }

  std::vector<int> get_palette();

  double get_period();

  double get_hold();

  double get_zoom();

  int peak() {
    return peak_cache_count;
  }
};

#endif
