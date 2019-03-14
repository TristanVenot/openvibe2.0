
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include "TypeMatrix.h"

#include "Decoder.h"
#include "Encoder.h"

namespace OpenViBETracker
{

template<> bool DecoderAdapter<TypeMatrix, OpenViBEToolkit::TStreamedMatrixDecoder<BoxAlgorithmProxy> >::getHeaderImpl(TypeMatrix::Header& target)
{
	OpenViBE::IMatrix* decoded = m_DecoderImpl.getOutputMatrix();
	OpenViBEToolkit::Tools::Matrix::copy(target.m_header, *decoded);

	return true;
}


template<> bool DecoderAdapter<TypeMatrix, OpenViBEToolkit::TStreamedMatrixDecoder<BoxAlgorithmProxy> >::getBufferImpl(TypeMatrix::Buffer& target)
{
	const OpenViBE::IMatrix* decoded = m_DecoderImpl.getOutputMatrix();
	OpenViBEToolkit::Tools::Matrix::copy(target.m_buffer, *decoded);

	return true;
}

template<> bool EncoderAdapter<TypeMatrix, OpenViBEToolkit::TStreamedMatrixEncoder<BoxAlgorithmProxy> >::encodeHeaderImpl(const TypeMatrix::Header& src)
{
	OpenViBE::IMatrix* header = m_EncoderImpl.getInputMatrix();
	OpenViBEToolkit::Tools::Matrix::copy(*header, src.m_header);

	return m_EncoderImpl.encodeHeader();
};

template<> bool EncoderAdapter<TypeMatrix, OpenViBEToolkit::TStreamedMatrixEncoder<BoxAlgorithmProxy> >::encodeBufferImpl(const TypeMatrix::Buffer& src)
{
	OpenViBE::IMatrix* buffer = m_EncoderImpl.getInputMatrix();
	OpenViBEToolkit::Tools::Matrix::copy(*buffer, src.m_buffer);

	return m_EncoderImpl.encodeBuffer();
};

};
