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

#include "StreamRendererChannelLocalization.h"

using namespace OpenViBE;
using namespace OpenViBETracker;

OpenViBE::CString StreamRendererChannelLocalization::renderAsText(uint32_t indent) const
{
	const TypeChannelLocalization::Header& hdr = m_Stream->getHeader();

	std::stringstream ss;

	ss << std::string(indent, ' ') << "Dynamic: "
		<< (hdr.m_Dynamic ? "True" : "False") << std::endl;

	for (size_t i = 0; i < m_Stream->getChunkCount(); i++)
	{
		const TypeChannelLocalization::Buffer* buf = m_Stream->getChunk(i);

		ss << std::string(indent, ' ') << "Configuration at time " << ITimeArithmetics::timeToSeconds(buf->m_startTime)
			<< "s:" << std::endl;

		const OpenViBE::float64* ptr = buf->m_buffer.getBuffer();

		for (uint32_t chn = 0; chn < buf->m_buffer.getDimensionSize(0); chn++)
		{

			ss << std::string(indent, ' ')
				<< "  Channel " << chn << " (" << hdr.m_header.getDimensionLabel(0, chn) << ") "
				<< "x=" << ptr[chn * 3 + 0] << " "
				<< "y=" << ptr[chn * 3 + 1] << " "
				<< "z=" << ptr[chn * 3 + 2]
				<< std::endl;
		}	
	}

	return OpenViBE::CString(ss.str().c_str());
}

bool StreamRendererChannelLocalization::showChunkList(void)
{
	return StreamRendererLabel::showChunkList("Channel localization stream details");
}
