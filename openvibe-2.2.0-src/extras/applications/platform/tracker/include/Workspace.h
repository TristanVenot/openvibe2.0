
#pragma once

#include "StreamBundle.h"

#include "ProcExternalProcessing.h"

#include <openvibe/ov_all.h>

#include "WorkspaceNotes.h"
#include "Selection.h"
#include "ParallelExecutor.h"

namespace OpenViBETracker
{

/**
 * \class Workspace 
 * \brief Workspace is a set of Tracks (StreamBundles) that contain the .ov file content that the user wants to work with
 * \author J. T. Lindgren
 *
 */
class Workspace : protected Contexted {
public:

	Workspace(const OpenViBE::Kernel::IKernelContext& ctx);
	~Workspace(void);

	// Reset workspace to the state like it was after construction
	bool clear(void);

	// Manipulating tracks and streams currently in the workspace
	bool addTrack(const char *filename);	// Add an .ov file as a track

	bool moveTrack(size_t sourceIdx, size_t targetIdx);
	bool moveStream(size_t sourceTrack, size_t sourceStream, size_t targetTrack, size_t targetStream);

	bool clearTracks(void);
	bool removeTrack(size_t idx);
	bool removeStream(size_t track, size_t stream);

	size_t getNumTracks(void) const { return m_Tracks.size(); }

	size_t getNumTracksOnPlaylist(void) const { return m_Playlist.size(); }
	size_t getNumTracksDone(void) const { return m_TracksDone; };

	bool reloadTrack(size_t index);

	// Does not transfer pointer ownership
	StreamBundle* getTrack(size_t index) { 
		std::unique_lock<std::mutex>(m_om_Mutex);

		if (index >= m_Tracks.size())
		{
			return nullptr;
		}
		return m_Tracks[index];
	}

	// Transfers ownership of the pointer. Returns the index inserted to.
	bool setTrack(size_t index, StreamBundle* track)
	{
		std::unique_lock<std::mutex>(m_om_Mutex);
		if(index >= m_Tracks.size())
		{
			m_Tracks.resize(index+1, nullptr);
		}
		delete m_Tracks[index];
		m_Tracks[index] = track;

		return true;
	}

	bool play(bool playFast);
	bool stop(bool stopProcessor = true);

	// Push the selected part of the tracks to processor
	bool step(void);

	// get selection. Since it returns a reference, to set notes, simply modify the ref'd obj content.
	Selection& getSelection(void) { return m_Selection; }

	// Max duration of all tracks in the workspace, in fixed point seconds
	ovtime_t getMaxDuration(void) const;

	ovtime_t getProcessedTime(void) const;
	ovtime_t getPlaylistDuration(void) const { return m_PlaylistDuration; }

	// Set output for chunks
	// @todo refactor these to a getter/setter of a processor object?
	bool setProcessor(const char *scenarioXml);
	bool setProcessorFlags(bool noGUI, bool doSend, bool doReceive);
	bool getProcessorFlags(bool& noGUI, bool& doSend, bool& doReceive) const;
	const char *getProcessorFile(void) { return m_Processor.getFilename().c_str(); }
	bool configureProcessor(const char* filename) { return m_Processor.configure(filename); }
	bool setProcessorPorts(uint32_t sendPort, uint32_t recvPort);
	bool getProcessorPorts(uint32_t& sendPort, uint32_t& recvPort) const { return m_Processor.getProcessorPorts(sendPort, recvPort); }
	bool hasProcessor(void) { const char* fn = getProcessorFile(); return (fn && fn[0]!=0); }

	const OpenViBE::CString getProcessorArguments(void) const { return m_ProcessorArguments; }
	bool setProcessorArguments(const OpenViBE::CString& args) { m_ProcessorArguments = args; return true;  }
	
	OpenViBE::CString getWorkingPath(void) const { return m_WorkspacePath; }
	bool setWorkingPath(const OpenViBE::CString& path) { 
		if (path.length() == 0)
		{
			log() << OpenViBE::Kernel::LogLevel_Error << "Empty workspace path not allowed\n";
			return false;
		}
		m_WorkspacePath = path; return true;  
	}

