#include "Render.h"
#include "Pixer.h"

#include <stdio.h>
#include <stdlib.h>

#include <gd.h>

#include "Dbg.h"

#include <yarp/sig/ImageFile.h>
using namespace yarp::sig::file;

using namespace yarp::sig;

#define RR (4096/2)

static int render_count = 0;

static double distance(double x1, double y1,
		       double x2, double y2) {
  return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}

int Render::render_count() {
  return ::render_count;
}

void Render::check() {
  if (!mapping) {
    fprintf(stderr,"No mapping!\n");
    exit(1);
  }
}

void Render::add(const Input& in) {
  check();
  double active_scale = in.in_scale;
  active_scale /= 2; //mapping->scale;

  int w = mapping->light.width();
  int h = mapping->light.height();

  if (mapping->map1.width()<w) {
    return;
  }

  int off = (mapping->map2.height()-mapping->light.height())/2;

  if (mapping->map1.width()!=mapping->neutral.width()) return;
  if (mapping->map2.width()!=mapping->neutral.width()) return;

  IMGFOR(out,x,y) {
    PixelBgra& pix = out(x,y);
    const PixelBgra& lightPixel = mapping->light(x,y);
    const PixelBgra& darkPixel = mapping->dark(x,y);
    const PixelBgra& mapPixel = mapping->map1(x,y+off);
    const PixelBgra& selPixel = mapping->map2(x,y+off);
    bool d = (selPixel.r==in.layer);
    if (!d) continue;
    int act = mapPixel.a;
    if (act<=25) continue;
    
    int mod = mapPixel.b;
    int ymod = mod/16;
    int xmod = mod%16;
    double x1 = mapPixel.r + 256*xmod - RR;
    double y1 = mapPixel.g + 256*ymod - RR;
      
    PixelBgra m;

    double x12 = x1;
    double y12 = y1;
    double x13 = x1;
    double y13 = y1;

    if (x<w-1&&y<h-1) {
      const PixelBgra& mdx = mapping->map1(x+1,y+off);
      const PixelBgra& mdy = mapping->map1(x,y+1+off);
      if (mdx.a>127 && mdy.a>127) {
	const PixelBgra& idx2 = mapping->map2(x+1,y+off);
	const PixelBgra& idx3 = mapping->map2(x,y+1+off);
	if (idx2.r==in.layer && idx3.r==in.layer) {
	  int mod2 = mdx.b;
	  int ymod2 = mod2/16;
	  int xmod2 = mod2%16;
	  x12 = mdx.r + 256*xmod2 - RR;
	  y12 = mdx.g + 256*ymod2 - RR;

	  int mod3 = mdy.b;
	  int ymod3 = mod3/16;
	  int xmod3 = mod3%16;
	  x13 = mdy.r + 256*xmod3 - RR;
	  y13 = mdy.g + 256*ymod3 - RR;
	  //printf("WORKING %g %g / %g %g\n", x12-x1, y12-y1,
	  //x13-x1, y13-y1);
	}

	double da = distance(x1,y1,x12,y12);
	double db = distance(x1,y1,x13,y13);
	if (da>400.0||db>400.0) {
	  x12 = x1;
	  y12 = y1;
	  x13 = x1;
	  y13 = y1;
	}
      }
    }

    x1 *= in.xs;
    y1 *= in.ys;
    double xx =  in.xa*x1 + in.ya*y1;
    double yy = -in.ya*x1 + in.xa*y1;
    xx += RR + in.xo;
    yy += RR + in.yo;
    xx = in.in_x0 + active_scale*xx/RR;
    yy = in.in_y0 + active_scale*yy/RR;
    
    x12 *= in.xs;
    y12 *= in.ys;
    double xxa =  in.xa*x12 + in.ya*y12;
    double yya = -in.ya*x12 + in.xa*y12;
    xxa += RR + in.xo;
    yya += RR + in.yo;
    xxa = in.in_x0 + active_scale*xxa/RR;
    yya = in.in_y0 + active_scale*yya/RR;
    
    x13 *= in.xs;
    y13 *= in.ys;
    double xxb =  in.xa*x13 + in.ya*y13;
    double yyb = -in.ya*x13 + in.xa*y13;
    xxb += RR + in.xo;
    yyb += RR + in.yo;
    xxb = in.in_x0 + active_scale*xxb/RR;
    yyb = in.in_y0 + active_scale*yyb/RR;
    
    xxa -= xx;
    yya -= yy;
    xxb -= xx;
    yyb -= yy;
    
    Pixer mo = sampleLinear(in.get(),xx,yy);
    
    Pixer m2 = sampleLinear(in.get(),xx+xxa/2.0,yy+yya/2.0);
    Pixer m3 = sampleLinear(in.get(),xx-xxa/2.0,yy-yya/2.0);
    Pixer m4 = sampleLinear(in.get(),xx+xxb/2.0,yy+yyb/2.0);
    Pixer m5 = sampleLinear(in.get(),xx-xxb/2.0,yy-yyb/2.0);
    
    Pixer m2b = sampleLinear(in.get(),xx+(xxa+xxb)/2.0,yy+(yya+yyb)/2.0);
    Pixer m3b = sampleLinear(in.get(),xx+(xxa-xxb)/2.0,yy+(yya-yyb)/2.0);
    Pixer m4b = sampleLinear(in.get(),xx-(xxa+xxb)/2.0,yy-(yya+yyb)/2.0);
    Pixer m5b = sampleLinear(in.get(),xx-(xxa-xxb)/2.0,yy-(yya-yyb)/2.0);
    
    mo = (mo*4.0 + (m2+m3+m4+m5)*2.0 + (m2b+m3b+m4b+m5b))/16.0;
    m.r = mo.r;
    m.g = mo.g;
    m.b = mo.b;
    m.a = mo.a;
    //} else {
    //m = in.get().safePixel((int)xx,(int)yy);
    //}

    //const PixelBgra& m = in.get().safePixel((int)xx,(int)yy);
    PixelBgra result;
    if (d && act>25) {
      result.r = darkPixel.r + ((lightPixel.r-darkPixel.r)*m.r)/255;
      result.g = darkPixel.g + ((lightPixel.g-darkPixel.g)*m.g)/255;
      result.b = darkPixel.b + ((lightPixel.b-darkPixel.b)*m.b)/255;
      result.a = m.a;
      if (darkPixel.a<result.a) {
	result.a = darkPixel.a;
      }
      if (result.a>0) {
	PixelBgra& outPixel = out.pixel(x,y);
	if (result.a>250) {
	  outPixel = result;
	} else {
	  outPixel.r += ((result.r-outPixel.r)*result.a)/255;
	  outPixel.g += ((result.g-outPixel.g)*result.a)/255;
	  outPixel.b += ((result.b-outPixel.b)*result.a)/255;
	}
      }
    }
  }
}


