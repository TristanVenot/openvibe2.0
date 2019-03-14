
#pragma once

#include <iostream>
#include <thread>
#include <deque>
#include <vector>

#include "../../../../plugins/processing/file-io/src/ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <system/ovCTime.h>
#include <fs/Files.h>

#include "Contexted.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;

namespace OpenViBETracker
{

/**
 * \class EBMLSourceFile 
 * \brief Reads bytes from an .ov file
 * \author J. T. Lindgren
 *
 * @note some ov files which have lots of stimulation chunks take a long time to import
 * when launching the Tracker from visual studio. this is probably due to memory allocation, similar to slow simple dsp grammar parsing.
 *
 *
 */
class EBMLSourceFile : protected Contexted {
public:
	EBMLSourceFile(const OpenViBE::Kernel::IKernelContext& ctx) : Contexted(ctx) { };

	bool initialize(const char* signalFile) 
	{ 
		log() << LogLevel_Trace << "EBMLSource: Initializing with "	<< signalFile << "\n";

		m_pFile = FS::Files::open(signalFile, "rb");
		if(!m_pFile)
		{
			return false;
		}
		
		m_Source = std::string(signalFile);

		return true; 
	}

	bool uninitialize(void) 
	{ 
		log() << LogLevel_Trace << "EBMLSource: Uninitializing\n";

		if(m_pFile) 
		{
			::fclose(m_pFile);
			m_pFile = nullptr;
		}

		return true;
	}

	bool isEOF(void)
	{
		if(!m_pFile) { return true; }

		return (feof(m_pFile)!=0 ? true : false);
	}

	bool read(std::vector<uint8_t>& bytes,size_t numBytes)
	{
		if(!m_pFile)
		{
			log() << LogLevel_Error << "Error: No EBML source file set\n";
			return false;
		}

		if(isEOF())
		{
			log() << LogLevel_Trace << "EBML Source file EOF reached\n";
			return false;
		}

		bytes.resize(numBytes);
		size_t s=::fread(&bytes[0], sizeof(uint8), numBytes, m_pFile);
		bytes.resize(s); // array size tells how much was read

		return true;
	}

	const std::string& getSource(void) const { return m_Source; };

protected:

	std::string m_Source;

	FILE* m_pFile = nullptr;
};


};
