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

#include "StreamRendererExperimentInformation.h"

using namespace OpenViBETracker;

OpenViBE::CString StreamRendererExperimentInformation::renderAsText(uint32_t indent) const
{

	const TypeExperimentInformation::Header& hdr = m_Stream->getHeader();

	std::stringstream ss;

	ss << std::string(indent, ' ') << "Experiment id: " << hdr.m_Experiment_Identifier << std::endl;
	ss << std::string(indent, ' ') << "Experiment date: " << hdr.m_Experiment_Date << std::endl;

	ss << std::string(indent, ' ') << "Subject id: " << hdr.m_Subject_Identifier << std::endl;
	ss << std::string(indent, ' ') << "Subject name: " << hdr.m_Subject_Name << std::endl;
	ss << std::string(indent, ' ') << "Subject age: " << hdr.m_Subject_Age << std::endl;
	ss << std::string(indent, ' ') << "Subject gender: " << hdr.m_Subject_Gender << std::endl;

	ss << std::string(indent, ' ') << "Laboratory id: " << hdr.m_Context_LaboratoryIdentifier << std::endl;
	ss << std::string(indent, ' ') << "Laboratory name: " << hdr.m_Context_LaboratoryName << std::endl;
	ss << std::string(indent, ' ') << "Technician id: " << hdr.m_Context_TechnicianIdentifier << std::endl;
	ss << std::string(indent, ' ') << "Technician name: " << hdr.m_Context_TechnicianName << std::endl;

	//	ss << std::string(indent, ' ') << "Channels: " << m_Header.m_header.getDimensionSize(0) << std::endl;
	//	ss << std::string(indent, ' ') << "Samples per chunk: " << m_Header.m_header.getDimensionSize(1) << std::endl;
	return OpenViBE::CString(ss.str().c_str());
}

bool StreamRendererExperimentInformation::showChunkList(void)
{
	return StreamRendererLabel::showChunkList("Experiment information stream details");
}
