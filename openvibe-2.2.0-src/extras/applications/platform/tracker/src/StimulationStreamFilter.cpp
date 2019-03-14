//
// OpenViBE Tracker
//
// @author J.T. Lindgren
// 

#include "Stream.h"
#include "StimulationStreamFilter.h"

#include "TypeStimulation.h"

using namespace OpenViBE;
using namespace OpenViBETracker;

namespace OpenViBETracker 
{

// @note At some point I thought to make a derived class of stimulation stream that simply wouldn't pass through
// the filtered stimulations on calling the getChunk(), but this resulted in some issues. Since modifying
// the original stream would not have been appropriate, I'd have had to choose one of the following, 
// 1) make getChunk() in general copy all data instead of returning pointers 2) drop const qualifier 
// from the getChunk() to keep track of allocated memory internally in the derived class or 3) make getChunk() 
// return a smart pointer. I wasn't very happy about these options, so instead we just make a modded copy of the stream.
std::shared_ptr< StreamBase > filterStimulationStreamEndPoints(std::shared_ptr< const StreamBase > source, const OpenViBE::Kernel::IKernelContext& ctx) 
{
	const std::vector< uint64_t > stims = {OVTK_StimulationId_ExperimentStop , OVTK_StimulationId_EndOfFile , OVTK_GDF_End_Of_Session};

	auto result = filterStimulationStream(source, ctx, stims);
	return result;
}

std::shared_ptr< StreamBase > filterStimulationStream(std::shared_ptr< const StreamBase > source, const OpenViBE::Kernel::IKernelContext& ctx, const std::vector<uint64_t>& stimsToFilter)
{
	if(source->getTypeIdentifier() != OV_TypeId_Stimulations)
	{
		return nullptr;
	}

	auto typedSrc = std::static_pointer_cast< const Stream<TypeStimulation> >(source);
	auto target = std::make_shared< Stream<TypeStimulation> >(ctx);

	target->clear();

	target->getHeader().m_startTime = typedSrc->getHeader().m_startTime;
	target->getHeader().m_endTime = typedSrc->getHeader().m_endTime;

	for(size_t chk = 0 ; chk < typedSrc->getChunkCount(); chk++)
	{
		const auto chunk = typedSrc->getChunk(chk);
		auto newChunk = new TypeStimulation::Buffer;

		for(size_t i=0;i<chunk->m_buffer.getStimulationCount();i++)
		{
			const uint64_t id =  chunk->m_buffer.getStimulationIdentifier(i);
			if(std::none_of(stimsToFilter.begin(), stimsToFilter.end(), [id](uint64_t val) { return val==id; }))
			{
				const uint64_t timestamp = chunk->m_buffer.getStimulationDate(i);
				const uint64_t duration = chunk->m_buffer.getStimulationDuration(i);
				newChunk->m_buffer.appendStimulation(id, timestamp, duration);
			}
		}

		newChunk->m_startTime = chunk->m_startTime;
		newChunk->m_endTime = chunk->m_endTime;

		target->push(newChunk);
	}

	target->getEnd().m_startTime = typedSrc->getEnd().m_startTime;
	target->getEnd().m_endTime = typedSrc->getEnd().m_endTime;

	return target;
}

};
