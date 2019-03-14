
IF(OV_DISABLE_GTK)
  MESSAGE(STATUS  "Skipping Tracker, no GTK")
  RETURN()
ENDIF(OV_DISABLE_GTK)

PROJECT(openvibe-tracker)

SET(PROJECT_VERSION_MAJOR ${OV_GLOBAL_VERSION_MAJOR})
SET(PROJECT_VERSION_MINOR ${OV_GLOBAL_VERSION_MINOR})
SET(PROJECT_VERSION_PATCH ${OV_GLOBAL_VERSION_PATCH})
SET(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

FILE(GLOB_RECURSE source_files src/*.cpp src/*.h src/*.inl include/*.h)

# @FIXME bad hack, we are cannibalizing a lot of non-exported code from Designer and Kernel
# Its better than copy-paste but it'd be preferable to either export the necessary materials 
# or make another solution.
SET(OV_AVP_PATH "${CMAKE_SOURCE_DIR}/../designer/plugins/visualization/ovp-advanced-visualization/src/")
LIST(APPEND source_files ${OV_AVP_PATH}/m_defines.hpp
  ${OV_AVP_PATH}/m_GtkGL.cpp
  ${OV_AVP_PATH}/m_GtkGL.hpp
  ${OV_AVP_PATH}/m_VisualizationTools.cpp
  ${OV_AVP_PATH}/m_VisualizationTools.hpp
  ${OV_AVP_PATH}/mTGtkGLWidget.hpp
  ${OV_AVP_PATH}/mIRuler.hpp
)

FILE(GLOB RULER_FILES RELATIVE ${OV_AVP_PATH}/ruler/ *.hpp)
# MESSAGE(STATUS "FOUND ${RULER_FILES} in ${OV_AVP_PATH}")
# AUX_SOURCE_DIRECTORY(${OV_AVP_PATH}/ruler RULER_FILES)
LIST(APPEND source_files ${RULER_FILES})

# from wip-jlindgre-feature-renderer-offset
ADD_DEFINITIONS("-DRENDERER_SUPPORTS_OFFSET")

SET(OV_DESIGNER_PATH "${CMAKE_SOURCE_DIR}/../designer/applications/platform/designer/src/")
FILE(GLOB designer_files 
  ${OV_DESIGNER_PATH}/dynamic_settings/*.h ${OV_DESIGNER_PATH}/dynamic_settings/*.cpp
  ${OV_DESIGNER_PATH}/ovd_base.h
  ${OV_DESIGNER_PATH}/ovdCBoxConfigurationDialog.*
  ${OV_DESIGNER_PATH}/ovdCLogListenerDesigner.* 
  ${OV_DESIGNER_PATH}/ovdCCommentEditorDialog.*    
  ${OV_DESIGNER_PATH}/ovdCSettingCollectionHelper.*
)
LIST(APPEND source_files ${designer_files})

configure_file(${OV_DESIGNER_PATH}/../share/interface.ui-base "${CMAKE_CURRENT_BINARY_DIR}/designer-interface.ui")
configure_file(${OV_DESIGNER_PATH}/../share/interface-settings.ui-base "${CMAKE_CURRENT_BINARY_DIR}/designer-interface-settings.ui")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/designer-interface.ui" DESTINATION ${DIST_DATADIR}/openvibe/applications/tracker)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/designer-interface-settings.ui" DESTINATION ${DIST_DATADIR}/openvibe/applications/tracker)
 
SET(OV_KERNEL_PATH "${CMAKE_SOURCE_DIR}/../sdk/kernel/src/kernel/")
FILE(GLOB kernel_files
  ${OV_KERNEL_PATH}/scenario/ovkCBoxProto.*
  ${OV_KERNEL_PATH}/scenario/ovkTAttributable.h    
  ${OV_KERNEL_PATH}/ovkTKernelObject.h
)
LIST(APPEND source_files ${kernel_files})

ADD_EXECUTABLE(${PROJECT_NAME} ${source_files})
INCLUDE_DIRECTORIES("include")
INCLUDE_DIRECTORIES(${OV_AVP_PATH})
INCLUDE_DIRECTORIES(${OV_DESIGNER_PATH})
INCLUDE_DIRECTORIES(${OV_KERNEL_PATH})
INCLUDE_DIRECTORIES(${OV_KERNEL_PATH}/scenario)

INCLUDE("FindOpenViBE")
INCLUDE("FindOpenViBECommon")
INCLUDE("FindOpenViBEToolkit")
INCLUDE("FindOpenViBEModuleEBML")
INCLUDE("FindOpenViBEModuleSocket")
INCLUDE("FindOpenViBEModuleSystem")
INCLUDE("FindOpenViBEModuleFS")
INCLUDE("FindOpenViBEModuleMessaging")
INCLUDE("FindOpenViBEModuleXML")
INCLUDE("FindThirdPartyGTK")
INCLUDE("FindThirdPartyBoost_System")
INCLUDE("FindThirdPartyOpenGL")
INCLUDE("FindOpenViBEVisualizationToolkit")
INCLUDE("FindLibMensiaAdvancedVisualisation")

IF(WIN32)	
	# @FIXME CERT getting timeBeginPeriod() linker issues without this
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} winmm)
ENDIF(WIN32)

INCLUDE("FindThirdPartyBoost")
INCLUDE("FindThirdPartyRT")  # external stims shared memory needs this

# ---------------------------------
# Target macros
# Defines target operating system
# Defines target architecture
# Defines target compiler
# ---------------------------------
IF(WIN32)
	ADD_DEFINITIONS(-DWIN32_LEAN_AND_MEAN)
ENDIF(WIN32)

# ----------------------
# Generate launch script
# ----------------------
OV_INSTALL_LAUNCH_SCRIPT(SCRIPT_PREFIX "${PROJECT_NAME}" EXECUTABLE_NAME  "${PROJECT_NAME}")

# -----------------------------
# Install files
# -----------------------------
INSTALL(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION ${DIST_BINDIR}
	LIBRARY DESTINATION ${DIST_LIBDIR}
	ARCHIVE DESTINATION ${DIST_LIBDIR})

configure_file(share/tracker.ui-base "${CMAKE_CURRENT_BINARY_DIR}/tracker.ui" @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/tracker.ui" DESTINATION ${DIST_DATADIR}/openvibe/applications/tracker)

INSTALL(DIRECTORY signals     DESTINATION ${DIST_DATADIR}/openvibe/scenarios/)
INSTALL(DIRECTORY share/      DESTINATION ${DIST_DATADIR}/openvibe/applications/tracker PATTERN "*-base" EXCLUDE)
INSTALL(DIRECTORY workspaces  DESTINATION ${DIST_DATADIR}/openvibe/scenarios/)

