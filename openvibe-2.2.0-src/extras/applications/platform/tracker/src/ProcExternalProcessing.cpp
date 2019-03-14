
#include "ProcExternalProcessing.h"

#include <iostream>
#include <thread>
#include <deque>

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <system/ovCTime.h>

#include <algorithm> // std::max

#include "CodecFactory.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBETracker;

// thread::
// wait for connection;
// while(connected ||!quit)
// {
//    waitForChunk();
//    pushChunk();
// }
// if(quit) {
//	  pushEndStim(); // XML have player controller that will quit
// }
// exit;

// #include "StreamChunk.h"
// #include "StreamSignalChunk.h"
// #include "StreamStimulationChunk.h"

#include "Stream.h"
#include "TypeSignal.h"
#include "TypeStimulation.h"

using namespace Communication;

void playerLaunch(const char *xmlFile, const char* args, bool playFast, bool noGUI, uint32_t identifier); // In processor.cpp



ProcExternalProcessing::ProcExternalProcessing(const OpenViBE::Kernel::IKernelContext& ctx, const ProcExternalProcessing& other) : Processor(ctx)
{
	uint32_t sendPort, recvPort;
	other.getProcessorPorts(sendPort, recvPort);
	setProcessorPorts(sendPort, recvPort);

	bool noGui, doSend, doReceive;
	other.getProcessorFlags(noGui, doSend, doReceive);
	setProcessorFlags(noGui, doSend, doReceive);

	setArguments(other.getArguments());

	initialize(other.getFilename());
}

bool ProcExternalProcessing::initialize(const std::string& xmlFile) 
{ 
	m_pushStartTime = BufferedClient::CLIENT_NOT_STARTED;
	m_pullStartTime = BufferedClient::CLIENT_NOT_STARTED;

	if(xmlFile.length()==0)
	{
//		log() << OpenViBE::Kernel::LogLevel_Trace << "No processor configured\n";
		m_xmlFilename.clear();
		return true;
	}

//	log() << OpenViBE::Kernel::LogLevel_Trace << "Processor: Initializing with " << xmlFile << "\n";
	
	m_xmlFilename = xmlFile;

	return true; 
}

bool ProcExternalProcessing::uninitialize(void) 
{ 
//	log() << OpenViBE::Kernel::LogLevel_Trace << "Processor: Uninitializing\n";

	stop();

	for (auto decoder : m_vDecoders)
	{
		delete decoder;
	}
	m_vDecoders.clear();
	for (auto encoder : m_vEncoders)
	{
		delete encoder;
	}
	m_vEncoders.clear();

	return true;
}

ovtime_t ProcExternalProcessing::getCurrentTime(void) const
{
	const ovtime_t startTime = (m_DoSend ? m_pushStartTime : m_pullStartTime);

	if (startTime == BufferedClient::CLIENT_NOT_STARTED) {
		return 0;
	}

	const ovtime_t refTime = (m_PushClient ? m_PushClient->getTime() : m_PullClient->getTime());
 	const ovtime_t currentTime = refTime - startTime;
// 	log() << LogLevel_Info << "Its " << ITimeArithmetics::timeToSeconds(currentTime) << "\n";

	return currentTime;
}

// If source track changes, we need a new codec since they are bound to the streams
// @todo make streamless?
bool ProcExternalProcessing::setNewSource(StreamBundle* source, bool sendHeader, bool sendEnd)
{
	m_SendHeader = sendHeader;
	m_SendEnd = sendEnd;
	m_Source = source;
	
	for (auto encoder : m_vEncoders)
	{
		delete encoder;
	}	
	m_vEncoders.clear();
	return true;
}

bool ProcExternalProcessing::setNewTarget(StreamBundle* target) 
{
	m_Target = target; 

	for (auto decoder : m_vDecoders)
	{
		delete decoder;
	}
	m_vDecoders.clear();

	return true;
}


