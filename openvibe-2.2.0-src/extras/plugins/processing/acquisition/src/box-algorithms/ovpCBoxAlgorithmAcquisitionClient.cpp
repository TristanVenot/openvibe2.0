#include "ovpCBoxAlgorithmAcquisitionClient.h"
#include <cstdlib>
#include <limits>

#include <openvibe/ovITimeArithmetics.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Acquisition;

uint64_t CBoxAlgorithmAcquisitionClient::getClockFrequency(void)
{
	return 64LL<<32;
}

bool CBoxAlgorithmAcquisitionClient::initialize(void)
{
	m_pAcquisitionStreamDecoder = &getAlgorithmManager().getAlgorithm(getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_AcquisitionStreamDecoder));

	m_pAcquisitionStreamDecoder->initialize();

	ip_pAcquisitionMemoryBuffer.initialize(m_pAcquisitionStreamDecoder->getInputParameter(OVP_GD_Algorithm_AcquisitionStreamDecoder_InputParameterId_MemoryBufferToDecode));
	op_ui64BufferDuration.initialize(m_pAcquisitionStreamDecoder->getOutputParameter(OVP_GD_Algorithm_AcquisitionStreamDecoder_OutputParameterId_BufferDuration));
	op_pExperimentInformationMemoryBuffer.initialize(m_pAcquisitionStreamDecoder->getOutputParameter(OVP_GD_Algorithm_AcquisitionStreamDecoder_OutputParameterId_ExperimentInformationStream));
	op_pSignalMemoryBuffer.initialize(m_pAcquisitionStreamDecoder->getOutputParameter(OVP_GD_Algorithm_AcquisitionStreamDecoder_OutputParameterId_SignalStream));
	op_pStimulationMemoryBuffer.initialize(m_pAcquisitionStreamDecoder->getOutputParameter(OVP_GD_Algorithm_AcquisitionStreamDecoder_OutputParameterId_StimulationStream));
	op_pChannelLocalisationMemoryBuffer.initialize(m_pAcquisitionStreamDecoder->getOutputParameter(OVP_GD_Algorithm_AcquisitionStreamDecoder_OutputParameterId_ChannelLocalisationStream));
	op_pChannelUnitsMemoryBuffer.initialize(m_pAcquisitionStreamDecoder->getOutputParameter(OVP_GD_Algorithm_AcquisitionStreamDecoder_OutputParameterId_ChannelUnitsStream));

	m_ui64LastChunkStartTime = 0;
	m_ui64LastChunkEndTime = 0;
	m_pConnectionClient = nullptr;

	const IBox& l_rStaticBoxContext = this->getStaticBoxContext();
	if (l_rStaticBoxContext.getOutputCount() < 5)
	{
		this->getLogManager() << LogLevel_Error << "Code expects at least 5 box outputs. Did you update the box?\n";
		return false;
	}

	return true;
}

bool CBoxAlgorithmAcquisitionClient::uninitialize(void)
{
	if(m_pConnectionClient)
	{
		m_pConnectionClient->close();
		m_pConnectionClient->release();
		m_pConnectionClient= nullptr;
	}

	op_pChannelUnitsMemoryBuffer.uninitialize();
	op_pChannelLocalisationMemoryBuffer.uninitialize();
	op_pStimulationMemoryBuffer.uninitialize();
	op_pSignalMemoryBuffer.uninitialize();
	op_pExperimentInformationMemoryBuffer.uninitialize();
	op_ui64BufferDuration.uninitialize();
	ip_pAcquisitionMemoryBuffer.uninitialize();

	m_pAcquisitionStreamDecoder->uninitialize();

	getAlgorithmManager().releaseAlgorithm(*m_pAcquisitionStreamDecoder);

	m_pAcquisitionStreamDecoder= nullptr;

	return true;
}

bool CBoxAlgorithmAcquisitionClient::processClock(IMessageClock& rMessageClock)
{
	if(!m_pConnectionClient)
	{

		CString l_sServerName = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
		uint32_t l_ui32ServerPort = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);
		if(l_sServerName.length() == 0)
		{
			this->getLogManager() << LogLevel_Warning << "Empty server name, please set it to a correct value or set AcquisitionServer_HostName in config files. Defaulting to \"localhost\".\n";
			l_sServerName = "localhost";
		}
		if(l_ui32ServerPort == std::numeric_limits<uint32_t>::max() || l_ui32ServerPort == std::numeric_limits<uint32_t>::min())
		{
			this->getLogManager() << LogLevel_Error << "Invalid value for port : " << l_ui32ServerPort << ". Please set the port to a positive non-zero integer value.\n";
			return false;
		}

		m_pConnectionClient=Socket::createConnectionClient();
		m_pConnectionClient->connect(l_sServerName, l_ui32ServerPort);
		if(!m_pConnectionClient->isConnected())
		{
			this->getLogManager() << LogLevel_Error << "Could not connect to server " << l_sServerName << ":" << l_ui32ServerPort << ". Make sure the server is running and in Play state.\n";
			return false;
		}
	}

	if(m_pConnectionClient && m_pConnectionClient->isReadyToReceive() /* && rMessageClock.getTime()>m_ui64LastChunkEndTime */)
	{
		getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	}

	return true;
}

