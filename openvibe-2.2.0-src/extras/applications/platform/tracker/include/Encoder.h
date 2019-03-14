
#pragma once

#include <string.h> // memcpy() on Ubuntu

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include "EncodedChunk.h"

#include "BoxAlgorithmProxy.h"
#include "Chunk.h"


#include "Stream.h"

namespace OpenViBETracker
{

/**
 * \class EncoderBase
 * \brief Base, non-typed abstract class for encoders
 * \author J. T. Lindgren
 *
 */
class EncoderBase
{
public:

	virtual ~EncoderBase() { };

	// Encodes the current chunk of the attached stream (in the derived class)
	// chkType will contain details about the specific chunk type (since EncodedChunk doesn't show it explicitly)
	virtual bool encode(EncodedChunk& chk, ChunkType& chkType) = 0;

	// Advance all timestamps of the encoded chunks by this amount
	// Used for catenating multiple tracks
	virtual bool setEncodeOffset(ovtime_t newOffset) = 0;


};

// Fwd declare, realized by Codecs*
template <class TYPENAME> class EncoderImpl;

/**
 * \class Encoder
 * \brief Encoder class taking in typed Stream and it into EBML-containing encoded chunks.
 * \details The constructor is passed a Stream to encode from. The encode() call pulls the next chunk from the Stream and returns an EncodedChunk.
 * \author J. T. Lindgren
 *
 */
template<class T> class Encoder : public EncoderBase
{
public:
	Encoder(const OpenViBE::Kernel::IKernelContext& ctx, Stream<T>& stream) : m_Stream(stream), m_Encoder(ctx) { };

	// Encode the current chunk. The caller must step the stream.
	bool encode(EncodedChunk& chk, ChunkType& chkType)  override
	{
		const size_t position = m_Stream.getPosition();

		if (position == (size_t)(-1))
		{
			//			std::cout << "Encode header\n";
			chkType = CHUNK_TYPE_HEADER;
			return m_Encoder.encodeHeader(chk, m_Stream.getHeader());
		}
		else if (position < m_Stream.getChunkCount())
		{
			chkType = CHUNK_TYPE_BUFFER;

			const bool retVal = m_Encoder.encodeBuffer(chk, *m_Stream.getChunk(position));
			// 	std::cout << "Encoder gave " << chk.bufferData.size() << "\n";

			return retVal;
		}
		else if (position == m_Stream.getChunkCount())
		{
			chkType = CHUNK_TYPE_END;

			auto end = m_Stream.getEnd();

			// Some streams might not have an end in the file. Here we force one to be present.
			// @fixme move to load
			if (end.m_startTime == (ovtime_t)(-1) || end.m_endTime == (ovtime_t)(-1))
			{
				// @note modifies the stream
				end.m_startTime = m_Stream.getDuration();
				end.m_endTime = end.m_startTime;
			}

			return m_Encoder.encodeEnd(chk, end);
		}
		else
		{
			return false;
		}
	};

	bool setEncodeOffset(ovtime_t newOffset) override { return m_Encoder.setEncodeOffset(newOffset); }

protected:

	Stream<T>& m_Stream;
	EncoderImpl<T> m_Encoder;
};

/**
 * \class EncoderAdapter
 * \brief Adapter to use implementations from Toolkit
 *
 * Note that this class does not set the encoded chunk times or index, these are currently 
 * handled outside the encoder.
 *
 * \author J. T. Lindgren
 *
 */
template<class T, class TOOLKITENCODER> class EncoderAdapter : protected Contexted {
public:
	EncoderAdapter(const OpenViBE::Kernel::IKernelContext& ctx) : Contexted(ctx), m_Box(ctx), m_EncoderImpl(m_Box, 0) {};
	virtual ~EncoderAdapter()
	{

	}

	// If chunk timestamps are adjusted, this function should be used to adjust any possible
	// timestamps inside the chunks accordingly
	bool setEncodeOffset(ovtime_t newOffset) { m_EncodeOffset = newOffset; return true; };

	bool encodeHeader(EncodedChunk& chk, const typename T::Header& inputHdr)
	{
		m_Box.dummy.setOutputChunkSize(0, 0, true);
		chk.m_startTime = inputHdr.m_startTime + m_EncodeOffset;
		chk.m_endTime = inputHdr.m_endTime + m_EncodeOffset;

		encodeHeaderImpl(inputHdr);

		fillChunk(chk);

		return true;
	}

	bool encodeBuffer(EncodedChunk& chk, const typename T::Buffer& inputBuf)
	{
		m_Box.dummy.setOutputChunkSize(0, 0, true);
		chk.m_startTime = inputBuf.m_startTime + m_EncodeOffset;
		chk.m_endTime =  inputBuf.m_endTime + m_EncodeOffset;

		encodeBufferImpl(inputBuf);

		fillChunk(chk);

		return true;
	}

	bool encodeEnd(EncodedChunk& chk, const typename T::End& inputEnd) 
	{
		m_Box.dummy.setOutputChunkSize(0, 0, true);

		chk.m_startTime = inputEnd.m_startTime + m_EncodeOffset;
		chk.m_endTime = inputEnd.m_endTime + m_EncodeOffset;

		encodeEndImpl(inputEnd);

		fillChunk(chk);

		return true;
	}

protected:

	// These two must be written as specializations (Codec*cpp)
	bool encodeHeaderImpl(const typename T::Header& hdr);
	bool encodeBufferImpl(const typename T::Buffer& buf);

	bool encodeEndImpl(const typename T::End& end) { return m_EncoderImpl.encodeEnd(); };

	bool fillChunk(EncodedChunk& chk)
	{
		const size_t chunkSize = static_cast<size_t>(m_Box.dummy.getOutputChunkSize(0));
		chk.bufferData.resize(chunkSize);
		if (chunkSize > 0)
		{
			memcpy(&chk.bufferData[0], m_Box.dummy.getOutputChunkBuffer(0), chunkSize);
		}
		return true;
	}

	BoxAlgorithmProxy m_Box;

	TOOLKITENCODER m_EncoderImpl;

	ovtime_t m_EncodeOffset = 0;

};

};

