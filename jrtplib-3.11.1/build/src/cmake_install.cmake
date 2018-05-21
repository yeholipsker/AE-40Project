# Install script for directory: C:/Users/User/Source/Repos/jrtplib-3.11.1/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Users/User/Source/Repos")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/jrtplib3" TYPE FILE FILES
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtcpapppacket.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtcpbyepacket.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtcpcompoundpacket.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtcpcompoundpacketbuilder.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtcppacket.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtcppacketbuilder.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtcprrpacket.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtcpscheduler.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtcpsdesinfo.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtcpsdespacket.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtcpsrpacket.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtcpunknownpacket.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpaddress.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpcollisionlist.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/build/src/rtpconfig.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpdebug.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpdefines.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtperrors.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtphashtable.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpinternalsourcedata.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpipv4address.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpipv4destination.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpipv6address.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpipv6destination.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpkeyhashtable.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtplibraryversion.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpmemorymanager.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpmemoryobject.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtppacket.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtppacketbuilder.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtppollthread.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtprandom.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtprandomrand48.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtprandomrands.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtprandomurandom.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtprawpacket.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpsession.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpsessionparams.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpsessionsources.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpsourcedata.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpsources.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpstructs.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtptimeutilities.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtptransmitter.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtptypes_win.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/build/src/rtptypes.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpudpv4transmitter.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpudpv6transmitter.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpbyteaddress.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpexternaltransmitter.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpsecuresession.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpsocketutil.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpabortdescriptors.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtpselect.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtptcpaddress.h"
    "C:/Users/User/Source/Repos/jrtplib-3.11.1/src/rtptcptransmitter.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Users/User/Source/Repos/lib/jrtplib_d.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "C:/Users/User/Source/Repos/lib" TYPE STATIC_LIBRARY FILES "C:/Users/User/Source/Repos/jrtplib-3.11.1/build/src/Debug/jrtplib_d.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Users/User/Source/Repos/lib/jrtplib.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "C:/Users/User/Source/Repos/lib" TYPE STATIC_LIBRARY FILES "C:/Users/User/Source/Repos/jrtplib-3.11.1/build/src/Release/jrtplib.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Users/User/Source/Repos/lib/jrtplib.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "C:/Users/User/Source/Repos/lib" TYPE STATIC_LIBRARY FILES "C:/Users/User/Source/Repos/jrtplib-3.11.1/build/src/MinSizeRel/jrtplib.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Users/User/Source/Repos/lib/jrtplib.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "C:/Users/User/Source/Repos/lib" TYPE STATIC_LIBRARY FILES "C:/Users/User/Source/Repos/jrtplib-3.11.1/build/src/RelWithDebInfo/jrtplib.lib")
  endif()
endif()

