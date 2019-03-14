//
// OpenViBE Tracker
//
// @author J.T. Lindgren
// 

#include <iostream>
#include <ctime>     // std::ctime
#include <algorithm> // std::replace, std::max
#include <iomanip>   //std::setw, setfill
#include <fs/Files.h>

#include "Workspace.h"
#include "StreamBundleImportExport.h"

#include "StimulationStreamFilter.h"

using namespace OpenViBE;
using namespace OpenViBETracker;

Workspace::Workspace(const OpenViBE::Kernel::IKernelContext& ctx)
: Contexted(ctx), m_WorkspaceFile(""), m_WorkspacePath(""), m_Processor(ctx), m_Selection(ctx, m_Tracks)
{
	setUniqueWorkingPath();
};

Workspace::~Workspace(void)
{
	for (size_t i = 0; i < m_Tracks.size(); i++)
	{
		if (m_Tracks[i])
		{
			m_Tracks[i]->uninitialize();
			delete m_Tracks[i];
		}
	}
	m_Tracks.clear();
}

bool Workspace::setUniqueWorkingPath(void)
{
	CString workspacePrefix = m_KernelContext.getConfigurationManager().expand("${Path_UserData}/tracker-workspace-");

	auto currentTime = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(currentTime);
	std::string stringTime = std::ctime(&t);
	std::replace(stringTime.begin(), stringTime.end(), ' ', '-');
	std::replace(stringTime.begin(), stringTime.end(), ':', '-');
	std::replace(stringTime.begin(), stringTime.end(), '/', '-');
	std::replace(stringTime.begin(), stringTime.end(), '\\', '-');
	std::replace(stringTime.begin(), stringTime.end(), '\n', '-');

	// This is not a super safe way to create a directory name but it is unlikely the user 
	// would run many trackers at the same time
	std::string workspacePath = std::string(workspacePrefix.toASCIIString()) + stringTime;
	uint32_t counter = 0;
	while (FS::Files::directoryExists((workspacePath + std::to_string(counter)).c_str()))
	{
		counter++;
	}
	
	setWorkingPath(CString((workspacePath + std::to_string(counter)).c_str()));

	return true;
}

bool Workspace::step(void)
{
	if(m_Executor->isIdle())
	{
		log() << LogLevel_Info << "All jobs finished\n";
		stop();
		return false;
	}
	else
	{
		System::Time::sleep(1);
		return true;
	}
}

bool Workspace::spoolRecordingToDisk(size_t trackIndex)
{
	if (trackIndex >= m_Tracks.size())
	{
		log() << LogLevel_Error << "Index " << trackIndex << " is outside array.\n";
		return false;
	}

	// Spool the new track to disk
	std::stringstream filename;
	filename << m_WorkspacePath << "/workspace" 
		<< "-track" << std::setw(3) << std::setfill('0') << trackIndex + 1 
		<< "-rev" << std::setw(3) << std::setfill('0') << m_Revision << ".ov";

	saveStreamBundleToFile(m_KernelContext, m_Tracks[trackIndex], filename.str().c_str());
	m_Tracks[trackIndex]->setSource(filename.str());

	if (m_MemorySaveMode)
	{
		// Load back in memory save mode
		delete m_Tracks[trackIndex];
		m_Tracks[trackIndex] = readStreamBundleFromFile(m_KernelContext, filename.str().c_str(), true);
	}

	return true;
}

std::string Workspace::getProcessorArguments(size_t index)
{
	std::string trackSource("None");
	if(index>=0 && m_Tracks[index]->getSource().length()>0)
	{
		trackSource = m_Tracks[index]->getSource();
	}

	// Append generic configuration tokens. n.b. make sure no define argument is an empty string.
	std::stringstream ss; 
	ss << " ";
	ss << "--define Tracker_Workspace_File " << "\"" << (m_WorkspaceFile.length()>0 ? m_WorkspaceFile : "None") << "\" ";
	ss << "--define Tracker_Workspace_Path " << "\"" << (m_WorkspacePath.length()>0 ? m_WorkspacePath : "None") << "\" ";
	ss << "--define Tracker_CurrentTrack_Number " << index+1 << " ";
	ss << "--define Tracker_CurrentTrack_Source " << "\"" + trackSource << "\" ";
	ss << "--define Tracker_CatenatePlayback " << (m_CatenateMode ? "True" : "False") << " ";

	// Include those potentially set by the user in the GUI
	std::string args = std::string(m_ProcessorArguments.toASCIIString()) + ss.str();

	return args;
}


