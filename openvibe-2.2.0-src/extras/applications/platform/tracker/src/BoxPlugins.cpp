
#include "BoxPlugins.h"
#include "ovkCBoxProto.h"

using namespace OpenViBE;
using namespace OpenViBETracker;

#define OVP_ClassId_BoxAlgorithm_TemporalFilter       OpenViBE::CIdentifier(0xB4F9D042, 0x9D79F2E5)
#define OVP_ClassId_BoxAlgorithm_TemporalFilterDesc   OpenViBE::CIdentifier(0x7BF6BA62, 0xAF829A37)

#define OVP_ClassId_BoxAlgorithm_SignalResampling     OpenViBE::CIdentifier(0x0E923A5E, 0xDA474058)
#define OVP_ClassId_BoxAlgorithm_SignalResamplingDesc OpenViBE::CIdentifier(0xA675A433, 0xC6690920)

#define OVP_ClassId_BoxAlgorithm_SimpleDSP            OpenViBE::CIdentifier(0x00E26FA1, 0x1DBAB1B2)
#define OVP_ClassId_BoxAlgorithm_SimpleDSPDesc        OpenViBE::CIdentifier(0x00C44BFE, 0x76C9269E)

#define OVP_ClassId_BoxAlgorithm_Crop                 OpenViBE::CIdentifier(0x7F1A3002, 0x358117BA)
#define OVP_ClassId_BoxAlgorithm_CropDesc             OpenViBE::CIdentifier(0x64D619D7, 0x26CC42C9)

#define OVP_ClassId_BoxAlgorithm_SpatialFilter        OpenViBE::CIdentifier(0xDD332C6C, 0x195B4FD4)
#define OVP_ClassId_BoxAlgorithm_SpatialFilterDesc    OpenViBE::CIdentifier(0x72A01C92, 0xF8C1FA24)

#define OVP_ClassId_BoxAlgorithm_TimeBasedEpoching     OpenViBE::CIdentifier(0x00777FA0, 0x5DC3F560)
#define OVP_ClassId_BoxAlgorithm_TimeBasedEpochingDesc OpenViBE::CIdentifier(0x00ABDABE, 0x41381683)

#define OVP_ClassId_BoxAlgorithm_StimulationFilter     OpenViBE::CIdentifier(0x02F96101, 0x5E647CB8)
#define OVP_ClassId_BoxAlgorithm_StimulationFilterDesc OpenViBE::CIdentifier(0x4D2A23FC, 0x28191E18)

#define OVP_ClassId_FastICA                                                             OpenViBE::CIdentifier(0x00649B6E, 0x6C88CD17)
#define OVP_ClassId_FastICADesc                                                         OpenViBE::CIdentifier(0x00E9436C, 0x41C904CA)

#define OVP_ClassId_BoxAlgorithm_CommonAverageReference     OpenViBE::CIdentifier(0x009C0CE3, 0x6BDF71C3)
#define OVP_ClassId_BoxAlgorithm_CommonAverageReferenceDesc OpenViBE::CIdentifier(0x0033EAF8, 0x09C65E4E)

#define OVP_ClassId_BoxAlgorithm_ChannelSelector     OpenViBE::CIdentifier(0x361722E8, 0x311574E8)
#define OVP_ClassId_BoxAlgorithm_ChannelSelectorDesc OpenViBE::CIdentifier(0x67633C1C, 0x0D610CD8)

#define OVP_ClassId_BoxAlgorithm_ChannelRename     OpenViBE::CIdentifier(0x1FE50479, 0x39040F40)
#define OVP_ClassId_BoxAlgorithm_ChannelRenameDesc OpenViBE::CIdentifier(0x20EA1F00, 0x7AED5645)

#define OVP_ClassId_BoxAlgorithm_ReferenceChannel     OpenViBE::CIdentifier(0x444721AD, 0x78342CF5)
#define OVP_ClassId_BoxAlgorithm_ReferenceChannelDesc OpenViBE::CIdentifier(0x42856103, 0x45B125AD)

#define OVP_ClassId_BoxAlgorithm_FrequencyBandSelector     OpenViBE::CIdentifier(0x140C19C6, 0x4E6E187B)
#define OVP_ClassId_BoxAlgorithm_FrequencyBandSelectorDesc OpenViBE::CIdentifier(0x13462C56, 0x794E3C07)



#include <algorithm>

