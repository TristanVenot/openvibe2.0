
#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include <ebml/IWriterHelper.h>
#include <ebml/IWriter.h>
#include <ebml/TWriterCallbackProxy.h>

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include "StreamBase.h"

namespace OpenViBETracker
{

/**
 * \class Stream
 * \brief A container class representing a stream of OpenViBE 
 * \author J. T. Lindgren
 * \details
 *
 * Stream is basically a specific kind of time-ordered container of timestamped, typed elements.
 *
 * Unlike OpenViBE streams in general, the Stream content in Tracker is stored in memory unencoded, 
 * and hence if its content is written to an .ov file, passed to a box, or Designer, it must be encoded first.
 *
 * Stream must have a 'header' representing the stream parameters, a sequence of 'buffers'
 * containing the data, and an 'end' indicating the end of the stream.
 *
 *         Time ----------------------->
 *            0    
 * "front" [header][chk1][chk2]....[end] "tail"
 *
 * The stream additionally has a 'position' that always points to one of the above elements
 * and starts from front. Stepping the stream moves the stream position +1 chunk forward towards the tail.
 *
 * The stream API can be used in a way that chunks can be pushed 
 * to the tail while the position counter is advancing from the head towards
 * the tail. Although this would allow interleaving pushes
 * and reads, currently the Tracker works in a way that when a track is loaded
 * or recorded, only push() operations are done until the finish. On the
 * other hand, when the track is sent out, only peek() and step() are used to 
 * retrieve the chunks.
 *
 * Currently Tracker Plugins are allowed to access the tracks in a random access fashion,
 * as well as the GUI.
 *   
 *
 */
template<class T> class Stream : public StreamBase {
public:
	Stream(const OpenViBE::Kernel::IKernelContext& ctx) : StreamBase(ctx)
	{ 
		m_End.m_startTime = m_End.m_endTime = (ovtime_t)(-1);
	};

	virtual ~Stream(void) { 
		/* if(m_Header) { delete m_Header; } */ 
		clear(); 
	};

	virtual OpenViBE::CIdentifier getTypeIdentifier(void) const override { return T::getTypeIdentifier(); };

	// Note: There is no corresponding setters, use the non-const versions to modify the header
	virtual const typename T::Header& getHeader(void) const { return m_Header; };
	virtual const typename T::End& getEnd(void) const { return m_End; };
	virtual typename T::Header& getHeader(void) { return m_Header; };
	virtual typename T::End& getEnd(void) { return m_End; };

	virtual bool push(typename T::Buffer* chunk) { m_Chunks.push_back(chunk); return true; };

	// Return the timestamps of the current chunk
	virtual bool peek(ovtime_t& startTime, ovtime_t& endTime) const
	{	
		return peek(m_position, startTime, endTime);
	}

	// Return timestamps of a specific chunk
	virtual bool peek(size_t index, ovtime_t& startTime, ovtime_t& endTime) const
	{
		if (index == (size_t)(-1))
		{
			startTime = endTime = 0;
		}
		else if (index < m_Chunks.size())
		{
			startTime = m_Chunks[index]->m_startTime;
			endTime = m_Chunks[index]->m_endTime;
		}
		else if (index == m_Chunks.size() && m_Chunks.size()>0)
		{
			startTime = m_Chunks[index - 1]->m_endTime;
			endTime = m_Chunks[index - 1]->m_endTime;
		}
		else
		{
			return false;
		}
		return true;
	}
	virtual size_t getChunkCount(void) const override { return m_Chunks.size(); };
	virtual bool getChunk(size_t idx, typename T::Buffer** ptr) const {
		if (idx < m_Chunks.size())
		{
			*ptr = m_Chunks[idx]; return true;
		}
		else
		{
			*ptr = nullptr; return false;
		}
	};

	virtual const typename T::Buffer* getChunk(size_t idx) const {
		if (idx < m_Chunks.size())
		{
			return m_Chunks[idx];
		}
		else
		{
			return nullptr;
		}
	};

	virtual bool clear(void) {
		std::for_each(m_Chunks.begin(), m_Chunks.end(), [](typename T::Buffer* ptr) { delete ptr; });
		m_Chunks.clear(); StreamBase::m_position = (size_t)(-1); 
		return true;
	};

	virtual ovtime_t getDuration(void) const override
	{
		const size_t chunkCount = getChunkCount();
		if (chunkCount == 0)
		{
			return 0;
		}
		else
		{
			return m_Chunks[chunkCount-1]->m_endTime;
		}
	}

	virtual ovtime_t getStartTime(void) const override
	{
		const size_t chunkCount = getChunkCount();
		if (chunkCount == 0)
		{
			return 0;
		}
		else
		{
			return m_Chunks[0]->m_startTime;
		}
	}

	// @fixme efficiency
	virtual bool getOverlapping(void) const override { 
		for(size_t i=1;i<m_Chunks.size(); i++)
		{
			if (m_Chunks[i]->m_startTime < m_Chunks[i-1]->m_endTime)
			{
				return true;
			}
		}
		return false;
	};

	// @fixme efficiency
	virtual bool getNoncontinuous(void) const override { 
		for(size_t i=1;i<m_Chunks.size(); i++)
		{
			if (m_Chunks[i]->m_startTime > m_Chunks[i-1]->m_endTime)
			{
				return true;
			}
		}
		return false;
	};

	// @fixme efficiency
	uint64_t countChunks(ovtime_t startTime, ovtime_t endTime) const
	{
		// Count the chunks @fixme not very efficient
		uint64_t chunkCount = 0;
		for (size_t i = 0; i < m_Chunks.size(); i++)
		{
			if (m_Chunks[i]->m_startTime >= startTime)
			{
				if (m_Chunks[i]->m_endTime <= endTime)
				{
					chunkCount++;
				}
				else
				{
					break;
				}
			}
		}
		return chunkCount;
	}

	// Iterators and operators
	typename std::vector<typename T::Buffer*>::iterator begin() { return m_Chunks.begin(); };
	typename std::vector<typename T::Buffer*>::iterator end() { return m_Chunks.end(); };
	typename T::Buffer& operator[] (int idx)
	{
		return m_Chunks[idx];
	}

	// @fixme
	// bool setHeader(TypeError::Header *) { return true; };
	// bool setBuffer(TypeError::Buffer *) { return true; };

	// @fixme doesn't work yet
	bool copy(const StreamBase& other) override
	{
		if (T::getTypeIdentifier() != other.getTypeIdentifier()) 
		{
			return false;
		}

		const Stream<T>& otherStream = reinterpret_cast< const Stream<T>& >(other);

		m_Chunks.resize(otherStream.getChunkCount(), nullptr);

		// @fixme this can be implemented when CMatrix has a working copy/assignment operators
		/*
		m_Header = otherStream.getHeader();

		for (size_t c = 0; c < otherStream.getChunkCount(); c++)
		{
			m_Chunks[c] = new T::Buffer(otherStream.getChunk(c));
		}

		m_End = otherStream.getEnd();
		*/

		log() << OpenViBE::Kernel::LogLevel_Error << "Unimplemented method\n";

		return false;
	}

protected:

	// Header of a stream
	typename T::Header m_Header;

	// Buffers
 	std::vector<typename T::Buffer*> m_Chunks;

	// End of a stream
	typename T::End m_End;

};

};
