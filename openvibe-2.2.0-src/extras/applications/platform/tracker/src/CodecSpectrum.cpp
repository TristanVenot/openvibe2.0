
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include "TypeSpectrum.h"

#include "Decoder.h"
#include "Encoder.h"

namespace OpenViBETracker
{


template<> bool DecoderAdapter<TypeSpectrum, OpenViBEToolkit::TSpectrumDecoder<BoxAlgorithmProxy> >::getHeaderImpl(TypeSpectrum::Header& target)
{
	OpenViBE::IMatrix* decoded = m_DecoderImpl.getOutputMatrix();
	OpenViBEToolkit::Tools::Matrix::copy(target.m_header, *decoded);

	target.m_SamplingRate = m_DecoderImpl.getOutputSamplingRate();

	OpenViBE::IMatrix* abscissas = m_DecoderImpl.getOutputFrequencyAbscissa();
	OpenViBEToolkit::Tools::Matrix::copy(target.m_Abscissas, *abscissas);

	return true;
};

template<> bool DecoderAdapter<TypeSpectrum, OpenViBEToolkit::TSpectrumDecoder<BoxAlgorithmProxy> >::getBufferImpl(TypeSpectrum::Buffer& target)
{
	const OpenViBE::IMatrix* decoded = m_DecoderImpl.getOutputMatrix();

	OpenViBEToolkit::Tools::Matrix::copy(target.m_buffer, *decoded);

	return true;
};

template<> bool EncoderAdapter<TypeSpectrum, OpenViBEToolkit::TSpectrumEncoder<BoxAlgorithmProxy> >::encodeHeaderImpl(const TypeSpectrum::Header& src)
{
	m_EncoderImpl.getInputSamplingRate() = src.m_SamplingRate;

	OpenViBE::IMatrix* header = m_EncoderImpl.getInputMatrix();
	OpenViBEToolkit::Tools::Matrix::copy(*header, src.m_header);

	OpenViBE::IMatrix* abscissas = m_EncoderImpl.getInputFrequencyAbscissa();
	OpenViBEToolkit::Tools::Matrix::copy(*abscissas, src.m_Abscissas);

	return m_EncoderImpl.encodeHeader();
};

template<> bool EncoderAdapter<TypeSpectrum, OpenViBEToolkit::TSpectrumEncoder<BoxAlgorithmProxy> >::encodeBufferImpl(const TypeSpectrum::Buffer& src)
{
	OpenViBE::IMatrix* buffer = m_EncoderImpl.getInputMatrix();
	OpenViBEToolkit::Tools::Matrix::copy(*buffer, src.m_buffer);

	return m_EncoderImpl.encodeBuffer();
};


};
