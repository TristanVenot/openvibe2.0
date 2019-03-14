
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include "TypeChannelUnits.h"

#include "Decoder.h"
#include "Encoder.h"

namespace OpenViBETracker
{

template<> bool DecoderAdapter<TypeChannelUnits, OpenViBEToolkit::TChannelUnitsDecoder<BoxAlgorithmProxy> >::getHeaderImpl(TypeChannelUnits::Header& target)
{
	target.m_Dynamic = m_DecoderImpl.getOutputDynamic();

	OpenViBE::IMatrix* decoded = m_DecoderImpl.getOutputMatrix();
	OpenViBEToolkit::Tools::Matrix::copy(target.m_header, *decoded);

	return true;
};

template<> bool DecoderAdapter<TypeChannelUnits, OpenViBEToolkit::TChannelUnitsDecoder<BoxAlgorithmProxy> >::getBufferImpl(TypeChannelUnits::Buffer& target)
{
	OpenViBE::IMatrix* decoded = m_DecoderImpl.getOutputMatrix();
	OpenViBEToolkit::Tools::Matrix::copy(target.m_buffer, *decoded);

	return true;

}

template<> bool EncoderAdapter<TypeChannelUnits, OpenViBEToolkit::TChannelUnitsEncoder<BoxAlgorithmProxy> >::encodeHeaderImpl(const TypeChannelUnits::Header& src)
{
	m_EncoderImpl.getInputDynamic() = src.m_Dynamic;

	OpenViBE::IMatrix* header = m_EncoderImpl.getInputMatrix();
	OpenViBEToolkit::Tools::Matrix::copy(*header, src.m_header);

	return m_EncoderImpl.encodeHeader();
};

template<> bool EncoderAdapter<TypeChannelUnits, OpenViBEToolkit::TChannelUnitsEncoder<BoxAlgorithmProxy> >::encodeBufferImpl(const TypeChannelUnits::Buffer& src)
{
	OpenViBE::IMatrix* buffer = m_EncoderImpl.getInputMatrix();
	OpenViBEToolkit::Tools::Matrix::copy(*buffer, src.m_buffer);

	return m_EncoderImpl.encodeBuffer();
};

};

