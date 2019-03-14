# /*
 # * FindThirdPartyActiCHampAPI.cmake
 # *
 # * Copyright (c) 2012, Mensia Technologies SA. All rights reserved.
 # * -- Rights transferred to Inria, contract signed 21.11.2014
 # *
 # */

# ---------------------------------
# Finds ActiCHamp library
# Adds library to target
# Adds include path
# ---------------------------------

GET_PROPERTY(OV_PRINTED GLOBAL PROPERTY OV_TRIED_ThirdPartyActiCHampAPI)

IF(WIN32)
	FIND_PATH(PATH_ActiCHampAPI ActiChamp.h PATHS ${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES sdk_brainproducts_actichamp)
	IF(PATH_ActiCHampAPI)
		OV_PRINT(OV_PRINTED "  Found Brain Products actiCHamp API...")
		INCLUDE_DIRECTORIES(${PATH_ActiCHampAPI})

		IF("${PLATFORM_TARGET}" STREQUAL "x64")
			SET(ACTICHAMP_LIBPREFIX "ActiChamp_x64")
		ELSE()
			SET(ACTICHAMP_LIBPREFIX "ActiChamp_x86")		
		ENDIF()
		
		FIND_LIBRARY(LIB_ActiCHampAPI ${ACTICHAMP_LIBPREFIX}.lib PATHS ${PATH_ActiCHampAPI} )
		IF(LIB_ActiCHampAPI)
			OV_PRINT(OV_PRINTED "    [  OK  ] lib ${LIB_ActiCHampAPI}")
			TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_ActiCHampAPI} )
		ELSE()
			OV_PRINT(OV_PRINTED "    [FAILED] lib actiCHamp")
		ENDIF()

		FIND_FILE(FIRMWARE_ActiCHampAPI ActiChamp.bit PATHS ${PATH_ActiCHampAPI} )
		IF(FIRMWARE_ActiCHampAPI)
			OV_PRINT(OV_PRINTED "    [  OK  ] firmware ${FIRMWARE_ActiCHampAPI}")
		ELSE()
			OV_PRINT(OV_PRINTED "    [FAILED] firmware actiCHamp")
		ENDIF()

		# Copy the DLL file at install
		INSTALL(PROGRAMS "${PATH_ActiCHampAPI}/${ACTICHAMP_LIBPREFIX}.dll" DESTINATION ${DIST_BINDIR})

		# Copy the firmware file at install
		INSTALL(PROGRAMS "${FIRMWARE_ActiCHampAPI}" DESTINATION ${DIST_BINDIR})
		
		ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyActiCHampAPI)
	ELSE()
		OV_PRINT(OV_PRINTED "  FAILED to find Brain Products actiCHamp API (optional driver)")
	ENDIF()
ENDIF()

SET_PROPERTY(GLOBAL PROPERTY OV_TRIED_ThirdPartyActiCHampAPI "Yes")