bool Workspace::getNextTrack(size_t& nextTrack) const
{
	nextTrack = nextTrack + 1;
	while (nextTrack<getNumTracks() && !m_Selection.isTrackSelected(nextTrack))
	{
		nextTrack++;
	}

	if (nextTrack >= getNumTracks())
	{
		nextTrack = (size_t)(-1);
		return false;
	}

	return true;
}

bool Workspace::assemblePlaylist(void)
{
	m_Playlist.clear();

	m_PlaylistDuration = 0;

	size_t nextTrack = (size_t)(-1);
	while(getNextTrack(nextTrack))
	{
		StreamBundle* source = m_Tracks[nextTrack];
		m_PlaylistDuration += source->getMaxDuration();

		SourceTimePair tmp(source, 0);

		m_Playlist.push_back(tmp);

	}

	return true;
}


// Construct list of streamsubsets to process
// make processing each streamsubset a job
// connect input and output to the job
// monitor until jobs are complete
//
bool Workspace::play(bool playFast)
{
	m_PlayFast = playFast;
	m_PleaseQuit = false;
	m_TracksDone = 0;
	m_PlaylistDuration = 0;

	if(!m_Executor)
	{
		log() << LogLevel_Error << "Need a parallel executor set\n";
		return false;
	}

	if (!m_Selection.isSelectionConsistent())
	{
		log() << LogLevel_Error << "For processing, the selected streams for each track must have equal types, in equal amounts, and in the same stream type order.\n";
		return false;
	}

	if (!m_Processor.canPush() && !m_Processor.canPull())
	{
		this->getLogManager() << LogLevel_Error << "Please configure the processor to send, receive, or both.\n";
		return false;
	}

	if(!m_Processor.canPush())
	{
		return playReceiveOnly();
	}

	// Generate streamsubsets to process
	assemblePlaylist();

	if(m_CatenateMode)
	{
		return playCatenate();
	}
	else
	{
		return playNormal();
	}
}

bool Workspace::playCatenate(void)
{
	uint32_t portToUse,dummy;
	m_Processor.getProcessorPorts(portToUse, dummy);

	std::string filename = m_ProcessorFilename;

	bool playFast = m_PlayFast;
	auto& refProc = m_Processor;

	std::string args = getProcessorArguments(0);

	auto job = [playFast,portToUse,args,filename, &refProc, this](uint32_t threadNumber) {

		uint32_t playlistIndex = 0;
			
		StreamBundle* target = new StreamBundle(this->getKernelContext());

		ProcExternalProcessing proc(getKernelContext(),refProc);
		proc.setArguments(args.c_str());
		proc.setNewTarget(target);

		StreamBundle* loaded = nullptr;
		StreamBundle* subset = nullptr;

		auto quitCallback = [this,playlistIndex](ovtime_t spent) { 
			{
				std::unique_lock<std::mutex>(m_Mutex);
				this->m_Playlist[playlistIndex].second = spent;
			}
			return isQuitRequested(); 
		};

		auto nextTrackFun = [this,&playlistIndex,&proc,&subset, &loaded](void) 
		{
			delete subset;

			if(playlistIndex>=m_Playlist.size())
			{
				return false;
			}

			this->log() << LogLevel_Info << "Switching to process track " << playlistIndex+1 << " out of " << m_Playlist.size() << "\n";

			subset = new StreamBundle(this->m_KernelContext);

			auto ptr = m_Playlist[playlistIndex];

			StreamBundle* source = ptr.first;

			if (this->m_MemorySaveMode)
			{
				this->log() << LogLevel_Info << "Memory save mode: Loading " << source->getSource().c_str() << " from disk\n";

				loaded = readStreamBundleFromFile(this->m_KernelContext, source->getSource().c_str(), false);

				// copy selection from the one in memory
				for (size_t i = 0; i < source->getNumStreams(); i++)
				{
					loaded->getStream(i)->setSelected(source->getStream(i)->getSelected());
				}
				subset->copyFrom(*loaded);

				delete loaded;
			}
			else
			{
				// Select subset of streams to play
				subset->copyFrom(*source);
			}

			const bool isFirst = (playlistIndex == 0);
			const bool isLast = (playlistIndex >= m_Playlist.size()-1);

			proc.setNewSource(subset, isFirst, isLast);

			{
				std::unique_lock<std::mutex>(this->getMutex());
				this->m_TracksDone++;
			}

			playlistIndex++;

			return true;
		};

		proc.play(playFast, quitCallback, nextTrackFun);
			
		this->log() << LogLevel_Info << "Track complete.\n";


		if(target->getNumStreams()>0)
		{
			// Single threaded here, so can get m_Tracks.size() outside mutex
			const size_t newIndex = m_Tracks.size();
			this->setTrack(newIndex, target);
			// Spool result to disk, note that it might be incomplete as the user requested stop
			if (m_MemorySaveMode)
			{
				log() << LogLevel_Info << "Writing new track to disk.\n";
				spoolRecordingToDisk(newIndex);
			}
		}
		else
		{
			delete target;
		}
	};

	m_Executor->pushJob(job);

	return true;
}

