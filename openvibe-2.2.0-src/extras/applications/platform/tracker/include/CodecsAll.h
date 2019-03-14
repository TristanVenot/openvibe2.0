
#pragma once

#include "CodecImpl.h"

#include "TypeChannelLocalization.h"
#include "TypeChannelUnits.h"
#include "TypeExperimentInformation.h"
#include "TypeFeatureVector.h"
#include "TypeMatrix.h"
#include "TypeSignal.h"
#include "TypeSpectrum.h"
#include "TypeStimulation.h"

namespace OpenViBETracker {

CODEC_IMPL_VIA_TOOLKIT(TypeChannelLocalization, TChannelLocalisationEncoder, TChannelLocalisationDecoder)
CODEC_IMPL_VIA_TOOLKIT(TypeChannelUnits, TChannelUnitsEncoder, TChannelUnitsDecoder)
CODEC_IMPL_VIA_TOOLKIT(TypeExperimentInformation, TExperimentInformationEncoder, TExperimentInformationDecoder)
CODEC_IMPL_VIA_TOOLKIT(TypeFeatureVector, TFeatureVectorEncoder, TFeatureVectorDecoder)
CODEC_IMPL_VIA_TOOLKIT(TypeMatrix, TStreamedMatrixEncoder, TStreamedMatrixDecoder)
CODEC_IMPL_VIA_TOOLKIT(TypeSignal, TSignalEncoder, TSignalDecoder)
CODEC_IMPL_VIA_TOOLKIT(TypeSpectrum, TSpectrumEncoder, TSpectrumDecoder)
CODEC_IMPL_VIA_TOOLKIT(TypeStimulation, TStimulationEncoder, TStimulationDecoder)

};
