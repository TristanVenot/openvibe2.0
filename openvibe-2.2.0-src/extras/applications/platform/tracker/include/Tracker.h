//
// OpenViBE Tracker
//

#pragma once

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <iostream>

#include "Workspace.h"
#include "BoxPlugins.h"
#include "TrackerPlugins.h"
#include "Contexted.h"

#include "ParallelExecutor.h"

/**
 * \namespace OpenViBETracker
 * \author J.T. Lindgren (Inria)
 * \date 2018-09
 * \brief Main OpenViBE Tracker namespace
 *
 * All the classes defined in the OpenViBE Tracker are currently included in
 * this namespace.
 */
namespace OpenViBETracker {

/**
 * \class Tracker 
 * \author J. T. Lindgren
 * \brief Tracker is the main class of OpenViBE Tracker that encapsulates the Workspace
 * \details
 * The tracker is in one of the states Stopped, Playing, Paused.
 *
 * - play() will start sending data to a specified processor from the tracks in the workspace.
 * - step() will send out the chunk that is next in time order. Usually called by the GUI idle loop.
 * - stop() will stop sending. Tracker can also stop if the processor quits or if all the data has been sent.
 * 
 * In the stopped state, the Tracker also allows plugins to be applied to the Workspace content.
 *
 * In the future we can extend this class to host more than one workspace at the same time.
 *
 */
class Tracker : protected Contexted
{
public:
	Tracker(const OpenViBE::Kernel::IKernelContext& ctx);
	~Tracker(void);

	enum TrackerState {
		State_Stopped = 0,
		State_Playing,
		State_Paused,
		State_Stopping
	};

	bool initialize(void);

	bool play(bool playFast);
	bool stop(void);

	TrackerState step(void);

	// Useful getters
	TrackerState getCurrentState(void) const { return m_CurrentState; };
	Workspace& getWorkspace(void) { return m_Workspace; };
	const OpenViBE::Kernel::IKernelContext& getKernelContext(void) { return m_KernelContext; };

	// Plugin handling
	const BoxPlugins& getBoxPlugins(void) const { return m_BoxPlugins; }
	const TrackerPlugins& getTrackerPlugins(void) const { return m_TrackerPlugins; }
	bool applyBoxPlugin(size_t index);
	bool applyTrackerPlugin(size_t index);

	uint32_t getNumThreads(void) const { return m_Executor.getNumThreads(); };
	bool setNumThreads(uint32_t numThreads);

	// Configuration handling
	bool saveConfiguration(const OpenViBE::CString& filename);
	bool loadConfiguration(const OpenViBE::CString& filename);

protected:

	Workspace m_Workspace;

	TrackerState m_CurrentState = State_Stopped;
	
	BoxPlugins m_BoxPlugins;
	TrackerPlugins m_TrackerPlugins;

	ParallelExecutor m_Executor;

};


};

