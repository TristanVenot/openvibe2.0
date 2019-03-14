//
// OpenViBE Tracker
//
// @author J.T. Lindgren
// 

#include <iostream>
#include <algorithm>

#include "StreamBundle.h"

#include "Stream.h"

#include "StreamFactory.h"

using namespace OpenViBE::Kernel;
using namespace OpenViBETracker;

bool StreamBundle::deepCopy(const StreamBundle& other)
{
	bool retVal = true;

	// Clear streams
	initialize();

	// Copy each stream
	for (size_t i = 0; i < other.getNumStreams(); i++)
	{
		retVal &= createStream(i, other.getStream(i)->getTypeIdentifier());
		retVal &= getStream(i)->copy(*other.getStream(i));
	}

	m_Source = other.getSource();
	setDirtyBit(other.getDirtyBit());

	return retVal;
}

bool StreamBundle::copyFrom(StreamBundle& other)
{
	initialize();

	for (auto str : other.getAllStreams())
	{
		if (str->getSelected())
		{
			this->setStream(m_Streams.size(), str);
		}
	}

	setSource(other.getSource());
	setDirtyBit(other.getDirtyBit());

	// n.b. this will affect the streams we copied
	rewind();

	return true;
}

bool StreamBundle::initialize(void) 
{ 
	//reset first
	uninitialize();

	m_Dirty = true;

//	log() << LogLevel_Debug << "Streams initialized ok\n";

	return true;
};

bool StreamBundle::uninitialize(void)
{
	// Since m_Streams are shared pointers, no need to delete them

	m_Streams.clear();

	return true;
}

bool StreamBundle::rewind(void)
{
	bool returnValue = true;

	std::for_each(m_Streams.begin(), m_Streams.end(), [&returnValue]( StreamPtr entry) { if(entry) { returnValue &= entry->reset(); } } );

	return returnValue;
}

StreamPtrConst StreamBundle::getStream(size_t idx) const
{
	if(idx<m_Streams.size())
	{
		return m_Streams[idx];
	}
	else
	{
		return nullptr;
	}
}

// meh duplication
StreamPtr StreamBundle::getStream(size_t idx) 
{
	if(idx<m_Streams.size())
	{
		return m_Streams[idx];
	}
	else
	{
		return nullptr;
	}
}

bool StreamBundle::createStream(size_t idx, const OpenViBE::CIdentifier& typeId)
{
	if (idx >= m_Streams.size())
	{
		m_Streams.resize(idx + 1, nullptr);
	}


	if (m_Streams[idx] == nullptr)
	{
		StreamPtr stream = StreamFactory::getStream(m_KernelContext, typeId);

		if (!stream)
		{
			return false;
		}

		m_Streams[idx] = stream;

		setDirtyBit(true);

		return true;
	}

	log() << LogLevel_Error << "Error: Slot " << idx << " is already used\n";

	return false;
}

bool StreamBundle::deleteStream(size_t idx)
{
	if (idx > m_Streams.size())
	{
		log() << LogLevel_Error << "Error: Stream index exceeds array size\n";
		return false;
	}

	// m_Streams is shared ptrs, no need to delete

	m_Streams.erase(m_Streams.begin()+idx);
	
	setDirtyBit(true);

	return true;
}

bool StreamBundle::getNextStreamIndex(size_t& earliestIndex) const
{
	if(m_Streams.size()==0)
	{
		return false;
	}

	// Find the stream with the earliest chunk, return the stream
	ovtime_t earliestTime = std::numeric_limits<ovtime_t>::max();
	bool foundSomething = false;

	for(size_t i=0;i<m_Streams.size();i++)
	{
		StreamPtr ptr = m_Streams[i];

		ovtime_t startTime, endTime;
		if(ptr && ptr->peek(startTime,endTime) && startTime < earliestTime)
		{
			earliestTime = startTime;
			earliestIndex = (int)i;
			foundSomething = true;
		}
		/*
		if(ptr && ptr->isEof())
		{
			ovtime_t lastTime = ptr->getLastTime();
			earliestTime = std::min<ovtime_t>(earliestTime, lastTime);
			foundSomething = true;
		}
		*/
	}

	if(!foundSomething)
	{
//		log() << LogLevel_Info << "All streams exhausted\n";
		return false;
	}
	
	return true;
}

StreamPtr StreamBundle::getNextStream(size_t& index)
{
	index = -1;
	if(getNextStreamIndex(index))
	{
		return m_Streams[index];
	}
	else
	{
		return nullptr;
	}
}

ovtime_t StreamBundle::getMaxDuration(void) const 
{
	ovtime_t maxDuration = 0;
	for (size_t i = 0; i < m_Streams.size(); i++)
	{
		if (m_Streams[i])
		{
			ovtime_t streamDuration = m_Streams[i]->getDuration();
			maxDuration = std::max<ovtime_t>(maxDuration, streamDuration);
		}
	}
	return maxDuration;
}


bool StreamBundle::setStream(size_t idx, std::shared_ptr< StreamBase > ptr)
{
	if (idx >= m_Streams.size()) 
	{ 
		m_Streams.resize(idx + 1, nullptr); 
	}; 
	m_Streams[idx] = ptr; 
	setDirtyBit(true);  
	return true; 
}

bool StreamBundle::swapStreams(size_t idx1, size_t idx2) 
{
	if (idx1 >= m_Streams.size() || idx2 >= m_Streams.size())
	{
		return false;
	}

	auto it1 = m_Streams.begin() + idx1;
	auto it2 = m_Streams.begin() + idx2;

	std::iter_swap(it1, it2);

	setDirtyBit(true);

	return true;
}

bool StreamBundle::moveStream(size_t sourceIdx, size_t targetIdx)
{
	if (sourceIdx >= getNumStreams() || targetIdx >= getNumStreams())
	{
		return false;
	}
	if (sourceIdx == targetIdx)
	{
		return true;
	}

	auto oldPtr = m_Streams[sourceIdx];
	m_Streams.erase(m_Streams.begin() + sourceIdx);
	m_Streams.insert(m_Streams.begin() + targetIdx, oldPtr);

	setDirtyBit(true);

	return true;
}

