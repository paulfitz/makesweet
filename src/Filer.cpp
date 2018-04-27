#define USE_ZIP

#include "Filer.h"
#include "Dbg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gd.h>

#include <string>

#ifdef USE_ZIP
#include <zzip/zzip.h>
#endif

#define R gdTrueColorGetRed
#define G gdTrueColorGetGreen
#define B gdTrueColorGetBlue
#define A gdTrueColorGetAlpha

using namespace std;
using namespace yarp::sig;
using namespace yarp::sig::file;
using namespace yarp::sig::draw;







typedef struct dpIOCtx
{
  gdIOCtx ctx;
  FILE *f;
}
fileIOCtx;

gdIOCtx *newFileCtx (FILE * f);

static int fileGetbuf (gdIOCtx *, void *, int);
static int filePutbuf (gdIOCtx *, const void *, int);
static void filePutchar (gdIOCtx *, int);
static int fileGetchar (gdIOCtx * ctx);

static int fileSeek (struct gdIOCtx *, const int);
static long fileTell (struct gdIOCtx *);
static void gdFreeFileCtx (gdIOCtx * ctx);


gdIOCtx *myNewFileCtx (FILE * f) {
  fileIOCtx *ctx;

  ctx = (fileIOCtx *) new fileIOCtx;
  if (ctx == NULL)
    {
      return NULL;
    }

  ctx->f = f;

  ctx->ctx.getC = fileGetchar;
  ctx->ctx.putC = filePutchar;

  ctx->ctx.getBuf = fileGetbuf;
  ctx->ctx.putBuf = filePutbuf;

  ctx->ctx.tell = fileTell;
  ctx->ctx.seek = fileSeek;

  ctx->ctx.gd_free = gdFreeFileCtx;

  return (gdIOCtx *) ctx;
}

static void
gdFreeFileCtx (gdIOCtx * ctx)
{
  delete ctx;
}


static int
filePutbuf (gdIOCtx * ctx, const void *buf, int size)
{
  fileIOCtx *fctx;
  fctx = (fileIOCtx *) ctx;

  return fwrite (buf, 1, size, fctx->f);

}

static int
fileGetbuf (gdIOCtx * ctx, void *buf, int size)
{
  fileIOCtx *fctx;
  fctx = (fileIOCtx *) ctx;

  return (fread (buf, 1, size, fctx->f));

}

static void
filePutchar (gdIOCtx * ctx, int a)
{
  unsigned char b;
  fileIOCtx *fctx;
  fctx = (fileIOCtx *) ctx;

  b = a;

  putc (b, fctx->f);
}

static int
fileGetchar (gdIOCtx * ctx)
{
  fileIOCtx *fctx;
  fctx = (fileIOCtx *) ctx;

  return getc (fctx->f);
}


static int
fileSeek (struct gdIOCtx *ctx, const int pos)
{
  fileIOCtx *fctx;
  fctx = (fileIOCtx *) ctx;
  return (fseek (fctx->f, pos, SEEK_SET) == 0);
}

static long
fileTell (struct gdIOCtx *ctx)
{
  fileIOCtx *fctx;
  fctx = (fileIOCtx *) ctx;

  return ftell (fctx->f);
}







#ifdef USE_ZIP

#include <zzip/zzip.h>

typedef struct dpZipIOCtx
{
  gdIOCtx ctx;
  ZZIP_FILE *f;
}
zipIOCtx;

gdIOCtx *newZipCtx (ZZIP_FILE * f);

static int zipGetbuf (gdIOCtx *, void *, int);
static int zipPutbuf (gdIOCtx *, const void *, int);
static void zipPutchar (gdIOCtx *, int);
static int zipGetchar (gdIOCtx * ctx);

static int zipSeek (struct gdIOCtx *, const int);
static long zipTell (struct gdIOCtx *);
static void gdFreeZipCtx (gdIOCtx * ctx);


gdIOCtx *myNewZipCtx (ZZIP_FILE * f) {
  zipIOCtx *ctx;

  ctx = (zipIOCtx *) new zipIOCtx;
  if (ctx == NULL)
    {
      return NULL;
    }

  ctx->f = f;

  ctx->ctx.getC = zipGetchar;
  ctx->ctx.putC = zipPutchar;

  ctx->ctx.getBuf = zipGetbuf;
  ctx->ctx.putBuf = zipPutbuf;

  ctx->ctx.tell = zipTell;
  ctx->ctx.seek = zipSeek;

  ctx->ctx.gd_free = gdFreeZipCtx;

  return (gdIOCtx *) ctx;
}

static void
gdFreeZipCtx (gdIOCtx * ctx)
{
  delete ctx;
}


static int
zipPutbuf (gdIOCtx * ctx, const void *buf, int size)
{
  zipIOCtx *fctx;
  fctx = (zipIOCtx *) ctx;

  fprintf(stderr,"CANNOT WRITE TO ZIP\n");
  exit(1);

  return -1;

}

static int
zipGetbuf (gdIOCtx * ctx, void *buf, int size)
{
  zipIOCtx *fctx;
  fctx = (zipIOCtx *) ctx;

  return (zzip_fread (buf, 1, size, fctx->f));

}

