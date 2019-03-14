//
// OpenViBE Tracker
//
// @author J.T. Lindgren
// 

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <iostream>

#include "Tracker.h"

#include "GUI.h"

#include "Testclass.h"

#if defined TARGET_OS_Windows
#include "Windows.h"
#include "shellapi.h"
#endif


using namespace std;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBETracker;

typedef enum
{
	CommandLineFlag_None      = 0x00000000,
	CommandLineFlag_Define    = 0x00000040,
	CommandLineFlag_Config    = 0x00000080,
} ECommandLineFlag;

typedef struct _SConfiguration
{
	_SConfiguration(void)
	:
	m_eDefine(ECommandLineFlag::CommandLineFlag_None)
	, m_eConfig(ECommandLineFlag::CommandLineFlag_None)
	, m_help(false)
	{
	}

	OpenViBEDesigner::ECommandLineFlag getFlags(void)
	{
		return OpenViBEDesigner::ECommandLineFlag(
			m_eDefine
			| m_eConfig
			);
	}

	std::vector < std::pair < ECommandLineFlag, std::string > > m_vFlag;

	ECommandLineFlag m_eDefine;
	ECommandLineFlag m_eConfig;
	bool                               m_help;
	// to resolve warning: padding struct '_SConfiguration' with 4 bytes to align 'm_oTokenMap
	int32_t	                           m_i32StructPadding;
	std::map < std::string, std::string > m_oTokenMap;

} SConfiguration;


OpenViBE::boolean parse_arguments(int argc, char** argv, SConfiguration& rConfiguration)
{
	SConfiguration l_oConfiguration;

	std::vector < std::string > l_vArgValue;
#if defined TARGET_OS_Windows
	int argCount;
	LPWSTR* argListUtf16 = CommandLineToArgvW(GetCommandLineW(), &argCount);
	for (int i = 1; i < argCount; i++)
	{
		GError* error = nullptr;
		glong itemsRead, itemsWritten;
		char* argUtf8 = g_utf16_to_utf8(reinterpret_cast<gunichar2*>(argListUtf16[i]), static_cast<size_t>(wcslen(argListUtf16[i])), &itemsRead, &itemsWritten, &error);
		l_vArgValue.push_back(argUtf8);
		if (error)
		{
			g_error_free(error);
			return false;
		}
	}
#else
	l_vArgValue = std::vector<std::string>(argv + 1, argv + argc);
#endif
	l_vArgValue.push_back("");

	for (auto it = l_vArgValue.cbegin(); it != l_vArgValue.cend(); ++it)
	{
		if (*it == "")
		{
		}
		else if (*it == "-h" || *it == "--help")
		{
			l_oConfiguration.m_help = true;
			rConfiguration = l_oConfiguration;
			return false;
		}
		else if (*it == "-c" || *it == "--config")
		{
			if (*++it == "") { std::cout << "Error: Switch --config needs an argument\n"; return false; }
			l_oConfiguration.m_vFlag.push_back(std::make_pair(CommandLineFlag_Config, *it));
		}
		else if (*it == "-d" || *it == "--define")
		{
			if (*++it == "")
			{
				std::cout << "Error: Need two arguments after -d / --define.\n";
				return false;
			}

			// Were not using = as a separator for token/value, as on Windows its a problem passing = to the cmd interpreter
			// which is used to launch the actual designer exe.
			const std::string& l_rToken = *it;
			if (*++it == "")
			{
				std::cout << "Error: Need two arguments after -d / --define.\n";
				return false;
			}

			const std::string& l_rValue = *it;	// iterator will increment later

			l_oConfiguration.m_oTokenMap[l_rToken] = l_rValue;

		}
		else if (*it == "--g-fatal-warnings")
		{
			// Do nothing here but accept this gtk flag
		}
		else if(*it=="-v" || *it=="--version")
		{
#if defined(TARGET_OS_Windows)
			const std::string platform("Windows");
#elif defined(TARGET_OS_Linux)
			const std::string platform("Linux");
#else 
			const std::string platform("Other");
#endif
#if defined(TARGET_ARCHITECTURE_x64)
			const std::string arch("64bit");
#else
			const std::string arch("32bit");
#endif
#if defined(TARGET_BUILDTYPE_Debug)
			const std::string buildType("Debug");
#elif defined(TARGET_BUILDTYPE_Release)
			const std::string buildType("Release");
#else
			const std::string buildType("Unknown");
#endif
			std::cout << OV_PROJECT_NAME << " Tracker - Version " << OV_VERSION_MAJOR << "." << OV_VERSION_MINOR << "." << OV_VERSION_PATCH 
				<< " (" << platform << " " << arch << " " << buildType << " build)"
				<< std::endl;
			exit(0);
		}
		else
		{
			return false;
		}
	}

	rConfiguration = l_oConfiguration;

	return true;
}

