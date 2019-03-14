
#include "Processor.h"

#include <iostream>
#include <fstream>

#include <chrono>
#include <ctime>

#include <fs/Files.h>
#include <system/ovCMath.h>
#include <thread>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBETracker;

void playerLaunch(const char *xmlFile, const char *args, bool playFast, bool noGUI, uint32_t identifier)
{
	std::string Designer = std::string(OpenViBE::Directories::getBinDir().toASCIIString()) + "/openvibe-designer.exe";
	std::string argsAll = std::string(" --no-session-management ")
		+ (args ? args : "")
		+ (noGUI ? "--no-gui " : "")
		+ (playFast ? "--play-fast " : " --play ")
		+ " " + "\"" + xmlFile +"\"";
	if (!FS::Files::directoryExists(OpenViBE::Directories::getUserDataDir().toASCIIString()))
	{
		FS::Files::createPath(OpenViBE::Directories::getUserDataDir().toASCIIString());
	}

	std::stringstream ss; 
	ss << std::string(OpenViBE::Directories::getLogDir().toASCIIString()) << "/tracker-processor-dump-" << identifier << ".txt";
	std::string outputDump = ss.str();
	
	std::time_t now = std::chrono::system_clock::to_time_t( std::chrono::system_clock().now() );
	std::ofstream outStream(outputDump.c_str(), std::ios::app);
	outStream << std::endl << "Date of launch: " << std::ctime(&now) << std::endl;
	outStream << "Trying to launch: " << Designer << std::endl;
	outStream << "Args: " << argsAll << std::endl;
	outStream << "Logging to: " << outputDump << std::endl;
	outStream << std::endl;
	outStream.close();

#if TARGET_OS_Windows

	STARTUPINFO si;     
	PROCESS_INFORMATION pi;

	// set the size of the structures
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE; 

    HANDLE h = CreateFile(outputDump.c_str(),
        FILE_APPEND_DATA,
//		FILE_WRITE_DATA,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        &sa,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL );

    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdInput = NULL;
    si.hStdError = h;
    si.hStdOutput = h;

	argsAll = std::string("\"") + Designer + std::string("\"") + std::string(" ") + argsAll;
	
	LPSTR argsLp = const_cast<char *>(argsAll.c_str());

	BOOL retVal = CreateProcess( Designer.c_str(),   // the path
		argsLp,         // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		TRUE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
		);
		// Close process and thread handles. 
	if(!retVal)
	{
		std::cout << "err: " << GetLastError() << "\n";
	}
	else
	{
		WaitForSingleObject(pi.hProcess, INFINITE);
	}

	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );
	CloseHandle( h );
#else

	auto cmd = Designer + argsAll + " >" + outputDump;

//	std::cout << "Call is: " << cmd.c_str() << "\n";

	if (system(cmd.c_str()) != 0)
	{
		std::cout << "Launch of [" << cmd.c_str() << "] failed\n";
	}
#endif

}

bool Processor::configure(const char* filename)
{
	std::string usedFilename = (filename ? filename : m_xmlFilename.c_str());

	if(usedFilename.length()==0) 
	{
		log() << LogLevel_Error << "Error: Please set processor filename first\n";
		return false;
	}

	const CString expandedName = m_KernelContext.getConfigurationManager().expand(usedFilename.c_str());

	std::string Designer = std::string(OpenViBE::Directories::getBinDir().toASCIIString()) + "/openvibe-designer --no-session-management --open ";
	std::string OutputDump = std::string(OpenViBE::Directories::getDistRootDir().toASCIIString()) + "/tracker-processor-configure-dump.txt";

	auto cmd = Designer + expandedName.toASCIIString() + " >" + OutputDump;

	if(system(cmd.c_str())!=0)
	{
		log() << LogLevel_Error << "Launch of [" << cmd.c_str() << "] failed\n";
		return false;
	}
	return true;
}

