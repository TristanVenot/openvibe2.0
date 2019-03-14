//
// OpenViBE Tracker
//
//
// n.b. having this as a separate file so EBML/Demuxer dependencies do not get pulled into StreamBundle
//

#pragma once

#include <openvibe/ov_all.h>

#include "StreamBundle.h"

using namespace OpenViBE::Kernel;

namespace OpenViBETracker
{

// These functions import/export stream bundles from .ov files
OpenViBETracker::StreamBundle* readStreamBundleFromFile(const OpenViBE::Kernel::IKernelContext& ctx, const char *filename, bool memorySaveMode);
bool saveStreamBundleToFile(const OpenViBE::Kernel::IKernelContext& ctx, OpenViBETracker::StreamBundle* track, const char *filename);

};


