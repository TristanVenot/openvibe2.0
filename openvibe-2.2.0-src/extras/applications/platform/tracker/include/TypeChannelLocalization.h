
#pragma once

#include <openvibe/ov_all.h>

#include "TypeMatrix.h"

namespace OpenViBETracker
{

/**
 * \class TypeChannelLocalization
 * \brief Abstact class defining chunk types for Channel Localization streams
 * \author J. T. Lindgren
 *
 */
class TypeChannelLocalization
{
public:
	static OpenViBE::CIdentifier getTypeIdentifier(void) { return OV_TypeId_ChannelLocalisation; };

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
	TypeChannelLocalization()=delete;
	TypeChannelLocalization(const TypeChannelLocalization&)=delete;
	TypeChannelLocalization(TypeChannelLocalization&&)=delete;
};


};
