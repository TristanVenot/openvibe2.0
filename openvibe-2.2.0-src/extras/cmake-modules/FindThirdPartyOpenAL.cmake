# ---------------------------------
# Finds OpenAL, ALUT, OGGVORBIS
# Adds library to target
# Adds include path
# ---------------------------------

GET_PROPERTY(OV_PRINTED GLOBAL PROPERTY OV_TRIED_ThirdPartyOpenAL)

IF(WIN32)
	FIND_PATH(PATH_OPENAL include/al.h PATHS ${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES openal . NO_DEFAULT_PATH)
	FIND_PATH(PATH_ALUT include/AL/alut.h PATHS ${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES freealut .)
	FIND_PATH(PATH_OGG include/ogg/ogg.h PATHS ${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES libogg .)
	FIND_PATH(PATH_VORBIS include/vorbis/vorbisfile.h PATHS ${LIST_DEPENDENCIES_PATH} PATH_SUFFIXES libvorbis .)
	
	SET(BUILDTYPE_PATH_DEBUG "debug")
	SET(BUILDTYPE_PATH_RELEASE "release")
	
	IF(PATH_OPENAL)
		OV_PRINT(OV_PRINTED "  Found OpenAL...")
		IF("${PLATFORM_TARGET}" STREQUAL "x64")
			SET(OPENAL_SUBDIR "Win64")
		ELSE()
			SET(OPENAL_SUBDIR "Win32")		
		ENDIF()		
		FIND_LIBRARY(LIB_OPENAL OpenAL32 PATHS ${PATH_OPENAL}/libs/${OPENAL_SUBDIR}/ NO_DEFAULT_PATH)
		IF(LIB_OPENAL)
			OV_PRINT(OV_PRINTED "    [  OK  ] lib ${LIB_OPENAL}")
			INSTALL(DIRECTORY "${PATH_OPENAL}/libs/${OPENAL_SUBDIR}/" DESTINATION "${DIST_BINDIR}/"  FILES_MATCHING PATTERN "*.dll" PATTERN "EFX*" EXCLUDE)
		ELSE(LIB_OPENAL)
			OV_PRINT(OV_PRINTED "    [FAILED] lib OpenAL32")
			SET(OPENAL_MISSED_SOMETHING YES)
		ENDIF(LIB_OPENAL)
	ELSE(PATH_OPENAL)
		OV_PRINT(OV_PRINTED "  FAILED to find OpenAL32")
		SET(OPENAL_MISSED_SOMETHING YES)				
	ENDIF(PATH_OPENAL)

	IF(PATH_ALUT)
		FIND_LIBRARY(LIB_ALUT alut PATHS ${PATH_ALUT}/lib NO_DEFAULT_PATH)
		IF(LIB_ALUT)
			OV_PRINT(OV_PRINTED "    [  OK  ] lib ${LIB_ALUT}")
			INSTALL(DIRECTORY "${PATH_ALUT}/lib/" DESTINATION "${DIST_BINDIR}/" FILES_MATCHING PATTERN "*.dll")
		ELSE(LIB_ALUT)
			OV_PRINT(OV_PRINTED "    [FAILED] lib alut")
			SET(OPENAL_MISSED_SOMETHING YES)						
		ENDIF(LIB_ALUT)			
	ELSE(PATH_ALUT)
		OV_PRINT(OV_PRINTED "  -- FAILED to find ALUT")	
		SET(OPENAL_MISSED_SOMETHING YES)				
	ENDIF(PATH_ALUT)

	IF(PATH_OGG)
		FIND_LIBRARY(LIB_OGG_RELEASE libogg PATHS ${PATH_OGG}/win32/lib/${BUILDTYPE_PATH_RELEASE} NO_DEFAULT_PATH)
		IF(LIB_OGG_RELEASE)
			OV_PRINT(OV_PRINTED "    [  OK  ] lib ${LIB_OGG_RELEASE} (release)")
			# We only have release OpenAL, so we have to include release version for libogg, and libvorbis
			INSTALL(DIRECTORY "${PATH_OGG}/win32/bin/${BUILDTYPE_PATH_RELEASE}/" DESTINATION "${DIST_BINDIR}/") # CONFIGURATIONS Release)
		ELSE(LIB_OGG_RELEASE)
			OV_PRINT(OV_PRINTED "    [FAILED] lib ogg (release)")
			SET(OPENAL_MISSED_SOMETHING YES)	
		ENDIF(LIB_OGG_RELEASE)
		
		FIND_LIBRARY(LIB_OGG_DEBUG libogg PATHS ${PATH_OGG}/win32/lib/${BUILDTYPE_PATH_DEBUG} NO_DEFAULT_PATH)
		IF(LIB_OGG_DEBUG)
			OV_PRINT(OV_PRINTED "    [  OK  ] lib ${LIB_OGG_DEBUG} (debug)")
			# INSTALL(DIRECTORY "${PATH_OGG}/win32/bin/${BUILDTYPE_PATH_DEBUG}/" DESTINATION "${DIST_BINDIR}/" CONFIGURATIONS Debug)
		ELSE(LIB_OGG_DEBUG)
			OV_PRINT(OV_PRINTED "    [FAILED] lib ogg (debug)")
			SET(OPENAL_MISSED_SOMETHING YES)
		ENDIF(LIB_OGG_DEBUG)		
	ELSE(PATH_OGG)
		OV_PRINT(OV_PRINTED "  -- FAILED to find OGG")
		SET(OPENAL_MISSED_SOMETHING YES)				
	ENDIF(PATH_OGG)
	
	IF(PATH_VORBIS)
		FIND_LIBRARY(LIB_VORBIS_RELEASE libvorbis PATHS ${PATH_VORBIS}/win32/lib/${BUILDTYPE_PATH_RELEASE} NO_DEFAULT_PATH)
		IF(LIB_VORBIS_RELEASE)
			OV_PRINT(OV_PRINTED "    [  OK  ] lib ${LIB_VORBIS_RELEASE} (release)")
			INSTALL(FILES "${PATH_VORBIS}/win32/bin/${BUILDTYPE_PATH_RELEASE}/libvorbis.dll" DESTINATION "${DIST_BINDIR}") # CONFIGURATIONS Release)
		ELSE(LIB_VORBIS_RELEASE)
			OV_PRINT(OV_PRINTED "    [FAILED] lib vorbis (release)")
			SET(OPENAL_MISSED_SOMETHING YES)
		ENDIF(LIB_VORBIS_RELEASE)

		FIND_LIBRARY(LIB_VORBIS_DEBUG libvorbis PATHS ${PATH_VORBIS}/win32/lib/${BUILDTYPE_PATH_DEBUG} NO_DEFAULT_PATH)
		IF(LIB_VORBIS_DEBUG)
			OV_PRINT(OV_PRINTED "    [  OK  ] lib ${LIB_VORBIS_DEBUG} (debug)")
			# INSTALL(FILES "${PATH_VORBIS}/win32/bin/${BUILDTYPE_PATH_DEBUG}/libvorbis.dll" DESTINATION "${DIST_BINDIR}" CONFIGURATIONS Debug)
		ELSE(LIB_VORBIS_DEBUG)
			OV_PRINT(OV_PRINTED "    [FAILED] lib vorbis (debug)")
		SET(OPENAL_MISSED_SOMETHING YES)
			ENDIF(LIB_VORBIS_DEBUG)
			
		FIND_LIBRARY(LIB_VORBISFILE_RELEASE libvorbisfile PATHS ${PATH_VORBIS}/win32/lib/${BUILDTYPE_PATH_RELEASE} NO_DEFAULT_PATH)
		IF(LIB_VORBISFILE_RELEASE)
			OV_PRINT(OV_PRINTED "    [  OK  ] lib ${LIB_VORBISFILE_RELEASE} (release)")
			INSTALL(FILES "${PATH_VORBIS}/win32/bin/${BUILDTYPE_PATH_RELEASE}/libvorbisfile.dll" DESTINATION "${DIST_BINDIR}")# CONFIGURATIONS Release)
		ELSE(LIB_VORBISFILE_RELEASE)
			OV_PRINT(OV_PRINTED "    [FAILED] lib vorbisfile (release)")
			SET(OPENAL_MISSED_SOMETHING YES)
		ENDIF(LIB_VORBISFILE_RELEASE)
					
		FIND_LIBRARY(LIB_VORBISFILE_DEBUG libvorbisfile PATHS ${PATH_VORBIS}/win32/lib/${BUILDTYPE_PATH_DEBUG} NO_DEFAULT_PATH)
		IF(LIB_VORBISFILE_DEBUG)
			OV_PRINT(OV_PRINTED "    [  OK  ] lib ${LIB_VORBISFILE_DEBUG} (debug)")
			# INSTALL(FILES "${PATH_VORBIS}/win32/bin/${BUILDTYPE_PATH_DEBUG}/libvorbisfile.dll" DESTINATION "${DIST_BINDIR}" CONFIGURATIONS Debug)
		ELSE(LIB_VORBISFILE_DEBUG)
			OV_PRINT(OV_PRINTED "    [FAILED] lib vorbisfile (debug)")
			SET(OPENAL_MISSED_SOMETHING YES)
		ENDIF(LIB_VORBISFILE_DEBUG)				
	ELSE(PATH_VORBIS)
		OV_PRINT(OV_PRINTED " -- FAILED to find VORBIS")
		SET(OPENAL_MISSED_SOMETHING YES)		
	ENDIF(PATH_VORBIS)					

	IF(OPENAL_MISSED_SOMETHING) 
		OV_PRINT(OV_PRINTED "  FAILED to find everything required by OpenAL")
	ELSE(OPENAL_MISSED_SOMETHING)
		OV_PRINT(OV_PRINTED "  Found OpenAL and all its library dependencies...")
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_OPENAL})	
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_ALUT})						

		TARGET_LINK_LIBRARIES(${PROJECT_NAME} debug ${LIB_OGG_DEBUG} optimized ${LIB_OGG_RELEASE} )
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} debug ${LIB_VORBIS_DEBUG} optimized ${LIB_VORBIS_RELEASE} )	
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} debug ${LIB_VORBISFILE_DEBUG} optimized ${LIB_VORBISFILE_RELEASE})

		INCLUDE_DIRECTORIES(${PATH_OPENAL}/include ${PATH_ALUT}/include ${PATH_OGG}/include ${PATH_VORBIS}/include)
		ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyOpenAL)
	ENDIF(OPENAL_MISSED_SOMETHING)

