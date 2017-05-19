# - Find MantisAPI
# Find the MantisAPI includes and client library
# This module defines
#  MantisAPI_INCLUDE_DIR, where to find MantisAPI/MantisAPI.h
#  MantisAPI_LIBRARIES, the libraries needed to use MantisAPI.
#  MantisAPI_FOUND, If false, do not try to use MantisAPI.

if(MantisAPI_INCLUDE_DIR AND MantisAPI_LIBRARIES)
   set(MantisAPI_FOUND TRUE)

else(MantisAPI_INCLUDE_DIR AND MantisAPI_LIBRARIES)
    find_path(MantisAPI_INCLUDE_DIR MantisAPI.h
      ${CMAKE_INCLUDE_PATH}/mantis
      ${CMAKE_INSTALL_PREFIX}/include/mantis
      include
      /usr/include/
      /usr/local/include/
      /usr/include/mantis/
      /usr/local/include/mantis/
      /opt/mantis/include/
      $ENV{ProgramFiles}/mantis/*/include
      $ENV{SystemDrive}/mantis/*/include
    )

if(WIN32)
   find_library(MantisAPI_LIBRARIES NAMES MantisAPI
      PATHS
      $ENV{ProgramFiles}/aqueti/*/lib
      $ENV{SystemDrive}/aqueti/*/lib
      )
else(WIN32)
    find_library(MantisAPI_LIBRARIES NAMES MantisAPI
      PATHS
      ${CMAKE_LIB_PATH}
      /usr/lib
      /usr/lib/mantis
      /usr/local/lib
      /usr/local/lib/mantis
      /opt/mantis/lib
      )
endif(WIN32)
  if(MantisAPI_INCLUDE_DIR AND MantisAPI_LIBRARIES)
    set(MantisAPI_FOUND TRUE)
    message(STATUS "Found MantisAPI: ${MantisAPI_INCLUDE_DIR}, ${MantisAPI_LIBRARIES}")
  else(MantisAPI_INCLUDE_DIR AND MantisAPI_LIBRARIES)
    message(STATUS "Didn't find MantisAPI: ${MantisAPI_INCLUDE_DIR}, ${MantisAPI_LIBRARIES}")
    set(MantisAPI_FOUND FALSE)
    if (MantisAPI_FIND_REQUIRED)
		message(FATAL_ERROR "MantisAPI not found.")
	else (MantisAPI_FIND_REQUIRED)
		message(STATUS "MantisAPI not found.")
	endif (MantisAPI_FIND_REQUIRED)
  endif(MantisAPI_INCLUDE_DIR AND MantisAPI_LIBRARIES)

  mark_as_advanced(MantisAPI_INCLUDE_DIR MantisAPI_LIBRARIES)

endif(MantisAPI_INCLUDE_DIR AND MantisAPI_LIBRARIES)

