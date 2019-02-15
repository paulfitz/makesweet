set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(USE_OPENCV "Implement options that rely on OpenCV" FALSE)

OPTION(USE_DETAIL "Use detailed sample maps rather than single-pixel summaries" OFF)

IF(USE_DETAIL)
  # Protobuf!
  find_package(Protobuf REQUIRED)
  include_directories(${PROTOBUF_INCLUDE_DIRS})
  include_directories(${CMAKE_BINARY_DIR})
  PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS frame.proto)
  ADD_DEFINITIONS(-DUSE_DETAIL=1)
ELSE()
  SET(PROTO_SRCS)
  SET(PROTO_HDRS)
ENDIF()

ADD_LIBRARY(msmap msmap.h msmap.c)

ADD_EXECUTABLE(reanimator reanimator.cpp Prop.h Prop.cpp Pixer.h
  Mapping.h Mapping.cpp Repository.h Repository.cpp Render.h Render.cpp
  Input.h Input.cpp Inputs.h Renders.h Renders.cpp
  GifAnim.h GifAnim.cpp
  VidAnim.h VidAnim.cpp
  gd_topal.c ${PROTO_SRCS} ${PROTO_HDRS}
  Filer.cpp Filer.h)
# TARGET_LINK_LIBRARIES(reanimator filer)
TARGET_LINK_LIBRARIES(reanimator gd zzip)

if (USE_OPENCV)
  add_definitions("-DMAKESWEET_USE_OPENCV")
  option(NEED_VIDEOIO "Need to link videoio in this version of opencv" ON)
  TARGET_LINK_LIBRARIES(reanimator opencv_core opencv_highgui opencv_imgproc)
  if (NEED_VIDEOIO)
    TARGET_LINK_LIBRARIES(reanimator opencv_videoio)
  endif()
endif()

if (USE_DETAIL)
  TARGET_LINK_LIBRARIES(reanimator ${PROTOBUF_LIBRARIES})
endif()