static void
zipPutchar (gdIOCtx * ctx, int a)
{
  fprintf(stderr,"CANNOT WRITE TO ZIP\n");
  exit(1);
}

static int
zipGetchar (gdIOCtx * ctx)
{
  zipIOCtx *fctx;
  fctx = (zipIOCtx *) ctx;

  char buf;
  int r = (zzip_fread (&buf, 1, 1, fctx->f));
  if (r<0) return -1;
  return buf;
}


static int
zipSeek (struct gdIOCtx *ctx, const int pos)
{
  zipIOCtx *fctx;
  fctx = (zipIOCtx *) ctx;
  return (zzip_seek (fctx->f, pos, SEEK_SET) == 0);
}

static long
zipTell (struct gdIOCtx *ctx)
{
  zipIOCtx *fctx;
  fctx = (zipIOCtx *) ctx;

  return zzip_tell (fctx->f);
}

#endif





/*
static const char * ispng(const char * name, int & result) {
  FILE * png = fopen(name, "rb");
  char buf[32];
  if (!png) {
    return "ispng: Failure in fopen";
  }
  if (8 != fread(buf, 1, 8, png)) {
    fclose(png);
    return "ispng: couldn't read magic number";
  }
  result = !memcmp(buf, "\x89PNG\x0D\x0A\x1A\x0A", 8);
  fclose(png);
  return 0;
}

static const char * isgif(const char * name, int & result) {
  FILE * gif = fopen(name, "rb");
  char buf[32];
  if (!gif) {
    return "isgif: Failure in fopen";
  }
  if (6 != fread(buf, 1, 6, gif)) {
    fclose(gif);
    return "isgif: couldn't read magic number";
  }
  result = !memcmp(buf, "GIF89a", 6);
  fclose(gif);
  return 0;
}

static const char * isjpg(const char * name, int & result) {
  FILE * jpg = fopen(name, "rb");
  char buf[32];
  if (!jpg) {
    return "isjpg: Failure in fopen";
  }
  if (2 != fread(buf, 1, 2, jpg)) {
    fclose(jpg);
    return "isjpg: couldn't read magic number";
  }
  result = !memcmp(buf, "\xFF\xD8", 2);
  fclose(jpg);
  return 0;
}

*/


static const char * ispng(gdIOCtx * ctx, int & result) {
  char buf[32];
  ctx->seek(ctx,0);
  if (8 != ctx->getBuf(ctx, buf, 8)) {
    return "ispng: couldn't read magic number";
  }
  result = !memcmp(buf, "\x89PNG\x0D\x0A\x1A\x0A", 8);
  return 0;
}

static const char * isgif(gdIOCtx * ctx, int & result) {
  char buf[32];
  ctx->seek(ctx,0);
  if (6 != ctx->getBuf(ctx, buf, 6)) {
    return "isgif: couldn't read magic number";
  }
  result = !memcmp(buf, "GIF89a", 6);
  return 0;
}

static const char * isjpg(gdIOCtx * ctx, int & result) {
  char buf[32];
  ctx->seek(ctx,0);
  if (2 != ctx->getBuf(ctx, buf, 2)) {
    return "isjpg: couldn't read magic number";
  }
  result = !memcmp(buf, "\xFF\xD8", 2);
  return 0;
}

std::string Filer::load_text(const char *fname) {
  gdImagePtr img = NULL;
  gdIOCtx *ctx = NULL;

  FILE *fd = NULL;

#ifdef USE_ZIP
  ZZIP_DIR *zip = (ZZIP_DIR *)src_zip;

  ZZIP_FILE *zd = NULL;
  if (zip) {
    zd = zzip_file_open(zip,fname,ZZIP_CASELESS);
    if (!zd) {
      dbg_printf("Cannot open %s in zip\n", fname);
      return "";
    } else {
      dbg_printf("Got %s from zip\n", fname);
    }
    ctx = myNewZipCtx(zd);
  }
#endif

  if (!ctx) {
    fd = fopen(fname,"r");
    if (fd==NULL) {
      fprintf(stderr,"Cannot open %s\n", fname);
      return "";
    }
    ctx = myNewFileCtx(fd);
  }

  std::string result;
  if (ctx) {
    ctx->seek(ctx,0);

    char buf[1024];
    int r = 0;
    do {
      r = ctx->getBuf(ctx,buf,sizeof(buf));
      if (r>0) {
	result.append(buf,r);
      }
    } while (r>0);
    ctx->gd_free(ctx);
  }
  if (fd) {
    fclose(fd);
  }
#ifdef USE_ZIP
  if (zd) {
    zzip_file_close(zd);
  }
#endif
  return result;
}

