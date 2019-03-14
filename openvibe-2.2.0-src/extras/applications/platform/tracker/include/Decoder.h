
#pragma once

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include "BoxAlgorithmProxy.h"

#include "Chunk.h"

#include "EncodedChunk.h"

#include "Stream.h"

namespace OpenViBETracker
{

/**
 * \class DecoderBase
 * \brief Base, non-typed abstract class for decoders
 * \details Derived classes can be type-specific
 * \author J. T. Lindgren
 *
 */
class DecoderBase
{
public:
	virtual bool decode(const EncodedChunk& chk) = 0;
	virtual ~DecoderBase() { };
};

// Fwd declare, realized by Codecs*
template <typename TYPENAME> class DecoderImpl;

/**
 * \class Decoder
 * \brief Decoder for a specific type T. 
 * \details The constructor is passed a stream to decode to. The decode() call pushes chunks to that stream.
 * \author J. T. Lindgren
 *
 */
template<class T> class Decoder : public DecoderBase
{
public:
	Decoder(const OpenViBE::Kernel::IKernelContext& ctx, Stream<T>& stream) : m_Stream(stream), m_Decoder(ctx) { };

	virtual bool decode(const EncodedChunk& chk) override {
		m_Decoder.decode(chk);
		if (m_Decoder.isHeaderReceived())
		{
			return m_Decoder.getHeader(m_Stream.getHeader());
		}
		if (m_Decoder.isBufferReceived())
		{
			typename T::Buffer* chunk = new typename T::Buffer();
			if (m_Decoder.getBuffer(*chunk))
			{
				m_Stream.push(chunk);

				return true;
			}
			return false;
		}
		if (m_Decoder.isEndReceived())
		{
			return m_Decoder.getEnd(m_Stream.getEnd());
		}
		return false;
	};

protected:
	Stream<T>& m_Stream;
	DecoderImpl<T> m_Decoder;
};

/**
 * \class DecoderAdapter
 * \brief This decoder is a wrapper over the decoders in OpenViBEToolkit. 
 *
 * The toolkit decoders require a box context to run, and do not return class objects.
 * This decoder wrapper addresses those limitations.
 * The usage is to create e.g. Decoder<TypeSignal> dec; and use the member functions to 
 * obtain the chunks with the signal type.
 *
 * \author J. T. Lindgren
 *
 */
template<class T, class TOOLKITDECODER> class DecoderAdapter : protected Contexted {
public:
	DecoderAdapter(const OpenViBE::Kernel::IKernelContext& ctx) : Contexted(ctx), m_Box(ctx), m_DecoderImpl(m_Box, 0) {};
	virtual ~DecoderAdapter(void) { };

	virtual bool decode(const EncodedChunk& source) {
		if (source.bufferData.size() == 0)
		{
		//	std::cout << "Empty chunk received\n";
			return false;
		}
		m_Box.dummy.m_InBuffer.setSize(0, true);
		m_Box.dummy.m_InBuffer.append(&source.bufferData[0], source.bufferData.size());
		m_LastBufferStart = source.m_startTime;
		m_LastBufferEnd = source.m_endTime;
		return m_DecoderImpl.decode(0);
	}

	virtual bool getHeader(typename T::Header& h) {
		setTimeStamps(h);
		return getHeaderImpl(h);
	}

	virtual bool getBuffer(typename T::Buffer& b) {
		setTimeStamps(b);
		return getBufferImpl(b);
	}

	virtual bool getEnd(typename T::End& e)
	{
		setTimeStamps(e);
		return getEndImpl(e);
	}

	virtual bool isHeaderReceived() {
		return m_DecoderImpl.isHeaderReceived();
	};

	virtual bool isBufferReceived() {
		return m_DecoderImpl.isBufferReceived();
	};

	virtual bool isEndReceived() {
		return m_DecoderImpl.isEndReceived();
	};

protected:
	// These two must be written as specilizations (Codec*cpp)
	bool getHeaderImpl(typename T::Header& h);
	bool getBufferImpl(typename T::Buffer& b);
	
	bool getEndImpl(typename T::End& e) { return true; } // NOP regardless of type

protected:
	bool setTimeStamps(Chunk& chunk) {
		chunk.m_startTime = m_LastBufferStart;
		chunk.m_endTime = m_LastBufferEnd;
		return true;
	}

	BoxAlgorithmProxy m_Box;

	ovtime_t m_LastBufferStart = 0;
	ovtime_t m_LastBufferEnd = 0;

	TOOLKITDECODER m_DecoderImpl;
};


};
