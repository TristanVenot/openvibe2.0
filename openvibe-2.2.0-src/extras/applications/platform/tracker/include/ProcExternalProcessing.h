
#pragma once

#include "StreamBundle.h"

#include <iostream>
#include <thread>

#include "Processor.h"
#include "ProcExternalProcessingHelper.h" // Push and Pull clients

#include "Encoder.h"
#include "Decoder.h"

namespace OpenViBETracker 
{

/**
 * \class ProcExternalProcessing 
 * \brief A processor implemented by passing data to/from External Processing Boxes inserted into Designer scenarios
 * \author J. T. Lindgren
 *
 */
class ProcExternalProcessing : public Processor {
public:

	ProcExternalProcessing(const OpenViBE::Kernel::IKernelContext& KernelContext) : Processor(KernelContext) {};
	ProcExternalProcessing(const OpenViBE::Kernel::IKernelContext& KernelContext, const ProcExternalProcessing& other);

	virtual bool initialize(const std::string& xmlFile) override;
	virtual bool uninitialize(void) override;

	// Set targets for push and pull
	virtual bool setNewSource(StreamBundle* source, bool sendHeader, bool sendEnd) override;
	virtual bool setNewTarget(StreamBundle* target) override;

	virtual bool play(bool playFast, std::function<bool(ovtime_t)> quitCallback) override { return play(playFast, quitCallback, nullptr); };
	// @param nextTrackFun Callback to request more data (used for catenate mode)
	virtual bool play(bool playFast, std::function<bool(ovtime_t)> quitCallback, std::function<bool(void)> nextTrackFun);

	// virtual bool play(bool playFast) override;
	virtual bool stop(void) override;

	virtual bool setProcessorPorts(uint32_t sendPort, uint32_t recvPort) override {
		m_SendPort = sendPort; m_RecvPort = recvPort;
		return true;
	}

	virtual bool getProcessorPorts(uint32_t& sendPort, uint32_t& recvPort) const override {
		sendPort = m_SendPort; recvPort = m_RecvPort;
		return true;
	}

	virtual bool getProcessorFlags(bool& noGUI, bool& doSend, bool& doReceive) const { noGUI = m_NoGUI; doSend = m_DoSend; doReceive = m_DoReceive; return true; }
	virtual bool setProcessorFlags(bool noGUI, bool doSend, bool doReceive) { m_NoGUI = noGUI; m_DoSend = doSend; m_DoReceive = doReceive; return true; }

	virtual bool canPull(void) const { return m_DoReceive; };
	virtual bool canPush(void) const { return m_DoSend; };

	virtual ovtime_t getCurrentTime(void) const override;

	virtual bool isSynced(void) const {
		return (m_DoSend ? (m_pushStartTime != BufferedClient::CLIENT_NOT_STARTED) : true) &&
			(m_DoReceive ? (m_pullStartTime != BufferedClient::CLIENT_NOT_STARTED) : true);
	}

	virtual bool isRunning(void) const override { return m_IsRunning; }

	// Serialize state to configuration manager
	virtual bool save(void) override;
	virtual bool load(void) override;

protected:
	bool connectClient(Communication::MessagingClient& client, uint32_t port);

	// Send data to External Processing box
	virtual bool push(void);
	// Receive data from External Processing box
	virtual bool pop(void);

	// n.b. for each external processing box, we need to have a separate thread since we don't know for
	// sure in which order OpenViBE will schedule their execution.
	PushClient* m_PushClient = nullptr;
	std::thread* m_PushClientThread = nullptr;
	ovtime_t m_pushStartTime = BufferedClient::CLIENT_NOT_STARTED;

	PullClient* m_PullClient = nullptr;
	std::thread* m_PullClientThread = nullptr;
	ovtime_t m_pullStartTime = BufferedClient::CLIENT_NOT_STARTED;

	std::thread* m_pPlayerThread = nullptr;

	bool m_sentSomething = false;
	bool m_IsRunning = false;

	uint64_t m_ChunksSent = 0;
	ovtime_t m_PushLastTime = 0;
	ovtime_t m_PullLastTime = 0;

	// if either port is zero, then the communication from the scenario is expected to flow to one direction only
	uint32_t m_SendPort = 50011;
	uint32_t m_RecvPort = 50012;

	bool m_DoSend = true;
	bool m_DoReceive = true;

	// Used in continuous playing mode to get the start offset for the next stream
	ovtime_t m_PreviousEnd = 0;
	bool m_RequestNewOffset = false;

	bool m_SendHeader = true;
	bool m_SendEnd = true;

	bool m_NoGUI = true;

	StreamBundle* m_Source = nullptr;
	StreamBundle* m_Target = nullptr;

	std::vector<EncoderBase*> m_vEncoders;
	std::vector<DecoderBase*> m_vDecoders;

};

};

