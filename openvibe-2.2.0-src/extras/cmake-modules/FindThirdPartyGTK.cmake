# ---------------------------------
# Finds GTK toolkit
#
# Sets GTK_FOUND
# Sets GTK_LIBRARIES
# Sets GTK_LIBRARY_DIRS
# Sets GTK_LDFLAGS
# Sets GTK_LDFLAGS_OTHERS
# Sets GTK_INCLUDE_DIRS
# Sets GTK_CFLAGS
# Sets GTK_CFLAGS_OTHERS
# ---------------------------------

GET_PROPERTY(OV_PRINTED GLOBAL PROPERTY OV_TRIED_ThirdPartyGTK)

IF(OV_DISABLE_GTK)
  OV_PRINT(OV_PRINTED  "  GTK disabled")
  RETURN()
ENDIF(OV_DISABLE_GTK)

INCLUDE("FindThirdPartyPkgConfig")
IF(WIN32)
	pkg_check_modules(GTK gtk+-win32-2.0 gthread-2.0)
ELSE(WIN32)
	pkg_check_modules(GTK "gtk+-2.0" "gthread-2.0")
ENDIF(WIN32)

IF(${OV_DEPENDENCIES_PLATFORM_FOLDER_NAME} STREQUAL "dependencies")
	SET(GTK_LIB_SUBFOLDER "2.10.0")
	SET(LIB_Z_NAME "zdll")
ELSE()
	SET(GTK_LIB_SUBFOLDER "i686-pc-vs10")
	SET(LIB_Z_NAME "zlib1")
ENDIF()
IF(GTK_FOUND)
	OV_PRINT(OV_PRINTED "  Found GTK+...")

	# optimistic...
	SET(GTK_FOUND_EVERYTHING "True")
	
	#shouldn't add GTK_CFLAGS, this results in AdditionalIncludeDirectories becoming broken in visual studio
	#ADD_DEFINITIONS(${GTK_CFLAGS} ${GTK_CFLAGS_OTHERS} ${GTHREAD_CFLAGS}${GTHREAD_CFLAGS_OTHERS})
	#LINK_DIRECTORIES(${GTK_LIBRARY_DIRS} ${GTHREAD_LIBRARY_DIRS})
	IF(WIN32)
		SET( GTK_LIB_LIST ${GTK_LIBRARIES} ${GTHREAD_LIBRARIES} ${LIB_Z_NAME})
	ELSE(WIN32)
		SET( GTK_LIB_LIST ${GTK_LIBRARIES} ${GTHREAD_LIBRARIES} z)
	ENDIF(WIN32)

	IF(WIN32)
		# gdi32.lib could be under the MS Windows SDK
		INCLUDE("OvSetWindowsSDKPath")
	ENDIF(WIN32)
	
	FOREACH(GTK_LIB ${GTK_LIB_LIST})
		SET(GTK_LIB1 "GTK_LIB1-NOTFOUND")
		FIND_LIBRARY(GTK_LIB1 NAMES ${GTK_LIB} PATHS ${GTK_LIBRARY_DIRS} ${GTK_LIBDIR} NO_DEFAULT_PATH)
		FIND_LIBRARY(GTK_LIB1 NAMES ${GTK_LIB} PATHS ${GTK_LIBRARY_DIRS} ${GTK_LIBDIR})
		IF(WIN32)
			FIND_LIBRARY(GTK_LIB1 NAMES ${GTK_LIB} PATHS ${OV_MS_SDK_PATH}/lib)
		ENDIF(WIN32)
		IF(GTK_LIB1)
			OV_PRINT(OV_PRINTED "    [  OK  ] Third party lib ${GTK_LIB1}")
			TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${GTK_LIB1})
		ELSE(GTK_LIB1)
			OV_PRINT(OV_PRINTED "    [FAILED] Third party lib ${GTK_LIB}")
			SET(GTK_FOUND_EVERYTHING "-NOTFOUND")
		ENDIF(GTK_LIB1)
	ENDFOREACH(GTK_LIB)
ENDIF(GTK_FOUND)

IF(GTK_FOUND_EVERYTHING)
	INCLUDE_DIRECTORIES(${GTK_INCLUDE_DIRS} ${GTHREAD_INCLUDE_DIRS})
	ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyGTK)
ELSE(GTK_FOUND_EVERYTHING)
	OV_PRINT(OV_PRINTED "  FAILED to find GTK+ or its components...")
	SET(OV_DISABLE_GTK TRUE)
	IF(NOT PKG_CONFIG_FOUND) 
		OV_PRINT(OV_PRINTED "    Did not even find pkg-config exe")
	ENDIF(NOT PKG_CONFIG_FOUND)
ENDIF(GTK_FOUND_EVERYTHING)

SET_PROPERTY(GLOBAL PROPERTY OV_TRIED_ThirdPartyGTK "Yes")