bool Workspace::playNormal(void)
{
	uint32_t firstPort,dummy;
	m_Processor.getProcessorPorts(firstPort, dummy);

	const size_t lastFreeIndex = m_Tracks.size();
	bool playFast = m_PlayFast;
	const size_t totalTracks = m_Playlist.size();

	for(size_t playlistIndex=0; playlistIndex<totalTracks; playlistIndex++)
	{
		auto ptr = m_Playlist[playlistIndex];
		
		std::string args = getProcessorArguments(playlistIndex);
		std::string filename = m_ProcessorFilename;

		auto& refProc = m_Processor;

		auto job = [ptr,&refProc,args, playFast,filename, lastFreeIndex, playlistIndex, firstPort, totalTracks, this](uint32_t threadNumber) {
			StreamBundle* original = ptr.first;
			StreamBundle* target = new StreamBundle(this->m_KernelContext);
			StreamBundle* loaded = nullptr;
			StreamBundle* subset = new StreamBundle(this->m_KernelContext);

			this->log() << LogLevel_Trace << "Switching to process track " << playlistIndex+1 << " out of " << totalTracks << "\n";

			if (this->m_MemorySaveMode)
			{
				this->log() << LogLevel_Info << "Memory save mode: Loading " << original->getSource().c_str() << " from disk\n";

				loaded = readStreamBundleFromFile(this->m_KernelContext, original->getSource().c_str(), false);

				// copy selection from the one in memory
				for (size_t i = 0; i < original->getNumStreams(); i++)
				{
					loaded->getStream(i)->setSelected(original->getStream(i)->getSelected());
				}
				subset->copyFrom(*loaded);

				delete loaded;
			}
			else
			{
				// Select subset of streams to play
				subset->copyFrom(*original);
			}

			auto quitCallback = [this,playlistIndex](ovtime_t spent) { 
				{
					std::unique_lock<std::mutex>(m_Mutex);
					this->m_Playlist[playlistIndex].second = spent;
				}
				this->m_Playlist[playlistIndex].second = spent;
				return isQuitRequested(); 
			};

			// Process
			ProcExternalProcessing proc(this->getKernelContext(), refProc);
			proc.setNewSource(subset, true, true);
			proc.setNewTarget(target);
			proc.setArguments(args.c_str());
			proc.setProcessorPorts(firstPort+2*threadNumber, firstPort+2*threadNumber+1);
			proc.play(playFast, quitCallback);

			this->log() << LogLevel_Trace << "Track " << playlistIndex+1 << " complete.\n";
			{
				std::unique_lock<std::mutex>(this->getMutex());
				this->m_TracksDone++;
			}

			// Clean-up
			delete subset;
			
			if(target->getNumStreams()>0)
			{
				// if not inplacemode, attempt to insert in the same order as the sources were in the list
				const size_t newIndex = (this->getInplaceMode() ? playlistIndex : lastFreeIndex+playlistIndex);

				this->setTrack(newIndex, target);

				// Spool result to disk, note that it might be incomplete as the user requested stop
				if (m_MemorySaveMode)
				{
					log() << LogLevel_Info << "Writing new track to disk.\n";
					spoolRecordingToDisk(newIndex);
				}
			}
			else
			{
				// this->log() << LogLevel_Warnin
				delete target;
			}

		};

		m_Executor->pushJob(job);

	}

	return true;
}

