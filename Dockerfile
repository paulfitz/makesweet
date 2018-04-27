FROM ubuntu:14.04

RUN \
  apt-get update; \
  apt-get install -y build-essential

RUN \
  cd tmp; \
  apt-get update; \
  apt-get install -y cmake3 wget; \
  wget https://github.com/robotology/yarp/archive/v2.3.72.tar.gz; \
  tar xzvf v2.3.72.tar.gz; \
  mkdir yarp; \
  cd yarp; \
  cmake -DSKIP_ACE=TRUE ../yarp-*; \
  make 

RUN \
  apt-get update; \
  apt-get install -y libgd2-noxpm-dev libzzip-dev

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
