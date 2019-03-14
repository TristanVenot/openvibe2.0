
#pragma once

#include <openvibe/ov_all.h>

#include "Chunk.h"

namespace OpenViBETracker
{

/**
 * \class TypeError 
 * \brief Abstract class for error situations
 * \details
 *
 * \author J. T. Lindgren
 *
 */

class TypeError {
public:
	static OpenViBE::CIdentifier getTypeIdentifier(void) { return OV_UndefinedIdentifier; };

	class Header : public Chunk 
	{
	};

	class Buffer : public Chunk
	{
	};

	class End : public Chunk
	{
	};

	// Prevent constructing
	TypeError()=delete;
	TypeError(const TypeError&)=delete;
	TypeError(TypeError&&)=delete;
};

};
