//
// OpenViBE Tracker
// 
// @todo catch way more errors, currently most return values are ignored by callers
// @todo Change error handling to use the macros introduced in certivibe
// @todo clean the code to be more conforming to coding rules
// @todo Implement vertical slider / scale
// @todo Fix time units
// @todo Add track renderer only when the track appears
//
// @todo add dataset manager (workspace == datasets)
// @todo enable a view where only 'current track' is shown and the rest are minimized (unloaded?)
// @todo add option to collapse tracks to take less visual space
// @todo add record button
// @todo implement undo
// @todo fix issues with empty tracks/streams and if the processing fails (e.g. incompatible processor)
// @todo fix creation of noncontinuous streams in the catenate mode e.g. by padding all source streams to equal length?
// @todo write some tutorial processors, e.g. erp analysis, file export ... ? 
// @todo allow running several scenarios sequentially
// @todo selection could be saved in the .ovw file simply as track/stream tokens (small perf hit)?

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <iostream>

#include <system/ovCTime.h>

#include "openvibe/ovITimeArithmetics.h"

#include "Tracker.h"
#include "StreamBundleImportExport.h"

#include "Workspace.h"

using namespace OpenViBE::Kernel;
using namespace OpenViBETracker;

Tracker::Tracker(const OpenViBE::Kernel::IKernelContext& ctx) 
: Contexted(ctx), m_Workspace(ctx), m_BoxPlugins(ctx), m_TrackerPlugins(ctx)
{
	// Default config load deferred until initialize() which can be called after booting the GUI
	// --> so we can get the possible log messages to the GUI
}

Tracker::~Tracker(void)
{
	const OpenViBE::CString configFile = m_KernelContext.getConfigurationManager().expand("${Path_UserData}/openvibe-tracker.conf");

	saveConfiguration(configFile);

	m_Executor.uninitialize();
}

bool Tracker::initialize(void)
{
	const OpenViBE::CString configFile = m_KernelContext.getConfigurationManager().expand("${Path_UserData}/openvibe-tracker.conf");

	// n.b. Initing the executor is in loadConfiguration so its not done twice
	m_Workspace.setParallelExecutor(&m_Executor);

	return loadConfiguration(configFile);
}

bool Tracker::play(bool playFast)
{
	switch(m_CurrentState)
	{
	case State_Stopped:
		if(!m_Workspace.play(playFast))
		{
			log() << LogLevel_Error << "Error: play failed\n";
			return false;
		}
		m_CurrentState = State_Playing;
		break;
	case State_Playing:
		m_CurrentState = State_Paused;
		break;
	case State_Paused:
		m_CurrentState = State_Playing;
		break;
	default:
		break;
	}

	return true;
}

bool Tracker::stop(void)
{
	// Request workspace to stop, do not change state until it has stopped
	return m_Workspace.stop();
}

Tracker::TrackerState Tracker::step(void)
{
	if(m_CurrentState==State_Playing)
	{
		if(!m_Workspace.step())
		{
			m_CurrentState = State_Stopping;
		}
	} 
	else if(m_CurrentState==State_Stopping)
	{
		if(m_Executor.isIdle())
		{
			m_CurrentState = State_Stopped;
		}
	}
	else
	{
		System::Time::sleep(1);
	}

	return m_CurrentState;
}

struct Workpackage
{
	StreamBundle *source = nullptr;
	std::mutex oMutex;
	size_t trackIndex;
	std::deque<size_t> streamsToProcess;
};


