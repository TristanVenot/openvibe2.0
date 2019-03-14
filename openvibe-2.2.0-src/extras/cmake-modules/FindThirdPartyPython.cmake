# ---------------------------------
# Finds Python
# Adds library to target
# Adds include path
# ---------------------------------

GET_PROPERTY(OV_PRINTED GLOBAL PROPERTY OV_TRIED_ThirdPartyPython)

SET(Python_ADDITIONAL_VERSIONS 2.7)

IF(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
# This hackery is needed to try to sort situations where there might be both 32bit and 64bit python present
	FIND_PACKAGE(PythonLibs 2.7)
	IF(NOT PythonLibs_FOUND)
		IF(EXISTS "C:/python27_x64")
			SET(PYTHON_LIBRARY	"C:/python27_x64/libs/python27.lib")
			SET(PYTHON_INCLUDE_DIR "C:/python27_x64/Include/")
			FIND_PACKAGE(PythonLibs 2.7)
		ENDIF()
	ENDIF()
ELSE()
	FIND_PACKAGE(PythonLibs 2.7)
ENDIF()

# ENDIF()

IF(PythonLibs_FOUND)
	OV_PRINT(OV_PRINTED "  Found Python...")
	INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${PYTHON_LIBRARIES})

	# FILE(GLOB_RECURSE lib_files ${PATH_PYTHON}/libs/*.lib)
	
	# FOREACH(PYTHON_LIB ${lib_files})
		# OV_PRINT(OV_PRINTED "    [  OK  ] Third party lib ${PYTHON_LIB}")
		# TARGET_LINK_LIBRARIES(${PROJECT_NAME} debug ${PYTHON_LIB})
		# TARGET_LINK_LIBRARIES(${PROJECT_NAME} optimized ${PYTHON_LIB})
	# ENDFOREACH(PYTHON_LIB)
	
	IF(WIN32)
		# These are needed not to cause a popup on machines missing the dll 
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} optimized Delayimp )
		SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/DELAYLOAD:python27.dll")
	ENDIF()
	
	ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyPython)
ELSE()
	OV_PRINT(OV_PRINTED "  FAILED to find Python (needs v2.7 with bitness matching build target ${PLATFORM_TARGET})")
ENDIF()

SET_PROPERTY(GLOBAL PROPERTY OV_TRIED_ThirdPartyPython "Yes")

