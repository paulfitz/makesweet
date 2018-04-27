#include "Prop.h"

using namespace yarp::os;
using namespace std;

void Prop::scan_hx(const std::string& txt, yarp::os::Property& data) {
  data.clear();
  string line;
  for (int i=0; i<(int)txt.size(); i++) {
    char ch = txt[i];
    if (ch=='\r'||ch=='\n') {
      line = "";
      continue;
    }
    if (ch=='[') {
      line += '(';
      continue;
    }
    if (ch==']') {
      line += ')';
      continue;
    }
    if (ch==',') {
      line += ' ';
      continue;
    }
    if (ch!=';') {
      line += ch;
      continue;
    }
    Bottle b(line.c_str());
    
    string var = "dud";
    for (int j=1; j<b.size(); j++) {
      if (b.get(j-1).asString()=="var") {
	var = b.get(j).asString();
	continue;
      }
      if (b.get(j-1).asString()=="=") {
	Value v = b.get(j);
	ConstString s = v.asString();
	if (s=="true") {
	  data.put(var.c_str(),1);
	} else if (s=="false") {
	  data.put(var.c_str(),0);
	} else {
	  data.put(var.c_str(),v);
	}
      }
    }
  }
}


void Prop::scan_xml(const std::string& txt, yarp::os::Property& data) {
  data.clear();
  string line;
  for (int i=0; i<(int)txt.size(); i++) {
    char ch = txt[i];
    if (ch=='\"') {
      line += " ";
      continue;
    }
    if (ch!='\r'&&ch!='\n') {
      line += ch;
      continue;
    } 

    Bottle b(line.c_str());
    
    string var = "dud";
    for (int j=1; j<b.size(); j++) {
      if (b.get(j-1).asString()=="id=") {
	var = b.get(j).asString();
	continue;
      }
      if (b.get(j-1).asString()=="import=") {
	data.put(var.c_str(),b.get(j));
      }
    }
  }
}
