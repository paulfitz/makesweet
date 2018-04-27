#include <stdio.h>

#include <yarp/sig/Image.h>
#include <yarp/sig/ImageFile.h>
#include <yarp/sig/ImageDraw.h>

using namespace yarp::sig;
using namespace yarp::sig::file;

typedef ImageOf<PixelMono> Matrix;

Matrix getDither2x2() {
  Matrix m;
  m.resize(2,2);
  m(0,0) = 0;
  m(1,0) = 2;
  m(0,1) = 3;
  m(1,1) = 1;
  return m;
}

Matrix doubleDither(const Matrix& a) {
  Matrix b;
  int w = a.width();
  b.resize(w*2,w*2);
  for (int i=0; i<2; i++) {
    for (int j=0; j<2; j++) {
      for (int ii=0; ii<w; ii++) {
	for (int jj=0; jj<w; jj++) {
	  b(ii+i*w,jj+j*w) = a(ii,jj)*4 + a(i*(w/2),j*(w/2));
	}
      }
    }
  }
  return b;
}

/*
Matrix directDither(int c) {
  int w = 1;
  for (int i=0; i<c; i++) {
    w*=2;
  }
  Matrix m;
  m.resize(w,w);
  int r = w/2;
  int rr = r;
  m(0,0) = 0;
  m(r,0) = 2;
  m(0,r) = 3;
  m(r,r) = 1;
  for (int k=1; k<c; k++) {
    for (int i=0; i<2; i++) {
      for (int j=0; j<2; j++) {
	for (int ii=0; ii<2; ii++) {
	  for (int jj=0; jj<2; jj++) {
	    if (ii+jj>0) {
	      m(ii*r/2+i*r,jj*r/2+j*r) = 
		m(ii*r,jj*r)*4 + a(i*(w/2),j*(w/2));
	    }
	  }
	}
      }
    }
    r /= 2;
  }
}
*/

void codeDither(const Matrix& m) {
  printf("static private var rawDither : Array<Int> = [ ");
  for (int i=0; i<m.width(); i++) {
    for (int j=0; j<m.height(); j++) {
      int v = m(i,j);
      v = int(((1.0+v)/(1.0+m.width()*m.width()))*256.0);
      printf("%s0x%02x%02x%02x%02x",
	     (j>0||i>0)?", ":"",
	     255, v, v, v);
    }
  }
  printf(" ];\n");
  printf("static private var rawDitherDim : Int = %d;\n", m.width());
}

void codeDither2(const Matrix& m) {
  printf("static int raw_dither[%d][%d] = { ", m.width(), m.height());
  for (int i=0; i<m.width(); i++) {
    if (i>0) printf(", ");
    printf("{");
    for (int j=0; j<m.height(); j++) {
      int v = m(i,j);
      v = int(((1.0+v)/(1.0+m.width()*m.width()))*256.0);
      printf("%s%d",
	     (j>0)?", ":"",
	     v);
    }
    printf("}");
  }
  printf(" };\n");
  printf("static int raw_dither_dim = %d;\n", m.width());
}

void showDither(const Matrix& m) {
  for (int i=0; i<m.width(); i++) {
    for (int j=0; j<m.height(); j++) {
      printf("%d ", m(i,j));
    }
    printf("\n");
  }
  codeDither(m);
  codeDither2(m);
}

void dithers() {
  Matrix d2 = getDither2x2();
  showDither(d2);
  Matrix d4 = doubleDither(d2);
  showDither(d4);
  Matrix d8 = doubleDither(d4);
  showDither(d8);
  Matrix d16 = doubleDither(d8);
  showDither(d16);
}

int c2p(int c,float tweak) {
  int v = int(6*(c/256.0+tweak)+0.5);
  if (v<0) return 0;
  if (v>=6) return 5;
  return v;
}

int p2c(int p) {
  return 51*p;
}

int rgb2pal(const PixelRgb& pix,float tweak) {
  int v = 36*c2p(pix.r,tweak) + 6*c2p(pix.g,tweak) + c2p(pix.b,tweak);
}

void pal2rgb(int pal, PixelRgb& pix) {
  int bb = pal%6;
  pal /= 6;
  int gg = pal%6;
  pal /= 6;
  int rr = pal;
  pix.r = p2c(rr);
  pix.g = p2c(gg);
  pix.b = p2c(bb);
}

void dither(ImageOf<PixelRgb>& src, ImageOf<PixelRgb>& dest) {
  Matrix d = doubleDither(getDither2x2());
  //Matrix d = doubleDither(doubleDither(getDither2x2()));
  //Matrix d = doubleDither(doubleDither(doubleDither(getDither2x2())));
  showDither(d);
  write(d,"dither.ppm");
  int dd = d.width();

  dest.resize(src);
  IMGFOR(src,x,y) {
    int dx = x%dd;
    int dy = y%dd;
    int dith = d(dx,dy);

    int pal = rgb2pal(src(x,y),(1.0+dith)/(1.0+dd*dd)-0.5);
    pal2rgb(pal,dest(x,y));
    //dest(x,y).r = x%256;
    //dest(x,y).g = y%256;
    //dest(x,y).b = (x+y)%256;
  }
}

int main(int argc, char *argv[]) {
  dithers();
  return 1;

  if (argc<2) {
    printf("Need an in file, out file\n");
    return 1;
  }

  char *input_name = argv[1];
  char *output_name = argv[2];
  printf("Loading %s\n", input_name);
  ImageOf<PixelRgb> input, output;
  bool ok = read(input,input_name);
  if (!ok) {
    printf("Failed to load\n");
    return 1;
  }
  dither(input,output);
  printf("Writing %s\n", output_name);
  write(output,output_name);

  return 0;
}