bool Tracker::applyBoxPlugin(size_t index)
{
	auto& plugins = getBoxPlugins().getBoxPlugins();
	if (index >= plugins.size())
	{
		log() << LogLevel_Error << "Plugin index exceeds plugin array size\n";
		return false;
	}

	if (getWorkspace().getMemorySaveMode() && getWorkspace().getWorkingPath().length() == 0)
	{
		log() << LogLevel_Error << "Memory save mode requires a workspace path\n";
		return false;
	}

	ovtime_t startTime = System::Time::zgetTime();

	BoxAdapterStream* box = plugins[index];

	const OpenViBE::CString name = box->getBox().getName();
	log() << LogLevel_Info << "Applying method " << name << " (" << (m_Workspace.getInplaceMode() ? "Inplace" : "Normal") << " mode)\n";

	for (size_t trackIndex = 0; trackIndex < getWorkspace().getNumTracks(); trackIndex++)
	{
		Workpackage* wptr = new Workpackage;
		Workspace& ws = getWorkspace();

		StreamBundle* sourceTrack = ws.getTrack(trackIndex);

		// Count
		const size_t numStreams = ws.getTrack(trackIndex)->getNumStreams();
		for (size_t j = 0; j < numStreams; j++)
		{
			if (sourceTrack->getStream(j)->getSelected() && box->getBox().hasInputSupport(sourceTrack->getStream(j)->getTypeIdentifier()))
			{
				wptr->streamsToProcess.push_back(j);
			}
		}
		const size_t nToProcess = wptr->streamsToProcess.size();
		if(nToProcess==0)
		{
			continue;
		}
		
		wptr->trackIndex = trackIndex;
		wptr->source = (m_Workspace.getMemorySaveMode() ? nullptr : sourceTrack);

		for (size_t j = 0; j < nToProcess; j++)
		{
			const bool inplaceMode = m_Workspace.getInplaceMode();
			const bool multithread =  (m_Executor.getNumThreads() > 1);
			const bool memorySave =  m_Workspace.getMemorySaveMode();

			const IKernelContext& ctx = getKernelContext();

			auto job = [&ws, wptr, &ctx, box, multithread, inplaceMode, memorySave](uint32_t threadNumber) 
			{
				uint32_t sourceStreamIndex;
				{
					std::unique_lock<std::mutex>(wptr->oMutex);
					if(memorySave && !wptr->source)
					{
						wptr->source = readStreamBundleFromFile(ctx,
							ws.getTrack(wptr->trackIndex)->getSource().c_str(), false);
					}
					sourceStreamIndex = wptr->streamsToProcess.front();
					wptr->streamsToProcess.pop_front();
				}

				StreamBundle* sourceTrack = wptr->source;
				StreamPtr sourceStream = sourceTrack->getStream(sourceStreamIndex);

				// process

				if(!multithread)
				{
					ctx.getLogManager() << LogLevel_Info << "Processing track " << (wptr->trackIndex+1) << " stream " << (sourceStreamIndex+1) << "\n";
				}

				StreamPtr targetStream = nullptr;
				size_t targetStreamIndex = 0;
				{
					std::unique_lock<std::mutex>(wptr->oMutex);
				
					targetStreamIndex = sourceTrack->getNumStreams();
					sourceTrack->createStream(targetStreamIndex, sourceStream->getTypeIdentifier());
					targetStream = sourceTrack->getStream(targetStreamIndex);
				}

				// We need a copy of the box since otherwise different threads init the same box differently
				BoxAdapterStream* boxCopy = new BoxAdapterStream(ctx, box->getAlgorithmId());

				boxCopy->getBox().initializeFromExistingBox(box->getBox());

				boxCopy->setSource(sourceStream);
				boxCopy->setTarget(targetStream);

				if (!boxCopy->initialize()) 
				{
					std::unique_lock<std::mutex>(wptr->oMutex);

					sourceTrack->deleteStream(targetStreamIndex);
					delete boxCopy;

					return;
				}

				boxCopy->spool( !multithread ); 
				boxCopy->uninitialize(); 
				delete boxCopy;

				if (inplaceMode)
				{
					std::unique_lock<std::mutex>(wptr->oMutex);

					sourceTrack->swapStreams(sourceStreamIndex, targetStreamIndex);
					sourceTrack->deleteStream(targetStreamIndex);
				}
				else
				{
					targetStream->setSelected(false);
				}

				// Spool resources to disk and free memory
				{
					std::unique_lock<std::mutex>(wptr->oMutex);

					if(memorySave)
					{

						// Free resources if this thread is the last
						if (wptr->streamsToProcess.size() == 0)
						{
							if(wptr->source->getDirtyBit())
							{
								// @fixme not a good solution with the filenaming; rethink the whole thing

								std::stringstream ss;
								ss << ws.getWorkingPath() << "/workspace-track-" << (wptr->trackIndex+1) << ".ov";

								// In this mode, sourceTrack is not from the track array. We spool it to disk, then
								// set the filename of the track in the array, and reload it back.
								saveStreamBundleToFile(ctx, wptr->source, ss.str().c_str());
								ws.getTrack(wptr->trackIndex)->setSource(ss.str().c_str());
								ws.reloadTrack(wptr->trackIndex);
							}
							delete wptr->source;
						}				
					}
					if (wptr->streamsToProcess.size() == 0)
					{
						delete wptr;
					}
				}
			};

			m_Executor.pushJob(job);

			if(!multithread)
			{
				m_Executor.waitForAll();
			}
		}
	}

	m_Executor.waitForAll();

	// pBoxAlgorithmDescriptor->release();

	ovtime_t elapsed = System::Time::zgetTime() - startTime;
	log() << LogLevel_Info << "Applying plugin took " << OpenViBE::ITimeArithmetics::timeToSeconds(elapsed) << " sec.\n";

	return true;
}

