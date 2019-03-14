
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include "TypeSignal.h"

#include "Decoder.h"
#include "Encoder.h"

namespace OpenViBETracker
{

template<> bool DecoderAdapter<TypeSignal, OpenViBEToolkit::TSignalDecoder<BoxAlgorithmProxy> >::getHeaderImpl(TypeSignal::Header& target)
{
	target.m_SamplingRate = m_DecoderImpl.getOutputSamplingRate();

	OpenViBE::IMatrix* decoded = m_DecoderImpl.getOutputMatrix();
	OpenViBEToolkit::Tools::Matrix::copy(target.m_header, *decoded);

	return true;
};

template<> bool DecoderAdapter<TypeSignal, OpenViBEToolkit::TSignalDecoder<BoxAlgorithmProxy> >::getBufferImpl(TypeSignal::Buffer& target)
{
	const OpenViBE::IMatrix* decoded = m_DecoderImpl.getOutputMatrix();
	OpenViBEToolkit::Tools::Matrix::copy(target.m_buffer, *decoded);

	return true;
};

template<> bool EncoderAdapter<TypeSignal, OpenViBEToolkit::TSignalEncoder<BoxAlgorithmProxy> >::encodeHeaderImpl(const TypeSignal::Header& src)
{
	m_EncoderImpl.getInputSamplingRate() = src.m_SamplingRate;

	OpenViBE::IMatrix* header = m_EncoderImpl.getInputMatrix();
	OpenViBEToolkit::Tools::Matrix::copy(*header, src.m_header);

	return m_EncoderImpl.encodeHeader();
};

#include <iostream>

template<> bool EncoderAdapter<TypeSignal, OpenViBEToolkit::TSignalEncoder<BoxAlgorithmProxy> >::encodeBufferImpl(const TypeSignal::Buffer& src)
{
	OpenViBE::IMatrix* buffer = m_EncoderImpl.getInputMatrix();
	OpenViBEToolkit::Tools::Matrix::copy(*buffer, src.m_buffer);

	/*
	std::cout << "Copying " <<
	src.m_buffer.getDimensionSize(0) << " x " << src.m_buffer.getDimensionSize(1) << " : " << src.m_buffer.getBufferElementCount() << "\n";
	std::cout << "Res " <<
		buffer->getDimensionSize(0) << " x " << buffer->getDimensionSize(1) << " : " << buffer->getBufferElementCount() << "\n";
		*/

	return m_EncoderImpl.encodeBuffer();
};


};