bool CBoxAlgorithmAcquisitionClient::process(void)
{
	if(!m_pConnectionClient || !m_pConnectionClient->isConnected())
	{
		return false;
	}

	// IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	op_pExperimentInformationMemoryBuffer=l_rDynamicBoxContext.getOutputChunk(0);
	op_pSignalMemoryBuffer=l_rDynamicBoxContext.getOutputChunk(1);
	op_pStimulationMemoryBuffer=l_rDynamicBoxContext.getOutputChunk(2);
	op_pChannelLocalisationMemoryBuffer=l_rDynamicBoxContext.getOutputChunk(3);
	op_pChannelUnitsMemoryBuffer=l_rDynamicBoxContext.getOutputChunk(4);

	while(m_pConnectionClient->isReadyToReceive())
	{
		uint64_t l_ui64MemoryBufferSize=0;
		if(!m_pConnectionClient->receiveBufferBlocking(&l_ui64MemoryBufferSize, sizeof(l_ui64MemoryBufferSize)))
		{
			getLogManager() << LogLevel_Error << "Could not receive memory buffer size from the server. Is the server on 'Play'?\n";
			return false;
		}
		if(!ip_pAcquisitionMemoryBuffer->setSize(l_ui64MemoryBufferSize, true))
		{
			getLogManager() << LogLevel_Error << "Could not re allocate memory buffer with size " << l_ui64MemoryBufferSize << "\n";
			return false;
		}
		if(!m_pConnectionClient->receiveBufferBlocking(ip_pAcquisitionMemoryBuffer->getDirectPointer(), static_cast<uint32_t>(l_ui64MemoryBufferSize)))
		{
			getLogManager() << LogLevel_Error << "Could not receive memory buffer content of size " << l_ui64MemoryBufferSize << "\n";
			return false;
		}

		m_pAcquisitionStreamDecoder->process();


		if(m_pAcquisitionStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_AcquisitionStreamDecoder_OutputTriggerId_ReceivedHeader)
		 ||m_pAcquisitionStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_AcquisitionStreamDecoder_OutputTriggerId_ReceivedBuffer)
		 ||m_pAcquisitionStreamDecoder->isOutputTriggerActive(OVP_GD_Algorithm_AcquisitionStreamDecoder_OutputTriggerId_ReceivedEnd))
		{
			l_rDynamicBoxContext.markOutputAsReadyToSend(0, m_ui64LastChunkStartTime, m_ui64LastChunkEndTime);
			l_rDynamicBoxContext.markOutputAsReadyToSend(1, m_ui64LastChunkStartTime, m_ui64LastChunkEndTime);
			l_rDynamicBoxContext.markOutputAsReadyToSend(2, m_ui64LastChunkStartTime, m_ui64LastChunkEndTime);
			if(op_pChannelLocalisationMemoryBuffer->getSize()>0)
			{
				l_rDynamicBoxContext.markOutputAsReadyToSend(3, m_ui64LastChunkStartTime, m_ui64LastChunkEndTime);
			}
			else
			{
				l_rDynamicBoxContext.setOutputChunkSize(3, 0, true);
			}

			if(op_pChannelUnitsMemoryBuffer->getSize()>0)
			{
				l_rDynamicBoxContext.markOutputAsReadyToSend(4, m_ui64LastChunkStartTime, m_ui64LastChunkEndTime);
			}
			else
			{
				l_rDynamicBoxContext.setOutputChunkSize(4, 0, true);
			}
			m_ui64LastChunkStartTime=m_ui64LastChunkEndTime;
			m_ui64LastChunkEndTime+=op_ui64BufferDuration;
			// @todo ?
			// const float64 l_f64Latency=ITimeArithmetics::timeToSeconds(m_ui64LastChunkEndTime)-ITimeArithmetics::timeToSeconds(this->getPlayerContext().getCurrentTime());
			const float64 l_f64Latency=(int64(m_ui64LastChunkEndTime-this->getPlayerContext().getCurrentTime())/(1LL<<22))/1024.;
			this->getLogManager() << LogLevel_Debug << "Acquisition inner latency : " << l_f64Latency << "\n";
		}
	}

	return true;
}
