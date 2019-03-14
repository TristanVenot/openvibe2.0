
#pragma once

#include <communication/ovCMessagingClient.h>

#include <deque>

#include <iostream>

#include <mutex>
#include <condition_variable>

#include "EncodedChunk.h"

namespace OpenViBETracker 
{

/**
 * \class BufferedClient 
 * \brief A messaging client base class thats associated with a buffer of chunks.
 * \author J. T. Lindgren
 * \details
 *
 * The class is threaded. The thread connects to a single instance of OpenViBE's 
 * External Processing Box that is assumed to be monitoring a specific port.
 *
 * The derived clients are one-directional: they only send or receive data.
 *
 */
class BufferedClient : protected Communication::MessagingClient
{
public:

	BufferedClient(uint32_t port) : m_Port(port) { };

	void requestQuit(void);

	bool hasQuit(void);

	void start(void);

	ovtime_t getStartTime(void) {
		// @todo might be a bit inefficient to have mutex here
		std::lock_guard<std::mutex> oLock(m_oThreadMutex);
		return m_StartTime; 
	};

	ovtime_t getTime(void) { return MessagingClient::getTime(); }

	static const ovtime_t CLIENT_NOT_STARTED = (ovtime_t)(-1);

protected:
	
	bool connectClient(void);

	// Derived classes implement this to do either push or pull
	virtual bool step(void) = 0;

	uint32_t m_Port = 0;

	// The mutex is used with the variables declared after it
	std::mutex m_oThreadMutex;
	std::condition_variable m_oBufferCondition;

	std::deque < EncodedChunk* > m_vBuffer;
	bool m_bPleaseQuit = false;
	bool m_bHasQuit = false;

	// Time when the client has connected and synced, (ovtime_t)(-1) if not yet
	ovtime_t m_StartTime = CLIENT_NOT_STARTED;

};

/**
 * \class PushClient 
 * \brief A client dedicated to pushing data towards an External Processing Box
 * \author J. T. Lindgren
 *
 */
class PushClient : public BufferedClient
{
public:
	PushClient(uint32_t port) : BufferedClient(port) { };

	// Append a chunk to be sent out
	bool pushBuffer(const EncodedChunk& rEncodedChunk);

	// Force all pushed chunks to be sent
	void requestFlush(void);

protected:

	// implements push
	virtual bool step(void) override;

	// Under the base class' mutex
	bool m_bPleaseFlush = false;
};


/**
 * \class PullClient 
 * \brief A class dedicated to pulling data from an External Processing Box
 * \author J. T. Lindgren
 *
 */
class PullClient : public BufferedClient
{

public:
	PullClient(uint32_t port) : BufferedClient(port) { };

	// Get the oldest chunk received. Returns false if none.
	bool pullBuffer(EncodedChunk& rEncodedChunk);

	bool isEndReceived(void) { return BufferedClient::isEndReceived(); }

protected:

	// implements pull
	virtual bool step(void) override;

	// Keep polling the sender for chunks as long as there is one available
	virtual bool popMessagesToBuffer(void);

};

};

