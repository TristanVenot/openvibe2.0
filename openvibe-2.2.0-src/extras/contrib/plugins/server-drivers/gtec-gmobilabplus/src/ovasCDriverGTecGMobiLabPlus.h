/**
 * The gMobilab driver was contributed
 * by Lucie Daubigney from Supelec Metz
 */

#ifndef __OpenViBE_AcquisitionServer_CDriverGTecGMobiLabPlus_H__
#define __OpenViBE_AcquisitionServer_CDriverGTecGMobiLabPlus_H__

#if defined TARGET_HAS_ThirdPartyGMobiLabPlusAPI

#include "ovasIDriver.h"
#include "../ovasCHeader.h"

#include "../ovasCSettingsHelper.h"
#include "../ovasCSettingsHelperOperators.h"

#include <string>
#ifdef TARGET_OS_Windows
#include <Windows.h>
#endif

namespace OpenViBEAcquisitionServer
{
	class CDriverGTecGMobiLabPlusPrivate; // fwd declare

	/**
	 * \class CDriverGTecGMobiLabPlus
	 * \author Lucie Daubigney (Supelec Metz)
	 */
	class CDriverGTecGMobiLabPlus : public OpenViBEAcquisitionServer::IDriver
	{
	public:

		CDriverGTecGMobiLabPlus(OpenViBEAcquisitionServer::IDriverContext& rDriverContext);
		virtual ~CDriverGTecGMobiLabPlus(void);
		virtual void release(void);
		virtual const char* getName(void);

		virtual OpenViBE::boolean isFlagSet(
			const OpenViBEAcquisitionServer::EDriverFlag eFlag) const
		{
			return eFlag==DriverFlag_IsUnstable;
		}

		//configuration
		virtual OpenViBE::boolean isConfigurable(void);
		virtual OpenViBE::boolean configure(void);
		//initialisation
		virtual OpenViBE::boolean initialize(const OpenViBE::uint32 ui32SampleCountPerChannel, OpenViBEAcquisitionServer::IDriverCallback& rCallback);
		virtual OpenViBE::boolean uninitialize(void);
		virtual const OpenViBEAcquisitionServer::IHeader* getHeader(void);

		//acquisition
		virtual OpenViBE::boolean start(void);
		virtual OpenViBE::boolean stop(void);
		virtual OpenViBE::boolean loop(void);

	protected:

		SettingsHelper m_oSettings;

		//usefull data to communicate with OpenViBE
		OpenViBEAcquisitionServer::IHeader* m_pHeader;
		OpenViBEAcquisitionServer::IDriverCallback* m_pCallback;
		OpenViBE::uint32 m_ui32SampleCountPerSentBlock;//number of sample you want to send in a row
		OpenViBE::float32* m_pSample;//array containing the data to sent to OpenViBE once they had been recovered from the gTec module

		//params
		std::string m_oPortName;
		OpenViBE::boolean m_bTestMode;
				
		// Pointers do gtec-specific data and function pointers
		OpenViBEAcquisitionServer::CDriverGTecGMobiLabPlusPrivate* m_pGtec;
		
		// Register the function pointers from the dll. (The dll approach
		// is used with gMobilab to avoid conflicts with the gUSBAmp lib)
		OpenViBE::boolean registerLibraryFunctions(void);
		
#if defined(TARGET_OS_Windows)
		HINSTANCE m_pLibrary;
#elif defined(TARGET_OS_Linux)
		void* m_pLibrary;
#endif
	private:

		void allowAnalogInputs(OpenViBE::uint32 ui32ChannelIndex);
	};
};

#endif // TARGET_HAS_ThirdPartyGMobiLabPlusAPI

#endif // __OpenViBE_AcquisitionServer_CDriverGTecGMobiLabPlus_H__
