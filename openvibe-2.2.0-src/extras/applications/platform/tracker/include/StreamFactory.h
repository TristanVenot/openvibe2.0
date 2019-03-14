//
// OpenViBE Tracker
//


#pragma once

#include "CodecsAll.h"

#include "Stream.h"

namespace OpenViBETracker
{

/**
 * \class StreamFactory 
 * \brief A factory method returning an empty Stream object of the requested type
 * \author J. T. Lindgren
 *
 */
class StreamFactory {
public:
	// Factory
	static StreamPtr getStream(const OpenViBE::Kernel::IKernelContext& ctx, const OpenViBE::CIdentifier& typeId)
	{
		std::shared_ptr< StreamBase> stream = nullptr;

		// @todo others
		if (typeId == OVTK_TypeId_StreamedMatrix)
		{
			stream = std::make_shared< Stream<TypeMatrix> >(ctx) ;
		}
		else if (typeId == OVTK_TypeId_Signal)
		{
			stream = std::make_shared< Stream<TypeSignal> >(ctx) ;
		}
		else if (typeId == OVTK_TypeId_Stimulations)
		{
			stream = std::make_shared< Stream<TypeStimulation> >(ctx);
		}
		else if (typeId == OVTK_TypeId_ExperimentInformation)
		{
			stream = std::make_shared< Stream<TypeExperimentInformation> >(ctx);
		}
		else if (typeId == OV_TypeId_ChannelLocalisation) // bug in toolkit atm with OVTK_ define
		{
			stream = std::make_shared< Stream<TypeChannelLocalization> >(ctx);
		}
		else if (typeId == OV_TypeId_ChannelUnits) // bug in toolkit atm with OVTK_ define
		{
			stream = std::make_shared< Stream<TypeChannelUnits> >(ctx);
		}
		else if (typeId == OVTK_TypeId_Spectrum)
		{
			stream = std::make_shared< Stream<TypeSpectrum> >(ctx);
		}
		else if (typeId == OVTK_TypeId_FeatureVector)
		{
			stream = std::make_shared< Stream<TypeFeatureVector> >(ctx);
		}
		else
		{
			ctx.getLogManager() << OpenViBE::Kernel::LogLevel_Info << "Warning: Unknown stream type "
				<< ctx.getTypeManager().getTypeName(typeId)
				<< " " << OpenViBE::CIdentifier(typeId).toString()
				<< "\n";
			stream = std::make_shared< Stream<TypeError> >(ctx);
		}

		return stream;
	};

	// Prevent constructing
	StreamFactory() = delete;
	StreamFactory(const StreamFactory&) = delete;
	StreamFactory(StreamFactory&&) = delete;

};

};
