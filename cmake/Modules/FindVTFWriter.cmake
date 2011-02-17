IF(VTFWRITER_INCLUDES AND VTFWRITER_LIBRARIES)
  SET(VTFWRITER_FIND_QUIETLY TRUE)
ENDIF(VTFWRITER_INCLUDES AND VTFWRITER_LIBRARIES)

FIND_PATH(VTFWRITER_INCLUDES
  NAMES
  VTFAPI.h
  PATHS
  $ENV{HOME}/include
)

FIND_LIBRARY(VTFWRITER_LIBRARIES 
             VTFExpressAPI
             PATHS
             $ENV{HOME}/lib
)

IF(VTFWRITER_LIBRARIES AND CMAKE_COMPILER_IS_GNUCXX)
  SET(VTFWRITER_LIBRARIES ${VTFWRITER_LIBRARIES})
ENDIF(VTFWRITER_LIBRARIES AND CMAKE_COMPILER_IS_GNUCXX)

INCLUDE(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VTFWRITER DEFAULT_MSG
                                  VTFWRITER_INCLUDES VTFWRITER_LIBRARIES)

MARK_AS_ADVANCED(VTFWRITER_INCLUDES VTFWRITER_LIBRARIES)
