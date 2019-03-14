
#pragma once

#include <vector>

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include "Encoder.h"
#include "Decoder.h"

#include "TypeError.h"

// This macro makes specializations by instantiating an adapter that handles a type with
// the actual encoder and decoder from the toolkit 
#define CODEC_IMPL_VIA_TOOLKIT(TYPENAME, TOOLKITENCODER, TOOLKITDECODER) \
	template<> class DecoderImpl<TYPENAME> : public DecoderAdapter< TYPENAME, OpenViBEToolkit::TOOLKITDECODER<BoxAlgorithmProxy> > \
	{ \
	public: \
		DecoderImpl(const OpenViBE::Kernel::IKernelContext& ctx) : DecoderAdapter(ctx) { }; \
	};\
	template<> class EncoderImpl<TYPENAME> : public EncoderAdapter< TYPENAME, OpenViBEToolkit::TOOLKITENCODER<BoxAlgorithmProxy> > \
	{ \
	public: \
		EncoderImpl(const OpenViBE::Kernel::IKernelContext& ctx) : EncoderAdapter(ctx) { }; \
	}; 

/**
 * \class DecoderImpl
 * \brief Fallback class for invalid situations
 * \author J. T. Lindgren
 *
 */
namespace OpenViBETracker 
{ 

template<> class DecoderImpl<OpenViBETracker::TypeError> {
public:
	DecoderImpl(const OpenViBE::Kernel::IKernelContext& ctx) { };

	virtual bool decode(const EncodedChunk& source) {
		return false;
	};

	virtual bool getHeader(TypeError::Header& h) {
		return false;
	}

	virtual bool getBuffer(TypeError::Buffer& b) {
		return false;
	}

	virtual bool getEnd(TypeError::End& e)
	{
		return false;
	}

	virtual bool isHeaderReceived() {
		return false;
	};

	virtual bool isBufferReceived() {
		return false;
	};

	virtual bool isEndReceived() {
		return false;
	};

};

/**
 * \class EncoderImpl
 * \brief Fallback class for invalid situations
 * \author J. T. Lindgren
 *
 */
template<> class EncoderImpl<OpenViBETracker::TypeError>
{
public:
	EncoderImpl(const OpenViBE::Kernel::IKernelContext& ctx) { };

	bool encodeHeader(EncodedChunk& chk, const TypeError::Header& hdr)
	{
		return false;
	}

	bool encodeBuffer(EncodedChunk& chk, const TypeError::Buffer& buf)
	{
		return false;
	}

	bool encodeEnd(EncodedChunk& chk, const TypeError::End& end)
	{
		return false;
	}

	bool setEncodeOffset(ovtime_t newOffset) { 
		return false;
	}

};

};
