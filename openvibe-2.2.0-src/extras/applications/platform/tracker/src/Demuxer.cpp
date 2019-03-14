
//
// @note some ov files which have lots of stimulation chunks take long time to import
// when launching tracker from visual studio. this is probably due to memory allocation, similar to slow simple dsp grammar parsing.
//
#include "Demuxer.h"

#include <iostream>
#include <thread>
#include <deque>
#include <vector>

#include "../../../../plugins/processing/file-io/src/ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <system/ovCTime.h>

#include "Stream.h"

#include "CodecFactory.h"


using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBETracker;

bool Demuxer::initialize(void) 
{ 
	// log() << LogLevel_Info << "Source: Initializing with "	<< signalFile << "\n";

	m_ChunksSent = 0;
	m_bPending = false;
	m_target.setSource(m_origin.getSource());

	return true; 
}

bool Demuxer::uninitialize(void) 
{ 
	log() << LogLevel_Info << "Source: Uninitializing\n";

	m_origin.uninitialize();

	return true;
}

// bool Demuxer::pullChunk(MemoryBufferWithType& output)
bool Demuxer::step(void)
{
//	log() << LogLevel_Info << "Source: Trying to pull a chunk\n";

	while(!m_origin.isEOF())
	{

		while(!m_origin.isEOF() && m_oReader.getCurrentNodeIdentifier()==EBML::CIdentifier())
		{
			std::vector<uint8_t> bytes;
			m_origin.read(bytes, 1);

			/*
			OV_ERROR_UNLESS_KRF(
				s == 1 || l_bJustStarted,
				"Unexpected EOF in " << m_sFilename,
				OpenViBE::Kernel::ErrorType::BadParsing
			);
			*/
			if(bytes.size()>0)
			{
				m_oReader.processData(&bytes[0], bytes.size());
			}
		}
		if(!m_origin.isEOF() && m_oReader.getCurrentNodeSize()!=0)
		{
			std::vector<uint8_t> bytes;
			m_origin.read(bytes, static_cast<size_t>(m_oReader.getCurrentNodeSize()));

			/*
			OV_ERROR_UNLESS_KRF(
				s == m_oSwap.getSize(),
				"Unexpected EOF in " << m_sFilename,
				OpenViBE::Kernel::ErrorType::BadParsing
			);
			*/

			m_oPendingChunk.bufferData.resize(0);
			m_oPendingChunk.m_startTime = std::numeric_limits<ovtime_t>::max();
			m_oPendingChunk.m_endTime = std::numeric_limits<ovtime_t>::max();
			m_oPendingChunk.streamIndex = std::numeric_limits<uint32_t>::max();

			m_oReader.processData(&bytes[0], bytes.size());
		}

		if(m_bPending)
		{
			// We have dada
			// log() << LogLevel_Info << "Source: Found a chunk, queueing\n";
			size_t streamIndex = m_oPendingChunk.streamIndex;

			m_bPending = false;

			StreamPtr stream = m_target.getStream(streamIndex);
			if (!stream)
			{
				log() << LogLevel_Error << "Error: Trying to decode stream without creating it first (buffer before header in EBML?)\n";
				return false;
			}
			m_vDecoders[streamIndex]->decode(m_oPendingChunk);

			return true;
		}

	}

	if(m_origin.isEOF())
	{
		log() << LogLevel_Trace << "Source file EOF reached\n";
	}
	else
	{
		log() << LogLevel_Warning << "Issue with source file\n";
	}

	return false;
}


bool Demuxer::stop(void)
{

	return true;
}

EBML::boolean Demuxer::isMasterChild(const EBML::CIdentifier& rIdentifier)
{
	if(rIdentifier==EBML_Identifier_Header                        ) return true;
	if(rIdentifier==OVP_NodeId_OpenViBEStream_Header              ) return true;
	if(rIdentifier==OVP_NodeId_OpenViBEStream_Header_Compression  ) return false;
	if(rIdentifier==OVP_NodeId_OpenViBEStream_Header_StreamType  ) return false;
	if(rIdentifier==OVP_NodeId_OpenViBEStream_Buffer              ) return true;
	if(rIdentifier==OVP_NodeId_OpenViBEStream_Buffer_StreamIndex ) return false;
	if(rIdentifier==OVP_NodeId_OpenViBEStream_Buffer_StartTime    ) return false;
	if(rIdentifier==OVP_NodeId_OpenViBEStream_Buffer_EndTime      ) return false;
	if(rIdentifier==OVP_NodeId_OpenViBEStream_Buffer_Content      ) return false;
	return false;
}

