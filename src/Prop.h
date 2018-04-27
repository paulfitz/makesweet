#ifndef PROP_INC
#define PROP_INC

#include <string>
#include <yarp/os/Property.h>

class Prop {
public:
  static void scan_hx(const std::string& txt, yarp::os::Property& data);

  static void scan_xml(const std::string& txt, yarp::os::Property& data);
};

#endif
