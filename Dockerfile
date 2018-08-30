FROM ubuntu:18.04

RUN \
  apt-get update; \
  apt-get install -y build-essential

RUN \
  apt-get update; \
  apt-get install -y libgd-dev libzzip-dev libopencv-highgui-dev

RUN \
  cd tmp; \
  apt-get update; \
  apt-get install -y cmake wget; \
  wget https://github.com/robotology/yarp/archive/v2.3.72.tar.gz; \
  tar xzvf v2.3.72.tar.gz; \
  mkdir yarp; \
  cd yarp; \
  cmake -DSKIP_ACE=TRUE ../yarp-*; \
  make 

COPY . /makesweet/

RUN \
  cd /makesweet; \
  mkdir build; \
  cd build; \
  cmake -DYARP_DIR=/tmp/yarp ..; \
  make

RUN \
  echo "#!/bin/bash" > /reanimator; \
  echo "cd /share" >> /reanimator; \
  echo "/makesweet/build/bin/reanimator \"\$@\"" >> /reanimator; \
  chmod u+x /reanimator

ENTRYPOINT ["/reanimator"]
