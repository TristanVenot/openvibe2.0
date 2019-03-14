#ifndef __System_Defines_H__
#define __System_Defines_H__

#include <ov_common_defines.h>

#define OV_APPEND_TO_NAMESPACE System
#include <ov_common_types.h>
#undef OV_APPEND_TO_NAMESPACE

#if defined System_Shared
	#if defined TARGET_OS_Windows
		#define System_API_Export __declspec(dllexport)
		#define System_API_Import __declspec(dllimport)
	#elif defined TARGET_OS_Linux || defined TARGET_OS_MacOS
		#define System_API_Export __attribute__((visibility("default")))
		#define System_API_Import __attribute__((visibility("default")))
	#else
		#define System_API_Export
		#define System_API_Import
	#endif
#else
	#define System_API_Export
	#define System_API_Import
#endif

#if defined System_Exports
	#define System_API System_API_Export
#else
	#define System_API System_API_Import
#endif

#endif // __System_Defines_H__
