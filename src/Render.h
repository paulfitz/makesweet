#ifndef RENDER_INC
#define RENDER_INC

#include "Mapping.h"
#include "Filer.h"
#include "Input.h"
#include "Inputs.h"

#include <vector>

class CloudPoint {
public:
  int layer;
  float x, y;

  CloudPoint(int layer, float x, float y) {
    this->layer = layer;
    this->x = x;
    this->y = y;
  }
};

class Render {
private:
  const Mapping *mapping;
  yarp::sig::ImageOf<yarp::sig::PixelBgra> out;
  yarp::sig::ImageOf<yarp::sig::PixelBgra> out_scaled;
  std::string quality;

  // active variables

public:
  Render(const std::string& nquality = "none") {
    mapping = 0 /*NULL*/;
    quality = nquality;
  }

  void check();

  void attach_mapping(const Mapping *mapping) {
    if (!mapping) return;
    this->mapping = mapping;
  }

  void pre() {
    check();
    out.setQuantum(1);
    out.copy(this->mapping->neutral);
  }
  
  void add_simple(const Input& in);

  void add(const Input& in);

  void add_sampled(const Input& in);

  void apply(const Inputs& ins) {
    apply_scaled(ins,-1,-1);
  }

  void apply_scaled(const Inputs& ins, int w, int h);

  void getCloud(const Input& in, std::vector<CloudPoint>& cloud);

  bool auto_zoom(Inputs& ins);

  bool auto_zoom(Input& in);

  void post();

  bool save(const char *fname) {
    Filer filer;
    filer.save(fname,get_mod());
    return true;
  }

  const yarp::sig::ImageOf<yarp::sig::PixelBgra>& get() { 
    return get_mod(); 
  }

  yarp::sig::ImageOf<yarp::sig::PixelBgra>& get_mod() { 
    if (out_scaled.width()>0) {
      return out_scaled;
    }
    return out; 
  }

  static int render_count();
};



#endif
