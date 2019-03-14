
#pragma once

#include <openvibe/ov_all.h>

#include "TypeMatrix.h"

namespace OpenViBETracker
{

/**
 * \class TypeSignal
 * \brief Abstact class defining chunk types for Signal streams
 * \author J. T. Lindgren
 *
 */
class TypeSignal
{
public:
	static OpenViBE::CIdentifier getTypeIdentifier(void) { return OV_TypeId_Signal; };

	class Header : public TypeMatrix::Header
	{
	public:
		// Header
		uint64_t m_SamplingRate = 0;
	};

	class Buffer : public TypeMatrix::Buffer
	{
	};

	class End : public TypeMatrix::End { };

	// Prevent constructing
	TypeSignal()=delete;
	TypeSignal(const TypeSignal&)=delete;
	TypeSignal(TypeSignal&&)=delete;

//	class Decoder : public OpenViBEToolkit::TSignalDecoder<BoxAlgorithmProxy> { };

};



};
