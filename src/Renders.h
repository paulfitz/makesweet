#ifndef RENDERS_INC
#define RENDERS_INC

#include "Inputs.h"
#include "Repository.h"
#include "Render.h"

class Renders {
private:
  Inputs *inputs;
  Repository *repo;
  std::map<int,Render> renders;
  int w, h;
  int peak_cache_count;

public:
  Renders() {
    inputs = 0 /*NULL*/;
    repo = 0 /*NULL*/;
    w = h = -1;
    peak_cache_count = 0;
  }

  void check();

  void set_size(int w, int h) {
    this->w = w;
    this->h = h;
  }

  void attach_repository(Repository *repo) {
    this->repo = repo;
  }

  void attach_inputs(Inputs *inputs) {
    this->inputs = inputs;
  }

  bool auto_zoom();

  Render *get_render(int index = 0);

  void remove_render(int index);

  void remove_mapping(int index) {
    repo->remove_mapping(index);
  }

  int length() {
    return repo->length();
  }

  int peak() {
    return peak_cache_count;
  }
};

#endif
