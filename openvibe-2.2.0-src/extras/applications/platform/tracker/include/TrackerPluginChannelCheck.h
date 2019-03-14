
#pragma once

#include "ITrackerPlugin.h"

namespace OpenViBETracker
{

/**
 * \class TrackerPluginChannelCheck 
 * \brief Example of a Tracker plugin processing given Workspaces. It checks if all selected signal streams in all tracks have the same number of channels.
 * \author J. T. Lindgren
 *
 */
class TrackerPluginChannelCheck : public ITrackerPlugin {
public:

	TrackerPluginChannelCheck(const OpenViBE::Kernel::IKernelContext& ctx) : ITrackerPlugin(ctx) { };

	bool process(Workspace& wp, ParallelExecutor& exec) override;

	bool hasCapability(Capabilities capability) override { return (capability == PLUGIN_PROCESSES_WORKSPACE); };

	std::string getName(void) override { return std::string("Example: Channel check"); } 
};

};