bool Tracker::applyTrackerPlugin(size_t index)
{
	auto& plugins = getTrackerPlugins().getTrackerPlugins();
	if (index >= plugins.size())
	{
		log() << LogLevel_Error << "Plugin index exceeds plugin array size\n";
		return false;
	}

	if (getWorkspace().getMemorySaveMode() && getWorkspace().getWorkingPath().length() == 0)
	{
		log() << LogLevel_Error << "Memory save mode requires a workspace path\n";
		return false;
	}

	ITrackerPlugin* plugin = plugins[index];

	const OpenViBE::CString name(plugin->getName().c_str());
	log() << LogLevel_Info << "Applying method " << name << " (" << (m_Workspace.getInplaceMode() ? "Inplace" : "Normal") << " mode)\n";

	bool retVal = true;

	if(plugin->hasCapability(ITrackerPlugin::PLUGIN_PROCESSES_WORKSPACE))
	{
		retVal = plugin->process(getWorkspace(), m_Executor);
		if(!retVal)
		{
			log() << LogLevel_Error << "Error processing workspace with the plugin\n";
		}
	} 
	else if(plugin->hasCapability(ITrackerPlugin::PLUGIN_PROCESSES_TRACKS))
	{
		// @note since the different jobs handle different tracks, we don't do locking here.
		for (size_t trackIndex = 0; trackIndex < getWorkspace().getNumTracks(); trackIndex++)
		{
			ITrackerPlugin* pluginCopy = getTrackerPlugins().getPluginCopy(index);

			Workspace& ws = getWorkspace();
			const IKernelContext& ctx = getKernelContext();

			auto job = [pluginCopy,&ws,&ctx,trackIndex](uint32_t threadNumber)
			{
				StreamBundle* sourceTrack;
				if (ws.getMemorySaveMode())
				{
					ctx.getLogManager() << LogLevel_Info << "Loading " << ws.getTrack(trackIndex)->getSource().c_str()
						<< " from file\n";

					sourceTrack = readStreamBundleFromFile(ctx,
						ws.getTrack(trackIndex)->getSource().c_str(), false);
				}
				else
				{
					sourceTrack = ws.getTrack(trackIndex);
				}


				if (!pluginCopy->process(*sourceTrack))
				{
					// log() << LogLevel_Error << "Error processing track << " << (i + 1) << " with the plugin\n";
					// retVal = false;
				}
				delete pluginCopy;

				if (ws.getMemorySaveMode())
				{

					if (sourceTrack->getDirtyBit())
					{
						// @fixme not a good solution with the filenaming; rethink the whole thing

						std::stringstream ss;
						ss << ws.getWorkingPath() << "/workspace-track-" << (trackIndex + 1) << ".ov";

						// In this mode, sourceTrack is not from the track array. We spool it to disk, then
						// set the filename of the track in the array, and reload it back.
						saveStreamBundleToFile(ctx, sourceTrack, ss.str().c_str());
						ws.getTrack(trackIndex)->setSource(ss.str().c_str());
						ws.reloadTrack(trackIndex);
					}

					delete sourceTrack;
				}

			};

			m_Executor.pushJob(job);

			// @todo copy sourcetrack, process the copy, if the processing is
			// successful then replace the original (or not) depending on inplacemode setting

			if (m_Executor.getNumThreads()==1) 
			{
				m_Executor.waitForAll();
			}


		}
		// @todo in principle there's no need to freeze the GUI meanwhile, but allowing it to run in the bg would require locking
		m_Executor.waitForAll();
	}
	else
	{
		log() << LogLevel_Error << "Plugin does not have any known capabilities and cannot be run.\n";
	}

	// pBoxAlgorithmDescriptor->release();

	return retVal;
}

