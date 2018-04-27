#include "Renders.h"

#include <stdio.h>
#include <stdlib.h>

using namespace std;

void Renders::check() {
  if (!repo) {
    fprintf(stderr,"No repository!\n");
    exit(1);
  }
  if (!inputs) {
    fprintf(stderr,"No inputs!\n");
    exit(1);
  }
}


Render *Renders::get_render(int index) {
  check();
  map<int,Render>::iterator it = renders.find(index);
  if (it == renders.end()) {
    Render& render = renders[index] = Render();
    render.attach_mapping(repo->get_mapping(index));
    render.apply_scaled(*inputs,w,h);
    int rct = (int)renders.size();
    if (rct>peak_cache_count) peak_cache_count = rct;
    return &render;
  }

  return &(it->second);
}

void Renders::remove_render(int index) {
  map<int,Render>::iterator it = renders.find(index);
  if (it!=renders.end()) {
    renders.erase(it);
  }
}


bool Renders::auto_zoom() {
  check();
  Render render;
  render.attach_mapping(repo->get_mapping(0));
  return render.auto_zoom(*inputs);
}