void Render::add_simple(const Input& in) {
  check();
  double active_scale = in.in_scale;
  active_scale /= 2; //mapping->scale;
  int off = (mapping->map2.height()-mapping->light.height())/2;

  if (mapping->map1.width()<mapping->light.width()) {
    return;
  }

  IMGFOR(out,x,y) {
    PixelBgra& pix = out(x,y);
    const PixelBgra& lightPixel = mapping->light(x,y);
    const PixelBgra& darkPixel = mapping->dark(x,y);
    const PixelBgra& mapPixel = mapping->map1(x,y+off);
    const PixelBgra& selPixel = mapping->map2(x,y+off);

    int mod = mapPixel.b;
    int ymod = mod/16;
    int xmod = mod%16;
    int act = mapPixel.a;
    double x1 = mapPixel.r + 256*xmod - RR;
    double y1 = mapPixel.g + 256*ymod - RR;
    x1 *= in.xs;
    y1 *= in.ys;
    double xx =  in.xa*x1 + in.ya*y1;
    double yy = -in.ya*x1 + in.xa*y1;
    xx += RR + in.xo;
    yy += RR + in.yo;
    xx = in.in_x0 + active_scale*xx/RR;
    yy = in.in_y0 + active_scale*yy/RR;

    const PixelBgra& m = in.get().safePixel((int)xx,(int)yy);
    PixelBgra result;
    bool d = (selPixel.r==in.layer);
    if (d && act>25) {
      result.r = darkPixel.r + ((lightPixel.r-darkPixel.r)*m.r)/255;
      result.g = darkPixel.g + ((lightPixel.g-darkPixel.g)*m.g)/255;
      result.b = darkPixel.b + ((lightPixel.b-darkPixel.b)*m.b)/255;
      result.a = m.a;
      if (darkPixel.a<result.a) {
	result.a = darkPixel.a;
      }
      if (result.a>0) {
	PixelBgra& outPixel = out.pixel(x,y);
	if (result.a>250) {
	  outPixel = result;
	} else {
	  outPixel.r += ((result.r-outPixel.r)*result.a)/255;
	  outPixel.g += ((result.g-outPixel.g)*result.a)/255;
	  outPixel.b += ((result.b-outPixel.b)*result.a)/255;
	}
      }
    }
  }
}

