#ifndef __XML_Defines_H__
#define __XML_Defines_H__

#include <ov_common_defines.h>

#define OV_APPEND_TO_NAMESPACE XML
#include <ov_common_types.h>
#undef OV_APPEND_TO_NAMESPACE

#if defined XML_Shared
	#if defined TARGET_OS_Windows
		#define XML_API_Export __declspec(dllexport)
		#define XML_API_Import __declspec(dllimport)
	#elif defined TARGET_OS_Linux
		#define XML_API_Export __attribute__((visibility("default")))
		#define XML_API_Import __attribute__((visibility("default")))
	#else
		#define XML_API_Export
		#define XML_API_Import
	#endif
#else
	#define XML_API_Export
	#define XML_API_Import
#endif

#if defined XML_Exports
	#define XML_API XML_API_Export
#else
	#define XML_API XML_API_Import
#endif

#endif // __XML_Defines_H__
