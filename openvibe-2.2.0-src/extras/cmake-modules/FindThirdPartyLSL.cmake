# ---------------------------------
# Finds LabStreamingLayer (LSL) library
#
# Sets LSL_FOUND
# Sets LSL_INCLUDE_DIRS
# Sets LSL_LIBRARY_DIRS
# Sets LSL_LIBRARIES_RELEASE
#
# Adds library to target
# Adds include path
#
# Script @author Jussi T. Lindgren / Inria
#
# @note On Windows, trying to use the debug library of LSL 1.12 in the same way we did with LSL 1.04
# caused an obscure Windows error code when launching the linking app. To sidestep, we use only the 
# LSL release build library for now. To enable the debug lib, the following might be needed on Win 
# in addition to linking the debug lib (but did not seem to be sufficient to make it work):
# SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DLSL_DEBUG_BINDINGS")
#	
# ---------------------------------

GET_PROPERTY(OV_PRINTED GLOBAL PROPERTY OV_TRIED_ThirdPartyLSL)

# The first ${..}/liblsl path is for Windows, the second ${..}/ for Linux
FIND_PATH(PATH_LSL include/lsl_cpp.h PATHS ${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES . liblsl NO_DEFAULT_PATH)
FIND_PATH(PATH_LSL include/lsl_cpp.h PATHS ${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES . liblsl)

IF(PATH_LSL)
	SET(LSL_FOUND TRUE)
	SET(LSL_INCLUDE_DIRS ${PATH_LSL}/include/)
	SET(LSL_LIBRARY_DIRS ${PATH_LSL}/lib/)
	IF(WIN32)
		IF("${PLATFORM_TARGET}" STREQUAL "x64")	
			SET(LSL_LIBRARIES_RELEASE liblsl64.lib)
			SET(LSL_LIBRARIES_RELEASE_DLL liblsl64.dll)
		ELSE()
			SET(LSL_LIBRARIES_RELEASE liblsl32.lib)	
			SET(LSL_LIBRARIES_RELEASE_DLL liblsl32.dll)	
		ENDIF()
	ELSEIF(UNIX)
		SET(LSL_LIBRARIES_RELEASE liblsl.so)
	ENDIF()
ENDIF()

IF(LSL_FOUND)
	OV_PRINT(OV_PRINTED "  Found liblsl...")
	INCLUDE_DIRECTORIES(${LSL_INCLUDE_DIRS})
	
	SET(LSL_LIB_REL "LSL_LIB_REL-NOTFOUND")
	# OV_PRINT(OV_PRINTED "LSL: ${LSL_LIBRARIES_RELEASE} - ${LSL_LIBRARY_DIRS_RELEASE}")
	FIND_LIBRARY(LSL_LIB_REL NAMES ${LSL_LIBRARIES_RELEASE} PATHS ${LSL_LIBRARY_DIRS} NO_DEFAULT_PATH)
	IF(LSL_LIB_REL)
		OV_PRINT(OV_PRINTED "    [  OK  ] Third party lib ${LSL_LIB_REL}")

		TARGET_LINK_LIBRARIES(${PROJECT_NAME} optimized ${LSL_LIB_REL})
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} debug ${LSL_LIB_REL})			

		ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyLSL)	

		IF(WIN32)
			INSTALL(FILES "${LSL_LIBRARY_DIRS}/${LSL_LIBRARIES_RELEASE_DLL}" DESTINATION "${DIST_BINDIR}" CONFIGURATIONS Release Debug)	
		ENDIF()	
	ELSE()
		OV_PRINT(OV_PRINTED "    [FAILED] Third party lib ${LSL_LIB_REL}")
	ENDIF()	

ELSE()
	OV_PRINT(OV_PRINTED "  FAILED to find liblsl (optional) ...")
ENDIF()

SET_PROPERTY(GLOBAL PROPERTY OV_TRIED_ThirdPartyLSL "Yes")