bool Workspace::playReceiveOnly(void)
{
	const size_t lastFreeIndex = m_Tracks.size();
	bool playFast = m_PlayFast;
		
	std::string args = getProcessorArguments(-1);
	std::string filename = m_ProcessorFilename;

	auto& refProc = m_Processor;

	m_Playlist.clear();
	m_Playlist.push_back( SourceTimePair(nullptr, 0));

	auto job = [&refProc,args, playFast,filename, lastFreeIndex, this](uint32_t threadNumber) 
	{
		StreamBundle* target = new StreamBundle(this->m_KernelContext);

		this->log() << LogLevel_Info << "Recording a track (noSend configured)\n";

		auto quitCallback = [this](ovtime_t spent) { 
			// single thread, no cc
			this->m_PlaylistDuration = spent;
			this->m_Playlist[0].second = spent;
			return isQuitRequested(); 
		};

		// Process
		ProcExternalProcessing proc(this->getKernelContext(), refProc);
		proc.setNewSource(nullptr, true, true);
		proc.setNewTarget(target);
		proc.setArguments(args.c_str());
		proc.play(playFast, quitCallback);

		this->log() << LogLevel_Info << "Recording complete.\n";
			
		if(target->getNumStreams()>0)
		{
			// if not inplacemode, attempt to insert in the same order as the sources were in the list
			const size_t newIndex = lastFreeIndex;

			this->setTrack(newIndex, target);

			// Spool result to disk, note that it might be incomplete as the user requested stop
			if (m_MemorySaveMode)
			{
				log() << LogLevel_Info << "Writing new track to disk.\n";
				spoolRecordingToDisk(newIndex);
			}
		}
		else
		{
			delete target;
		}
	};

	m_Executor->pushJob(job);

	return true;
}

bool Workspace::stop(bool stopProcessor) 
{
	if(stopProcessor)
	{
		std::unique_lock<std::mutex> m_Mutex;
		m_Executor->clearPendingJobs();
		m_PleaseQuit = true;
	}
	return true;

}

bool Workspace::clearTracks(void)
{
	for (size_t i = 0; i<m_Tracks.size(); i++)
	{
		delete m_Tracks[i];
	}
	m_Tracks.clear();

	return true;
}

bool Workspace::removeTrack(size_t idx)
{
	if (idx >= m_Tracks.size())
	{
		return false;
	}
	delete m_Tracks[idx];
	m_Tracks.erase(m_Tracks.begin() + idx);

	return true;
}

bool Workspace::removeStream(size_t track, size_t stream)
{
	if (track >= getNumTracks() || stream >= getTrack(track)->getNumStreams())
	{
		return false;
	}
	if (getTrack(track)->deleteStream(stream)) 
	{
		return true;
	}
	return false;
}

