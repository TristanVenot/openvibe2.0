#ifndef __SamplePlugin_CNoiseGenerator_H__
#define __SamplePlugin_CNoiseGenerator_H__

#include "../ovp_defines.h"

#include <random>

#include <toolkit/ovtk_all.h>

#define OVP_TypeId_NoiseType                                                    OpenViBE::CIdentifier(0x2E85E95E, 0x8A1A8365)
#define OVP_TypeId_NoiseType_Uniform                                            OpenViBE::CIdentifier(0x00000000, 0x00000001)
#define OVP_TypeId_NoiseType_Gaussian                                           OpenViBE::CIdentifier(0x00000000, 0x00000002)

namespace OpenViBEPlugins
{
	namespace DataGeneration
	{
		class CNoiseGenerator : public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:

			CNoiseGenerator(void);

			virtual void release(void);

			virtual OpenViBE::uint64 getClockFrequency(void);
			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);

			virtual OpenViBE::boolean processClock(OpenViBE::Kernel::IMessageClock& rMessageClock);
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>, OVP_ClassId_NoiseGenerator)

		protected:


			OpenViBEToolkit::TSignalEncoder < CNoiseGenerator > m_oSignalEncoder;

			OpenViBE::boolean m_bHeaderSent;
			OpenViBE::uint64 m_ui64ChannelCount;
			OpenViBE::uint64 m_ui64SamplingFrequency;
			OpenViBE::uint64 m_ui64GeneratedEpochSampleCount;
			OpenViBE::uint64 m_ui64SentSampleCount;
			OpenViBE::uint64 m_ui64NoiseType;

			std::mt19937 m_oGenerator;

			std::normal_distribution<> m_oNormalDistribution{0.0,1.0};         // Mean=0, Var=1
			std::uniform_real_distribution<> m_oUniformDistribution{0.0,1.0};  // Min=0, Max=1
		};

		class CNoiseGeneratorDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }
			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Noise generator"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Jussi T. Lindgren"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Inria"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Simple random noise generator"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Generates uniform or Gaussian random data"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Data generation"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.1"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_NoiseGenerator; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::DataGeneration::CNoiseGenerator(); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-execute"); }

			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rPrototype) const
			{
				rPrototype.addOutput("Generated signal",              OV_TypeId_Signal);

				rPrototype.addSetting("Channel count",                OV_TypeId_Integer, "4");
				rPrototype.addSetting("Sampling frequency",           OV_TypeId_Integer, "512");
				rPrototype.addSetting("Generated epoch sample count", OV_TypeId_Integer, "32");
				rPrototype.addSetting("Noise type",                   OVP_TypeId_NoiseType, OVP_TypeId_NoiseType_Uniform.toString());

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_NoiseGeneratorDesc)
		};
	};
};

#endif // __SamplePlugin_CNoiseGenerator_H__
