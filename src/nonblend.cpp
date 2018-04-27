#include <stdio.h>
#include <math.h>

#include <yarp/sig/Image.h>
#include <yarp/sig/ImageFile.h>
#include <yarp/sig/ImageDraw.h>

using namespace yarp::sig;
using namespace yarp::sig::file;

double punfix(int r) {
  return (r/255.0)*2.0-1;
}

int pfix(double x) {
  x = (x+1)/2;
  int r = (int)(x*255.0+0.5);
  if (r>255) r = 255;
  if (r<0) r = 0;
  return r;
}

void process2(ImageOf<PixelRgb>& dest,ImageOf<PixelRgb>& overlay) {
  int w = 512;
  dest.resize(w,w);
  double maxerr = 0.0;
  IMGFOR(dest,x,y) {
    PixelRgb& pix = dest(x,y);
    PixelRgb over;
    double fx = x/((double)w);
    double fy = y/((double)w);
    // fx = [0:1)
    // fy = [0:1)
    double theta = fx*M_PI;
    double phi = fy*M_PI*2;
    double xx = sin(theta)*cos(phi);
    double yy = sin(theta)*sin(phi);
    double zz = cos(theta);
    // xx, yy, zz: [-1:1]
    int pr = pfix(xx);
    int pg = pfix(yy);
    int pb = pfix(zz);
    pix.r = pr;
    pix.g = pg;
    pix.b = pb;
    
    // inversion
    double x2=punfix(pix.r);
    double y2=punfix(pix.g);
    double z2=punfix(pix.b);
    double len = sqrt(x2*x2+y2*y2+z2*z2);
    double phi2 = atan2(y2,x2);
    double theta2 = acos(z2/len);
    if (theta2<0) theta2 += M_PI;
    if (phi2<0) phi2 += M_PI*2;


    double theta3 = asin(sqrt(xx*xx+yy*yy));
    if (theta3<0) theta3 += M_PI;
    if (fabs(theta2-theta3)>=0.1) {
      theta3 = M_PI-theta3;
    }

    if (fabs(theta2-theta3)<0.1) {
      theta2 = (theta2+theta3)/2;
      //printf("MATCH %g : %g\n", theta2, theta3);
    } else {
      printf("Mismatch %g : %g\n", theta2, theta3);
    }

    double ex = (theta2/(M_PI))*w;
    double ey = (phi2/(M_PI*2.0))*w;
    double err = sqrt((ex-x)*(ex-x)+(ey-y)*(ey-y));
    if (x>5 && y>5 && x<w-6 && y<w-6) {
      //if (err>10) {
      //printf("err %g: %g %g / %d %d\n", err, ex, ey, x, y);
      //}
      if (err>8) {
	printf("len %g err %g: %g %g / %d %d\n", len, err, ex, ey, x, y);
      }
      if (err>maxerr) maxerr = err;
    }
  }  
  printf("Maximum error is %g\n", maxerr);
}

void process(ImageOf<PixelRgb>& dest,ImageOf<PixelRgb>& overlay) {
  int w = 512;
  dest.resize(w,w);
  IMGFOR(dest,x,y) {
    PixelRgb& pix = dest(x,y);
    PixelRgb over;
    double fx = x/((double)w-1);
    double fy = y/((double)w-1);
    pix.r = int(fx*255+0.5);
    pix.g = int(fy*255+0.5);
    if (overlay.width()>0) {
      pix.b = 255-overlay(x,y).r;
      if (pix.b==0) {
	pix.b = (pix.r%51==0||pix.g%51==0)?(((x+y)%2)?128:255):pix.b;
      }
    } else {
      pix.b = (pix.r%51==0||pix.g%51==0)?255:0;
      if (fabs(x-w/2.0)+fabs(y-w/2.0)<w/16) {
	pix.b = (y<w/2)?255:((x<w/2)?128:64);
      }
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc<2) {
    printf("Need an out file\n");
    return 1;
  }
  char *overlay_name = NULL;
  if (argc>=3) {
    overlay_name = argv[2];
  }

  char *output_name = argv[1];
  ImageOf<PixelRgb> output, overlay;
  if (overlay_name!=NULL) {
    printf("template %s\n", overlay_name);
    read(overlay,overlay_name);
  }
  process2(output,overlay);
  printf("Writing %s\n", output_name);
  write(output,output_name);

  return 0;
}
