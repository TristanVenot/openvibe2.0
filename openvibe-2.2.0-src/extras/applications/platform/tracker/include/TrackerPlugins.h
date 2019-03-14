
#pragma once

#include <openvibe/ov_all.h>

#include "ITrackerPlugin.h"
#include "Contexted.h"

namespace OpenViBETracker
{

/**
 * \class TrackerPlugins 
 * \author J. T. Lindgren
 * \brief Container class for currently registered Tracker plugins. These plugins get direct access to StreamBundles (i.e. full track content).
 * \details See ITrackerPlugin.h
 *
  New Tracker Plugins can be added to the Tracker by registering them in the constructor of this class
 *
 */
class TrackerPlugins : protected Contexted
{
public:

	TrackerPlugins(const OpenViBE::Kernel::IKernelContext& ctx);
	~TrackerPlugins();

	// Get a list of all registered plugins
	const std::vector<ITrackerPlugin*>& getTrackerPlugins(void) const { return m_TrackerPlugins; }

	// Get a fresh plugin. The indexing is the same as in getTrackerPlugins() output
	ITrackerPlugin* getPluginCopy(size_t index) const;

protected:

	// List of currently registered plugins
	std::vector< ITrackerPlugin* > m_TrackerPlugins;

	// List of calls to create new copies of the plugins
	std::vector< std::function<ITrackerPlugin*(void)> > m_PluginCreateCalls;

};

};