bool Workspace::addTrack(const char *filename)
{
	if (!filename || !filename[0])
	{
		return false;
	}

	StreamBundle* newTrack = readStreamBundleFromFile(m_KernelContext, filename, m_MemorySaveMode);
	if(!newTrack)
	{
		return false;
	}

	log() << LogLevel_Debug << "The loaded track has " << newTrack->getNumStreams() << " streams\n";
	for (size_t i = 0; i < newTrack->getNumStreams(); i++)
	{
		if (newTrack->getStream(i))
		{
			log() << LogLevel_Debug << "  Stream " << i << " has type "
				<< newTrack->getStream(i)->getTypeIdentifier().toString() << " == "
				<< m_KernelContext.getTypeManager().getTypeName(newTrack->getStream(i)->getTypeIdentifier())
				<< "\n";
		}
		else
		{
			// @fixme this has the issue that even though the stream may have a definition, its lost currently if there's not even a header chunk in the stream
			log() << LogLevel_Info << "  Stream " << i << " has a type the Tracker couldn't decode (or the stream was empty)\n";
		}
	}

	m_Tracks.push_back(newTrack);

	for (auto str : newTrack->getAllStreams())
	{
		str->setSelected(true);
	}

	return true;

};

bool Workspace::moveStream(size_t sourceTrack, size_t sourceStream, size_t targetTrack, size_t targetStream)

{
	if (sourceTrack >= getNumTracks() || targetTrack >= getNumTracks())
	{
		return false;
	}
	if (sourceTrack == targetTrack && sourceStream == targetStream)
	{
		return true;
	}
	if (sourceStream >= m_Tracks[sourceTrack]->getNumStreams() ||
		targetStream >= m_Tracks[targetTrack]->getNumStreams())
	{
		return false;
	}

	if (sourceTrack != targetTrack)
	{
		auto oldPtr = m_Tracks[sourceTrack]->getStream(sourceStream);
	
		// Move pointer, do not free memory
		auto& allStreams = m_Tracks[sourceTrack]->getAllStreams();
		allStreams.erase(allStreams.begin() + sourceStream);
		m_Tracks[sourceTrack]->setDirtyBit(true);

		m_Tracks[targetTrack]->setStream(m_Tracks[targetTrack]->getNumStreams(), oldPtr);	 // append to the end
		return m_Tracks[targetTrack]->moveStream(m_Tracks[targetTrack]->getNumStreams()-1, targetStream);		
	}
	else
	{
		return m_Tracks[sourceTrack]->moveStream(sourceStream, targetStream);
	}

}

bool Workspace::moveTrack(size_t sourceIdx, size_t targetIdx)
{
	if (sourceIdx >= getNumTracks() || targetIdx >= getNumTracks())
	{
		return false;
	}
	if (sourceIdx == targetIdx)
	{
		return true;
	}

	auto oldPtr = m_Tracks[sourceIdx];
	m_Tracks.erase(m_Tracks.begin() + sourceIdx);
	m_Tracks.insert(m_Tracks.begin() + targetIdx, oldPtr);

	return true;
}


bool Workspace::reloadTrack(size_t index) {
	if (index >= m_Tracks.size())
	{
		return false;
	}
	StreamBundle* newTrack = readStreamBundleFromFile(m_KernelContext, m_Tracks[index]->getSource().c_str(), m_MemorySaveMode);

	delete m_Tracks[index];
	
	m_Tracks[index] = newTrack;

	return true;
}

ovtime_t Workspace::getMaxDuration(void) const {
	ovtime_t maxDuration = 0;
	for (size_t i = 0; i < m_Tracks.size(); i++)
	{
		if (m_Tracks[i])
		{
			maxDuration = std::max<ovtime_t>(maxDuration, m_Tracks[i]->getMaxDuration());
		}
	}
	return maxDuration;
}


bool Workspace::setProcessor(const char *scenarioXml) 
{ 
	m_KernelContext.getConfigurationManager().addOrReplaceConfigurationToken("Tracker_Workspace_Processor", scenarioXml);

	m_ProcessorFilename = scenarioXml;

	return m_Processor.initialize(scenarioXml);
};

bool Workspace::setProcessorFlags(bool noGUI, bool doSend, bool doReceive)
{
	return m_Processor.setProcessorFlags(noGUI, doSend, doReceive);
};

bool Workspace::getProcessorFlags(bool& noGUI, bool& doSend, bool& doReceive) const
{
	return m_Processor.getProcessorFlags(noGUI, doSend, doReceive);
};

bool Workspace::setProcessorPorts(uint32_t sendPort, uint32_t recvPort)
{
	return m_Processor.setProcessorPorts(sendPort, recvPort);
};

