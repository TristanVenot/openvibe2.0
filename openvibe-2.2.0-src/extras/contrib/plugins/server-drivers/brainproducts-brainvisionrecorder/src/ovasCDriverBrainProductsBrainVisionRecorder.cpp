#include "ovasCDriverBrainProductsBrainVisionRecorder.h"
#include "../ovasCConfigurationNetworkBuilder.h"

#include <system/ovCTime.h>

#include <cmath>

#include <iostream>

#include <cstdlib>
#include <cstring>

#include <openvibe/ovITimeArithmetics.h>

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace std;

//___________________________________________________________________//
//                                                                   //

CDriverBrainProductsBrainVisionRecorder::CDriverBrainProductsBrainVisionRecorder(IDriverContext& rDriverContext)
	:IDriver(rDriverContext)
	,m_oSettings("AcquisitionServer_Driver_BrainVisionRecorder", m_rDriverContext.getConfigurationManager())
	,m_pCallback(NULL)
	,m_pConnectionClient(NULL)
	,m_sServerHostName("localhost")
	,m_ui32ServerHostPort(51244)
	,m_ui32SampleCountPerSentBlock(0)
	,m_pStructRDA_MessageHeader(NULL)
	,m_uHeaderBufferSize(0)
{
	m_oSettings.add("Header", &m_oHeader);
	m_oSettings.add("ServerHostName", &m_sServerHostName);
	m_oSettings.add("ServerHostPort", &m_ui32ServerHostPort);
	m_oSettings.load();
}

CDriverBrainProductsBrainVisionRecorder::~CDriverBrainProductsBrainVisionRecorder(void)
{
}

const char* CDriverBrainProductsBrainVisionRecorder::getName(void)
{
	return "Brain Products amplifiers (through BrainVision Recorder)";
}

//___________________________________________________________________//
//                                                                   //

