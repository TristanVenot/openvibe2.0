
#pragma once

#include <string>
#include <vector>
#include <memory> // std::shared_ptr

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include "OvTime.h"
#include "Contexted.h"

// #include "ovkTAttributable.h"
// class StreamBase : public OpenViBE::Kernel::TAttributable<Contexted> {

namespace OpenViBETracker
{



/**
 * \class StreamBase 
 * \brief Abstract, non-typed base class for Streams. 
 * \details For details, please see the type-specific derived class Stream<T>.
 * \author J. T. Lindgren
 *
 */
class StreamBase : protected Contexted {
public:
	StreamBase(const OpenViBE::Kernel::IKernelContext& ctx) : Contexted(ctx), m_position((size_t)(-1)) { };

	virtual ~StreamBase(void) { } ; 

	virtual OpenViBE::CIdentifier getTypeIdentifier(void) const = 0;

	virtual bool peek(ovtime_t& startTime, ovtime_t& endTime) const = 0;
	virtual bool peek(size_t index, ovtime_t& startTime, ovtime_t& endTime) const = 0;

	virtual bool step(void) { if(m_position <= getChunkCount() || m_position==(size_t)(-1)) { m_position++; return true; } else { return false; } } ;
	virtual bool reset(void) { m_position = (size_t)(-1); return true; };

	virtual bool clear(void) = 0;

	virtual size_t getChunkCount(void) const = 0;

	virtual ovtime_t getDuration(void) const = 0;
	virtual ovtime_t getStartTime(void) const = 0;

	// Current play position
	size_t getPosition(void) const { return m_position; }
	bool setPosition(size_t position) { m_position = position; return true; };

	// Is the stream currently selected? (nb. this is not in the .ov file)
	bool getSelected(void) const { return m_bSelected; };
	bool setSelected(bool newState) { m_bSelected = newState; return true; };

	// Stream characteristics
	virtual bool getOverlapping(void) const = 0;
	virtual bool getNoncontinuous(void) const = 0;

	virtual bool copy(const StreamBase& other) { return false; };

protected:

	size_t m_position = (size_t)(-1);  // -1 == beginning of the stream, header

	// Is the stream currently selected?
	bool m_bSelected = true;
};

typedef std::shared_ptr<StreamBase> StreamPtr;
typedef std::shared_ptr<const StreamBase> StreamPtrConst;

};