ENDIF(WIN32)

IF(UNIX)
	INCLUDE("FindThirdPartyPkgConfig")
	pkg_check_modules(ALUT freealut)

	IF(ALUT_FOUND)
		OV_PRINT(OV_PRINTED "  Found ALUT...")
		INCLUDE_DIRECTORIES(${ALUT_INCLUDE_DIRS})
		ADD_DEFINITIONS(${ALUT_CFLAGS})
		ADD_DEFINITIONS(${ALUT_CFLAGS_OTHERS})
		# LINK_DIRECTORIES(${ALUT_LIBRARY_DIRS})
		FOREACH(ALUT_LIB ${ALUT_LIBRARIES})
			SET(ALUT_LIB1 "ALUT_LIB1-NOTFOUND")
			FIND_LIBRARY(ALUT_LIB1 NAMES ${ALUT_LIB} PATHS ${ALUT_LIBRARY_DIRS} ${ALUT_LIBDIR} NO_DEFAULT_PATH)
			FIND_LIBRARY(ALUT_LIB1 NAMES ${ALUT_LIB})
			IF(ALUT_LIB1)
				OV_PRINT(OV_PRINTED "    [  OK  ] Third party lib ${ALUT_LIB1}")
				TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${ALUT_LIB1})
			ELSE(ALUT_LIB1)
				OV_PRINT(OV_PRINTED "    [FAILED] Third party lib ${ALUT_LIB}")
			ENDIF(ALUT_LIB1)
		ENDFOREACH(ALUT_LIB)
	ELSE(ALUT_FOUND)
		OV_PRINT(OV_PRINTED "  FAILED to find ALUT...")
	ENDIF(ALUT_FOUND)
	
	pkg_check_modules(VORBIS vorbisfile)

	IF(VORBIS_FOUND)
		OV_PRINT(OV_PRINTED "  Found VORBIS...")
		INCLUDE_DIRECTORIES(${VORBIS_INCLUDE_DIRS})
		ADD_DEFINITIONS(${VORBIS_CFLAGS})
		ADD_DEFINITIONS(${VORBIS_CFLAGS_OTHERS})
		# LINK_DIRECTORIES(${VORBIS_LIBRARY_DIRS})
		FOREACH(VORBIS_LIB ${VORBIS_LIBRARIES})
			SET(VORBIS_LIB1 "VORBIS_LIB1-NOTFOUND")
			FIND_LIBRARY(VORBIS_LIB1 NAMES ${VORBIS_LIB} PATHS ${VORBIS_LIBRARY_DIRS} ${VORBIS_LIBDIR} NO_DEFAULT_PATH)
			FIND_LIBRARY(VORBIS_LIB1 NAMES ${VORBIS_LIB})
			IF(VORBIS_LIB1)
				OV_PRINT(OV_PRINTED "    [  OK  ] Third party lib ${VORBIS_LIB1}")
				TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${VORBIS_LIB1})
			ELSE(VORBIS_LIB1)
				OV_PRINT(OV_PRINTED "    [FAILED] Third party lib ${VORBIS_LIB}")
			ENDIF(VORBIS_LIB1)
		ENDFOREACH(VORBIS_LIB)
		ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyOpenAL)
	ELSE(VORBIS_FOUND)
		OV_PRINT(OV_PRINTED "  FAILED to find VORBIS...")
	ENDIF(VORBIS_FOUND)
	
ENDIF(UNIX)



SET_PROPERTY(GLOBAL PROPERTY OV_TRIED_ThirdPartyOpenAL "Yes")
