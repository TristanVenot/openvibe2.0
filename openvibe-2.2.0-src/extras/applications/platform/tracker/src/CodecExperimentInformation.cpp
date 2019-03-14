
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include "TypeExperimentInformation.h"

#include "Decoder.h"
#include "Encoder.h"

namespace OpenViBETracker
{

template<> bool DecoderAdapter<TypeExperimentInformation, OpenViBEToolkit::TExperimentInformationDecoder<BoxAlgorithmProxy> >::getHeaderImpl(TypeExperimentInformation::Header& target)
{
	target.m_Experiment_Identifier = m_DecoderImpl.getOutputExperimentIdentifier();
	target.m_Experiment_Date = (*m_DecoderImpl.getOutputExperimentDate()).toASCIIString();

	target.m_Subject_Identifier = m_DecoderImpl.getOutputSubjectIdentifier();
	target.m_Subject_Name = (*m_DecoderImpl.getOutputSubjectName()).toASCIIString();
	target.m_Subject_Age = m_DecoderImpl.getOutputSubjectAge();
	target.m_Subject_Gender = m_DecoderImpl.getOutputSubjectGender();

	target.m_Context_LaboratoryIdentifier = m_DecoderImpl.getOutputLaboratoryIdentifier();
	target.m_Context_LaboratoryName = (*m_DecoderImpl.getOutputLaboratoryName()).toASCIIString();
	target.m_Context_TechnicianIdentifier = m_DecoderImpl.getOutputTechnicianIdentifier();
	target.m_Context_TechnicianName = (*m_DecoderImpl.getOutputTechnicianName()).toASCIIString();

	return true;
};

template<> bool DecoderAdapter<TypeExperimentInformation, OpenViBEToolkit::TExperimentInformationDecoder<BoxAlgorithmProxy> >::getBufferImpl(TypeExperimentInformation::Buffer& target)
{
	// Should be no buffer in the experiment stream
	return true;
};

template<> bool EncoderAdapter<TypeExperimentInformation, OpenViBEToolkit::TExperimentInformationEncoder<BoxAlgorithmProxy> >::encodeHeaderImpl(const TypeExperimentInformation::Header& src)
{
	// @fixme the new() calls may imply memory leaks, a bit odd the codec takes pointers
	m_EncoderImpl.getInputExperimentIdentifier() = src.m_Experiment_Identifier;
	m_EncoderImpl.getInputExperimentDate() = new OpenViBE::CString(src.m_Experiment_Date.c_str());

	m_EncoderImpl.getInputSubjectIdentifier() = src.m_Subject_Identifier;
	m_EncoderImpl.getInputSubjectName() = new OpenViBE::CString(src.m_Subject_Name.c_str());
	m_EncoderImpl.getInputSubjectAge() = src.m_Subject_Age;
	m_EncoderImpl.getInputSubjectGender() = src.m_Subject_Gender;

	m_EncoderImpl.getInputLaboratoryIdentifier() = src.m_Context_LaboratoryIdentifier;
	m_EncoderImpl.getInputLaboratoryName() = new OpenViBE::CString(src.m_Context_LaboratoryName.c_str());
	m_EncoderImpl.getInputTechnicianIdentifier() = src.m_Context_TechnicianIdentifier;
	m_EncoderImpl.getInputTechnicianName() = new OpenViBE::CString(src.m_Context_TechnicianName.c_str());

	return m_EncoderImpl.encodeHeader();
};


template<> bool EncoderAdapter<TypeExperimentInformation, OpenViBEToolkit::TExperimentInformationEncoder<BoxAlgorithmProxy> >::encodeBufferImpl(const TypeExperimentInformation::Buffer& src)
{
	// Should be no buffer in the experiment stream
	return m_EncoderImpl.encodeBuffer();
}

};
