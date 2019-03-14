//
// OpenViBE Tracker
//

#pragma once

#include "StreamRendererBase.h"

namespace OpenViBETracker
{

/**
 * \class StreamRendererLabel 
 * \brief Renderer for any stream that is simply visualized as a labeled widget
 * \author J. T. Lindgren
 *
 * @ fixme this class inherits a lot of junk from the base class which is not initialized properly and
 * hence cannot be used. but if just the interface below is used, its alright. Can't make base class
 * private as we store the renderer as a pointer to the base.
 *
 */
class StreamRendererLabel : public StreamRendererBase {
public:
	StreamRendererLabel(const OpenViBE::Kernel::IKernelContext& ctx) : StreamRendererBase(ctx) { };

	virtual bool initialize(void) override;
	virtual bool uninitialize(void) override { return true; };
	virtual bool setTitle(const char *title) override;

	virtual bool spool(ovtime_t startTime, ovtime_t endTime) override { return true; };

	virtual bool setRulerVisibility(bool isVisible) override { return true; };

	virtual bool showChunkList(const char* title);

protected:

	virtual bool reset(ovtime_t startTime, ovtime_t endTime) override { return true; };
	virtual bool realize(void) override { return true; };

	GtkWidget *m_pLabel;
};

};
