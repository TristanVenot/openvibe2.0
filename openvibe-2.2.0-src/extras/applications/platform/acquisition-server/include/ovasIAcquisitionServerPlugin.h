#ifndef __OpenViBE_AcquisitionServer_IAcquisitionServerPlugin_H__
#define __OpenViBE_AcquisitionServer_IAcquisitionServerPlugin_H__

#include "ovas_base.h"

#include "../ovasCSettingsHelper.h"
#include "../ovasCSettingsHelperOperators.h"

#include "boost/variant.hpp"
#include <map>
#include <deque>

/**
  * \brief Interface for acquisition server plugins
  *
  * Contains an interface to the acquisition server plugins. Any plugin must inherit from this class in order to be able to register with the acquisition server.
  */

namespace OpenViBEAcquisitionServer
{
	class CAcquisitionServer;

	class IAcquisitionServerPlugin
	{
		public:
			// Interface of the plugin. To develop a new plugin override any of the Hook functions in your implementation.

			/// Hook called at the end of the AcquisitionServer constructor
			virtual void createHook() {}

			/// Hook called at the end of the start() function of AcquisitionServer. At this point the device has been connected to,
			/// and signal properties should already be correct.
			/// This function should return false if start failed.
			virtual bool startHook(const std::vector<OpenViBE::CString>& vSelectedChannelNames, OpenViBE::uint32 ui32SamplingFrequency, OpenViBE::uint32 ui32ChannelCount, OpenViBE::uint32 ui32SampleCountPerSentBlock) { return true; }

			/// Hook called at the end of the stop() function of AcquisitionServer
			virtual void stopHook() {}


			/** \brief Hook called in the loop() function of AcquisitionServer
			  *
			  * This hook is called before sending the stimulations or signal to the connected clients.
			  * It gets a reference to the current signal buffer and the stimulation set with its start and end dates.
			  *
			  * Note that the given input buffer may have more samples than what should be processed 
			  * per iteration. All operations on vPendingBuffer done in the hook should only consider
			  * the first sampleCountSentPerBlock samples. The later samples should be left as-is
			  * and will be provided on the next call.
			  */
			virtual void loopHook(std::deque < std::vector < OpenViBE::float32 > >& vPendingBuffer, 
								  OpenViBE::CStimulationSet& oStimulationSet, 
								  OpenViBE::uint64 start, 
								  OpenViBE::uint64 end,
								  OpenViBE::uint64 sampleTime) {}

			/// Hook called at the end of the acceptNewConnection() function of AcquisitionServer
			virtual void acceptNewConnectionHook() {}

		public:

			IAcquisitionServerPlugin(const OpenViBE::Kernel::IKernelContext& rKernelContext, const OpenViBE::CString &name) :
				m_rKernelContext(rKernelContext), m_oSettingsHelper(name, rKernelContext.getConfigurationManager())
			{}

			virtual ~IAcquisitionServerPlugin() {}

		public:
			const SettingsHelper& getSettingsHelper() const { return m_oSettingsHelper; }
			SettingsHelper& getSettingsHelper() { return m_oSettingsHelper; }

		protected:
			const OpenViBE::Kernel::IKernelContext& m_rKernelContext;
			SettingsHelper m_oSettingsHelper;

	};
}

#endif // __OpenViBE_AcquisitionServer_IAcquisitionServerPlugin_H__