
#include <iostream>
#include <thread>
#include <sstream>

#include <system/ovCTime.h>

#include "ProcExternalProcessingHelper.h"

using namespace OpenViBETracker;

void BufferedClient::requestQuit(void)
{
	std::lock_guard<std::mutex> oLock(m_oThreadMutex);
	m_bPleaseQuit = true;
	m_oBufferCondition.notify_one();
}

bool BufferedClient::hasQuit(void)
{
	std::lock_guard<std::mutex> oLock(m_oThreadMutex);
	return m_bHasQuit;
}

void BufferedClient::start(void)
{
	if (!connectClient()) {
		std::lock_guard<std::mutex> oLock(m_oThreadMutex);
		m_bHasQuit = true;
		return;
	}

	while (true)
	{
		// Push or pull, uses lock internally
		if (!step())
		{
			break;
		}

		{
			std::lock_guard<std::mutex> oLock(m_oThreadMutex);

			// Exit the loop if we're told to quit or if we've lost the connection
			if (m_bPleaseQuit || !isConnected() || isEndReceived())
			{
				break;
			}
		}


	}
	
	// Shut the connection 
	this->close();

	{
		std::lock_guard<std::mutex> oLock(m_oThreadMutex);
		m_bHasQuit = true;
	}

	// The thread will exit here and can be joined
}

bool BufferedClient::connectClient(void)
{
	static int connectId = 0;

	std::stringstream cId;
	cId << std::string("tracker");
	cId << connectId++;

	const std::string connectionID = cId.str();
	this->setConnectionID(connectionID);
	int errorCount = 0;
	while (!this->connect("127.0.0.1", m_Port))
	{
		MessagingClient::ELibraryError error = this->getLastError();

		if (error == MessagingClient::ELibraryError::Socket_FailedToConnect)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			if (errorCount++ > 20)
			{
				printf("Designer not responding on port %d, retried 5 secs\n", m_Port);
				return false;
			}
		}
		else
		{
			printf("Error %d\n", error);
			return false;
		}

		/*
		if (s_DidRequestForcedQuit)
		{
		exit(EXIT_SUCCESS);
		}
		*/
	}

	// Announce to server that the box has finished initializing and wait for acknowledgement
	errorCount = 0;
	while (!this->waitForSyncMessage())
	{
		if (errorCount++ > 10)
		{
			printf("Server not syncing in port %d\n", m_Port);
			this->close();
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	this->pushLog(Communication::ELogLevel::LogLevel_Info, "Received Ping");

	this->pushSync();
	this->pushLog(Communication::ELogLevel::LogLevel_Info, "Sent Pong");

	{
		std::lock_guard<std::mutex> oLock(m_oThreadMutex);
		m_StartTime = this->getTime();
	}

	return true;
}


bool PushClient::pushBuffer(const EncodedChunk& rEncodedChunk)
{
	{
		std::lock_guard<std::mutex> oLock(m_oThreadMutex);
		if (!m_bPleaseQuit)
		{
			EncodedChunk* l_pMemoryBuffer = new EncodedChunk(rEncodedChunk);
			m_vBuffer.push_back(l_pMemoryBuffer);
		}
	}

	// No big harm notifying in any case, though if in 'quit' state, the quit request has already notified
	m_oBufferCondition.notify_one();

	return true;
}

bool PushClient::step(void)
{
	std::unique_lock<std::mutex> oLock(m_oThreadMutex, std::defer_lock);

	oLock.lock();

	// Normal condition for the wait to exit is the flush request or having more data
	m_oBufferCondition.wait(oLock,
		[this]() {
		return (m_bPleaseFlush || m_vBuffer.size()>0 || m_bPleaseQuit || !isConnected() || isEndReceived());
	});

	while (m_vBuffer.size() > 0)
	{
		EncodedChunk* chunk = m_vBuffer[0];
		m_vBuffer.pop_front();

		if (!this->pushEBML(chunk->streamIndex, chunk->m_startTime, chunk->m_endTime, std::make_shared<const std::vector<uint8_t>>(chunk->bufferData)))
		{
			std::cerr << "Failed to push EBML.\n";
			std::cerr << "Error " << this->getLastError() << "\n";
			oLock.unlock();
			return false;
		}

		delete chunk;
	}

	if (m_bPleaseFlush)
	{
		this->pushSync();
		m_bPleaseFlush = false;

		// We don't use condition variable here as users of the client should never 
		// be able to interrupt this wait unless error state is reached
		while (!m_bPleaseQuit && isConnected() && !isEndReceived() && !this->waitForSyncMessage()) 
		{
			oLock.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			oLock.lock();
		}
	}

	oLock.unlock();

	return true;

}

void PushClient::requestFlush(void)
{
	std::lock_guard<std::mutex> oLock(m_oThreadMutex);
	m_bPleaseFlush = true;
	m_oBufferCondition.notify_one();
}

bool PullClient::pullBuffer(EncodedChunk& rEncodedChunk)
{
	std::lock_guard<std::mutex> oLock(m_oThreadMutex);

	if (m_vBuffer.size() == 0)
	{
		return false;
	}

	EncodedChunk* ptr = m_vBuffer[0];
	m_vBuffer.pop_front();
	rEncodedChunk = *ptr;
	delete ptr;

	return true;
}

bool PullClient::popMessagesToBuffer(void)
{
	uint64_t packetId, m_startTime, m_endTime;
	uint32_t streamIndex;
	std::shared_ptr<const std::vector<uint8_t>> ebml;
	bool gotSomething = false;

	while (this->popEBML(packetId, streamIndex, m_startTime, m_endTime, ebml))
	{
		// @todo optimize by passing in the buffer to popEBML already
		EncodedChunk* chunk = new EncodedChunk();

		chunk->m_startTime = m_startTime;
		chunk->m_endTime = m_endTime;
		chunk->streamIndex = streamIndex;

		// @fixme inefficient to query for each chunk since its stream specific
		uint32_t id;
		std::string streamName;
		this->getInput(chunk->streamIndex, id, chunk->streamType, streamName);

		chunk->bufferData.resize(ebml->size());
		for (size_t i = 0; i < ebml->size(); i++)
		{
			chunk->bufferData[i] = (*ebml)[i];
		}

		{
			std::lock_guard<std::mutex> oLock(m_oThreadMutex);
			m_vBuffer.push_back(chunk);
		}

		gotSomething = true;

		//				std::cout << "Got pkg " << packetId << " idx " << streamIndex << " siz " << chunk.bufferData.size() << "\n";
	}

	return gotSomething;

}

// pull
bool PullClient::step(void) 
{
	// Pull items until we get the sync message (no more to send)
	while (!this->waitForSyncMessage() && this->isConnected() && !this->isEndReceived())
	{
		popMessagesToBuffer();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		// std::this_thread::yield();
	}

	// Got sync, pull whatever remaining buffered on the sender
	popMessagesToBuffer();

	if (!this->isConnected() || this->isEndReceived())
	{
		return false;
	}

	// Notify we've processed everything
	this->pushSync();

	return true;

}
