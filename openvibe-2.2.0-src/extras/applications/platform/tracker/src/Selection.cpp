//
// OpenViBE Tracker
//
// @author J.T. Lindgren
// 

#include "Selection.h"

#include <sstream>
#include <iomanip>

using namespace OpenViBETracker;

bool Selection::reset(bool state)
{
	for (auto trk : m_Tracks)
	{
		for (auto str : trk->getAllStreams())
		{
			str->setSelected(state);
		}
	}
	return true;
}

bool Selection::isEmpty(void) const
{
	for (auto trk : m_Tracks)
	{
		if(trk->getNumStreams()>0)
		{
			return false;
		}
	}
	return true;
}

bool Selection::isSomethingSelected(void) const
{
	for (auto trk : m_Tracks)
	{
		for (auto str : trk->getAllStreams())
		{
			if (str->getSelected())
			{
				return true;
			}
		}
	}

	return false;
}



bool Selection::isTrackSelected(size_t track) const
{
	if (track >= m_Tracks.size())
	{
		return false;
	}

	for (auto str : m_Tracks[track]->getAllStreams())
	{
		if (str->getSelected())
		{
			return true;
		}
	}

	return false;
}

bool Selection::isSelectionConsistent(void) const
{
	if (isEmpty() || !isSomethingSelected())
	{
		return true;
	}

	std::vector<OpenViBE::CIdentifier> selectedTypesPrevious;

	// Check that selection for each track is identical to selection of previous track
	// (empty selections per track are ignored)
	for (auto trk : m_Tracks)
	{
		std::vector<OpenViBE::CIdentifier> selectedTypes;
		for (auto str : trk->getAllStreams())
		{
			if (str->getSelected())
			{
				selectedTypes.push_back(str->getTypeIdentifier());
			}
		}
		if (selectedTypesPrevious.size() > 0 && selectedTypes.size() > 0 && selectedTypesPrevious != selectedTypes)
		{
			return false;
		}
		if (selectedTypes.size() > 0)
		{
			selectedTypesPrevious = selectedTypes;
		}
	}

	return true;
}

size_t Selection::countSelectedTracks(void) const
{
	size_t selectedTracks = 0;

	for (auto trk : m_Tracks)
	{
		for (auto str : trk->getAllStreams())
		{
			if (str->getSelected())
			{
				selectedTracks++;
				break;
			}
		}
	}

	return selectedTracks;
}

size_t Selection::countSelectedStreams(size_t trackIndex) const
{
	if(trackIndex>=m_Tracks.size())
	{
		return 0;
	}
	
	size_t numSelected = 0;
	for(auto str : m_Tracks[trackIndex]->getAllStreams())
	{
		if(str && str->getSelected())
		{
			numSelected++;
		}
	}
	return numSelected;
}

bool Selection::save(const char* prefix) 
{
	auto& mgr = this->getKernelContext().getConfigurationManager();

	// @note with really huge datasets the .ovw file handling can get slow. should be profiled.
	for (size_t trk = 0; trk < m_Tracks.size(); trk++)
	{
		std::stringstream token; token << prefix << "Track_" << std::setw(3) << std::setfill('0') << (trk + 1) << "_Selected";
		std::stringstream value; 

		for (size_t str = 0; str < m_Tracks[trk]->getNumStreams(); str++)
		{
			if( m_Tracks[trk]->getStream(str)->getSelected() )
			{
				value << (str+1) << " ";
			}
		}

		mgr.addOrReplaceConfigurationToken(token.str().c_str(), value.str().c_str());
	}

	return true;
}

// Relies  on the current track/stream set being compatible with the tokens. no error checking
bool Selection::load(const char* prefix)
{
	auto& mgr = this->getKernelContext().getConfigurationManager();

	for (size_t trk = 0; trk < m_Tracks.size(); trk++)
	{
		std::stringstream token; token << prefix << "Track_" << std::setw(3) << std::setfill('0') << (trk + 1) << "_Selected";
		
		std::stringstream value( mgr.lookUpConfigurationTokenValue(token.str().c_str()).toASCIIString() );

		// Mark everything as unselected by default
		for (size_t str = 0; str < m_Tracks[trk]->getNumStreams(); str++)
		{
			m_Tracks[trk]->getStream(str)->setSelected(false);
		}
		
		// Load sparse selection from file
		std::string selected;
		while(std::getline(value, selected, ' '))
		{
			uint32_t selectedIdx = atoi( selected.c_str() ) - 1;
			if(selectedIdx<m_Tracks[trk]->getNumStreams())
			{
				m_Tracks[trk]->getStream(selectedIdx)->setSelected(true);
			}
		}
	}

	return true;
}
