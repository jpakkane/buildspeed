project(netdumpper CXX)
cmake_minimum_required(VERSION 2.8.9)

find_package(PkgConfig REQUIRED)

add_definitions(-DQT_NO_KEYWORDS -std=c++11)

pkg_check_modules(
  GLIB REQUIRED
  glib-2.0
  gio-2.0
)
include_directories(${GLIB_INCLUDE_DIRS})

find_package(Qt5DBus COMPONENTS Qt5DBusMacros REQUIRED)
include_directories(${Qt5DBus_INCLUDE_DIRS})

add_executable(i-n-extractor
  i-n-extractor.cpp
  eventprinter.cpp
  urfkillroot.cpp
  urfkillswitch.cpp
  nmroot.cpp
  nmsettings.cpp
  nmconnsettings.cpp
  nmactiveconnection.cpp
  nmwirelessdevice.cpp
  nmaccesspoint.cpp
  nmdevice.cpp
  nmmodem.cpp
  ofonoroot.cpp
  ofonomodemmodem.cpp
  ofonomodemsimmanager.cpp
  ofonomodemnetworkregistration.cpp
  ofononetworkoperator.cpp
  ofonomodemconnectionmanager.cpp
  numberadder.cpp
  systemstate.cpp
  indicatoradaptor.cpp
  indicatorexporter.cpp
  eventprinter.h
  urfkillroot.h
  urfkillswitch.h 
  nmroot.h
  nmsettings.h
  nmconnsettings.h
  nmactiveconnection.h
  nmwirelessdevice.h
  nmaccesspoint.h
  nmdevice.h
  nmmodem.h
  ofonoroot.h
  ofonomodemmodem.h
  ofonomodemsimmanager.h
  ofonomodemnetworkregistration.h
  ofononetworkoperator.h
  ofonomodemconnectionmanager.h
  numberadder.h
  systemstate.h
  indicatoradaptor.h
)

qt5_use_modules(i-n-extractor Core DBus)
set_target_properties(i-n-extractor PROPERTIES AUTOMOC TRUE)
target_link_libraries(i-n-extractor ${GLIB_LDFLAGS})