// Sends all chunks up to the current time point t
bool ProcExternalProcessing::push(void)
{
	if (m_xmlFilename.length() == 0)
	{
		return false;
	}

	if (!m_DoSend)
	{
		// This processor is configured to receive, so we do nothing on send
		return true;
	}

	if (!m_PushClient || m_PushClient->hasQuit())
	{
		this->stop();
		return false;
	}

	if(!m_Source)
	{
		return false;
	}

	if (m_pushStartTime == BufferedClient::CLIENT_NOT_STARTED)
	{
		m_pushStartTime = m_PushClient->getStartTime();
		if (m_pushStartTime == BufferedClient::CLIENT_NOT_STARTED)
		{
			// Not yet synced
			return true;
		}
	}
	
	bool foundSomething = false;
	m_sentSomething = false;

	uint64_t chunksSent = 0;

	while (true)
	{
		size_t streamIndex;
		StreamPtr stream = m_Source->getNextStream(streamIndex);

		if(!stream)
		{
			break;
		}

		if (m_vEncoders.size() <= streamIndex)
		{
			m_vEncoders.resize(streamIndex+1, nullptr);
			m_vEncoders[streamIndex] = CodecFactory::getEncoder(m_KernelContext, *stream);
		}

		ovtime_t chunkStartTime, chunkEndTime;
		if (!stream->peek(chunkStartTime, chunkEndTime))
		{
			// if peek fails maybe the stream has ended?
			break;
		}
			
		// There is something to send, now or later
		foundSomething = true;

		const ovtime_t elapsedTime = getCurrentTime();
	
		// Send if its time
		if (elapsedTime >= chunkStartTime + m_PreviousEnd)
		{
			EncodedChunk chunk; ChunkType outputType;
			m_vEncoders[streamIndex]->setEncodeOffset(m_PreviousEnd);
			if (m_vEncoders[streamIndex]->encode(chunk, outputType))
			{
				const bool dontPush = (!m_SendHeader && outputType == ChunkType::CHUNK_TYPE_HEADER) || 
					(!m_SendEnd && outputType == ChunkType::CHUNK_TYPE_END);
	
				if(!dontPush)
				{
					chunk.streamIndex = streamIndex;
					m_PushClient->pushBuffer(chunk);
					m_PushLastTime = chunk.m_endTime;

					/*
					log() << LogLevel_Info << "Enc str " << streamIndex << " at " << ITimeArithmetics::timeToSeconds(elapsedTime)
						<< " chk [" << ITimeArithmetics::timeToSeconds(chunk.m_startTime)
						<< "," << ITimeArithmetics::timeToSeconds(chunk.m_endTime)
						<< "]\n";
						*/
		 	 		m_sentSomething = true;
					chunksSent++;
				}

				stream->step();

			}
			else
			{
			// 	log() << OpenViBE::Kernel::LogLevel_Error << "Error: Failed to encode chunk\n";
				return false;
			}
		}
		else
		{
			// We are early
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			break;
		}

		//	} while (sentSomething || m_PlayFast); // we loop until we have sent everything up to this moment. 
	};
	
	if (m_sentSomething)
	{
//		log() << LogLevel_Info << "flush\n";
		m_PushClient->requestFlush();
	}


	// @fixme note that the while loop above may take long to return the control flow for some
	// bad streams


	if (!foundSomething)
	{
		// Maybe all streams ended?
		// log() << LogLevel_Info << "Nothing to send - all streams ended?\n";

		// Update time offset for 'continuous sending' mode
		// n.b. we can always do this, since for noncontinuous mode, new play() will be called, resetting these
		if (m_Source->isFinished() && !m_RequestNewOffset)
		{
			const ovtime_t maxEndTime = m_Source->getMaxDuration();
			m_PreviousEnd += maxEndTime; // Incremental for more than 2 tracks
			m_RequestNewOffset = true;

//			log() << LogLevel_Info << "New offset at " << ITimeArithmetics::timeToSeconds(m_PreviousEnd) << "\n";
		}

		// We do this to keep the External Processing box running on the designer side
		m_PushClient->requestFlush();

		std::this_thread::sleep_for(std::chrono::milliseconds(1));	

		return false;
	}

	if(!m_PlayFast)
	{
		// std::this_thread::sleep_for(std::chrono::milliseconds(1));	
	//	std::this_thread::yield();
	}

	return true;
}