bool Workspace::saveAll(void)
{

	if (m_WorkspacePath.length() == 0)
	{
		log() << LogLevel_Error << "Error: Workspace path not set...\n";
		return false;
	}
	if (!FS::Files::directoryExists(m_WorkspacePath.toASCIIString()))
	{
		if (!FS::Files::createPath(m_WorkspacePath.toASCIIString()))
		{
			log() << LogLevel_Error << "Error: Unable to create directory " << m_WorkspacePath << "\n";
			return false;
		}
	}

	bool retVal = true;

	if (getNumTracks() > 0)
	{
		log() << OpenViBE::Kernel::LogLevel_Info << "Saving modified tracks to " << m_WorkspacePath << " ...\n";
	}

	uint32_t tracksSaved = 0;
	for (size_t i = 0; i < getNumTracks(); i++)
	{
		if (!getTrack(i)->getDirtyBit())
		{
			log() << OpenViBE::Kernel::LogLevel_Trace << "Skipping track " << i + 1 << " / " << getNumTracks() << ", no modifications ...\n";
			continue;
		}
		log() << OpenViBE::Kernel::LogLevel_Trace << "Saving track " << i + 1 << " / " << getNumTracks() << " ...\n";

		std::stringstream filename;
		filename << m_WorkspacePath << "/workspace" 
			<< "-track" << std::setw(3) << std::setfill('0') << i + 1 
			<< "-rev" << std::setw(3) << std::setfill('0') << m_Revision << ".ov";

		// If we're in the memory save mode, we need to copy contents of the track to the new location
		StreamBundle* track = (m_MemorySaveMode ?
				readStreamBundleFromFile(m_KernelContext, getTrack(i)->getSource().c_str(), false) : getTrack(i));

		retVal &= saveStreamBundleToFile(m_KernelContext, track, filename.str().c_str());

		tracksSaved++;

		if (m_MemorySaveMode)
		{
			getTrack(i)->setSource(track->getSource());
			getTrack(i)->setDirtyBit(false);
			delete track;
		}

	}

	log() << LogLevel_Info << "Done, " << tracksSaved << " had changed and needed to be saved.\n";

	return retVal;
}

bool Workspace::setMemorySaveMode(bool active) 
{ 
	if (active != m_MemorySaveMode)
	{
		m_MemorySaveMode = active;
		if (active == false)
		{
			// Going from memory save mode to the normal mode
			log() << LogLevel_Info << "Switching to full mode. Loading tracks from disk.\n";
		}
		else
		{
			// Going from normal mode to memory save mode
		
			// We need to spool all the modifications to the disk so the other mode can get them
			log() << LogLevel_Info << "Switching to memory save mode. Saving all modified tracks to disk.\n";

			saveAll();
		}
		
		// Read tracks back in the new mode
		for (size_t i = 0; i < m_Tracks.size(); i++)
		{
			const std::string filename = m_Tracks[i]->getSource();
			delete m_Tracks[i];
			m_Tracks[i] = readStreamBundleFromFile(m_KernelContext, filename.c_str(), m_MemorySaveMode);
		}
	}
	
	return true; 
}

bool Workspace::clear(void)
{
	clearTracks();
	setFilename("");
	setUniqueWorkingPath();

	return true;
}

std::vector< std::pair<std::string, std::string> > Workspace::getConfigurationTokens(void) const
{
	std::vector < std::pair < std::string, std::string> > tokens; // For sorting

	CIdentifier iter = OV_UndefinedIdentifier;
	while ((iter = m_KernelContext.getConfigurationManager().getNextConfigurationTokenIdentifier(iter)) != OV_UndefinedIdentifier)
	{
		std::string prefix("Tracker_Workspace_");
		std::string token(m_KernelContext.getConfigurationManager().getConfigurationTokenName(iter).toASCIIString());

		// See if token has the prefix?
		auto res = std::mismatch(prefix.begin(), prefix.end(), token.begin());
		if (res.first == prefix.end())
		{
			std::string value(m_KernelContext.getConfigurationManager().getConfigurationTokenValue(iter));
			tokens.push_back(std::pair<std::string, std::string>(token, value));

			// ::fprintf(l_pFile, "%s = %s\n", token.c_str(), value.toASCIIString());
		}
	}

	std::sort(tokens.begin(), tokens.end());

	return tokens;
}