BoxPlugins::BoxPlugins(const OpenViBE::Kernel::IKernelContext& ctx) :
	Contexted(ctx)
{
	// Register some boxes that can be used as filters

	create(OV_TypeId_Signal, OVP_ClassId_BoxAlgorithm_TemporalFilter, OVP_ClassId_BoxAlgorithm_TemporalFilterDesc);
	create(OV_TypeId_Signal, OVP_ClassId_BoxAlgorithm_SimpleDSP, OVP_ClassId_BoxAlgorithm_SimpleDSPDesc);
	create(OV_TypeId_Signal, OVP_ClassId_BoxAlgorithm_TimeBasedEpoching, OVP_ClassId_BoxAlgorithm_TimeBasedEpochingDesc);
	create(OV_TypeId_Signal, OVP_ClassId_BoxAlgorithm_Crop, OVP_ClassId_BoxAlgorithm_CropDesc);
	create(OV_TypeId_Signal, OVP_ClassId_BoxAlgorithm_SpatialFilter, OVP_ClassId_BoxAlgorithm_SpatialFilterDesc);
	create(OV_TypeId_Signal, OVP_ClassId_BoxAlgorithm_SignalResampling, OVP_ClassId_BoxAlgorithm_SignalResamplingDesc);
	create(OV_TypeId_Signal, OVP_ClassId_FastICA, OVP_ClassId_FastICADesc);
	create(OV_TypeId_Signal, OVP_ClassId_BoxAlgorithm_CommonAverageReference, OVP_ClassId_BoxAlgorithm_CommonAverageReferenceDesc);

	create(OV_TypeId_Signal, OVP_ClassId_BoxAlgorithm_ChannelSelector, OVP_ClassId_BoxAlgorithm_ChannelSelectorDesc);
	create(OV_TypeId_Signal, OVP_ClassId_BoxAlgorithm_ChannelRename, OVP_ClassId_BoxAlgorithm_ChannelRenameDesc);
	create(OV_TypeId_Signal, OVP_ClassId_BoxAlgorithm_ReferenceChannel, OVP_ClassId_BoxAlgorithm_ReferenceChannelDesc);

	create(OV_TypeId_Spectrum, OVP_ClassId_BoxAlgorithm_FrequencyBandSelector, OVP_ClassId_BoxAlgorithm_FrequencyBandSelectorDesc);

	create(OV_TypeId_Stimulations, OVP_ClassId_BoxAlgorithm_StimulationFilter, OVP_ClassId_BoxAlgorithm_StimulationFilterDesc);

	std::sort(m_BoxPlugins.begin(), m_BoxPlugins.end(),
		[](BoxAdapterStream* a, BoxAdapterStream *b) { return (a->getBox().getName()) < (b->getBox().getName()); }
	);

}

BoxPlugins::~BoxPlugins(void)
{
	for(auto ptr : m_BoxPlugins)
	{
		delete ptr;
	}
}

bool BoxPlugins::create(const OpenViBE::CIdentifier& streamType, const OpenViBE::CIdentifier& alg, const OpenViBE::CIdentifier& desc)
{
	BoxAdapterStream* ptr = new BoxAdapterStream(m_KernelContext, alg);

	// Initialize the context by polling the descriptor
	const Plugins::IPluginObjectDesc* l_pPluginObjectDescriptor = getKernelContext().getPluginManager().getPluginObjectDesc(desc);
	if (!l_pPluginObjectDescriptor)
	{
		log() << OpenViBE::Kernel::LogLevel_Error << "Unable to load box algorithm " << alg.toString() << "\n";
		return false;
	}

	const Plugins::IBoxAlgorithmDesc* pBoxAlgorithmDescriptor = dynamic_cast<const Plugins::IBoxAlgorithmDesc*>(l_pPluginObjectDescriptor);
	OpenViBE::Kernel::CBoxProto boxProto(m_KernelContext, ptr->getBox());
	pBoxAlgorithmDescriptor->getBoxPrototype(boxProto);

	// We need to force this so we don't launch the plugin on streams of other types, and not all boxes declare 
	// their capabilities. Here we assume that if the box is created for this stream type, it can support it.
	boxProto.addInputSupport(streamType);

	CIdentifier typeId;
	ptr->getBox().getInputType(0, typeId);
	if (typeId != streamType) 
	{
		ptr->getBox().setInputType(0, streamType);
	}
	ptr->getBox().getOutputType(0, typeId);
	if (typeId != streamType)
	{
		ptr->getBox().setOutputType(0, streamType);
	}

	const CString boxName = m_KernelContext.getTypeManager().getTypeName(streamType) + CString(" : ") + pBoxAlgorithmDescriptor->getName();
	ptr->getBox().setName(boxName);

	m_BoxPlugins.push_back(ptr);

	return true;
}
