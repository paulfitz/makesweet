#include "GifAnim.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gd.h>
#include "gd_mod.h"

#include <yarp/sig/Image.h>
using namespace yarp::sig;
using namespace yarp::os;

#include "Dbg.h"

static int raw_dither[16][16] = { {0, 192, 48, 240, 12, 204, 60, 252, 3, 195, 51, 243, 15, 207, 63, 255}, {128, 64, 176, 112, 140, 76, 188, 124, 131, 67, 179, 115, 143, 79, 191, 127}, {32, 224, 16, 208, 44, 236, 28, 220, 35, 227, 19, 211, 47, 239, 31, 223}, {160, 96, 144, 80, 172, 108, 156, 92, 163, 99, 147, 83, 175, 111, 159, 95}, {8, 200, 56, 248, 4, 196, 52, 244, 11, 203, 59, 251, 7, 199, 55, 247}, {136, 72, 184, 120, 132, 68, 180, 116, 139, 75, 187, 123, 135, 71, 183, 119}, {40, 232, 24, 216, 36, 228, 20, 212, 43, 235, 27, 219, 39, 231, 23, 215}, {168, 104, 152, 88, 164, 100, 148, 84, 171, 107, 155, 91, 167, 103, 151, 87}, {2, 194, 50, 242, 14, 206, 62, 254, 1, 193, 49, 241, 13, 205, 61, 253}, {130, 66, 178, 114, 142, 78, 190, 126, 129, 65, 177, 113, 141, 77, 189, 125}, {34, 226, 18, 210, 46, 238, 30, 222, 33, 225, 17, 209, 45, 237, 29, 221}, {162, 98, 146, 82, 174, 110, 158, 94, 161, 97, 145, 81, 173, 109, 157, 93}, {10, 202, 58, 250, 6, 198, 54, 246, 9, 201, 57, 249, 5, 197, 53, 245}, {138, 74, 186, 122, 134, 70, 182, 118, 137, 73, 185, 121, 133, 69, 181, 117}, {42, 234, 26, 218, 38, 230, 22, 214, 41, 233, 25, 217, 37, 229, 21, 213}, {170, 106, 154, 90, 166, 102, 150, 86, 169, 105, 153, 89, 165, 101, 149, 85} };

static int raw_dither_dim = 16;

GifAnim::~GifAnim() {
  for (int i=0; i<(int)blocks.size(); i++) {
    gdFree(blocks[i].get());
  }
  blocks.clear();
}

void GifAnim::check() {
  if (!renders) {
    fprintf(stderr,"No renders for GifAnim!\n");
    exit(1);
  }
}

static void sync_palette(gdImagePtr from, gdImagePtr to) {
  for (int i = 0; i<gdMaxColors; i++) {
    to->red[i] = from->red[i];
    to->blue[i] = from->blue[i];
    to->green[i] = from->green[i];
    to->alpha[i] = from->alpha[i];
    to->open[i] = from->open[i];
  }
  to->colorsTotal = from->colorsTotal;
}

inline int crop(int x) {
  if (x>255) return 255;
  return x;
}

