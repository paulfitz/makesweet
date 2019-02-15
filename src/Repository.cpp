#include "Repository.h"
#include "Prop.h"
#include "Dbg.h"

#include <stdio.h>
#include <stdlib.h>

using namespace std;
using namespace yarp::os;

void Repository::load_part(const char *postfix, int idx) {
  string base = "Disp";
  if (idx==-2) base = "Thumb";
  string dark = base + "DarkData";
  string light = base + "LightData";
  string neutral = base + "TransparentData";
  string map1 = base + "MapData";
  string map2 = base + "SelData";
  string samples = base + "Samples";
  if (postfix[0]!='\0') {
    dark += postfix;
    light += postfix;
    neutral += postfix;
    map1 += postfix;
    map2 += postfix;
    samples += postfix;
  }

  if (mappings.find(idx)==mappings.end()) {
    mappings[idx] = Mapping();
  }

  Mapping& s = mappings[idx];
  bool ok = inventory.check(dark.c_str()) &&
    inventory.check(light.c_str()) &&
    inventory.check(map1.c_str()) &&
    inventory.check(map2.c_str());
  if (!ok) {
    fprintf(stderr,"Missing some map data (%s/%s/%s/%s)\n",
	    dark.c_str(), light.c_str(), map1.c_str(), map2.c_str());
    return;
  }
  
  bool have_neutral = inventory.check(neutral.c_str());
  bool have_samples = inventory.check(samples.c_str());

  ConstString light_name = inventory.find(light.c_str()).asString();
  ConstString dark_name = inventory.find(dark.c_str()).asString();
  ConstString neutral_name = inventory.find(neutral.c_str()).asString();
  ConstString map1_name = inventory.find(map1.c_str()).asString();
  ConstString map2_name = inventory.find(map2.c_str()).asString();
  ConstString samples_name = inventory.find(samples.c_str()).asString();

  dbg_printf("Loading light %s\n", light_name.c_str());
  if (!filer.load(light_name.c_str(),s.light)) exit(1);
  s.light_name = light_name;
  dbg_printf("Loading dark\n");
  if (!filer.load(dark_name.c_str(),s.dark)) exit(1);
  s.dark_name = dark_name;
  if (!have_samples) {
    dbg_printf("Loading map1\n");
    if (!filer.load(map1_name.c_str(),s.map1)) exit(1);
    s.map1_name = map1_name;
  }
  dbg_printf("Loading map2\n");
  if (!filer.load(map2_name.c_str(),s.map2)) exit(1);
  s.map2_name = map2_name;
  if (have_neutral) {
    dbg_printf("Loading neutral\n");
    if (!filer.load(neutral_name.c_str(),s.neutral)) exit(1);
    s.neutral_name = neutral_name;
  } else {
    s.neutral = s.light;
    s.neutral_name = light_name;
  }
  if (have_samples) {
    dbg_printf("Loading samples\n");
    s.load_samples(samples_name.c_str(), filer);
  } else {
    s.load_samples(NULL, filer);
  }

  s.scale = 1;
}

void Repository::load(const char *fname) {
  bool ok = filer.load_zip(fname);
  if (!ok) {
    fprintf(stderr,"Cannot load zip %s\n", fname);
    exit(1);
  }

  Prop prop;

  std::string txt = filer.load_text("MakeSweetConfig.hx");
  if (txt=="") {
    dbg_printf("No config file\n");
  }

  prop.scan_hx(txt,config);

  txt = filer.load_text("inventory.xml");
  if (txt=="") {
    fprintf(stderr, "No inventory file\n");
    exit(1);
  }
  prop.scan_xml(txt,inventory);

  dbg_printf("Props: %s\n", config.toString().c_str());
  dbg_printf("Inventory: %s\n", inventory.toString().c_str());

  flag_animation = false;
  if (config.check("animation",Value(0)).asInt()) {
    flag_animation = true;
  } else {
    flag_animation = false;
  }
  frames = 1;
  if (flag_animation) {
    frames = config.check("frames",Value(1)).asInt();
  }

  dbg_printf("Loaded all files / animation %d\n", flag_animation);
}


Mapping *Repository::get_mapping(int index) {
  if (index<0) {
    map<int,Mapping>::iterator it = mappings.find(index);
    if (it == mappings.end()) {
      load_part("",index);
      it = mappings.find(index);
    }
    return &(it->second);
  }

  if (!flag_animation) {
    map<int,Mapping>::iterator it = mappings.find(0);
    if (it == mappings.end()) {
      load_part("",0);
      it = mappings.find(0);
      int rct = (int)mappings.size();
      if (rct>peak_cache_count) peak_cache_count = rct;
    }
    return &(it->second);
  }

  map<int,Mapping>::iterator it = mappings.find(index);
  if (it == mappings.end()) {
    char buf[256];
    sprintf(buf,"%d", index);
    load_part(buf,index);
    it = mappings.find(index);
    int rct = (int)mappings.size();
    if (rct>peak_cache_count) peak_cache_count = rct;
  }
  return &(it->second);
}

void Repository::remove_mapping(int index) {
  map<int,Mapping>::iterator it = mappings.find(index);
  if (it!=mappings.end()) {
    mappings.erase(it);
  }
}


std::vector<int> Repository::get_palette() {
  vector<int> frames;
  Bottle *lst = config.find("palette").asList();
  if (!lst) {
    int idx = config.find("palette").asInt();
    frames.push_back(idx);
    return frames;
  }
  for (int i=0; i<lst->size(); i++) {
    frames.push_back(lst->get(i).asInt());
  }
  return frames;
}

double Repository::get_period() {
  double period = config.check("delay",Value(1.0)).asDouble();
  double speed = config.check("speed_step",Value(1.0)).asDouble();
  return period*speed;
}

double Repository::get_hold() {
  double hold = config.check("hold",Value(1.0)).asDouble();
  return hold;
}

double Repository::get_zoom() {
  double zoom = config.check("zoom",Value(1.0)).asDouble();
  return zoom;
}

