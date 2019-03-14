//
// OpenViBE Tracker
//

#pragma once

#include <openvibe/ov_all.h>

#include "EncodedChunk.h"
#include "Encoder.h"
#include "Decoder.h"

#include "StreamBase.h"
#include "Stream.h"

#include "CodecsAll.h"

namespace OpenViBETracker
{

/**
 * \class CodecFactory 
 * \brief A factory class for obtaining encoders and decoders for streams.
 * \details In OpenViBE, every stream is passed around as EBML encoded due to an old design choice. 
 * The encoders and decoders are used to pass between the plain and encoded formats of the data.
 * \author J. T. Lindgren
 *
 */
class CodecFactory {
public:

	// Factory for encodes
	static EncoderBase* getEncoder(const OpenViBE::Kernel::IKernelContext& ctx, StreamBase& stream)
	{
		EncoderBase* encoder =nullptr;
		const OpenViBE::CIdentifier typeId = stream.getTypeIdentifier();

		if (typeId == OVTK_TypeId_StreamedMatrix)
		{
			encoder = new Encoder<TypeMatrix>(ctx, reinterpret_cast< Stream<TypeMatrix>& >(stream));
		}
		else if (typeId == OVTK_TypeId_Signal)
		{
			encoder = new Encoder<TypeSignal>(ctx, reinterpret_cast< Stream<TypeSignal>& >(stream));
		}
		else if (typeId == OVTK_TypeId_Stimulations)
		{
			encoder = new Encoder<TypeStimulation>(ctx, reinterpret_cast< Stream<TypeStimulation>& >(stream));
		}
		else if (typeId == OVTK_TypeId_ExperimentInformation)
		{
			encoder = new Encoder<TypeExperimentInformation>(ctx, reinterpret_cast< Stream<TypeExperimentInformation>& >(stream));
		}
		else if (typeId == OV_TypeId_ChannelLocalisation) // bug in toolkit atm with OVTK_ define
		{
			encoder = new Encoder<TypeChannelLocalization>(ctx, reinterpret_cast< Stream<TypeChannelLocalization>& >(stream));
		}
		else if (typeId == OV_TypeId_ChannelUnits) // bug in toolkit atm with OVTK_ define
		{
			encoder = new Encoder<TypeChannelUnits>(ctx, reinterpret_cast< Stream<TypeChannelUnits>& >(stream));
		}
		else if (typeId == OVTK_TypeId_Spectrum)
		{
			encoder = new Encoder<TypeSpectrum>(ctx, reinterpret_cast< Stream<TypeSpectrum>& >(stream));
		}
		else if (typeId == OVTK_TypeId_FeatureVector)
		{
			encoder = new Encoder<TypeFeatureVector>(ctx, reinterpret_cast< Stream<TypeFeatureVector>& >(stream));
		}
		else
		{
			ctx.getLogManager() << OpenViBE::Kernel::LogLevel_Info << "Warning: Unknown Decoder type "
				<< ctx.getTypeManager().getTypeName(typeId)
				<< " " << OpenViBE::CIdentifier(typeId).toString()
				<< "\n";
			encoder = new Encoder<TypeError>(ctx, reinterpret_cast< Stream<TypeError>& >(stream));
		}

		return encoder;
	};

	// Factory for decoders
	static DecoderBase* getDecoder(const OpenViBE::Kernel::IKernelContext& ctx, StreamBase& stream)
	{
		DecoderBase* decoder =nullptr;
		const OpenViBE::CIdentifier typeId = stream.getTypeIdentifier();

		if (typeId == OVTK_TypeId_StreamedMatrix)
		{
			decoder = new Decoder<TypeMatrix>(ctx, reinterpret_cast< Stream<TypeMatrix>& >(stream));
		}
		else if (typeId == OVTK_TypeId_Signal)
		{
			decoder = new Decoder<TypeSignal>(ctx, reinterpret_cast< Stream<TypeSignal>& >(stream));
		}
		else if (typeId == OVTK_TypeId_Stimulations)
		{
			decoder = new Decoder<TypeStimulation>(ctx, reinterpret_cast< Stream<TypeStimulation>& >(stream));
		}
		else if (typeId == OVTK_TypeId_ExperimentInformation)
		{
			decoder = new Decoder<TypeExperimentInformation>(ctx, reinterpret_cast< Stream<TypeExperimentInformation>& >(stream));
		}
		else if (typeId == OV_TypeId_ChannelLocalisation) // bug in toolkit atm with OVTK_ define
		{
			decoder = new Decoder<TypeChannelLocalization>(ctx, reinterpret_cast< Stream<TypeChannelLocalization>& >(stream));
		}
		else if (typeId == OV_TypeId_ChannelUnits) // bug in toolkit atm with OVTK_ define
		{
			decoder = new Decoder<TypeChannelUnits>(ctx, reinterpret_cast< Stream<TypeChannelUnits>& >(stream));
		}
		else if (typeId == OVTK_TypeId_Spectrum)
		{
			decoder = new Decoder<TypeSpectrum>(ctx, reinterpret_cast< Stream<TypeSpectrum>& >(stream));
		}
		else if (typeId == OVTK_TypeId_FeatureVector)
		{
			decoder = new Decoder<TypeFeatureVector>(ctx, reinterpret_cast< Stream<TypeFeatureVector>& >(stream));
		}
		else
		{
			ctx.getLogManager() << OpenViBE::Kernel::LogLevel_Info << "Warning: Unknown Decoder type "
				<< ctx.getTypeManager().getTypeName(typeId)
				<< " " << OpenViBE::CIdentifier(typeId).toString()
				<< "\n";
			decoder = new Decoder<TypeError>(ctx, reinterpret_cast< Stream<TypeError>& >(stream));
		}

		return decoder;
	};

	// Prevent constructing
	CodecFactory() = delete;
	CodecFactory(const CodecFactory&) = delete;
	CodecFactory(CodecFactory&&) = delete;

};

};