void GifAnim::apply() {
  bool local_color = false;
  int loops = 0;
  bool have_prev = false;
  bool dither = true;

  int step = int(period*100+0.5);
  int last_step = int(hold*100+0.5);
  dbg_printf("Step is %d (from %g)\n", step, period);

  check();
  int pals = (int)palette_frames.size();
  int pal_index = -1;
  if (pals<1) {
    fprintf(stderr,"No palette frames\n");
    exit(1);
  }
  ImageOf<PixelBgra> pal_image;
  ImageOf<PixelBgra> *p_pal_image = &pal_image;
  for (int i=0; i<pals; i++) {
    int idx = palette_frames[i];
    Render *r = renders->get_render(idx);
    renders->remove_mapping(idx);
    if (pals==1) {
      p_pal_image = &r->get_mod();
      pal_index = idx;
    } else {
      const ImageOf<PixelBgra>& img = r->get();
      pal_image.resize(img);
      int w = pal_image.width();
      int h = pal_image.height();
      for (int k=i; k<w*h; k+=pals) {
	int xx = k%w;
	int yy = k/w;
	pal_image(xx,yy) = img(xx,yy);
      }
    }
  }

  int ww = p_pal_image->width();
  int hh = p_pal_image->height();

  gdImageStruct gd_wrap;
  gdImagePtr im_tmp;
  im_tmp = gdImageCreateTrueColor(ww,hh);
  gd_wrap = *im_tmp;
  gdImageDestroy(im_tmp);
  gdImageSaveAlpha(&gd_wrap, 1);
  gdImageAlphaBlending(&gd_wrap, 0);

  gdImagePtr gd_palette = gdImageCreate(ww,hh);
  gd_wrap.tpixels = (int**)p_pal_image->getRowArray();

  double tot = 0;
  IMGFOR(*p_pal_image,ww,hh) {
    PixelBgra pix = p_pal_image->pixel(ww,hh);
    tot += pix.r;
  }

  void *quantizer = pf_gdQuantizeInit(&gd_wrap,gd_palette);
  pf_gdQuantizeApply(quantizer,&gd_wrap,1,gd_palette);

  gdImagePtr gd_prev = gdImageCreate(ww,hh);
  gdImagePtr gd_curr = gdImageCreate(ww,hh);
  gdImagePtr gd_pre_prev = gdImageCreate(ww,hh);

  sync_palette(gd_palette,gd_prev);
  sync_palette(gd_palette,gd_curr);
  sync_palette(gd_palette,gd_pre_prev);

  int frames = renders->length();

  int sz = 0;
  void *mem = gdImageGifAnimBeginPtr(gd_palette,&sz,
				     1,
				     loops);
  blocks.push_back(Bytes((char*)mem,(size_t)sz));

  int accum_delay = 0;
  //  bool have_r = false;
  //Render *r_prev = NULL;
  //Render *r_pre_prev = NULL;
  int emit_ct = 0;
  int step_pending = 0;
  for (int i=0; i<frames; i++) {
    step_pending += step;
    dbg_printf("Working on frame %d\n", i);

    if (i!=pal_index) {
      Render *r = renders->get_render(i);
      renders->remove_mapping(i);
      ImageOf<PixelBgra>& img = r->get_mod();
      gd_wrap.tpixels = (int**)img.getRowArray();
      pf_gdQuantizeApply(quantizer,&gd_wrap,1,gd_curr);
    } else {
      dbg_printf("Substitute in palette frame\n");
      gdImagePtr tmp = gd_palette;
      gd_palette = gd_curr;
      gd_curr = tmp;
    }
    renders->remove_render(i);

    bool change = false;
    if (i>0) {
      for (int x=0; x<ww && !change; x++) {
	for (int y=0; y<hh; y++) {
	  int p0 = gd_prev->pixels[y][x];
	  int p1 = gd_curr->pixels[y][x];
	  if (p0!=p1) {
	    change = true;
	    break;
	  }
	}
      }
    }

    if (change) {
      mem = gdImageGifAnimAddPtr(gd_prev,&sz,
				 local_color?1:0,
				 0,0,
				 step_pending,1,
				 (emit_ct>=1)?gd_pre_prev:NULL);
      dbg_printf("  Generated frame, %d ticks\n", step_pending);
      step_pending = 0;
      emit_ct++;
      blocks.push_back(Bytes((char*)mem,(size_t)sz));
    }

    if (change) {
      //r_pre_prev = r_prev;
      gdImagePtr tmp = gd_pre_prev;
      gd_pre_prev = gd_prev;
      gd_prev = gd_curr;
      gd_curr = tmp;
    } else {
      gdImagePtr tmp = gd_prev;
      gd_prev = gd_curr;
      gd_curr = tmp;
    }
    //r_prev = r;

    // ...
    //renders->remove_render(i);
    //renders->remove_mapping(i);
  }

  step_pending += step;
  step_pending += last_step;
  if (frames) {
    mem = gdImageGifAnimAddPtr(gd_prev,&sz,
			       local_color?1:0,
			       0,0,
			       step_pending,1,
			       (emit_ct>=1)?gd_pre_prev:NULL);
    emit_ct++;
    blocks.push_back(Bytes((char*)mem,(size_t)sz));
    dbg_printf("  Generated final frame, %d ticks\n", 
	       step_pending);
  }

  pf_gdQuantizeFini(quantizer);
  quantizer = NULL;

  gdImageDestroy(gd_curr);
  gdImageDestroy(gd_prev);
  gdImageDestroy(gd_pre_prev);
  gdImageDestroy(gd_palette);

  mem = gdImageGifAnimEndPtr(&sz);
  blocks.push_back(Bytes((char*)mem,(size_t)sz));
}


void GifAnim::save(const char *fname) {
  FILE *fout = fopen(fname,"wb");
  if (!fout) {
    fprintf(stderr,"Cannot write GIF\n");
    exit(1);
  }
  for (int i=0; i<(int)blocks.size(); i++) {
    Bytes& b = blocks[i];
    fwrite(b.get(),1,b.length(),fout);
  }
  fclose(fout);
}


