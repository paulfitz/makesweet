#include <stdio.h>

#include <yarp/sig/Image.h>
#include <yarp/sig/ImageFile.h>
#include <yarp/sig/ImageDraw.h>

#include "msmap.h"

using namespace yarp::sig;
using namespace yarp::sig::file;


int diff(PixelRgb& p1, PixelRgb& p2) {
  return 
    (p1.r-p2.r)*(p1.r-p2.r)+
    (p1.g-p2.g)*(p1.g-p2.g)+
    (p1.b-p2.b)*(p1.b-p2.b);
}

void process(ImageOf<PixelRgb>& src, ImageOf<PixelRgb>& light, ImageOf<PixelRgb>& dark) {
  printf("Size %d %d\n", src.width(), src.height());
  msmap_enable();
  msmap_set_frame(0);
  int k = 2;
  int d = 2;
  IMGFOR(src,x,y) {
    PixelRgb& pix = src(x,y);
    if (pix.r>0||pix.g>0||pix.r>0) {
      PixelRgb& li = light(x,y);
      PixelRgb& da = dark(x,y);
      if (diff(li,da)>10) {
	msmap_set_output(x,src.height()-1-y);
	double fx = pix.r/255.0;
	double fy = pix.g/255.0;
	msmap_set_input(0,fx,1-fy);
	// smoothing
	for (int kk=0; kk<k; kk++) {
	  for (int xx=-d; xx<=d; xx++) {
	    for (int yy=-d; yy<=d; yy++) {
	      if (x+xx>=0 && y+yy>=0 && x+xx<src.width() &&
		  y+yy<src.height()) {
		msmap_set_aux(xx!=0||yy!=0);
		msmap_set_output(x+xx,src.height()-1-y-yy);
		msmap_set_input(0,fx,1-fy);
	      }
	    }
	  }
	}
      }
    }
  }
  msmap_render(src.width(),src.height());
}

int main(int argc, char *argv[]) {
  if (argc<4) {
    printf("Need an input map file, a light file, a dark file\n");
    return 1;
  }

  char *input_name = argv[1];
  char *light_name = argv[2];
  char *dark_name = argv[3];
  ImageOf<PixelRgb> input, light, dark;
  read(input,input_name);
  read(light,light_name);
  read(dark,dark_name);
  process(input,light,dark);

  return 0;
}