// @fixme for multiple workspaces this solution needs to be reworked
bool Workspace::save(const OpenViBE::CString& filename)
{
	if (m_Tracks.size() > 0 && !saveAll())
	{
		return false;
	}

	// Save selection to configuration manager
	m_Selection.save("Tracker_Workspace_");

	// Save processor configuration to manager
	m_Processor.save();

	// Set workspaces own configuration tokens to manager
	// @todo Why not already do this in the setters?
	std::stringstream trackCount; trackCount << m_Tracks.size();
	std::stringstream revision; revision << m_Revision;

	auto& mgr = m_KernelContext.getConfigurationManager();
	mgr.addOrReplaceConfigurationToken("Tracker_Workspace_Path", m_WorkspacePath.toASCIIString());
	mgr.addOrReplaceConfigurationToken("Tracker_Workspace_CatenatePlayback", (m_CatenateMode ? "true" : "false"));
	mgr.addOrReplaceConfigurationToken("Tracker_Workspace_MemorySaveMode", (m_MemorySaveMode ? "true" : "false"));
	mgr.addOrReplaceConfigurationToken("Tracker_Workspace_InplaceMode", (m_InplaceMode ? "true" : "false"));
	mgr.addOrReplaceConfigurationToken("Tracker_Workspace_Track_Count", trackCount.str().c_str());
	mgr.addOrReplaceConfigurationToken("Tracker_Workspace_Processor_Arguments", m_ProcessorArguments.toASCIIString());
	mgr.addOrReplaceConfigurationToken("Tracker_Workspace_Revision", revision.str().c_str());

	FILE* l_pFile=::fopen(filename.toASCIIString(), "wt");
	if(l_pFile)
	{
		::fprintf(l_pFile, "# Workspace configuration file generated by OpenViBE Tracker\n");
		::fprintf(l_pFile, "#\n");
		::fprintf(l_pFile, "\n");

		for (size_t i = 0; i < m_Tracks.size(); i++)
		{
			std::stringstream token; token << "Tracker_Workspace_Track_" << std::setw(3) << std::setfill('0') << (i + 1);
			mgr.addOrReplaceConfigurationToken(token.str().c_str(), m_Tracks[i]->getSource().c_str());
		}

		// Spool all generic Tracker tokens from the manager to the file
		auto tokens = getConfigurationTokens();
		for (auto& token : tokens)
		{
			fprintf(l_pFile, "%s = %s\n", token.first.c_str(), token.second.c_str() );
		}

		::fprintf(l_pFile, "\n");

		fclose(l_pFile);

	} 
	else
	{
		log() << LogLevel_Error << "Error: Couldn't open " << filename.toASCIIString() << " for writing\n";
		return false;
	}

	m_WorkspaceFile = filename;

	// Save workspace notes
	if (m_Notes.getText().length() > 0 && m_WorkspacePath.length()>0)
	{
		OpenViBE::CString notesFile = m_WorkspacePath + OpenViBE::CString("/workspace-notes.txt");
		m_Notes.save(notesFile);
	}

	return true;
}
	
