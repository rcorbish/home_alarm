cmake_minimum_required (VERSION 3.18)

include(FetchContent)
include(CheckLanguage)

project (home_alarm LANGUAGES CXX C)

include(GNUInstallDirs)

set( CMAKE_CXX_FLAGS "-std=c++20" )
add_compile_options( -g )
add_compile_options( -Wfatal-errors )

FetchContent_Declare( 
                mongoose 
                GIT_REPOSITORY  "https://github.com/cesanta/mongoose.git"
                GIT_TAG         "master"  # need 7.3 for cpp support
    )

FetchContent_MakeAvailable( mongoose )  
set( MONGOOSE_SRC ${FETCHCONTENT_BASE_DIR}/mongoose-src )

find_package( OpenSSL )
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

set(SHARED_SOURCES
      src/radio.cpp 
      src/signal_processor.cpp 
      src/sensor_signal_processor.cpp 
      src/monitor_signal_processor.cpp 
      src/decoder.cpp 
      src/sensors.cpp 
      src/sensor_event.cpp 
      src/listener.cpp )

add_executable( home_alarm src/main.cpp ${SHARED_SOURCES})
target_include_directories( home_alarm PUBLIC ${LIBUSB_INCLUDE_DIR} ${LIBRTLSDR_INCLUDE_DIR} )
target_link_libraries( home_alarm PRIVATE ${LIBUSB_LIBRARY} ${LIBRTLSDR_LIBRARY} Threads::Threads )


add_executable( webserver src/webserver.cpp ${SHARED_SOURCES} ${MONGOOSE_SRC}/mongoose.c )
if( OPENSSL_FOUND )
    message( "Compile & link server with OpenSSL" ) 
    target_compile_definitions(webserver PUBLIC MG_ENABLE_OPENSSL )
    target_compile_definitions(webserver PUBLIC MG_TLS=MG_TLS_OPENSSL )
    # target_link_libraries( webserver PRIVATE  )
endif()
target_include_directories ( webserver PUBLIC ${MONGOOSE_SRC} ${LIBUSB_INCLUDE_DIR} ${LIBRTLSDR_INCLUDE_DIR} )
target_link_libraries( webserver PRIVATE ${LIBUSB_LIBRARY} ${LIBRTLSDR_LIBRARY} Threads::Threads ${OPENSSL_LIBRARIES})

add_executable( filter src/filter.cpp  src/sensors.cpp src/listener.cpp)
target_include_directories ( filter PUBLIC )
target_link_libraries( filter PRIVATE Threads::Threads )
