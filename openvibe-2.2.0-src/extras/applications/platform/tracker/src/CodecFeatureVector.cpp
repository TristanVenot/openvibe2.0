
// @todo this is identical to CodecFeatureMatrix. Refactor?

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include "TypeFeatureVector.h"

#include "Decoder.h"
#include "Encoder.h"

namespace OpenViBETracker
{


template<> bool DecoderAdapter<TypeFeatureVector, OpenViBEToolkit::TFeatureVectorDecoder<BoxAlgorithmProxy> >::getHeaderImpl(TypeFeatureVector::Header& target)
{
	OpenViBE::IMatrix* decoded = m_DecoderImpl.getOutputMatrix();
	OpenViBEToolkit::Tools::Matrix::copy(target.m_header, *decoded);

	return true;
};

template<> bool DecoderAdapter<TypeFeatureVector, OpenViBEToolkit::TFeatureVectorDecoder<BoxAlgorithmProxy> >::getBufferImpl(TypeFeatureVector::Buffer& target)
{
	const OpenViBE::IMatrix* decoded = m_DecoderImpl.getOutputMatrix();
	OpenViBEToolkit::Tools::Matrix::copy(target.m_buffer, *decoded);

	return true;
};

template<> bool EncoderAdapter<TypeFeatureVector, OpenViBEToolkit::TFeatureVectorEncoder<BoxAlgorithmProxy> >::encodeHeaderImpl(const TypeFeatureVector::Header& src)
{
	OpenViBE::IMatrix* buffer = m_EncoderImpl.getInputMatrix();

	OpenViBEToolkit::Tools::Matrix::copy(*buffer, src.m_header);

	return m_EncoderImpl.encodeHeader();

};

template<> bool EncoderAdapter<TypeFeatureVector, OpenViBEToolkit::TFeatureVectorEncoder<BoxAlgorithmProxy> >::encodeBufferImpl(const TypeFeatureVector::Buffer& src)
{
	OpenViBE::IMatrix* buffer = m_EncoderImpl.getInputMatrix();

	OpenViBEToolkit::Tools::Matrix::copy(*buffer, src.m_buffer);

	return m_EncoderImpl.encodeBuffer();
};

};
