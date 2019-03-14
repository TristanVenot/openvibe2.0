//
// OpenViBE Tracker
//
// @author J.T. Lindgren
// 

#pragma once

#include <memory>

#include "StreamBase.h"

using namespace OpenViBE;
using namespace OpenViBETracker;

namespace OpenViBETracker
{

// Returns a copy of a stream with stimulations suggesting end of stream dropped
std::shared_ptr< StreamBase >  filterStimulationStreamEndPoints(std::shared_ptr< const StreamBase > source, const OpenViBE::Kernel::IKernelContext& ctx);

// Returns a copy of a stream with given stimulations dropped
std::shared_ptr< StreamBase > filterStimulationStream(std::shared_ptr< const StreamBase > source, const OpenViBE::Kernel::IKernelContext& ctx, const std::vector<uint64_t>& stimsToFilter);

};