	// Creates a 'unique' working path and assigns it, but does not create the folder
	bool setUniqueWorkingPath(void);

	OpenViBE::CString getFilename(void) const 
	{ 
		return m_WorkspaceFile;
	};

	bool setFilename(const OpenViBE::CString& filename) 
	{
		if (filename.length() == 0)
		{
			// unset
			m_WorkspaceFile = OpenViBE::CString("");
			return true;
		}
		else
		{
			return this->save(filename);
		}
	}

	// get workspace notes. Since it returns a reference, to set notes, simply modify the ref'd obj content.
	OpenViBE::Kernel::IComment& getNotes(void) { return m_Notes; }

	// @todo make generic property getter/setter?
	bool getCatenateMode(void) const { return m_CatenateMode; }
	bool setCatenateMode(bool newState) { m_CatenateMode = newState; return true; }

	// @fixme rework the setter to spool files to/from disk when mode changes
	bool getMemorySaveMode(void) const { return m_MemorySaveMode; };
	bool setMemorySaveMode(bool active);

	// Modify tracks in place?
	bool getInplaceMode(void) const { return m_InplaceMode; }
	bool setInplaceMode(bool newState) { m_InplaceMode = newState; return true; }

	uint64_t getRevision(void) const { return m_Revision; }
//	uint64_t getNumRevisions(void) const { return m_NumRevisions; }

	// The processors can poll this now and then to find out if they should quit ahead of the file end
	bool isQuitRequested(void) const {  
		std::unique_lock<std::mutex> m_Mutex;
		return m_PleaseQuit;
	}

	std::mutex& getMutex(void) const { return m_Mutex; }

	// Clear configuration tokens with a given prefix
	bool wipeConfigurationTokens(const std::string& prefix);

	bool setParallelExecutor(ParallelExecutor* ptr) { m_Executor = ptr; return true; };

	// Save and load workspace
	bool save(const OpenViBE::CString& filename);
	bool load(const OpenViBE::CString& filename);
	
	bool incrementRevisionAndSave(const OpenViBE::CString& filename);

	// get sorted workspace specific tokens
	std::vector< std::pair<std::string, std::string> > getConfigurationTokens(void) const;

protected:

	bool saveAll(void);

	bool getNextTrack(size_t& nextTrack) const;
	bool assemblePlaylist(void);

	bool spoolRecordingToDisk(size_t trackIndex);

	bool playReceiveOnly(void);
	bool playCatenate(void);
	bool playNormal(void);

	// Assemble user-specified and generated arguments to the processor
	std::string getProcessorArguments(size_t trackIndex);

	// Generic mutex to protect access to certain variables from multiple threads
	mutable std::mutex m_Mutex;

	std::vector< StreamBundle* > m_Tracks;

	bool m_PlayFast = false;
	bool m_CatenateMode = false;
	bool m_InplaceMode = false;
	bool m_MemorySaveMode = false;
	bool m_PleaseQuit = false;

	uint32_t m_TracksDone = 0;

	OpenViBE::CString m_WorkspaceFile;
	OpenViBE::CString m_WorkspacePath;

	// Component to send and receive data to/from, actually just a store for settings
	ProcExternalProcessing m_Processor;
	std::string m_ProcessorFilename;
	OpenViBE::CString m_ProcessorArguments;

	typedef std::pair<StreamBundle*,ovtime_t> SourceTimePair;
	std::vector< SourceTimePair > m_Playlist;
	ovtime_t m_PlaylistDuration = 0;

	// Text notes about the current workspace; defined as a class to be able to reuse Designer's CCommentEditorDialog
	WorkspaceNotes m_Notes;

	// Which tracks/streams are currently selected to be processed?
	Selection m_Selection;

	ParallelExecutor* m_Executor = nullptr;

	// uint64_t m_NumRevisions = 1;
	uint64_t m_Revision = 1;

};

};