static gdImagePtr gd_load(const char *fname, void *src_zip) {
  gdImagePtr img = NULL;
  gdIOCtx *ctx = NULL;

  FILE *fd = NULL;

#ifdef USE_ZIP
  ZZIP_DIR *zip = (ZZIP_DIR *)src_zip;
  ZZIP_FILE *zd = NULL;
  if (zip) {
    zd = zzip_file_open(zip,fname,ZZIP_CASELESS);
    if (!zd) {
      fprintf(stderr,"Cannot open %s in zip\n", fname);
      exit(1);
    } else {
      dbg_printf("Got %s from zip\n", fname);
    }
    ctx = myNewZipCtx(zd);
  }
#endif

  if (!ctx) {
    fd = fopen(fname,"r");
    if (fd==NULL) {
      fprintf(stderr,"Cannot open %s\n", fname);
      return NULL;
    }
    ctx = myNewFileCtx(fd);
  }

  if (ctx) {

    int png = 0;
    int jpg = 0;
    int gif = 0;
    ispng(ctx,png);
    if (!png) {
      isjpg(ctx,jpg);
      if (!jpg) {
	isgif(ctx,gif);
      }
    }
    if (!(png||jpg||gif)) {
      fprintf(stderr,"Cannot process %s\n", fname);
      return NULL;
    }
    ctx->seek(ctx,0);
    
    if (png) {
      img = gdImageCreateFromPngCtx(ctx);
    } else if (jpg) {
      img = gdImageCreateFromJpegCtx(ctx);
    } else if (gif) {
      img = gdImageCreateFromGifCtx(ctx);
    }
    ctx->gd_free(ctx);
  }
  bool found = false;
  if (img!=NULL) {
    found = true;
    gdImageSaveAlpha(img, 1);
    gdImageAlphaBlending(img, 0);
  }
  if (fd) {
    fclose(fd);
  }
#ifdef USE_ZIP
  if (zd) {
    zzip_file_close(zd);
  }
#endif
  if (img) {
    dbg_printf("  loaded image of size %dx%d\n", img->sx, img->sy);
  }
  return img;
}


template <class T>
static bool yarp_load(const char *fname, 
		      yarp::sig::ImageOf<T>& src, void *src_zip) {
  
  bool found = false;
  gdImagePtr img = gd_load(fname,src_zip);

  if (img!=NULL) {
    found = true;
    src.resize(img->sx,img->sy);
    IMGFOR(src,x,y) {
      int pi = gdImageGetPixel(img,x,y);
      int r = R(pi);
      int g = G(pi);
      int b = B(pi);
      int a = A(pi);
      a *= 2;
      if (a>253) a = 255;
      src(x,y) = T(r,g,b,255-a);
    }
    gdFree(img);
  }
  return found;
}

bool Filer::load(const char *fname, 
		 yarp::sig::ImageOf<yarp::sig::PixelRgba>& src) {
  return yarp_load(fname,src,src_zip);
}

bool Filer::load(const char *fname, 
		 yarp::sig::ImageOf<yarp::sig::PixelBgra>& src) {
  return yarp_load(fname,src,src_zip);
}



template <class T>
static bool yarp_save(const char *fname, 
		      yarp::sig::ImageOf<T>& src) {

  string n = fname;
  bool is_png = false;
  bool is_jpg = false;

  if (n.rfind(".png")==n.length()-4) {
    is_png = true;
  }
  if (n.rfind(".jpg")==n.length()-4) {
    is_jpg = true;
  }
  gdImageStruct access;
  ImageOf<PixelBgra> idest;
  gdImagePtr im2;
  im2 = gdImageCreateTrueColor(src.width(),src.height());
  access = *im2;
  gdImageDestroy(im2);

  gdImageSaveAlpha(&access, 1);
  gdImageAlphaBlending(&access, 0);

  idest.copy(src);
  IMGFOR(idest,x,y) {
    PixelBgra& pix = idest(x,y);
    pix.a = 255-pix.a;
    pix.a /= 2;
  }
  access.tpixels = (int**)idest.getRowArray();
  access.sx = idest.width();
  access.sy = idest.height();

  FILE *test = fopen(fname,"wb");
  if (is_png) {
    gdImagePng(&access,test);
  } else if (is_jpg) {
    gdImageJpeg(&access,test,95);
  } else {
    fprintf(stderr,"unknown image format\n");
    exit(1);
  }
  fclose(test);
  return true;
}

bool Filer::save(const char *fname, 
		 yarp::sig::ImageOf<yarp::sig::PixelRgba>& src) {
  return yarp_save(fname,src);
}



bool Filer::save(const char *fname, 
		 yarp::sig::ImageOf<yarp::sig::PixelBgra>& src) {
  return yarp_save(fname,src);
}



bool Filer::load_zip(const char *fname) {
#ifdef USE_ZIP
  dbg_printf("Loading zip %s\n", fname);
  src_zip = zzip_opendir(fname);
  if (!src_zip) {
    fprintf(stderr, "failed to load zip %s\n", fname);
    exit(1);
  }
  return true;
#else
  return false;
#endif
}

bool Filer::unload_zip() {
#ifdef USE_ZIP
  ZZIP_DIR *zip = (ZZIP_DIR *)src_zip;
  if (zip) {
    zzip_closedir(zip);
    src_zip = NULL;
  }
#endif
  return true;
}
