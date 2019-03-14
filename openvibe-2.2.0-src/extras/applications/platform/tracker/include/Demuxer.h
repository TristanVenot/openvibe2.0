
#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <stack>
#include <map>
#include <vector>

#include <openvibe/ov_all.h>
#include <openvibe/ovCMatrix.h>

#include <socket/IConnectionServer.h>

#include <fs/Files.h>

#include <ebml/CReader.h>
#include <ebml/CReaderHelper.h>

#include "StreamBundle.h"

#include "EBMLSourceFile.h"
#include "Contexted.h"

#include "EncodedChunk.h"
#include "Decoder.h"

class CClientHandler;
class OutputEncoder;

class EBMLSource;

namespace OpenViBETracker
{

/**
 * \class Demuxer 
 * \brief Demuxes (and decodes) EBML streams
 * \details EBML containers such as .ov multiplex one or more streams. Demuxer takes an EBML source and splits and decodes it to a Stream Bundle.
 *
 * It has similar purpose as the Generic Stream Reader box.
 *
 * \author J. T. Lindgren
 *
 */
class Demuxer : public EBML::IReaderCallback, protected Contexted {
public:

	Demuxer(const OpenViBE::Kernel::IKernelContext& ctx, EBMLSourceFile& origin, StreamBundle& target)
		: Contexted(ctx)
		, m_origin(origin)
		, m_target(target)
		, m_oReader(*this)
	{
		initialize(); 
	};

 	bool initialize(void);
	bool uninitialize(void);

	// Reads some data from the origin and decodes it to target. Returns false on EOF.
	bool step(void);
	bool stop(void);
	
protected:

	EBMLSourceFile& m_origin;
	StreamBundle& m_target;

	bool playingStarted;

	EBML::CReader m_oReader;
	EBML::CReaderHelper m_oReaderHelper;

	EncodedChunk m_oPendingChunk;

	uint32_t m_ChunksSent = 0;

	OpenViBE::boolean m_bPending = false;
	OpenViBE::boolean m_bHasEBMLHeader = false;

	std::stack < EBML::CIdentifier > m_vNodes;
	std::map < OpenViBE::uint32, OpenViBE::uint32 > m_vStreamIndexToOutputIndex;
	std::map < OpenViBE::uint32, OpenViBE::CIdentifier > m_vStreamIndexToTypeIdentifier;

	std::vector<OpenViBE::uint64> m_ChunkStreamType;

	std::vector<DecoderBase*> m_vDecoders;

private:

	virtual EBML::boolean isMasterChild(const EBML::CIdentifier& rIdentifier);
	virtual void openChild(const EBML::CIdentifier& rIdentifier);
	virtual void processChildData(const void* pBuffer, const EBML::uint64 ui64BufferSize);
	virtual void closeChild(void);

};

};

