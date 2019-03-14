
#pragma once

#include <openvibe/ov_all.h>

#include "Chunk.h"

namespace OpenViBETracker
{

/**
 * \class TypeStimulation
 * \brief Abstact class defining chunk types for Stimulation streams
 * \author J. T. Lindgren
 *
 */
class TypeStimulation
{
public:
	static OpenViBE::CIdentifier getTypeIdentifier(void) { return OV_TypeId_Stimulations; };

	class Header : public Chunk
	{
	};

	class Buffer : public Chunk
	{
	public:
		// Payload
		OpenViBE::CStimulationSet m_buffer;

		// Make sure not copied until we have proper implementations 
		Buffer() { };
		Buffer& operator=(const Buffer& in) = delete;
		Buffer(const Buffer& in) = delete;
	};

	class End : public Chunk { };

	// Prevent constructing
	TypeStimulation()=delete;
	TypeStimulation(const TypeStimulation&)=delete;
	TypeStimulation(TypeStimulation&&)=delete;
};


};
