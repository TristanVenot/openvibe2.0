
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include "TypeChannelLocalization.h"

#include "Decoder.h"
#include "Encoder.h"

namespace OpenViBETracker
{

template<> bool DecoderAdapter<TypeChannelLocalization, OpenViBEToolkit::TChannelLocalisationDecoder<BoxAlgorithmProxy> >::getHeaderImpl(TypeChannelLocalization::Header& target)
{
	target.m_Dynamic = m_DecoderImpl.getOutputDynamic();

	OpenViBE::IMatrix* decoded = m_DecoderImpl.getOutputMatrix();
	OpenViBEToolkit::Tools::Matrix::copy(target.m_header, *decoded);

	return true;
};

template<> bool DecoderAdapter<TypeChannelLocalization, OpenViBEToolkit::TChannelLocalisationDecoder<BoxAlgorithmProxy> >::getBufferImpl(TypeChannelLocalization::Buffer& target)
{
	OpenViBE::IMatrix* decoded = m_DecoderImpl.getOutputMatrix();
	OpenViBEToolkit::Tools::Matrix::copy(target.m_buffer, *decoded);

	return true;
};

template<> bool EncoderAdapter<TypeChannelLocalization, OpenViBEToolkit::TChannelLocalisationEncoder<BoxAlgorithmProxy> >::encodeHeaderImpl(const TypeChannelLocalization::Header& src)
{
	m_EncoderImpl.getInputDynamic() = src.m_Dynamic;

	OpenViBE::IMatrix* header = m_EncoderImpl.getInputMatrix();
	OpenViBEToolkit::Tools::Matrix::copy(*header, src.m_header);

	return m_EncoderImpl.encodeHeader();
};

template<> bool EncoderAdapter<TypeChannelLocalization, OpenViBEToolkit::TChannelLocalisationEncoder<BoxAlgorithmProxy> >::encodeBufferImpl(const TypeChannelLocalization::Buffer& src)
{
	OpenViBE::IMatrix* buffer = m_EncoderImpl.getInputMatrix();
	OpenViBEToolkit::Tools::Matrix::copy(*buffer, src.m_buffer);

	return m_EncoderImpl.encodeBuffer();
}

};
