#include <stdio.h>
#include <stdlib.h>

#include <yarp/sig/Image.h>
#include <yarp/sig/ImageFile.h>
#include <yarp/sig/ImageDraw.h>
#include <yarp/os/Network.h>

#include "Filer.h"
#include "Prop.h"
#include "Repository.h"
#include "Input.h"
#include "Render.h"
#include "Renders.h"
#include "GifAnim.h"
#include "VidAnim.h"
#include "Dbg.h"

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::sig::file;

int __ms_verbose = 0;

int main(int argc, char *argv[]) {
  if (Network::getEnvironment("VERBOSE")=="1") {
    __ms_verbose = 1;
  }

  Property options;
  options.fromCommand(argc,argv);

  Repository repo;

  if (!options.check("zip")) {
    fprintf(stderr, "reanimator --zip foo.zip --in layer1.png layer2.png\n");
    fprintf(stderr, "reanimator --zip foo.zip --in layer1.png layer2.png +layer2_more.png\n");
    fprintf(stderr, "reanimator ... --w 200 --h 150\n");
    fprintf(stderr, "reanimator ... --first 2\n");
    fprintf(stderr, "reanimator ... --last 4\n");
    fprintf(stderr, "reanimator ... --single\n");
    fprintf(stderr, "reanimator ... --auto_zoom\n");
    fprintf(stderr, "reanimator ... --start 10  # frame to start at\n");
    fprintf(stderr, "reanimator ... --save \"frame_%%06d.jpg\"\n");
    fprintf(stderr, "reanimator ... --gif example.gif\n");
    fprintf(stderr, "reanimator ... --stats\n");
    fprintf(stderr, "reanimator ... --files\n");
    
    exit(1);
  }
  
  repo.load(options.find("zip").asString().c_str());

  Inputs ins;

  Bottle& lst = options.findGroup("in");
  int layer = 1;
  for (int i=1; i<lst.size(); i++) {
    Input& in = ins.add();
    ConstString str = lst.get(i).asString();
    if (str[0] == '+' && layer > 1) {
      layer--;
      str = str.substr(1);
    }
    in.load(str.c_str());
    in.layer = layer;
    layer++;
  }

  Renders renders;
  renders.attach_repository(&repo);
  renders.attach_inputs(&ins);
  if (options.check("w")&&options.check("h")) {
    renders.set_size(options.find("w").asInt(),options.find("h").asInt());
  }
  int first = options.check("first",Value(0)).asInt();
  int last = options.check("last",Value(repo.length()-1)).asInt();

  if (options.check("single")) {
    first = last = 0;
  }

  if (options.check("auto_zoom")) {
    renders.auto_zoom();
  }

  if (options.check("scan")) {
    int frame = -1;
    if (options.check("start")) {
      frame = options.check("start",Value(0)).asInt();
    }
    renders.scan(frame);
  }

  if (options.check("save")) {
    for (int i=first; i<=last; i++) {
      Render *r = renders.get_render(i);
      if (!r) exit(1);
      char buf[256];
      sprintf(buf,options.check("save",Value("frame_%06d.jpg")).asString().c_str(),i);
      r->save(buf);
      dbg_printf("Wrote to %s\n", buf);
    }
  }

  if (options.check("gif")) {
    GifAnim anim;
    anim.attach_renders(&renders);
    anim.set_palette(repo.get_palette());
    anim.set_timing(repo.get_period(),repo.get_hold());
    if (options.check("start")) {
      anim.set_first_frame(options.check("start",Value(0)).asInt());
    }
    anim.apply();
    anim.save(options.find("gif").asString().c_str());
  }

  if (options.check("vid")) {
    VidAnim anim;
    anim.attach_renders(&renders);
    anim.set_timing(repo.get_period(),repo.get_hold());
    if (options.check("start")) {
      anim.set_first_frame(options.check("start",Value(0)).asInt());
    }
    anim.apply(options.find("vid").asString().c_str());
  }

  if (options.check("files")) {
    printf("animation=%s\n", repo.is_animation()?"true":"false");
    printf("length=%d\n", repo.length());
    printf("period=%g\n", repo.get_period());
    printf("hold=%g\n", repo.get_hold());
    printf("zoom=%g\n", repo.get_zoom());
    Mapping *mapping = repo.get_mapping();
    if (mapping) {
      printf("light=\"%s\"\n", mapping->light_name.c_str());
      printf("dark=\"%s\"\n", mapping->dark_name.c_str());
      printf("map1=\"%s\"\n", mapping->map1_name.c_str());
      printf("map2=\"%s\"\n", mapping->map2_name.c_str());
      printf("neutral=\"%s\"\n", mapping->neutral_name.c_str());
    }
    mapping = repo.get_thumb_mapping();
    if (mapping) {
      printf("thumb_light=\"%s\"\n", mapping->light_name.c_str());
      printf("thumb_dark=\"%s\"\n", mapping->dark_name.c_str());
      printf("thumb_map1=\"%s\"\n", mapping->map1_name.c_str());
      printf("thumb_map2=\"%s\"\n", mapping->map2_name.c_str());
      printf("thumb_neutral=\"%s\"\n", mapping->neutral_name.c_str());
    }
    if (repo.is_animation()) {
      for (int i=0; i<repo.length(); i++) {
	char prefix[256];
	sprintf(prefix,"frame_%d_", i);
	mapping = repo.get_mapping(i);
	printf("%slight=\"%s\"\n", prefix, mapping->light_name.c_str());
	printf("%sdark=\"%s\"\n", prefix, mapping->dark_name.c_str());
	printf("%smap1=\"%s\"\n", prefix, mapping->map1_name.c_str());
	printf("%smap2=\"%s\"\n", prefix, mapping->map2_name.c_str());
	printf("%sneutral=\"%s\"\n", prefix, mapping->neutral_name.c_str());
      }
    }
  }

  if (options.check("stats")) {
    fprintf(stderr,"Peak number of mappings cached: %d\n", repo.peak());
    fprintf(stderr,"Peak number of renders cached: %d\n", renders.peak());
    fprintf(stderr,"Total number of frames: %d\n", repo.length());
    fprintf(stderr,"Total number of renders: %d\n", 
	    Render::render_count());
    fprintf(stderr,"Palette frame(s):");
    vector<int> pals = repo.get_palette();
    for (int i=0; i<(int)pals.size(); i++) {
      fprintf(stderr," %d", pals[i]);
    }
    fprintf(stderr,"\n");
  }

  return 0;
}
