//
// OpenViBE Tracker
//
// @author J.T. Lindgren
// 

// n.b. having this as a separate file so EBML/Demuxer dependencies do not get pulled into StreamBundle

#include <iostream>
#include <algorithm>

#include "StreamBundleImportExport.h"

#include "EBMLSourceFile.h"
#include "Demuxer.h"
#include "BoxAdapterGenericStreamWriter.h"

using namespace OpenViBE::Kernel;
using namespace OpenViBETracker;

StreamBundle* OpenViBETracker::readStreamBundleFromFile(const OpenViBE::Kernel::IKernelContext& ctx, const char *filename, bool memorySaveMode) 
{
	StreamBundle* newTrack = new StreamBundle(ctx);

	EBMLSourceFile origin(ctx);
	if (!origin.initialize(filename))
	{
		ctx.getLogManager() << LogLevel_Error << "Unable to read from file " << filename << "\n";
		delete newTrack;
		return nullptr;
	}

	// Construct a track from the origin stream
	Demuxer demux(ctx, origin, *newTrack);

	if (memorySaveMode)
	{
		// Try to get the stream types but don't pull the buffers
		bool go = true;
		while (go)
		{
			if (!demux.step())
			{
				ctx.getLogManager() << LogLevel_Trace << "Demuxer EOF\n";
				go = false;
			}

			// n.b. here we use a heuristic that if at least a single chunk has been loaded,
			// then all the headers should have been demuxed before that. There is no guarantee
			// that this would always be the case, but it is very likely if the stream originates from
			// an .ov file created by OpenViBE
			for (size_t j = 0; j < newTrack->getNumStreams(); j++)
			{
				if (newTrack->getStream(j)->getDuration()>0)
				{
					go = false;
					break;
				}
			}
		}

		origin.uninitialize();

		// Release memory
		for (size_t i = 0; i < newTrack->getNumStreams(); i++)
		{
			newTrack->getStream(i)->clear();
		}
	}
	else
	{
		// Just read the whole file to memory
		while (1)
		{
			if (!demux.step())
			{
				ctx.getLogManager() << LogLevel_Trace << "Demuxer EOF\n";
				break;
			}
		}
	}

	origin.uninitialize();

	newTrack->setDirtyBit(false);

	return newTrack;
}

bool OpenViBETracker::saveStreamBundleToFile(const OpenViBE::Kernel::IKernelContext& ctx, StreamBundle* track, const char *filename)
{
	BoxAdapterGenericStreamWriter writer(ctx, *track, filename);

	bool retval = true;

	retval &= writer.initialize();
	retval &= writer.spool(false);
	retval &= writer.uninitialize();

	if (retval)
	{
		retval &= track->setSource(filename);
		track->setDirtyBit(false);
	}

	return retval;
}
