#include "VidAnim.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <yarp/sig/Image.h>
using namespace yarp::sig;
using namespace yarp::os;

#include <opencv2/highgui/highgui.hpp>
 
using namespace std;
using namespace cv;

#include "Dbg.h"

VidAnim::~VidAnim() {
}

void VidAnim::check() {
  if (!renders) {
    fprintf(stderr,"No renders for VidAnim!\n");
    exit(1);
  }
}

void VidAnim::apply(const char *fname) {
  int loops = 0;
  bool have_prev = false;

  // period and hold available in seconds

  check();

  int frames = renders->length();

  int rate = 30;
  float frame_rate = 1.0/rate;

  float curr = 0;
  float target = -1;
  int base = 0;

  Render *r = renders->get_render(0);

  // for some reason filename / container needs to be avi?
  VideoWriter video(fname, CV_FOURCC('M','P','4','V'), rate, Size(r->get().width(),
                                                                  r->get().height()));
  Mat m;
  ImageOf<PixelBgr> v1;
  while (true) {
    int i = (base + ((first_frame >= 0) ? first_frame : 0)) % frames;
    if (curr >= target) {
      if (base >= frames) break;
      float step = period;
      if (i == frames - 1) {
        step += hold;
      }
      target = curr + step;
      Render *r = renders->get_render(i);
      v1.copy(r->get());
      renders->remove_render(i);
      m = cvarrToMat(static_cast<const IplImage*>(v1.getIplImage()));
      base++;
    }
    video.write(m);
    printf("Rendered: %d of %d frames\n", base, frames);
    fflush(stdout);
    curr += frame_rate;
  }
  video.release();
}