bool CDriverBrainProductsBrainVisionRecorder::initialize(
	const uint32 ui32SampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
	if(m_rDriverContext.isConnected()) { return false; }

	// Initialize GUID value
	M_DEFINE_GUID(GUID_RDAHeader,
		1129858446, 51606, 19590, uint8(175), uint8(74), uint8(152), uint8(187), uint8(246), uint8(201), uint8(20), uint8(80)
	);

	// Builds up client connection
	m_pConnectionClient=Socket::createConnectionClient();

	// Tries to connect to server
	m_pConnectionClient->connect(m_sServerHostName, m_ui32ServerHostPort);

	// Checks if connection is correctly established
	if(!m_pConnectionClient->isConnected())
	{
		// In case it is not, try to reconnect
		m_pConnectionClient->connect(m_sServerHostName, m_ui32ServerHostPort);
	}

	if(!m_pConnectionClient->isConnected())
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "Connection problem! Tried 2 times without success! :(\n";
		m_rDriverContext.getLogManager() << LogLevel_Error << "Verify port number and/or Hostname...\n";
		return false;
	}

	m_rDriverContext.getLogManager() << LogLevel_Trace << "> Client connected\n";

	// Initialize vars for reception
	m_pStructRDA_MessageHeader = nullptr;
	RDA_MessageHeader l_structRDA_MessageHeader;
	m_pcharStructRDA_MessageHeader = (char*)&l_structRDA_MessageHeader;

	uint32 l_ui32Received = 0;
	uint32 l_ui32ReqLength = 0;
	uint32 l_ui32Result = 0;
	uint32 l_ui32Datasize = sizeof(l_structRDA_MessageHeader);

	// Receive Header
	while(l_ui32Received < l_ui32Datasize)
	{
		l_ui32ReqLength = l_ui32Datasize -  l_ui32Received;
		l_ui32Result = m_pConnectionClient->receiveBuffer((char*)m_pcharStructRDA_MessageHeader, l_ui32ReqLength);

		l_ui32Received += l_ui32Result;
		m_pcharStructRDA_MessageHeader += l_ui32Result;
	}

	// Check for correct header GUID.
	if (!M_COMPARE_GUID(l_structRDA_MessageHeader.guid, GUID_RDAHeader))
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "GUID received is not correct!\n";
		return false;
	}

	// Check for correct header nType
	if (l_structRDA_MessageHeader.nType !=1)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "First Message received is not an header!\n";
		m_rDriverContext.getLogManager() << LogLevel_Error << "Try to reconnect....\n";
		return false;
	}

	// Check if we need a larger buffer, allocate
	if(!reallocateHeaderBuffer(l_structRDA_MessageHeader.nSize))
	{
		return false;
	}

	// Retrieve rest of data
	memcpy(*(&m_pStructRDA_MessageHeader), &l_structRDA_MessageHeader, sizeof(l_structRDA_MessageHeader));
	m_pcharStructRDA_MessageHeader = (char*)(*(&m_pStructRDA_MessageHeader)) + sizeof(l_structRDA_MessageHeader);
	l_ui32Received=0;
	l_ui32ReqLength = 0;
	l_ui32Result = 0;
	l_ui32Datasize = l_structRDA_MessageHeader.nSize - sizeof(l_structRDA_MessageHeader);
	while(l_ui32Received < l_ui32Datasize)
	{
		l_ui32ReqLength = l_ui32Datasize -  l_ui32Received;
		l_ui32Result = m_pConnectionClient->receiveBuffer((char*)m_pcharStructRDA_MessageHeader, l_ui32ReqLength);

		l_ui32Received += l_ui32Result;
		m_pcharStructRDA_MessageHeader += l_ui32Result;
	}

	m_pStructRDA_MessageStart = (RDA_MessageStart*)m_pStructRDA_MessageHeader; // pHeader will retain the pointer ownership

	m_rDriverContext.getLogManager() << LogLevel_Trace << "> Header received\n";

	// Save Header info into m_oHeader
	//m_oHeader.setExperimentIdentifier();
	//m_oHeader.setExperimentDate();

	//m_oHeader.setSubjectId();
	//m_oHeader.setSubjectName();
	//m_oHeader.setSubjectAge(m_structHeader.subjectAge);
	//m_oHeader.setSubjectGender();

	//m_oHeader.setLaboratoryId();
	//m_oHeader.setLaboratoryName();

	//m_oHeader.setTechnicianId();
	//m_oHeader.setTechnicianName();

	m_oHeader.setChannelCount((uint32)m_pStructRDA_MessageStart->nChannels);

	char* l_pszChannelNames = (char*)m_pStructRDA_MessageStart->dResolutions + (m_pStructRDA_MessageStart->nChannels * sizeof(m_pStructRDA_MessageStart->dResolutions[0]));
	for(uint32 i=0; i<m_oHeader.getChannelCount(); i++)
	{
		m_oHeader.setChannelName(i, l_pszChannelNames);
		m_oHeader.setChannelGain(i, (float32)((m_pStructRDA_MessageStart->dResolutions[i])));
		m_oHeader.setChannelUnits(i, OVTK_UNIT_Volts, OVTK_FACTOR_Micro);
		l_pszChannelNames += strlen(l_pszChannelNames) + 1;
	}

	m_oHeader.setSamplingFrequency((uint32)(1000000/m_pStructRDA_MessageStart->dSamplingInterval)); //dSamplingInterval in microseconds

	m_ui32SampleCountPerSentBlock=ui32SampleCountPerSentBlock;

	m_pCallback=&rCallback;

	m_ui32IndexIn = 0;
	m_ui32IndexOut = 0;
	m_ui32BuffDataIndex = 0;

	m_ui32MarkerCount =0;
	m_ui32NumberOfMarkers = 0;

	return true;
}

bool CDriverBrainProductsBrainVisionRecorder::start(void)
{
	if(!m_rDriverContext.isConnected()) { return false; }
	if(m_rDriverContext.isStarted()) { return false; }
	return true;
}

