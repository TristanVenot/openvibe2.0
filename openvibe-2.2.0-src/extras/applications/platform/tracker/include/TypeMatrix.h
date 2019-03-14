
#pragma once

#include <openvibe/ov_all.h>

#include "Chunk.h"

namespace OpenViBETracker
{

/**
 * \class TypeMatrix
 * \brief Abstact class defining chunk types for Matrix streams
 * \author J. T. Lindgren
 *
 */
class TypeMatrix
{
public:
	static OpenViBE::CIdentifier getTypeIdentifier(void) { return OV_TypeId_StreamedMatrix; };

	class Header : public Chunk
	{
	public:
		// Header
		OpenViBE::CMatrix m_header;	

		// Make sure not copied until we have proper implementations 
		Header() { };
		Header& operator=(const Header& in) = delete;
		Header(const Header& in) = delete;
	};

	class Buffer : public Chunk
	{
	public:
		// Payload
		OpenViBE::CMatrix m_buffer;	

		// Make sure not copied until we have proper implementations 
		Buffer() { };
		Buffer& operator=(const Buffer& in) = delete;
		Buffer(const Buffer& in) = delete;
	};

	class End : public Chunk
	{ 

	};

	// Prevent constructing
	TypeMatrix()=delete;
	TypeMatrix(const TypeMatrix&)=delete;
	TypeMatrix(TypeMatrix&&)=delete;
};

};

