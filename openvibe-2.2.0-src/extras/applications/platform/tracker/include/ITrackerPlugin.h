
#pragma once

#include <openvibe/ov_all.h>

#include "Contexted.h"
#include "StreamBundle.h"
#include "Workspace.h"
#include "Selection.h"
#include "ParallelExecutor.h"

namespace OpenViBETracker 
{

/**
 * \class ITrackerPlugin 
 * \brief Brief
 * \author J. T. Lindgren
 * \details
 *
 * Tracker Plugins are processing plugins specific to the OpenViBE Tracker. 
 * The idea is to allow plugins to access the whole track content (StreamBundle)
 * with a simple interface and minimal overhead.
 *
 * In detail, the difference between Tracker Plugins and Box Plugins is as follows. 
 * A box plugin wraps openvibe box code, and subsequently the code will have 
 * to deal with the classical openvibe objects such as encoders, decoders, and so on. 
 * The Tracker Plugin, on the other hand, gives the programmer access
 * to the StreamBundle structure. This allows the plugin code to request and manipulate
 * the streams and their chunks freely. A tracker plugin can also add or remove streams
 * to the bundle.
 *
 * Tracker plugins do not currently have explicit parameters, but you
 * can pass parameters in using configuration tokens, and then request/set them
 * via m_KernelContext.getConfigurationManager() interface.
 *
 * Note that the StreamBundle given as input to a Tracker Plugin is in not
 * safeguarded against "bad" modifications by the plugin.
 *
 * @todo tracker and box plugins might be refactorable to be under same interface.
 * @todo allow non-inplace mode operation
 * @todo derive different subclasses for the 'capabilities' instead of one monolithic interface?
 *
 */
class ITrackerPlugin : protected Contexted {
public:

	// For the moment these are mutually exclusive capabilities; Tracker will prefer 'workspace' if the plugin supports it.
	enum Capabilities {
		PLUGIN_PROCESSES_TRACKS    = 1LL,
		PLUGIN_PROCESSES_WORKSPACE = 2LL
	};

	// Constructor
	ITrackerPlugin(const OpenViBE::Kernel::IKernelContext& ctx) : Contexted(ctx)	{ };

	// @param track The input track. It can be modified by the process() call.
	// @return True on success, False otherwise
	// @note If the plugin changes the track, it must set the tracks 'dirty bit' as true.
	virtual bool process(StreamBundle& track) { return false; }

	// @param wp The input workspace. It can be modified by the process() call.
	// @param exec A reference to a parallel executor that the plugin can use (if it prefers)
	// @return True on success, False otherwise
	// @note If the plugin changes any track in the workspace, it must set the tracks 'dirty bit' as true.
	// @note Passing the parallel executor as a parameter is not too neat, ideally it'd be part of IKernelContext but that'd require importing the exec to SDK.
	virtual bool process(Workspace& wp, ParallelExecutor& exec) { return false; }

	// @param Capability to ask for
	// @return True if the plugin supports this
	// @note By default, the plugins are expected to have the track processing capability. If not, override this default.
	virtual bool hasCapability(Capabilities capability) { return (capability == PLUGIN_PROCESSES_TRACKS); };

	// @return the name of the plugin
	virtual std::string getName(void) { return std::string("Unnamed"); }
};

};