bool CDriverBrainProductsBrainVisionRecorder::loop(void)
{
	if(!m_rDriverContext.isConnected()) { return false; }
	if(!m_rDriverContext.isStarted()) { return true; }

	M_DEFINE_GUID(GUID_RDAHeader,
		1129858446, 51606, 19590, uint8(175), uint8(74), uint8(152), uint8(187), uint8(246), uint8(201), uint8(20), uint8(80)
	);

	// Initialize var to receive buffer of data
	m_pStructRDA_MessageHeader = nullptr;
	RDA_MessageHeader l_structRDA_MessageHeader;
	m_pcharStructRDA_MessageHeader = (char*)&l_structRDA_MessageHeader;
	uint32 l_ui32Received = 0;
	uint32 l_ui32ReqLength = 0;
	uint32 l_ui32Result = 0;
	uint32 l_ui32Datasize = sizeof(l_structRDA_MessageHeader);

	// Receive Header
	while(l_ui32Received < l_ui32Datasize)
	{
		l_ui32ReqLength = l_ui32Datasize -  l_ui32Received;
		l_ui32Result = m_pConnectionClient->receiveBuffer((char*)m_pcharStructRDA_MessageHeader, l_ui32ReqLength);
		l_ui32Received += l_ui32Result;
		m_pcharStructRDA_MessageHeader += l_ui32Result;
	}

	// Check for correct header nType
	if (l_structRDA_MessageHeader.nType == 1)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "Message received is a header!\n";
		return false;
	}
	if (l_structRDA_MessageHeader.nType == 3)
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "Message received is a STOP!\n";
		return false;
	}
	if (l_structRDA_MessageHeader.nType !=4)
	{
		return true;
	}

	// Check for correct header GUID.
    if (!M_COMPARE_GUID(l_structRDA_MessageHeader.guid, GUID_RDAHeader))
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "GUID received is not correct!\n";
		return false;
	}

	// Check if we need a larger buffer, allocate
	if(!reallocateHeaderBuffer(l_structRDA_MessageHeader.nSize))
	{
		return false;
	}

	// Retrieve rest of block.
	memcpy(*(&m_pStructRDA_MessageHeader), &l_structRDA_MessageHeader, sizeof(l_structRDA_MessageHeader));
	m_pcharStructRDA_MessageHeader = (char*)(*(&m_pStructRDA_MessageHeader)) + sizeof(l_structRDA_MessageHeader);
	l_ui32Received=0;
	l_ui32ReqLength = 0;
	l_ui32Result = 0;
	l_ui32Datasize = l_structRDA_MessageHeader.nSize - sizeof(l_structRDA_MessageHeader);
	while(l_ui32Received < l_ui32Datasize)
	{
		l_ui32ReqLength = l_ui32Datasize -  l_ui32Received;
		l_ui32Result = m_pConnectionClient->receiveBuffer((char*)m_pcharStructRDA_MessageHeader, l_ui32ReqLength);

		l_ui32Received += l_ui32Result;
		m_pcharStructRDA_MessageHeader += l_ui32Result;
	}
	m_ui32BuffDataIndex++;

	// Put the data into MessageData32 structure
	m_pStructRDA_MessageData32 = nullptr;
	m_pStructRDA_MessageData32 = (RDA_MessageData32*)m_pStructRDA_MessageHeader;

	//////////////////////
	//Markers
	if (m_pStructRDA_MessageData32->nMarkers > 0)
	{
// 		if (m_pStructRDA_MessageData32->nMarkers == 0)
// 		{
// 			return true;
// 		}

		m_pStructRDA_Marker = (RDA_Marker*)((char*)m_pStructRDA_MessageData32->fData + m_pStructRDA_MessageData32->nPoints * m_oHeader.getChannelCount() * sizeof(m_pStructRDA_MessageData32->fData[0]));

		m_ui32NumberOfMarkers = m_pStructRDA_MessageData32->nMarkers;

		m_vStimulationIdentifier.assign(m_ui32NumberOfMarkers, 0);
		m_vStimulationDate.assign(m_ui32NumberOfMarkers, 0);
		m_vStimulationSample.assign(m_ui32NumberOfMarkers, 0);

		for (uint32 i = 0; i < m_pStructRDA_MessageData32->nMarkers; i++)
		{
			char* pszType = m_pStructRDA_Marker->sTypeDesc;
			char* pszDescription = pszType + strlen(pszType) + 1;

			m_vStimulationIdentifier[i] = atoi(strtok (pszDescription,"S"));
			m_vStimulationDate[i] = ITimeArithmetics::sampleCountToTime(m_oHeader.getSamplingFrequency(), m_pStructRDA_Marker->nPosition );
			m_vStimulationSample[i] = m_pStructRDA_Marker->nPosition;
			m_pStructRDA_Marker = (RDA_Marker*)((char*)m_pStructRDA_Marker + m_pStructRDA_Marker->nSize);
		}

		m_ui32MarkerCount += m_pStructRDA_MessageData32->nMarkers;
	}

	const uint32 l_ui32DataSize = m_oHeader.getChannelCount()*(uint32)m_pStructRDA_MessageData32->nPoints;
	if(m_vSignalBuffer.size() < l_ui32DataSize)
	{
		m_vSignalBuffer.resize(l_ui32DataSize);
	}

	for (uint32 i=0; i < m_oHeader.getChannelCount(); i++)
	{
		for (uint32 j=0; j < (uint32)m_pStructRDA_MessageData32->nPoints; j++)
		{
			m_vSignalBuffer[j + (i*(uint32)m_pStructRDA_MessageData32->nPoints)] = (float32)m_pStructRDA_MessageData32->fData[(m_oHeader.getChannelCount()*j) + i]*m_oHeader.getChannelGain(i);
		}
	}

	// send data
	CStimulationSet l_oStimulationSet;
	l_oStimulationSet.setStimulationCount(m_ui32NumberOfMarkers);
	for (uint32 i = 0; i < m_ui32NumberOfMarkers; i++)
	{
		l_oStimulationSet.setStimulationIdentifier(i, OVTK_StimulationId_Label(m_vStimulationIdentifier[i]));
		l_oStimulationSet.setStimulationDate(i, m_vStimulationDate[i]);
		l_oStimulationSet.setStimulationDuration(i, 0);
	}

	m_pCallback->setSamples(&m_vSignalBuffer[0],(uint32)m_pStructRDA_MessageData32->nPoints);
	m_pCallback->setStimulationSet(l_oStimulationSet);
	m_rDriverContext.correctDriftSampleCount(m_rDriverContext.getSuggestedDriftCorrectionSampleCount());

	m_ui32NumberOfMarkers = 0;

	return true;

}

