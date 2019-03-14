
#pragma once

#include "BoxAdapter.h"

static const OpenViBE::CIdentifier streamWriterID = OpenViBE::CIdentifier(0x09C92218, 0x7C1216F8);

namespace OpenViBETracker
{

/**
 * \class BoxAdapterGenericStreamWriter 
 * \brief A specific wrapper for Generic Stream Writer allowing to write tracks as .ov files from the Tracker
 * \details
 * This class can be considered as the counterpart of the Demuxer class, except here we simply wrap the Generic Stream Writer
 * code instead of developing a new class. 
 * \author J. T. Lindgren
 *
 */
class BoxAdapterGenericStreamWriter : public BoxAdapterBundle {

public:

	BoxAdapterGenericStreamWriter(const OpenViBE::Kernel::IKernelContext& ctx, StreamBundle& source, const std::string& filename)
		: BoxAdapterBundle(ctx, streamWriterID), m_Filename(filename)
	{
		setSource(&source);
	}

	virtual bool initialize(void) override
	{

		// @fixme these should actually come from the box description
		OpenViBE::Kernel::IBox* staticBoxContext = const_cast<OpenViBE::Kernel::IBox*>(m_BoxAlgorithmContext.getStaticBoxContext());
		staticBoxContext->addSetting("Filename", 0, OpenViBE::CString(m_Filename.c_str()));
		staticBoxContext->addSetting("Compress", 0, "false");

		BoxAdapterBundle::initialize();
	
		return true;
	}

protected:


	std::string m_Filename;
};

};