void Render::post() {
  ::render_count++;
  check();
  ImageOf<PixelBgra> pre = out;
  int w = out.width();
  int h = out.height();

  if (mapping->map1.width()<mapping->light.width()) {
    return;
  }

  if (mapping->map1.width()!=mapping->neutral.width()) return;
  if (mapping->map2.width()!=mapping->neutral.width()) return;

  IMGFOR(out,x,y) {
    PixelBgra& pix = out(x,y);
    const PixelBgra& selPixel = mapping->map2(x,y);
    if (selPixel.b>0) {
      /*
      pix.r = 0;
      pix.g = 0;
      pix.b = 0;
      */
      if (x>0 && y>0 && x<w-1 && y<h-1) {
	PixelBgra back1 = pre(x-1,y);
	PixelBgra idx1 = mapping->map2(x-1,y);

	PixelBgra back2 = pre(x+1,y);
	PixelBgra idx2 = mapping->map2(x+1,y);

	PixelBgra back3 = pre(x,y-1);
	PixelBgra idx3 = mapping->map2(x,y-1);

	PixelBgra back4 = pre(x,y+1);
	PixelBgra idx4 = mapping->map2(x,y+1);

	Pixer total = {0,0,0,0};
	double ct = 0;

	if (idx1.b<127) {
	  total += back1;
	  ct++;
	}

	if (idx2.b<127) {
	  total += back2;
	  ct++;
	}

	if (idx3.b<127) {
	  total += back3;
	  ct++;
	}

	if (idx4.b<127) {
	  total += back4;
	  ct++;
	}

	if (ct>0.5) {
	  total = total/ct;
	  pix.r = total.r;
	  pix.g = total.g;
	  pix.b = total.b;
	}
      }
    }
  }
}


void Render::getCloud(const Input& in, std::vector<CloudPoint>& cloud) {
  check();
  double active_scale = in.in_scale;
  active_scale /= 2;
  int off = (mapping->map2.height()-mapping->light.height())/2;

  if (mapping->map1.width()<mapping->light.width()) {
    printf("Nothing\n");
    return;
  }

  IMGFOR(mapping->light,x,y) {
    const PixelBgra& lightPixel = mapping->light(x,y);
    const PixelBgra& darkPixel = mapping->dark(x,y);
    const PixelBgra& mapPixel = mapping->map1(x,y+off);
    const PixelBgra& selPixel = mapping->map2(x,y+off);

    int mod = mapPixel.b;
    int ymod = mod/16;
    int xmod = mod%16;
    int act = mapPixel.a;
    double x1 = mapPixel.r + 256*xmod - RR;
    double y1 = mapPixel.g + 256*ymod - RR;
    x1 *= in.xs;
    y1 *= in.ys;
    double xx =  in.xa*x1 + in.ya*y1;
    double yy = -in.ya*x1 + in.xa*y1;
    xx += RR + in.xo;
    yy += RR + in.yo;
    xx = in.in_x0 + active_scale*xx/RR;
    yy = in.in_y0 + active_scale*yy/RR;
    if (selPixel.r != 0 && act>25) {
      cloud.push_back(CloudPoint(selPixel.r, xx, yy));
    }
  }
}

