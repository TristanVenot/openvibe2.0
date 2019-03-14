//
// OpenViBE Tracker
//
#pragma once

#include <string>
#include <vector>

#include <openvibe/ov_types.h>

#include "StreamBundle.h"
#include "Contexted.h"

namespace OpenViBETracker
{

/**
 * \class Selection 
 * \brief Some convenience functions for Track/Stream selections
 * \author J. T. Lindgren
 *
 */
class Selection : protected Contexted {
public:
	Selection(const OpenViBE::Kernel::IKernelContext& ctx, const std::vector<StreamBundle*>& tracks) : Contexted(ctx), m_Tracks(tracks) { };
	
	// Set the selection status of all streams on all tracks to the given state
	bool reset(bool selected);

	// Is any stream selected on the track?
	bool isTrackSelected(size_t track) const;
	// Nothing selectable?
	bool isEmpty(void) const;
	// Is something currently selected?
	bool isSomethingSelected(void) const;
	// Are selections of different tracks compatible?
	bool isSelectionConsistent(void) const;
	// How many tracks have at least one stream selected?
	size_t countSelectedTracks(void) const;
	// How many streams does a current track have selected?
	size_t countSelectedStreams(size_t trackIndex) const;

	// Serialize state to ConfigurationManager
	bool save(const char* prefix);
	bool load(const char* prefix);

protected:

	const std::vector<StreamBundle*>& m_Tracks;

};


};
