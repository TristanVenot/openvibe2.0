# ---------------------------------
# Finds X11 toolkit
#
# Sets X11_FOUND
# Sets X11_LIBRARIES
# Sets X11_LIBRARY_DIRS
# Sets X11_LDFLAGS
# Sets X11_LDFLAGS_OTHERS
# Sets X11_INCLUDE_DIRS
# Sets X11_CFLAGS
# Sets X11_CFLAGS_OTHERS
# ---------------------------------

GET_PROPERTY(OV_PRINTED GLOBAL PROPERTY OV_TRIED_ThirdPartyX11)

IF(UNIX)
	INCLUDE("FindThirdPartyPkgConfig")
	pkg_check_modules(X11 x11)

	IF(X11_FOUND)
		OV_PRINT(OV_PRINTED "  Found X11...")
		INCLUDE_DIRECTORIES(${X11_INCLUDE_DIRS})
		ADD_DEFINITIONS(${X11_CFLAGS})
		ADD_DEFINITIONS(${X11_CFLAGS_OTHERS})
		# LINK_DIRECTORIES(${X11_LIBRARY_DIRS})
		FOREACH(X11_LIB ${X11_LIBRARIES})
			SET(X11_LIB1 "X11_LIB1-NOTFOUND")
			FIND_LIBRARY(X11_LIB1 NAMES ${X11_LIB} PATHS ${X11_LIBRARY_DIRS} ${X11_LIBDIR} NO_DEFAULT_PATH)
			FIND_LIBRARY(X11_LIB1 NAMES ${X11_LIB})
			IF(X11_LIB1)
				OV_PRINT(OV_PRINTED "    [  OK  ] Third party lib ${X11_LIB1}")
				TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${X11_LIB1})
			ELSE(X11_LIB1)
				OV_PRINT(OV_PRINTED "    [FAILED] Third party lib ${X11_LIB}")
			ENDIF(X11_LIB1)
		ENDFOREACH(X11_LIB)
	ELSE(X11_FOUND)
		OV_PRINT(OV_PRINTED "  FAILED to find X11...")
	ENDIF(X11_FOUND)
ENDIF(UNIX)

SET_PROPERTY(GLOBAL PROPERTY OV_TRIED_ThirdPartyX11 "Yes")
