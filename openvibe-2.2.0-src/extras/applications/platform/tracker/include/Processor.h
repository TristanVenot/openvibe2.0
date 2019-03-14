
#pragma once

#include <openvibe/ov_all.h>
#include <functional>

#include "StreamBundle.h"
#include "Contexted.h"

namespace OpenViBETracker 
{

/**
 * \class Processor 
 * \author J. T. Lindgren
 * \brief A signal processing component that can receive/return data
 * \details
 *
 * Processor in OpenViBE tracker is a kind of bridge to Designer that is 
 * used to send data in, do some processing with a scenario, and get the processed
 * data back.  A processor can also be one-directional.
 *
 *
 */
class Processor : protected Contexted {
public:

	Processor(const OpenViBE::Kernel::IKernelContext& ctx) : Contexted(ctx) {};

	// Set the processor XML file
	virtual bool initialize(const std::string& xmlFile) = 0;
	virtual bool uninitialize(void) = 0;

	// Get the processor XML filename
	virtual const std::string& getFilename(void) const { return m_xmlFilename; }

	// Connect the processor to a specific StreamBundle to read from
	virtual bool setNewSource(StreamBundle* source, bool sendHeader, bool sendEnd) = 0;
	// Connect the processor to a specific StreamBundle to write to
	virtual bool setNewTarget(StreamBundle* target) = 0;

	// Launch Designer to configure the processor
	virtual bool configure(const char* filename); // If filename is NULL, use internal

	// Launch the Player
	virtual bool play(bool playFast, std::function<bool(ovtime_t)> quitCallback) = 0;
	// Stop the Player
	virtual bool stop(void) = 0;
	virtual bool isRunning(void) const = 0;

	virtual ovtime_t getCurrentTime(void) const = 0;

	// Communication port tcp/ip port IDs
	virtual bool setProcessorPorts(uint32_t sendPort, uint32_t recvPort) = 0;
	virtual bool getProcessorPorts(uint32_t& sendPort, uint32_t& recvPort) const = 0;

	// Command line arguments passed to the processor (Designer) launch call
	bool setArguments(const char* args) { m_Arguments = std::string(args); return true; }
	const char* getArguments(void) const { return m_Arguments.c_str(); }

	// Serialize state to configuration manager
	virtual bool save(void) = 0;
	virtual bool load(void) = 0;

protected:

	std::string m_xmlFilename;

	std::string m_Arguments;

	bool m_PlayFast = false;

};

};

