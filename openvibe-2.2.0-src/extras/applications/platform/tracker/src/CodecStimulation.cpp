
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include "TypeStimulation.h"

#include "Decoder.h"
#include "Encoder.h"

namespace OpenViBETracker
{


template<> bool DecoderAdapter<TypeStimulation, OpenViBEToolkit::TStimulationDecoder<BoxAlgorithmProxy> >::getHeaderImpl(TypeStimulation::Header& target)
{
	return true;
}

template<> bool DecoderAdapter<TypeStimulation, OpenViBEToolkit::TStimulationDecoder<BoxAlgorithmProxy> >::getBufferImpl(TypeStimulation::Buffer& target)
{
	OpenViBE::IStimulationSet* decoded = m_DecoderImpl.getOutputStimulationSet();

	target.m_buffer.clear();
	for (size_t i = 0; i < decoded->getStimulationCount(); i++)
	{
		target.m_buffer.appendStimulation(decoded->getStimulationIdentifier(i),
			decoded->getStimulationDate(i),
			decoded->getStimulationDuration(i)
			);
	}
	return true;
};

template<> bool EncoderAdapter<TypeStimulation, OpenViBEToolkit::TStimulationEncoder<BoxAlgorithmProxy> >::encodeHeaderImpl(const TypeStimulation::Header& src)
{
	return m_EncoderImpl.encodeHeader();
}

template<> bool EncoderAdapter<TypeStimulation, OpenViBEToolkit::TStimulationEncoder<BoxAlgorithmProxy> >::encodeBufferImpl(const TypeStimulation::Buffer& src)
{
	OpenViBE::IStimulationSet* inputSet = m_EncoderImpl.getInputStimulationSet();

	inputSet->clear();
	for (size_t i = 0; i < src.m_buffer.getStimulationCount(); i++)
	{
		inputSet->appendStimulation(src.m_buffer.getStimulationIdentifier(i),
			src.m_buffer.getStimulationDate(i) + m_EncodeOffset,
			src.m_buffer.getStimulationDuration(i)
			);
	}

	return m_EncoderImpl.encodeBuffer();
}

};
