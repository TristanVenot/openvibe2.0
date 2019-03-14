# ---------------------------------
# Finds OpenGL toolkit
#
# Sets OpenGL_FOUND
# Sets OpenGL_LIBRARIES
# Sets OpenGL_LIBRARY_DIRS
# Sets OpenGL_LDFLAGS
# Sets OpenGL_LDFLAGS_OTHERS
# Sets OpenGL_INCLUDE_DIRS
# Sets OpenGL_CFLAGS
# Sets OpenGL_CFLAGS_OTHERS
# ---------------------------------

GET_PROPERTY(OV_PRINTED GLOBAL PROPERTY OV_TRIED_ThirdPartyOpenGL)

IF(WIN32)
	FIND_LIBRARY(LIB_STANDARD_MODULE_GL opengl32)
	FIND_LIBRARY(LIB_STANDARD_MODULE_GLU glu32)
	IF(LIB_STANDARD_MODULE_GL AND LIB_STANDARD_MODULE_GLU)
		OV_PRINT(OV_PRINTED "  Found gl...")
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_STANDARD_MODULE_GL})
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_STANDARD_MODULE_GLU})
		ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyOpenGL)
	ELSE(LIB_STANDARD_MODULE_GL AND LIB_STANDARD_MODULE_GLU)
		MESSAGE(WARNING "  FAILED to find gl...")
	ENDIF(LIB_STANDARD_MODULE_GL AND LIB_STANDARD_MODULE_GLU)
ELSE(WIN32)
	INCLUDE("FindPkgConfig")
	pkg_check_modules(OpenGL glu)

	IF(OpenGL_FOUND)
		OV_PRINT(OV_PRINTED "  Found OpenGL...")
		INCLUDE_DIRECTORIES(${OpenGL_INCLUDE_DIRS})
		ADD_DEFINITIONS(${OpenGL_CFLAGS})
		ADD_DEFINITIONS(${OpenGL_CFLAGS_OTHERS})
		LINK_DIRECTORIES(${OpenGL_LIBRARY_DIRS})
		IF(NOT(APPLE))
			LINK_DIRECTORIES(${OpenGL_LIBRARY_DIRS}/mesa)
		ENDIF()
		FOREACH(OpenGL_LIB ${OpenGL_LIBRARIES} z)
			SET(OpenGL_LIB1 "OpenGL_LIB1-NOTFOUND")
			FIND_LIBRARY(OpenGL_LIB1 NAMES ${OpenGL_LIB} PATHS ${OpenGL_LIBRARY_DIRS} ${OpenGL_LIBDIR} NO_DEFAULT_PATH)
			FIND_LIBRARY(OpenGL_LIB1 NAMES ${OpenGL_LIB} PATHS ${OpenGL_LIBRARY_DIRS}/mesa ${OpenGL_LIBDIR}/mesa NO_DEFAULT_PATH)
			FIND_LIBRARY(OpenGL_LIB1 NAMES ${OpenGL_LIB})
			IF(OpenGL_LIB1)
				OV_PRINT(OV_PRINTED "    [  OK  ] Third party lib ${OpenGL_LIB1}")
				TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${OpenGL_LIB1})
			ELSE(OpenGL_LIB1)
				MESSAGE(WARNING "    [FAILED] Third party lib ${OpenGL_LIB}")
			ENDIF(OpenGL_LIB1)
		ENDFOREACH(OpenGL_LIB)
		ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyOpenGL)
	ELSE(OpenGL_FOUND)
		MESSAGE(WARNING "  FAILED to find OpenGL...")
	ENDIF(OpenGL_FOUND)
ENDIF(WIN32)