bool Workspace::load(const OpenViBE::CString& filename)
{
	// @todo might wipe only specific tokens in the future, esp. if multiple workspaces become supported
	wipeConfigurationTokens("Tracker_Workspace_");

	auto& mgr = m_KernelContext.getConfigurationManager();

	if (!mgr.addConfigurationFromFile(filename))
	{
		return false;
	}

	clearTracks();

	m_WorkspaceFile = filename;

	CString savedWorkspacePath = mgr.expand("${Tracker_Workspace_Path}");
	if (savedWorkspacePath.length() > 0)
	{
		m_WorkspacePath = savedWorkspacePath;
	}

	setCatenateMode( mgr.expandAsBoolean("${Tracker_Workspace_CatenatePlayback}", m_CatenateMode));

	// Here we don't use the setter as we're loading from scratch
	m_MemorySaveMode = mgr.expandAsBoolean("${Tracker_Workspace_MemorySaveMode}", m_MemorySaveMode);
	m_InplaceMode = mgr.expandAsBoolean("${Tracker_Workspace_InplaceMode}", m_InplaceMode);

	m_Revision = mgr.expandAsUInteger("${Tracker_Workspace_Revision}", m_Revision);
	// m_NumRevisions = mgr.expandAsUInteger("${Tracker_Workspace_NumRevisions}", m_NumRevisions);

	m_ProcessorArguments = mgr.expand("${Tracker_Workspace_Processor_Arguments}");

	const uint32_t trackCount = (uint32_t)mgr.expandAsUInteger("${Tracker_Workspace_Track_Count}", 0);
	for (uint32_t i = 0; i < trackCount; i++)
	{
		std::stringstream token; token << "Tracker_Workspace_Track_" << std::setw(3) << std::setfill('0') << (i+1);
		if (mgr.lookUpConfigurationTokenIdentifier(token.str().c_str()) != OV_UndefinedIdentifier)
		{
			CString tokenValue = mgr.lookUpConfigurationTokenValue(token.str().c_str());
			CString trackFile = mgr.expand(tokenValue);
			log() << LogLevel_Debug << "Loading track " << i+1 << " : " << trackFile << "\n";
			addTrack(trackFile.toASCIIString());
		}
	}
	log() << LogLevel_Info << "Loaded " << m_Tracks.size() << " tracks of the workspace\n";
	
	// Load processor config
	m_Processor.load();

	// Load workspace notes
	OpenViBE::CString notesFile = m_WorkspacePath + OpenViBE::CString("/workspace-notes.txt");
	m_Notes.load(notesFile);

	// Load selection
	m_Selection.load("Tracker_Workspace_");

	return true;
}

bool Workspace::incrementRevisionAndSave(const OpenViBE::CString& filename)
{
	if(m_WorkspacePath.length()==0)
	{
		log() << LogLevel_Error << "Please set workspace path before saving revision\n";
		return false;
	}

	for(auto& ptr : m_Tracks)
	{
		ptr->setDirtyBit(true);
	}

	m_Revision++;
//	m_NumRevisions++;

	log() << LogLevel_Info << "Revision updated to " << m_Revision << "\n";

	bool retVal = save( m_WorkspaceFile );

	std::stringstream ss;
	ss << m_WorkspacePath + "/revision-"  << std::setw(3) << std::setfill('0') <<  m_Revision << "-backup.ovw";
	std::string fn = ss.str();
	FS::Files::remove(fn.c_str());
	if(FS::Files::copyFile(m_WorkspaceFile.toASCIIString(), fn.c_str()))
	{
		log() << LogLevel_Info << "Revision backup saved to " << ss.str().c_str() << "\n";
	}
	else
	{
		log() << LogLevel_Info << "Error saving backup to " << ss.str().c_str() << "\n";	
	}

	return retVal;
}


bool Workspace::wipeConfigurationTokens(const std::string& prefix)
{
	CIdentifier iter = OV_UndefinedIdentifier, prev = OV_UndefinedIdentifier;
	while ((iter = m_KernelContext.getConfigurationManager().getNextConfigurationTokenIdentifier(iter)) != OV_UndefinedIdentifier)
	{
		std::string token(m_KernelContext.getConfigurationManager().getConfigurationTokenName(iter).toASCIIString());

		// See if token has the prefix?
		auto res = std::mismatch(prefix.begin(), prefix.end(), token.begin());
		if (res.first == prefix.end()) {
			m_KernelContext.getConfigurationManager().releaseConfigurationToken(iter);
			iter = prev;
		}
		else
		{
			prev = iter;
		}
	}
	return true;
}


ovtime_t Workspace::getProcessedTime(void) const
{
	std::unique_lock<std::mutex> m_Mutex;

	ovtime_t processed = 0;
	for(auto& ptr : m_Playlist)
	{
		processed += ptr.second;
	}
	return processed;
}

