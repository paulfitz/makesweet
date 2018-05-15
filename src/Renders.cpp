#include "Renders.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef MAKESWEET_USE_OPENCV
#include <opencv2/imgproc/imgproc.hpp>
#endif

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

class Obs {
public:
  int nin;
  int nout;
  float q;
};

class Tweak {
public:
  float scale;
  float x;
  float y;

  Tweak() {
    scale = 1.0;
    x = y = 0;
  }
};

bool is_active(const yarp::sig::PixelBgra& pix) {
  return pix.a > 25 && (pix.r < 250 || pix.g < 250 || pix.b < 250);
}

#ifdef MAKESWEET_USE_OPENCV
Obs tweakOne(const yarp::sig::ImageOf<yarp::sig::PixelBgra>& img,
             float x0, float y0, float x1, float y1, float dx, float dy,
             cv::Subdiv2D subdiv, float scale, float sx, float sy) {
  int w = img.width();
  int h = img.height();
  Obs obs;
  obs.nin = obs.nout = 0;
  cv::Point2f pto, ptd;
  IMGFOR(img, x, y) {
    // speed up a bit - for precision remove this line
    if (x % 4 != 0 || x % 4 != 0) continue;

    if (is_active(img(x, y))) {
      int edge=0, vertex=0;
      float xx = dx + sx + (((x - w/2)*scale) + w/2);
      float yy = dy + sy + (((y - h/2)*scale) + h/2);
      bool dud = false;
      if (xx < x0 || xx > x1|| yy < y0 || yy > y1) {
        dud = true;
        edge = 0;
      } else {
        subdiv.locate(cv::Point2f(xx, yy), edge, vertex);
      }
      if (edge != 0) {
        for (int i=0; i<=3; i++) {
          subdiv.edgeDst(edge, &ptd);
          subdiv.edgeOrg(edge, &pto);
          if (ptd.x < x0 || ptd.x > x1 || pto.x < x0 || pto.x > x1 ||
              ptd.y < y0 || ptd.y > y1 || pto.y < y0 || pto.y > y1) {
            dud = true;
          }
          edge = subdiv.getEdge(edge, cv::Subdiv2D::NEXT_AROUND_LEFT);
        }
      }
      if (dud) {
        obs.nout++;
      } else {
        obs.nin++;
      }
    }
  }
  if (obs.nout == 0) {
    obs.q = 1.0;
  } else  {
    obs.q = obs.nin / float(obs.nin + obs.nout);
  }
  return obs;
}

Tweak tweak(const yarp::sig::ImageOf<yarp::sig::PixelBgra>& img,
            float x0, float y0, float x1, float y1, float dx, float dy,
            float tx, float ty,
            cv::Subdiv2D subdiv) {
  bool first = true;
  float ix0 = 0, iy0 = 0, ix1 = 0, iy1 = 0;
  int actives = 0;
  IMGFOR(img, x, y) {
    if (is_active(img(x, y))) {
      actives++;
      if (first) {
        ix0 = ix1 = x;
        iy0 = iy1 = y;
        first = false;
      } else {
        if (x > ix1) ix1 = x;
        if (x < ix0) ix0 = x;
        if (y > iy1) iy1 = y;
        if (y < iy0) iy0 = y;
      }
    }
  }
  if (actives >= img.width() * img.height() * 0.9) {
    printf("Skipping opaque image\n");
    return Tweak();
  }
  float ix = (ix0 + ix1) / 2;
  float iy = (iy0 + iy1) / 2;
  float scale = 0.1;
  float q = 1.0;
  float good_scale = scale;
  float sx = - (ix - (tx - dx));
  float sy = - (iy - (ty - dy));
  sx = 0; // constrain x offset, not interesting at this point
  float lo = 0.1, hi = 3.0;
  while (fabs(lo-hi) > 0.01) {
    float curr = (lo+hi)/2;
    Obs obs = tweakOne(img, x0, y0, x1, y1, dx, dy, subdiv, curr, sx, sy);
    printf("in/out %d %d q %g scale %g\n", obs.nin, obs.nout, obs.q, curr);
    q = obs.q;
    if (q > 0.9999) {
      if (curr > good_scale) good_scale = curr;
      lo = curr;
    } else {
      hi = curr;
    }
  }
  printf("Good scale: %g\n", good_scale);
  Tweak t;
  t.scale = good_scale;
  t.x += sx;
  t.y += sy;
  return t;
}

void Renders::scan(int frame) {
  if (frame < 0) frame = 0;
  check();
  if (inputs->get().size() == 0) { return; }
  int ins = inputs->get().size();
  for (int k=0; k<ins; k++) {
    printf("Scan input %d frame %d\n", k, frame);
    Render render;
    render.attach_mapping(repo->get_mapping(frame));
    std::vector<CloudPoint> pts;
    Input& in = inputs->get_mod()[k];
    render.getCloud(in, pts);
    std::vector<cv::Point2f> pts1;
  
    bool first = true;
    float x0 = 0, y0 = 0, x1 = 0, y1 = 0;
    float tx = 0, ty = 0;
    int ct = 0;
    for (std::vector<CloudPoint>::iterator it=pts.begin(); it!=pts.end(); it++) {
      if (it->layer != in.layer) continue;
      if (first) {
        x0 = x1 = it->x;
        y0 = y1 = it->y;
        first = false;
      } else {
        if (it->x > x1) x1 = it->x;
        if (it->x < x0) x0 = it->x;
        if (it->y > y1) y1 = it->y;
        if (it->y < y0) y0 = it->y;
      }
      tx += it->x;
      ty += it->y;
      ct++;
      pts1.push_back(cv::Point2f(it->x, it->y));
    }
    if (ct > 0) {
      tx /= ct;
      ty /= ct;
    }
    tx = (x0 + x1) / 2;
    ty = (y0 + y1) / 2;
    printf("I have %d cloud points\n", (int)pts.size());
    printf("I have %d inputs\n", (int)inputs->get().size());
    cv::Subdiv2D subdiv;
    int del = 100;
    float dx = x0 + (x1 - x0);
    float dy = y0 + (y1 - y0);
    x1 += dx;
    y1 += dy;
    x0 += dx;
    y0 += dy;
    tx += dx;
    ty += dy;
    for (std::vector<cv::Point2f>::iterator it=pts1.begin(); it!=pts1.end(); it++) {
      it->x += dx;
      it->y += dy;
    }
    subdiv.initDelaunay(cv::Rect(x0 - 5, y0 - 5, x1 - x0 + 11, y1 - y0 + 11));
    printf("Window %g %g %g %g\n", x0, y0, x1, y1);
    for (std::vector<cv::Point2f>::iterator it=pts1.begin(); it!=pts1.end(); it++) {
      subdiv.insert(cv::Point2f(it->x, it->y));
    }

    const yarp::sig::ImageOf<yarp::sig::PixelBgra>& img = in.get();
    Tweak t = tweak(img, x0, y0, x1, y1, dx, dy, tx, ty, subdiv);
    printf("adjusting scale by factor: %g\n", t.scale);
    in.xs /= t.scale;
    in.ys /= t.scale;
    in.in_x0 -= t.x;
    in.in_y0 -= t.y;
  }
}

#else
void Renders::scan(int frame) {
  printf("Scan skipped, needs OpenCV\n");
}
#endif

bool Renders::auto_zoom() {
  check();
  Render render;
  render.attach_mapping(repo->get_mapping(0));
  return render.auto_zoom(*inputs);
}
