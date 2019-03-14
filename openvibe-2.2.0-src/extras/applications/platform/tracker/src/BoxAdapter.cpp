
#include <openvibe/ov_all.h>

#include <fs/Files.h>

#include "BoxAdapter.h"
#include "CodecFactory.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBETracker;


bool BoxAdapter::initialize(void){
	m_pBoxAlgorithm = m_KernelContext.getPluginManager().createBoxAlgorithm(m_AlgorithmId, NULL);
	if (!m_pBoxAlgorithm)
	{
		log() << LogLevel_Error << "Error: failed to create algorithm for id " << m_AlgorithmId.toString() << "\n";
		return false;
	}
	m_BoxAlgorithmContext.getTrackerBoxIO()->initialize( m_BoxAlgorithmContext.getStaticBoxContext() );

	return true;
}

bool BoxAdapter::uninitialize(void)
{
	if (m_pBoxAlgorithm)
	{
		m_pBoxAlgorithm->uninitialize(m_BoxAlgorithmContext);
		
		m_KernelContext.getPluginManager().releasePluginObject(m_pBoxAlgorithm);

		m_pBoxAlgorithm = nullptr;
	}

	return true;
}

BoxAdapterBundle::~BoxAdapterBundle()
{
	for (auto decoder : m_vDecoders)
	{
		delete decoder;
	}
	m_vDecoders.clear();
	for (auto encoder : m_vEncoders)
	{
		delete encoder;
	}
	m_vEncoders.clear();
}

bool BoxAdapterBundle::initialize(void)
{
	if (!BoxAdapter::initialize())
	{
		return false;
	}

	OpenViBE::Kernel::IBox* staticBoxContext = const_cast<OpenViBE::Kernel::IBox*>(m_BoxAlgorithmContext.getStaticBoxContext());

	if (m_Source)
	{
		for (size_t i = 0; i < m_Source->getNumStreams(); i++)
		{
			// @fixme test boxes input support here
			std::stringstream ss; ss << "Stream" << i; // not really used since there's no GUI for the box
			staticBoxContext->addInput(ss.str().c_str(), m_Source->getStream(i)->getTypeIdentifier());
			m_vEncoders.push_back(CodecFactory::getEncoder(m_KernelContext, *m_Source->getStream(i)));
			if (m_Target)
			{
				m_Target->createStream(i, m_Source->getStream(i)->getTypeIdentifier());
				m_vDecoders.push_back(CodecFactory::getDecoder(m_KernelContext, *m_Target->getStream(i)));
				staticBoxContext->addOutput(ss.str().c_str(), m_Target->getStream(i)->getTypeIdentifier());
			}
		}
	}

	m_pBoxAlgorithm->initialize(m_BoxAlgorithmContext);

	return true;
}

bool BoxAdapterBundle::spool(bool /* verbose */ )
{
	OpenViBE::Kernel::IBoxIO* dynamicBoxContext = const_cast<OpenViBE::Kernel::IBoxIO*>(m_BoxAlgorithmContext.getDynamicBoxContext());
	TrackerBoxIO* ioContext = static_cast<TrackerBoxIO*>(dynamicBoxContext);

	if (!m_Source)
	{
		log() << LogLevel_Error << "Error: box wrapper doesn't yet support processing without source\n";
		return false;
	}

	m_Source->rewind();

	uint32_t outputCount = m_BoxAlgorithmContext.getStaticBoxContext()->getOutputCount();

	while (1)
	{
		size_t index;
		StreamPtr stream = m_Source->getNextStream(index);
		if (!stream)
		{
			break;
		}
		EncodedChunk chk; ChunkType outputType;
		m_vEncoders[index]->encode(chk, outputType);

		ioContext->addInputChunk(index, chk);

		m_pBoxAlgorithm->process(m_BoxAlgorithmContext);

		ioContext->clearInputChunks();

		if (m_Target)
		{
			for (uint32_t j = 0; j < outputCount; j++)
			{
				if (ioContext->isReadyToSend(j))
				{
					EncodedChunk chk;
					ioContext->getOutputChunk(j, chk);
					m_vDecoders[j]->decode(chk);
					ioContext->deprecateOutput(j);
				}
			}
		}
		stream->step();
	}

	m_Source->rewind();

	return true;
}

bool BoxAdapterStream::initialize(void)
{
	if (!BoxAdapter::initialize())
	{
		return false;
	}

	OpenViBE::Kernel::IBox* staticBoxContext = const_cast<OpenViBE::Kernel::IBox*>(m_BoxAlgorithmContext.getStaticBoxContext());

	OpenViBE::CIdentifier typeId;
	staticBoxContext->getInputType(0, typeId);
	if (typeId != m_Source->getTypeIdentifier())
	{
		log() << LogLevel_Error << "Error: Box alg first input stream is wrong type\n";
		return false;
	}
	staticBoxContext->getOutputType(0, typeId);
	if (typeId != m_Target->getTypeIdentifier())
	{
		log() << LogLevel_Error << "Error: Box alg first output stream is wrong type\n";
		return false;
	}

	m_pBoxAlgorithm->initialize(m_BoxAlgorithmContext);

	return true;
}

bool BoxAdapterStream::spool(bool verbose)
{
	OpenViBE::Kernel::IBoxIO* dynamicBoxContext = const_cast<OpenViBE::Kernel::IBoxIO*>(m_BoxAlgorithmContext.getDynamicBoxContext());
	TrackerBoxIO* ioContext = static_cast<TrackerBoxIO*>(dynamicBoxContext);

	if (!m_Source)
	{
		log() << LogLevel_Error << "Error: box wrapper doesn't yet support processing without source\n";
		return false;
	}

	m_Source->reset();

	EncoderBase* encoder = CodecFactory::getEncoder(m_KernelContext, *m_Source);
	DecoderBase* decoder = CodecFactory::getDecoder(m_KernelContext, *m_Target);

	if(verbose) log() << LogLevel_Info;

	bool finished = false; uint64_t cnt = 0;
	while (!finished)
	{
		if (m_Source)
		{
			EncodedChunk chk; ChunkType outputType;
			if (encoder->encode(chk, outputType))
			{
				// @fixme here we assume the box takes data in at the first slot
				ioContext->clearInputChunks();
				ioContext->addInputChunk(0, chk);

				m_Source->step();
			}
			else
			{
				finished = true;
			}
		}

		m_pBoxAlgorithm->process(m_BoxAlgorithmContext);

		if (m_Target)
		{
			if (ioContext->isReadyToSend(0))
			{
				EncodedChunk chk;
				ioContext->getOutputChunk(0, chk);
				decoder->decode(chk);
				ioContext->deprecateOutput(0);
			}
		}
		if (verbose && cnt++ % 100 == 0)
		{
			log() << ".";
		}
	}
	if( verbose ) log() << "\n";

	m_Source->reset();

	ioContext->clearInputChunks();

	delete encoder;
	delete decoder;

	return true;
}