bool Tracker::setNumThreads(uint32_t numThreads) 
{ 
	if( numThreads!=m_Executor.getNumThreads() ) 
	{
		if(numThreads<1)
		{
			log() << LogLevel_Warning << "Minimum number of threads is 1, setting that.\n";
			numThreads = 1;
		}
		else if(numThreads>1)
		{
			log() << LogLevel_Info << "Using " << numThreads << " threads. Concurrency control has not been carefully tested. If you notice issues, switch to 1 thread.\n";
		}
		m_Executor.uninitialize(); 
		return m_Executor.initialize(numThreads); 
	}

	return true; 
};

bool Tracker::loadConfiguration(const OpenViBE::CString& filename) 
{
	if(!m_KernelContext.getConfigurationManager().addConfigurationFromFile(filename))
	{
		m_Executor.initialize(1);

		return false;
	}

	const uint32_t numThreads = (uint32_t)m_KernelContext.getConfigurationManager().expandAsUInteger("${Tracker_NumThreads}", 1);
	m_Executor.initialize(numThreads);
//	m_Executor.launchTest();

	OpenViBE::CString workspaceFile = m_KernelContext.getConfigurationManager().expand("${Tracker_Last_Workspace}");

	//if (m_KernelContext.getConfigurationManager().lookUpConfigurationTokenIdentifier("Tracker_Last_Workspace"))
	//	!= OV_UndefinedIdentifier)
	//{
	//	m_Workspace.load(m_KernelContext.getConfigurationManager().expand("${Tracker_Last_Workspace}");
	//	Tracker_Last_Workspace
	// )
	if (workspaceFile.length() != 0)
	{
		return m_Workspace.load(workspaceFile.toASCIIString());
	}

	return false;
}

// @note : does not save the workspace itself
bool Tracker::saveConfiguration(const OpenViBE::CString& filename)
{
	FILE* l_pFile=::fopen(filename, "wt");
	if(l_pFile)
	{
		::fprintf(l_pFile, "# Configuration file for OpenViBE Tracker, autosaved on Tracker exit\n");
		::fprintf(l_pFile, "#\n");
		::fprintf(l_pFile, "\n");
		::fprintf(l_pFile, "# Last settings used in the Tracker\n");
		::fprintf(l_pFile, "Tracker_Last_Workspace = %s\n", m_Workspace.getFilename().toASCIIString());
		::fprintf(l_pFile, "Tracker_NumThreads = %d\n", m_Executor.getNumThreads() );

		fclose(l_pFile);
	}
	else
	{
		return false;
	}

	return true;
}

#if 0
void testCode(void)
{
	Workspace wp(*kernelWrapper.m_KernelContext);

// 	TestClass tmp(*kernelWrapper.m_KernelContext);

/*

	const CString eegFile = OpenViBE::Directories::getDataDir() + CString("/scenarios/signals/bci-motor-imagery.ov");
//	const CString eegFile = CString("E:/jl/noise-test.ov");
	const CString scenarioFile = OpenViBE::Directories::getDataDir() + CString("/applications/tracker/tracker-debug-display.xml");
	
	if(!wp.setTrack(eegFile.toASCIIString()))
	{
		return 2;
	}
	if(!wp.setprocessor(scenarioFile.toASCIIString()))
	{
		return 3;
	}

	// Push some chunks to selection
	Selection& selection = wp.m_track.m_Selection;
	selection.addRange(Range(3,5));
	selection.addRange(Range(9,11));

	if(!wp.play())
	{
		return 4;
	}
*/	
}
#endif

