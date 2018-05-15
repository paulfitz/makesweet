set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(USE_OPENCV "Implement options that rely on OpenCV" TRUE)

ADD_LIBRARY(msmap msmap.h msmap.c)
ADD_LIBRARY(filer Filer.cpp Filer.h)
TARGET_LINK_LIBRARIES(filer gd zzip)

ADD_EXECUTABLE(reanimator reanimator.cpp Prop.h Prop.cpp Pixer.h
  Mapping.h Repository.h Repository.cpp Render.h Render.cpp
  Input.h Input.cpp Inputs.h Renders.h Renders.cpp
  GifAnim.h GifAnim.cpp
  gd_topal.c)
TARGET_LINK_LIBRARIES(reanimator filer)

if (USE_OPENCV)
  add_definitions("-DMAKESWEET_USE_OPENCV")
  TARGET_LINK_LIBRARIES(reanimator opencv_core opencv_imgproc)
endif()

