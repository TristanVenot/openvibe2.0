//
// OpenViBE Tracker
//

#pragma once

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include "StreamRendererMatrix.h"
#include "StreamRendererSignal.h"
#include "StreamRendererStimulation.h"
#include "StreamRendererSpectrum.h"
#include "StreamRendererExperimentInformation.h"
#include "StreamRendererLabel.h"
#include "StreamRendererChannelUnits.h"
#include "StreamRendererChannelLocalization.h"
#include "StreamRendererNothing.h"

#include "Stream.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace std;

namespace OpenViBETracker
{

/**
 * \class StreamRendererFactory 
 * \brief Factory method for getting Stream Renderers of different types
 * \author J. T. Lindgren
 *
 */
class StreamRendererFactory {
public:

	// Factory special case
	static StreamRendererBase* getDummyRenderer(const OpenViBE::Kernel::IKernelContext& ctx)
	{
		return new StreamRendererNothing(ctx);
	}

	// Generic factory
	static StreamRendererBase* getRenderer(const OpenViBE::Kernel::IKernelContext& ctx, StreamPtrConst stream)
	{
		if (!stream)
		{
			ctx.getLogManager() << LogLevel_Warning << "Nullptr stream, using label renderer\n";
		}

		CIdentifier typeIdentifier = (stream ? stream->getTypeIdentifier() : OV_UndefinedIdentifier);

		StreamRendererBase* renderer = nullptr;

		if (typeIdentifier == OV_TypeId_Signal)
		{
			renderer = new StreamRendererSignal(ctx, std::static_pointer_cast< const Stream<TypeSignal> >(stream));
		}
		else if (typeIdentifier == OV_TypeId_Spectrum)
		{
			renderer = new StreamRendererSpectrum(ctx, std::static_pointer_cast< const Stream<TypeSpectrum> >(stream));
		}
		else if (typeIdentifier == OV_TypeId_Stimulations)
		{
			// @todo should be pushed to all other renderers
			renderer = new StreamRendererStimulation(ctx, std::static_pointer_cast< const Stream<TypeStimulation> >(stream));
		}
		else if (typeIdentifier == OV_TypeId_StreamedMatrix)
		{
			renderer = new StreamRendererMatrix(ctx, std::static_pointer_cast< const Stream<TypeMatrix> >(stream));
		}
		else if (typeIdentifier == OV_TypeId_FeatureVector)
		{
			// Since featurevector is just a constrained matrix, we use the matrix renderer
			renderer = new StreamRendererMatrix(ctx, std::static_pointer_cast< const Stream<TypeMatrix> >(stream));
		}
		else if (typeIdentifier == OV_TypeId_ExperimentInformation)
		{
			// Since featurevector is just a constrained matrix, we use the matrix renderer
			renderer = new StreamRendererExperimentInformation(ctx, std::static_pointer_cast<const Stream<TypeExperimentInformation> >(stream));
		}
		else if (typeIdentifier == OV_TypeId_ChannelUnits)
		{
			renderer = new StreamRendererChannelUnits(ctx,
				std::static_pointer_cast< const Stream<TypeChannelUnits> >(stream));
		}
		else if (typeIdentifier == OV_TypeId_ChannelLocalisation)
		{
			renderer = new StreamRendererChannelLocalization(ctx,
				std::static_pointer_cast< const Stream<TypeChannelLocalization> >(stream));
		}
		else
		{
			ctx.getLogManager() << LogLevel_Trace << "Using label renderer for stream of type "
				<< ctx.getTypeManager().getTypeName(typeIdentifier) << "\n";

			renderer = new StreamRendererLabel(ctx);
		}

		return renderer;
	}

	// Prevent constructing
	StreamRendererFactory() = delete;
	StreamRendererFactory(const StreamRendererFactory&) = delete;
	StreamRendererFactory(StreamRendererFactory&&) = delete;

};


};
