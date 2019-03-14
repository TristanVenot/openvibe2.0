
#pragma once

#include <openvibe/ov_all.h>

#include "BoxAdapter.h"
#include "Contexted.h"

namespace OpenViBETracker {

class BoxPlugin
{
public:
	OpenViBE::CIdentifier algorithm;
	OpenViBE::CIdentifier descriptor;
};

/**
 * \class BoxPlugins 
 * \brief Box Plugins is a factory-like class that keeps a list of box plugins that can be 'applied' to streams in Tracker
 * \details Boxes can be registered in the constructor
 * \author J. T. Lindgren
 *
 */
class BoxPlugins : protected Contexted
{
public:

	BoxPlugins(const OpenViBE::Kernel::IKernelContext& ctx);
	~BoxPlugins(void);

	// Get a list of available box plugins
	const std::vector<BoxAdapterStream*>& getBoxPlugins(void) const { return m_BoxPlugins; }

protected:

	bool create(const OpenViBE::CIdentifier& streamType, const OpenViBE::CIdentifier& alg, const OpenViBE::CIdentifier& desc);

	std::vector<BoxAdapterStream*> m_BoxPlugins;

};

};

