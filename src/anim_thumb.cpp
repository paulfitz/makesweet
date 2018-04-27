
#include <stdio.h>
#include <stdlib.h>
#include "Filer.h"

using namespace yarp::sig;

int __ms_verbose = 0;

bool fetch_image(ImageOf<PixelRgba>& img,
		 const char *prefix,
		 const char *mode,
		 int index) {
  Filer filer;
  char buf[1000];
  snprintf(buf,sizeof(buf),"%s%d_%s.png",prefix,index,mode);
  bool ok = filer.load(buf,img);
  if (!ok) {
    printf("Failed to load %s\n", buf);
    exit(1);
  } else {
    printf("Found %s\n", buf);
  }
  return ok;
}

bool save_image(ImageOf<PixelRgba>& img,
		const char *prefix,
		const char *mode) {
  Filer filer;
  char buf[1000];
  snprintf(buf,sizeof(buf),"%sall_%s.png",prefix,mode);
  bool ok = filer.save(buf,img);
  if (!ok) {
    printf("Failed to save %s\n", buf);
    exit(1);
  } else {
    printf("Made %s\n", buf);
  }
  return ok;
}

void add(ImageOf<PixelRgba>& ilight,
	 ImageOf<PixelRgba>& olight,
	 int i,
	 int N) {
  int iw = 0;
  int ih = 0;
  int ow = 0;
  int oh = 0;
  int tw = 0;
  int th = 0;
  float twe = 0;
  float the = 0;
  iw = ilight.width();
  ih = ilight.height();
  if (iw<=1) {
    return;
  }
  tw = iw/8;
  th = ih/8;
  twe = iw/8.0;
  the = ih/8.0;
  ow = iw;
  oh = th;
  ImageOf<PixelRgba> tmp;
  tmp.copy(ilight,tw,th);
  for (int xx=0; xx<tw; xx++) {
    for (int yy=0; yy<th; yy++) {
      olight.safePixel(int(i*twe)+xx,yy) = tmp(xx,yy);
    }
  }
}

void process() {
  const char *prefix = "/tmp/pack_makesweet/thumb_";
  int N = 8;
  int iw = 0;
  int ih = 0;
  int ow = 0;
  int oh = 0;
  int tw = 0;
  int th = 0;
  float twe = 0;
  float the = 0;

  ImageOf<PixelRgba> olight, odark, omap, osel;
  for (int i=0; i<N; i++) {
    ImageOf<PixelRgba> ilight, idark, imap, isel;
    fetch_image(ilight,prefix,"light",i);
    fetch_image(idark,prefix,"dark",i);
    fetch_image(imap,prefix,"map",i);
    fetch_image(isel,prefix,"sel",i);
    if (iw==0) {
      iw = ilight.width();
      ih = ilight.height();
      tw = iw/8;
      th = ih/8;
      twe = iw/8.0;
      the = ih/8.0;
      ow = iw;
      oh = th;
      olight.resize(ow,oh);
      odark.resize(ow,oh);
      omap.resize(ow,oh);
      osel.resize(ow,oh);
      olight.zero();
      odark.zero();
      omap.zero();
      osel.zero();
    }

    add(ilight,olight,i,N);
    add(idark,odark,i,N);
    add(isel,osel,i,N);
    add(imap,omap,i,N);
  }
  save_image(olight,prefix,"light");
  save_image(odark,prefix,"dark");
  save_image(omap,prefix,"map");
  save_image(osel,prefix,"sel");
}

int main() {
  printf("Animation thumbnail\n");
  process();
  return 0;
}

