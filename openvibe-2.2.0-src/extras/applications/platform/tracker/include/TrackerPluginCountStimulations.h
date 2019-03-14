
#pragma once

#include "ITrackerPlugin.h"

namespace OpenViBETracker
{

/**
 * \class TrackerPluginCountStimulations 
 * \brief Example of a Tracker plugin that processes given tracks. It counts the occurrences of different stimulations in each stimulation stream.
 * \author J. T. Lindgren
 *
 */
class TrackerPluginCountStimulations : public ITrackerPlugin {
public:

	TrackerPluginCountStimulations(const OpenViBE::Kernel::IKernelContext& ctx) : ITrackerPlugin(ctx) { };

	bool process(StreamBundle& track) override;

	std::string getName(void) override { return std::string("Example: Count stimulations"); } 
};

};
