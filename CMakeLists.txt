cmake_minimum_required (VERSION 3.18)

include(FetchContent)
include(CheckLanguage)

project (home_alarm LANGUAGES CXX C)

include(GNUInstallDirs)

set( CMAKE_CXX_FLAGS "-std=c++17" )
add_compile_options( -g )
add_compile_options( -Wfatal-errors )

FetchContent_Declare( 
                mongoose 
                GIT_REPOSITORY  "https://github.com/cesanta/mongoose.git"
                GIT_TAG         "master"  # need 7.3 for cpp support
    )

FetchContent_MakeAvailable( mongoose )  
set( MONGOOSE_SRC ${FETCHCONTENT_BASE_DIR}/mongoose-src )

find_package( OpenSSL QUIET )
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_path(LIBUSB_INCLUDE_DIR
  NAMES libusb.h
  PATH_SUFFIXES "include" "libusb" "libusb-1.0" )

find_library(LIBUSB_LIBRARY
  NAMES usb
  PATH_SUFFIXES "lib64" "lib32" "lib" ) 


find_path(LIBRTLSDR_INCLUDE_DIR
  NAMES rtl-sdr.h
  PATH_SUFFIXES "include" "libusb" "libusb-1.0" )

find_library(LIBRTLSDR_LIBRARY
  NAMES rtlsdr
  PATH_SUFFIXES "lib64" "lib32" "lib" ) 

add_executable( home_alarm src/main.cpp src/radio.cpp src/signal_processor.cpp src/decoder.cpp src/sensor_event_receiver.cpp src/sensor_event.cpp )
target_include_directories( home_alarm PUBLIC ${LIBUSB_INCLUDE_DIR} ${LIBRTLSDR_INCLUDE_DIR} )
target_link_libraries( home_alarm PRIVATE Threads::Threads )
target_link_libraries( home_alarm PRIVATE ${LIBUSB_LIBRARY} ${LIBRTLSDR_LIBRARY} )

#add_executable( test src/test.cpp src/radio.cpp src/signal_processor.cpp src/decoder.cpp src/sensor_event_receiver.cpp src/sensor_event.cpp )
#target_link_libraries( test PRIVATE ${LIBUSB_LIBRARY} ${LIBRTLSDR_LIBRARY} )
