
#pragma once

#include <openvibe/ov_all.h>

#include "TypeMatrix.h"

namespace OpenViBETracker
{

/**
 * \class TypeChannelUnits
 * \brief Abstact class defining chunk types for Channel Units streams
 * \author J. T. Lindgren
 *
 */
class TypeChannelUnits
{
public:
	static OpenViBE::CIdentifier getTypeIdentifier(void) { return OV_TypeId_ChannelUnits; };

	class Header : public TypeMatrix::Header
	{
	public:
		bool m_Dynamic;
	};
	
	class Buffer : public TypeMatrix::Buffer
	{
	public:
		// Payload
	};

	class End : public TypeMatrix::End { };

	// Prevent constructing
	TypeChannelUnits()=delete;
	TypeChannelUnits(const TypeChannelUnits&)=delete;
	TypeChannelUnits(TypeChannelUnits&&)=delete;
};


};