void user_info(char ** argv, ILogManager* l_rLogManager)
{
	const std::vector<std::string> messages =
	{
		"Syntax : " + std::string(argv[0]) + " [ switches ]\n",
		"Possible switches :\n",
		"  --help                  : displays this help message and exits\n",
		"  --config filename       : path to config file\n",
		"  --define token value    : specify configuration token with a given value\n"
		"  --version               : shows version information and exits"
	};

	if (l_rLogManager)
	{
		for (const auto &m : messages)
		{
			(*l_rLogManager) << LogLevel_Info << m.c_str();
		}
	}
	else
	{
		for (const auto &m : messages)
		{
			cout << m;
		}
	}
}


class KernelWrapper
{
public:
	KernelWrapper() : m_KernelContext(nullptr), m_KernelDesc(nullptr)
	{

	}

	~KernelWrapper()
	{

		if(m_KernelContext)
		{
			std::cout << "Unloading kernel" << endl;
			OpenViBEToolkit::uninitialize(*m_KernelContext);
			m_KernelDesc->releaseKernel(m_KernelContext);
			m_KernelContext = nullptr;
		}

		std::cout << "Unloading loader" << endl;
		m_KernelLoader.uninitialize();
		m_KernelLoader.unload();	
	}

	bool initialize(const SConfiguration& config)
	{

		std::cout << "[  INF  ] Created kernel loader, trying to load kernel module" << std::endl;
		CString l_sError;
#if defined TARGET_OS_Windows
		CString l_sKernelFile = OpenViBE::Directories::getLibDir() + "/openvibe-kernel.dll";
#else
		CString l_sKernelFile = OpenViBE::Directories::getLibDir() + "/libopenvibe-kernel.so";
#endif
		if(!m_KernelLoader.load(l_sKernelFile, &l_sError))
		{
			std::cout << "[ FAILED ] Error loading kernel from [" << l_sKernelFile << "]: " << l_sError << "\n";
			return false;
		}

		std::cout << "[  INF  ] Kernel module loaded, trying to get kernel descriptor" << std::endl;
		m_KernelLoader.initialize();
		m_KernelLoader.getKernelDesc(m_KernelDesc);
		if(!m_KernelDesc)
		{
			cout<<"[ FAILED ] No kernel descriptor"<<endl;
			return false;
		}

		cout<<"[  INF  ] Got kernel descriptor, trying to create kernel"<<endl;

		CString l_sConfigFile = CString(OpenViBE::Directories::getDataDir() + "/kernel/openvibe.conf");

		m_KernelContext=m_KernelDesc->createKernel("tracker", l_sConfigFile);
		if(!m_KernelContext)
		{
			cout<<"[ FAILED ] No kernel created by kernel descriptor"<<endl;
			return false;
		}

		m_KernelContext->initialize();

		// Gets the 'stimulation id,name' mappings loaded to TypeManager etc
		OpenViBEToolkit::initialize(*m_KernelContext);

		IConfigurationManager& l_rConfigurationManager=m_KernelContext->getConfigurationManager();

		// l_rConfigurationManager.addConfigurationFromFile(l_rConfigurationManager.expand("${Path_Data}/applications/acquisition-server/acquisition-server-defaults.conf"));

		// User configuration mods
		l_rConfigurationManager.addConfigurationFromFile(l_rConfigurationManager.expand("${Path_UserData}/openvibe-tracker.conf"));

		// File pointed to by --config flag overrides earlier
		for (auto it : config.m_vFlag)
		{
			if (it.first == CommandLineFlag_Config)
			{
				l_rConfigurationManager.addConfigurationFromFile(l_rConfigurationManager.expand(it.second.c_str()));
			}
		}
		// Explicit --define tokens override all earlier
		for (auto it : config.m_oTokenMap)
		{
			l_rConfigurationManager.addOrReplaceConfigurationToken(it.first.c_str(), it.second.c_str());
		}

		// Load all the plugins. Note that most are not needed by tracker, but will avoid some confusion
		// when somebody adds a plugin
		m_KernelContext->getPluginManager().addPluginsFromFiles(l_rConfigurationManager.expand("${Kernel_Plugins}"));

		return true;
	}
	
	IKernelContext* m_KernelContext;
	IKernelDesc* m_KernelDesc;
	CKernelLoader m_KernelLoader;

};

int main(int argc, char *argv[])
{
	SConfiguration l_oConfiguration;
	bool bArgParseResult = parse_arguments(argc, argv, l_oConfiguration);
	if (!bArgParseResult )
	{
		if (l_oConfiguration.m_help)
		{
			user_info(argv, NULL);
			return 0;
		}
	}

	KernelWrapper kernelWrapper;

	if(!kernelWrapper.initialize(l_oConfiguration))
	{
		return 1;
	}

	Tracker app(*kernelWrapper.m_KernelContext);

	GUI gui(argc, argv, app);

	// We initialize the app after launching the GUI so we get the log into the GUI as early as possible
	app.initialize();

	bool retVal = gui.run();

	return (retVal == true ? 0 : 1);
}