bool CDriverBrainProductsBrainVisionRecorder::stop(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "> Connection stopped\n";

	if(!m_rDriverContext.isConnected()) { return false; }
	if(!m_rDriverContext.isStarted()) { return false; }
	return true;
}

bool CDriverBrainProductsBrainVisionRecorder::uninitialize(void)
{
	if(!m_rDriverContext.isConnected()) { return false; }
	if(m_rDriverContext.isStarted()) { return false; }

	if (m_pStructRDA_MessageHeader) 
	{
		free(m_pStructRDA_MessageHeader);
		m_pStructRDA_MessageHeader= nullptr;
	}

	m_pcharStructRDA_MessageHeader= nullptr;
	m_pStructRDA_MessageStart= nullptr;
	m_pStructRDA_MessageStop= nullptr;
	m_pStructRDA_MessageData32= nullptr;
	m_pStructRDA_Marker= nullptr;

	m_pCallback= nullptr;

	m_uHeaderBufferSize = 0;
	m_vSignalBuffer.clear();

	// Cleans up client connection
	m_pConnectionClient->close();
	m_pConnectionClient->release();
	m_pConnectionClient= nullptr;
	m_rDriverContext.getLogManager() << LogLevel_Trace << "> Client disconnected\n";

	return true;
}

//___________________________________________________________________//
//                                                                   //

bool CDriverBrainProductsBrainVisionRecorder::isConfigurable(void)
{
	return true;
}

bool CDriverBrainProductsBrainVisionRecorder::configure(void)
{
	CConfigurationNetworkBuilder l_oConfiguration(OpenViBE::Directories::getDataDir() + "/applications/acquisition-server/interface-BrainProducts-BrainVisionRecorder.ui");

	l_oConfiguration.setHostName(m_sServerHostName);
	l_oConfiguration.setHostPort(m_ui32ServerHostPort);

	if(l_oConfiguration.configure(m_oHeader))
	{
		m_sServerHostName=l_oConfiguration.getHostName();
		m_ui32ServerHostPort=l_oConfiguration.getHostPort();

		m_oSettings.save();

		return true;
	}

	return false;
}

bool CDriverBrainProductsBrainVisionRecorder::reallocateHeaderBuffer(size_t newSize)
{
	// Reallocate buffer?
	if(newSize>m_uHeaderBufferSize)
	{
		if(m_pStructRDA_MessageHeader) 
		{
			free(m_pStructRDA_MessageHeader);
		}
		m_uHeaderBufferSize = newSize;
		m_pStructRDA_MessageHeader = (RDA_MessageHeader*)malloc(m_uHeaderBufferSize);
		if(!m_pStructRDA_MessageHeader)
		{
			m_rDriverContext.getLogManager() << LogLevel_Error << "Couldn't allocate memory\n";
			return false;		
		}
	}
	return true;
}