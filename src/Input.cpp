#include "Input.h"
#include "Dbg.h"

#include <stdio.h>
#include <stdlib.h>

using namespace std;
using namespace yarp::os;

bool Input::load(const char *fname) {
  Filer filer;
  dbg_printf("Loading %s\n", fname);
  if (!filer.load(fname,src)) {
    fprintf(stderr, "Failed to load %s\n", fname);
    exit(1);
  }
  src.safePixel(-1,-1).a = 0;
  in_scale = src.width();
  in_x0 = 0;
  in_y0 = 0;
  if (src.height()>src.width()) {
    in_scale = src.height();
    in_x0 = -(-src.width()+src.height())/2;
  } else {
    in_y0 = -(src.width()-src.height())/2;
  }
  xa = cos(theta);
  ya = sin(theta);
  return true;
}
