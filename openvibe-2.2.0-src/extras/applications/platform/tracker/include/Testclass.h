
#pragma once

#include <openvibe/ov_all.h>

namespace OpenViBETracker
{

/**
 * \class TestClass 
 * \brief Class to try out stuff in the Tracker
 * \author J. T. Lindgren
 *
 */
class TestClass {
public:

	TestClass(OpenViBE::Kernel::IKernelContext& ctx);

	OpenViBE::Kernel::IKernelContext& m_ctx;

};

};