void Demuxer::openChild(const EBML::CIdentifier& rIdentifier)
{
	m_vNodes.push(rIdentifier);

	EBML::CIdentifier& l_rTop=m_vNodes.top();

	if(l_rTop == EBML_Identifier_Header)
	{
		m_bHasEBMLHeader = true;
	}
	if(l_rTop==OVP_NodeId_OpenViBEStream_Header)
	{
		if(!m_bHasEBMLHeader)
		{
//			this->getLogManager() << LogLevel_Info << "The file " << m_sFilename << " uses an outdated (but still compatible) version of the .ov file format\n";
		}
	}
	if(l_rTop==OVP_NodeId_OpenViBEStream_Header)
	{
		m_vStreamIndexToOutputIndex.clear();
		m_vStreamIndexToTypeIdentifier.clear();
	}
}

void Demuxer::processChildData(const void* pBuffer, const uint64 ui64BufferSize)
{
	EBML::CIdentifier& l_rTop=m_vNodes.top();

	// Uncomment this when ebml version will be used
	//if(l_rTop == EBML_Identifier_EBMLVersion)
	//{
	//	const uint64 l_ui64VersionNumber=(uint64)m_oReaderHelper.getUIntegerFromChildData(pBuffer, ui64BufferSize);
	//}

	if(l_rTop==OVP_NodeId_OpenViBEStream_Header_Compression)
	{
		if(m_oReaderHelper.getUIntegerFromChildData(pBuffer, ui64BufferSize) != 0)
		{
//			OV_WARNING_K("Impossible to use compression as it is not yet implemented");
		}
	}
	else if(l_rTop==OVP_NodeId_OpenViBEStream_Header_StreamType)
	{
		const uint64_t typeId = m_oReaderHelper.getUIntegerFromChildData(pBuffer, ui64BufferSize);
		const size_t index = m_target.getNumStreams();
		m_target.createStream(index, typeId);
		DecoderBase* decoder = CodecFactory::getDecoder(m_KernelContext, *m_target.getStream(index));
		m_vDecoders.push_back(decoder);
	}
	else if(l_rTop==OVP_NodeId_OpenViBEStream_Buffer_StreamIndex)
	{
		m_oPendingChunk.streamIndex = (uint32_t) m_oReaderHelper.getUIntegerFromChildData(pBuffer, ui64BufferSize); // @note if trying to do -1 to  map to [0,...] convention, something breaks
	
//		log() << LogLevel_Info << "Run into index " << m_oPendingChunk.streamIndex << "\n";
	}
	else if(l_rTop==OVP_NodeId_OpenViBEStream_Buffer_StartTime)
	{
		m_oPendingChunk.m_startTime =m_oReaderHelper.getUIntegerFromChildData(pBuffer, ui64BufferSize);
	}
	else if(l_rTop==OVP_NodeId_OpenViBEStream_Buffer_EndTime)
	{
		m_oPendingChunk.m_endTime =m_oReaderHelper.getUIntegerFromChildData(pBuffer, ui64BufferSize);
	}
	else if(l_rTop==OVP_NodeId_OpenViBEStream_Buffer_Content)
	{
		m_oPendingChunk.bufferData.resize(static_cast<size_t>(ui64BufferSize));
		memcpy(&m_oPendingChunk.bufferData[0], reinterpret_cast<const EBML::uint8*>(pBuffer), static_cast<size_t>(ui64BufferSize));
	}
}

void Demuxer::closeChild(void)
{
	EBML::CIdentifier& l_rTop=m_vNodes.top();

	if(l_rTop==OVP_NodeId_OpenViBEStream_Header)
	{
		// Assign file streams to outputs here
	}

	if(l_rTop==OVP_NodeId_OpenViBEStream_Buffer)
	{
		m_bPending = ((m_oPendingChunk.streamIndex != std::numeric_limits<uint32_t>::max()) &&
					  (m_oPendingChunk.m_startTime != std::numeric_limits<ovtime_t>::max()) &&
					  (m_oPendingChunk.m_endTime != std::numeric_limits<ovtime_t>::max()));
	}

	m_vNodes.pop();
}
 
