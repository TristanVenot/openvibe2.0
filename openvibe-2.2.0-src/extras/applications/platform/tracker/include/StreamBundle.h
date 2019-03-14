//
// OpenViBE Tracker
//

#pragma once

#include <string>
#include <vector>

#include <memory> // shared_ptr

#include <openvibe/ov_all.h>

#include "StreamBase.h"

#include "Contexted.h"


namespace OpenViBETracker
{

/**
 * \class StreamBundle 
 * \brief StreamBundle is a container of one or more typed streams. It corresponds to a 'track' in Tracker and can represent an .ov file.
 * \details
 *
 * StreamBundle can be queried for streams in time order: getNextStream() call can be used to find out 
 * the stream which has the earliest chunk position pointer (in time). Stepping the stream may make some 
 * other stream to be returned on the next get call.
 *
 * \author J. T. Lindgren
 *
 */
class StreamBundle : protected Contexted {
public:

	StreamBundle(const OpenViBE::Kernel::IKernelContext& ctx) : Contexted(ctx) {}; 

	// Copy everything, allocate new memory for content
	bool deepCopy(const StreamBundle& other);
	// Copy selected subset of streams from "other".
	// Other will retain ownership of any internal pointers.
	// Since the copy gets write access to content of other, "other" is not const.
	bool copyFrom(StreamBundle& other);

	bool initialize(void);
	bool uninitialize(void);

	// Rewind all streams
	bool rewind(void);

	// Returns the stream which has a position with the earliest beginning timestamp
	StreamPtr getNextStream(size_t& index);
	bool getNextStreamIndex(size_t& index) const;

	size_t getNumStreams(void) const { return m_Streams.size(); };
	StreamPtrConst getStream(size_t idx) const;
	StreamPtr getStream(size_t idx);
	std::vector< StreamPtr >& getAllStreams(void) { return m_Streams; };

	// A factory method that creates a stream of a specific type into the slot idx
	bool createStream(size_t idx, const OpenViBE::CIdentifier& typeId);
	bool deleteStream(size_t idx);
	bool setStream(size_t idx, std::shared_ptr< StreamBase >);
	bool swapStreams(size_t idx1, size_t idx2);
	bool moveStream(size_t sourceIdx, size_t targetIdx);

	// Returns the duration of the longest stream in the bundle
	ovtime_t getMaxDuration(void) const;

	// Have all streams ended in the bundle?
	bool isFinished(void) const { size_t dummy; return !getNextStreamIndex(dummy); }

	// The name of the .ov file this bundle corresponds to
	const std::string& getSource(void) const { return m_Source; };
	bool setSource(const std::string& src) { m_Source = src; return true;  };

	bool getDirtyBit(void) const { return m_Dirty; }
	bool setDirtyBit(const bool newState) { m_Dirty = newState; return true; }

protected:

	// As StreamBundles can consist of streams from diverse origins where some of them would
	// like to pass the ownership and some to keep it, instead of writing messy ownership tracking code, 
	// we use shared pointers for streams.
	std::vector< StreamPtr > m_Streams;

	std::string m_Source; // Identifies the .ov file of the bundle on disk

	bool m_Dirty = true;	// True if the stream has not been saved to disk after last modification

};

};
