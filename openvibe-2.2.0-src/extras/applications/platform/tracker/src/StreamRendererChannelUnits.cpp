//
// OpenViBE Tracker
//
// @author J.T. Lindgren
// 

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <iostream>

#include <system/ovCTime.h>
#include <openvibe/ovITimeArithmetics.h>

#include "StreamRendererChannelUnits.h"

using namespace OpenViBE;
using namespace OpenViBETracker;

OpenViBE::CString StreamRendererChannelUnits::renderAsText(uint32_t indent) const
{

	const TypeChannelUnits::Header& hdr = m_Stream->getHeader();

	std::stringstream ss;

	ss << std::string(indent, ' ') << "Dynamic: "
		<< (hdr.m_Dynamic ? "True" : "False") << std::endl;

	for (size_t i = 0; i < m_Stream->getChunkCount(); i++)
	{
		const TypeChannelUnits::Buffer* buf = m_Stream->getChunk(i);
		
		ss << std::string(indent, ' ') << "Configuration at time " << ITimeArithmetics::timeToSeconds(buf->m_startTime)
			<< "s:" << std::endl;

		const OpenViBE::float64* ptr = buf->m_buffer.getBuffer();

		for (uint32_t chn = 0; chn < buf->m_buffer.getDimensionSize(0); chn++)
		{
			const CString l_sUnit = m_KernelContext.getTypeManager().getEnumerationEntryNameFromValue(OV_TypeId_MeasurementUnit,
				static_cast<uint64>(ptr[chn * 2 + 0]));
			const CString l_sFactor = m_KernelContext.getTypeManager().getEnumerationEntryNameFromValue(OV_TypeId_Factor,
				static_cast<uint64>(ptr[chn * 2 + 1]));

			ss << std::string(indent, ' ') 
				<< "  Channel " << chn << " (" << hdr.m_header.getDimensionLabel(0, chn) << ") "
				<< "Unit: " << l_sUnit << ", Factor: " << l_sFactor << std::endl;
		}
	}

	return OpenViBE::CString(ss.str().c_str());
}

bool StreamRendererChannelUnits::showChunkList(void)
{
	return StreamRendererLabel::showChunkList("Channel units stream details");
}