// Sends all chunks up to the current time point t
bool ProcExternalProcessing::pop(void)
{
	if (m_xmlFilename.length() == 0)
	{
		return false;
	}

	if (!m_DoReceive)
	{
		// This processor is configured to send, so we do nothing on receive
		return true;
	}

	if (!m_PullClient || !m_Target)
	{
		return false;
	}

	if (m_PullClient->hasQuit())
	{
		this->stop();
		return false;
	}

	if (m_pullStartTime == BufferedClient::CLIENT_NOT_STARTED)
	{
		m_pullStartTime = m_PullClient->getStartTime();
		if (m_pullStartTime == BufferedClient::CLIENT_NOT_STARTED)
		{
			// Not yet synced
			return true;
		}
	}

	EncodedChunk chunk;
	bool gotSomething = false;
	while (m_PullClient->pullBuffer(chunk))
	{
		//		log() << LogLevel_Info << "Got chunk " << chunk.m_startTime << "," << chunk.m_endTime << "\n";
		StreamPtr targetStream = m_Target->getStream(chunk.streamIndex);
		if (!targetStream)
		{
			if (!m_Target->createStream(chunk.streamIndex, chunk.streamType))
			{
				continue;
			}
			targetStream = m_Target->getStream(chunk.streamIndex);
			if (m_vDecoders.size() <= chunk.streamIndex)
			{
				m_vDecoders.resize(chunk.streamIndex + 1, nullptr);
			}
			m_vDecoders[chunk.streamIndex] = CodecFactory::getDecoder(m_KernelContext, *targetStream);
		}

		// @note Decoder doesn't need to add offsets as these times are coming from the
		// external processor, so they are already offset by the past length.
		if (chunk.bufferData.size() > 0) {
			m_vDecoders[chunk.streamIndex]->decode(chunk);

			/*
			log() << LogLevel_Info << "Dec str " << chunk.streamIndex 
				<< " chk [" << ITimeArithmetics::timeToSeconds(chunk.m_startTime)
				<< "," << ITimeArithmetics::timeToSeconds(chunk.m_endTime)
				<< "]\n";
				*/
		}

		m_PullLastTime = chunk.m_endTime;

		gotSomething = true;
	}

	if(!gotSomething)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	if (m_PullClient->hasQuit()) 
	{
		return false;
	}

	return true;
}

