
#pragma once

#include <openvibe/ov_all.h>

#include "OvTime.h"

namespace OpenViBETracker
{

/**
 * \class Chunk
 * \brief Base class for all OpenViBE stream content (chunk) types
 * \details
 *
 * Regardless of the type, all chunks in OpenViBE have start and end timestamps. Hence
 * they should derive from this class. In particular, Header, Buffer and End chunks
 * for some particular type should ultimately derive from this class. However,
 * this class defines no content.
 *
 * @todo In the future if these types turn out to be more generally useful, we could consider
 * moving them to the Kernel.
 *
 * \author J. T. Lindgren
 *
 */

class Chunk {
	public:
		ovtime_t m_startTime = 0;
		ovtime_t m_endTime = 0;

};

};
