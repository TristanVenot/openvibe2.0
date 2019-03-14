
#pragma once

#include <openvibe/ov_all.h>

#include "Contexted.h"

#include "StreamBundle.h"

#include "BoxAdapterHelper.h"

#include "Encoder.h"
#include "Decoder.h"

namespace OpenViBETracker
{

/**
 * \class BoxAdapter 
 * \brief A partially abstract class allowing processing data with OpenViBE boxes using a simple interface. 
 *
 * The class implements a few of its members, but derived classes should implement the method spool() that feeds the data to the box,
 * as well as members allowing setting the sources and destinations of the data.
 *
 * \author J. T. Lindgren
 *
 */
class BoxAdapter : protected Contexted {
public:

	BoxAdapter(const OpenViBE::Kernel::IKernelContext& ctx, const OpenViBE::CIdentifier& algorithmId)
		: 
		Contexted(ctx)
		, m_BoxAlgorithmContext(ctx)
		, m_AlgorithmId(algorithmId)
	{

	}

	virtual ~BoxAdapter() { }; 

	virtual bool initialize(void);
	virtual bool spool(bool verbose) = 0;
	virtual bool uninitialize(void);

	virtual TrackerBox& getBox(void) { return m_BoxAlgorithmContext.m_StaticBoxContext; }
	virtual OpenViBE::CIdentifier getAlgorithmId(void) { return m_AlgorithmId; }

protected:

	TrackerBoxAlgorithmContext m_BoxAlgorithmContext;

	OpenViBE::Plugins::IBoxAlgorithm* m_pBoxAlgorithm = nullptr;

	OpenViBE::CIdentifier m_AlgorithmId;

};

/**
 * \class BoxAdapterStream 
 * \brief A box adapter that reads from a Stream and writes to another Stream, having effect equivalent to outStream = process(inStream);
 * \author J. T. Lindgren
 *
 */
class BoxAdapterStream : public BoxAdapter
{
public: 

	BoxAdapterStream(const OpenViBE::Kernel::IKernelContext& ctx,
		const OpenViBE::CIdentifier& algorithmId)
		: BoxAdapter(ctx, algorithmId) { };

	virtual bool setSource(StreamPtr source) { m_Source = source; return true; }
	virtual bool setTarget(StreamPtr target) { m_Target = target; return true; }

	virtual bool initialize(void) override;

	// Process the whole stream from source to target
	virtual bool spool(bool verbose) override;

protected:

	StreamPtr m_Source = nullptr;
	StreamPtr m_Target = nullptr;
};

/**
 * \class BoxAdapterBundle 
 * \brief A box adapter that reads from a StreamBundle and writes to another StreamBundle
 * \author J. T. Lindgren
 *
 */
class BoxAdapterBundle : public BoxAdapter
{
public:

	BoxAdapterBundle(const OpenViBE::Kernel::IKernelContext& ctx,
		const OpenViBE::CIdentifier& algorithmId)
	   : BoxAdapter(ctx, algorithmId) { };

	~BoxAdapterBundle();

	virtual bool setSource(StreamBundle* source) { m_Source = source; return true; }
	virtual bool setTarget(StreamBundle* target) { m_Target = target; return true; }

	virtual bool initialize(void) override;

	// Process the whole bundle from source to target
	virtual bool spool(bool verbose) override;

protected:

	StreamBundle* m_Source = nullptr;
	StreamBundle* m_Target = nullptr;

	std::vector<EncoderBase*> m_vEncoders;
	std::vector<DecoderBase*> m_vDecoders;
};

};

