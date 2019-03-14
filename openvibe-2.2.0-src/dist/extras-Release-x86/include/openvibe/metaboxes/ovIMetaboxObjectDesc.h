#pragma once

#include "../plugins/ovIBoxAlgorithmDesc.h"

namespace OpenViBE
{
	namespace Metabox
	{
		class OV_API IMetaboxObjectDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:
			virtual OpenViBE::CString getMetaboxDescriptor(void) const = 0;

			_IsDerivedFromClass_(OpenViBE::Plugins::IBoxAlgorithmDesc, OV_ClassId_Metaboxes_MetaboxObjectDesc)
		};
	};
};