bool Render::auto_zoom(Input& in) {
  check();
  double active_scale = in.in_scale;
  active_scale /= 2; //mapping->scale;
  int off = (mapping->map2.height()-mapping->light.height())/2;

  if (mapping->map1.width()<mapping->light.width()) {
    return false;
  }

  double x_min = in.get().width();
  double x_max = 0;
  double y_min = in.get().height();
  double y_max = 0;

  IMGFOR(mapping->light,x,y) {
    const PixelBgra& lightPixel = mapping->light(x,y);
    const PixelBgra& darkPixel = mapping->dark(x,y);
    const PixelBgra& mapPixel = mapping->map1(x,y+off);
    const PixelBgra& selPixel = mapping->map2(x,y+off);

    int mod = mapPixel.b;
    int ymod = mod/16;
    int xmod = mod%16;
    int act = mapPixel.a;
    double x1 = mapPixel.r + 256*xmod - RR;
    double y1 = mapPixel.g + 256*ymod - RR;
    x1 *= in.xs;
    y1 *= in.ys;
    double xx =  in.xa*x1 + in.ya*y1;
    double yy = -in.ya*x1 + in.xa*y1;
    xx += RR + in.xo;
    yy += RR + in.yo;
    xx = in.in_x0 + active_scale*xx/RR;
    yy = in.in_y0 + active_scale*yy/RR;

    const PixelBgra& m = in.get().safePixel((int)xx,(int)yy);
    PixelBgra result;
    bool d = (selPixel.r==1);
    if (d && act>25) {
      if (xx<x_min) x_min = xx;
      if (xx>x_max) x_max = xx;
      if (yy<y_min) y_min = yy;
      if (yy>y_max) y_max = yy;
    }
  }
  double ww = in.get().width();
  double hh = in.get().height();
  dbg_printf("zoom %g %g %g %g (%g %g)\n", x_min, y_min, x_max, y_max, ww, hh);
  if (y_max-y_min<hh*0.75) {
    in.xs *= 2;
    in.ys *= 2;
    return true;
  }
  return false;
}


void Render::apply_scaled(const Inputs& ins, int w, int h) {
  pre();
  const std::vector<Input>& data = ins.get();
  for (std::vector<Input>::const_iterator it = data.begin();
       it != data.end(); it++) {
    add(*it);
  }
  post();
  if (w>0&&h>0 && (w!=out.width()||h!=out.height())) {
    int wi = out.width();
    int hi = out.height();

    // we have wi,hi - want to present it in w,h
    // pad with whitespace if needed to preserve aspect ratio
    double fi = (double)wi/hi;
    double f = (double)w/h;

    int xo = 0;
    int yo = 0;
    int wo = w;
    int ho = h;

    out_scaled.resize(w,h);

    if (fi>f+0.001) {
      // input is wider than output
      // pad top, bottom
      wo = w;
      ho = (int)(wo/fi);
      yo = (h-ho)/2;
    } else if (fi<f-0.001) {
      // input is thinner than output (or same)
      // pad left, right
      ho = h;
      wo = (int)(h*fi);
      xo = (w-wo)/2;
    } 

    if (xo!=0||yo!=0) {
      IMGFOR(out_scaled,x,y) {
	out_scaled(x,y) = PixelBgra(255,255,255,0);
      }
    }
    
    gdImageStruct src;
    src.trueColor = true;
    src.sx = out.width();
    src.sy = out.height();
    src.cx1 = src.cy1 = 0;
    src.cx2 = src.sx-1;
    src.cy2 = src.sy-1;
    src.trueColor = true;
    src.tpixels = (int**)out.getRowArray();

    gdImageStruct dest;
    dest.trueColor = true;
    dest.sx = out_scaled.width();
    dest.sy = out_scaled.height();
    dest.cx1 = dest.cy1 = 0;
    dest.cx2 = dest.sx-1;
    dest.cy2 = dest.sy-1;
    dest.trueColor = true;
    dest.tpixels = (int**)out_scaled.getRowArray();
    dest.saveAlphaFlag = 1;
    dest.alphaBlendingFlag = 0;
    
    gdImageCopyResampled(&dest,&src,xo,yo,0,0,wo,ho,
			 src.sx,src.sy);

    IMGFOR(out_scaled,x,y) {
      out_scaled(x,y).a = 255;
    }

    out.resize(1,1);
  }
}


bool Render::auto_zoom(Inputs& ins) {
  std::vector<Input>& data = ins.get_mod();
  bool changed = false;
  for (std::vector<Input>::iterator it = data.begin();
       it != data.end(); it++) {
    changed = changed || auto_zoom(*it);
  }
  return changed;
}



