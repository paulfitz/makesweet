#ifndef FILER_INC
#define FILER_INC

#include <string>

#include <yarp/sig/all.h>

class Filer {
private:
   void *src_zip;
public:
   Filer() {
     src_zip = NULL;
   }

   ~Filer() {
     if (src_zip) unload_zip();
   }

   bool load_zip(const char *fname);
   bool unload_zip();

   bool load(const char *fname, 
	     yarp::sig::ImageOf<yarp::sig::PixelRgba>& src);

   bool save(const char *fname, 
	     yarp::sig::ImageOf<yarp::sig::PixelRgba>& src);


   // GD compatible, apart from alpha channel
   bool load(const char *fname, 
	     yarp::sig::ImageOf<yarp::sig::PixelBgra>& src);

   bool save(const char *fname, 
	     yarp::sig::ImageOf<yarp::sig::PixelBgra>& src);

   std::string load_text(const char *fname);

};

#endif