bool ProcExternalProcessing::play(bool playFast, std::function<bool(ovtime_t)> quitCallback, std::function<bool(void)> nextTrackFun)
{
	m_PushLastTime = 0;
	m_PullLastTime = 0;
	m_IsRunning = false;

	if (m_xmlFilename.length() == 0)
	{
		log() << OpenViBE::Kernel::LogLevel_Error << "Error: No processor initialized\n";
		return false;
	}

	m_ChunksSent = 0;
//	m_PreviousChunkEnd = 0;
	m_PlayFast = playFast;	// @todo To work neatly it'd be better to be able to pass in the chunk times to the designer side
	m_PreviousEnd = 0;
	m_RequestNewOffset = false;

	for (auto decoder : m_vDecoders)
	{
		delete decoder;
	}
	m_vDecoders.clear();
	for (auto encoder : m_vEncoders)
	{
		delete encoder;
	}
	m_vEncoders.clear();

//	log() << LogLevel_Info << "Reset offset to " << ITimeArithmetics::timeToSeconds(m_PreviousEnd) << "\n";

	const CString expandedName = m_KernelContext.getConfigurationManager().expand(m_xmlFilename.c_str());

	std::stringstream ss;
	ss << "--define Tracker_Port_Send " << m_SendPort << " ";
	ss << "--define Tracker_Port_Receive " << m_RecvPort << " ";

	std::string allArgs = m_Arguments + " " + ss.str();

	m_pPlayerThread = new std::thread(std::bind(&playerLaunch, expandedName, allArgs.c_str(), m_PlayFast, m_NoGUI, m_SendPort));

	// @fixme this ad-hoc sleep is pretty terrible, but it seems that if player launch is slow enough and
	// we launch the client threads immediately below, in Windows it results in deadlocks and errors, suggesting that
	// the concurrency control logic is not really correct in the communication with the external processing box.
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	if (m_DoSend)
	{
		m_PushClient = new PushClient(m_SendPort);
		m_PushClientThread = new std::thread(&PushClient::start, m_PushClient);
	}
	if (m_DoReceive)
	{
		m_PullClient = new PullClient(m_RecvPort);
		m_PullClientThread = new std::thread(&PullClient::start, m_PullClient);
	}

//	log() << OpenViBE::Kernel::LogLevel_Debug << "External processing thread(s) launched, waiting sync\n";

	for (uint64_t retries = 0; retries<20; retries++)
	{
		if (m_DoSend)
		{
			m_pushStartTime = m_PushClient->getStartTime();
		}
		if (m_DoReceive)
		{
			m_pullStartTime = m_PullClient->getStartTime();
		}
		if (isSynced())
		{
//			log() << OpenViBE::Kernel::LogLevel_Debug << "Clients synced after " << retries << " retries\n";
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	if (!isSynced())
	{
//		log() << OpenViBE::Kernel::LogLevel_Error << "Error syncing client(s) after 10 secs of retries\n";
		if (m_PushClient) m_PushClient->requestQuit();
		if (m_PullClient) m_PullClient->requestQuit();
		return false;
	}

	m_IsRunning = true;
	while(m_IsRunning)
	{
		m_IsRunning &= push();
		m_IsRunning &= pop();
		
		if(!m_IsRunning && nextTrackFun)
		{
			m_IsRunning = nextTrackFun();
		}
		// @fixme the quit callback might be a bit expensive, shouldn't hammer it
		if(quitCallback && quitCallback( (m_DoSend ? m_PushLastTime : m_PullLastTime) ))
		{
			m_IsRunning = false;
		}
	}
	
	stop();

	return true;
}

bool ProcExternalProcessing::stop(void)
{
	m_IsRunning = false;

	if (m_PushClient)
	{
//		log() << OpenViBE::Kernel::LogLevel_Info << "Stopping external processing push client\n";

		m_PushClient->requestQuit();
		if (m_PushClientThread)
		{
			m_PushClientThread->join();
		}
		delete m_PushClientThread;
		m_PushClientThread = nullptr;
		delete m_PushClient;
		m_PushClient = nullptr;
	}

	if (m_PullClient)
	{
//		log() << OpenViBE::Kernel::LogLevel_Info << "Stopping external processing pull client\n";

		m_PullClient->requestQuit();
		if (m_PullClientThread)
		{
			m_PullClientThread->join();
		}
		delete m_PullClientThread;
		m_PullClientThread = nullptr;
		delete m_PullClient;
		m_PullClient = nullptr;
	}

	// tear down the player object
	if(m_pPlayerThread)
	{
//		log() << OpenViBE::Kernel::LogLevel_Trace << "Joining player thread\n";
		m_pPlayerThread->join();
		delete m_pPlayerThread;
		m_pPlayerThread = nullptr;
	}

	m_pushStartTime = BufferedClient::CLIENT_NOT_STARTED;
	m_pullStartTime = BufferedClient::CLIENT_NOT_STARTED;

	return true;
}

bool ProcExternalProcessing::save(void)
{
	auto& mgr = m_KernelContext.getConfigurationManager();

	std::stringstream sPort; sPort << m_SendPort;
	// std::stringstream rPort; rPort << m_RecvPort;

	mgr.addOrReplaceConfigurationToken("Tracker_Workspace_Processor", m_xmlFilename.c_str());
	mgr.addOrReplaceConfigurationToken("Tracker_Workspace_Processor_FirstPort", sPort.str().c_str());
	// mgr.addOrReplaceConfigurationToken("Tracker_Workspace_Processor_RecvPort", rPort.str().c_str());
	mgr.addOrReplaceConfigurationToken("Tracker_Workspace_Processor_NoGUI", (m_NoGUI ? "true" : "false"));
	mgr.addOrReplaceConfigurationToken("Tracker_Workspace_Processor_DoSend", (m_DoSend ? "true" : "false"));
	mgr.addOrReplaceConfigurationToken("Tracker_Workspace_Processor_DoReceive", (m_DoReceive ? "true" : "false"));

	return true;
}

bool ProcExternalProcessing::load(void)
{
	auto& mgr = m_KernelContext.getConfigurationManager();

	if (mgr.lookUpConfigurationTokenIdentifier("Tracker_Workspace_Processor") != OV_UndefinedIdentifier)
	{
		initialize( mgr.lookUpConfigurationTokenValue("Tracker_Workspace_Processor").toASCIIString() );
	}

	if (mgr.lookUpConfigurationTokenIdentifier("Tracker_Workspace_Processor_FirstPort") != OV_UndefinedIdentifier)
	{
		const uint32_t port = static_cast<uint32_t>(mgr.expandAsUInteger("${Tracker_Workspace_Processor_FirstPort}"));

		setProcessorPorts(port, port+1);
	}

	m_NoGUI = mgr.expandAsBoolean("${Tracker_Workspace_Processor_NoGUI}", m_NoGUI);
	m_DoSend = mgr.expandAsBoolean("${Tracker_Workspace_Processor_DoSend}", m_DoSend);
	m_DoReceive = mgr.expandAsBoolean("${Tracker_Workspace_Processor_DoReceive}", m_DoReceive);

	return true;
}
