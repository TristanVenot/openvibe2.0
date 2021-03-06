#ifndef __OpenViBEPlugins_BoxAlgorithm_ROCCurve_H__
#define __OpenViBEPlugins_BoxAlgorithm_ROCCurve_H__

#if defined(TARGET_HAS_ThirdPartyGTK)

#include "../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include "ovpCROCCurveDraw.h"

#include <gtk/gtk.h>

#include <set>
#include <map>
#include <iostream>
#include <sstream>

#include <visualization-toolkit/ovviz_all.h>

#define OVP_ClassId_BoxAlgorithm_ROCCurve OpenViBE::CIdentifier(0x06FE5B1B, 0xDE066FEC)
#define OVP_ClassId_BoxAlgorithm_ROCCurveDesc OpenViBE::CIdentifier(0xCB5DFCEA, 0xAF41EAB2)

namespace OpenViBEPlugins
{
	namespace Evaluation
	{
		typedef std::pair < OpenViBE::uint64, OpenViBE::uint64 > CTimestampLabelPair;
		typedef std::pair < OpenViBE::uint64, OpenViBE::float64* > CTimestampValuesPair;
		typedef std::pair < OpenViBE::uint64, OpenViBE::float64* > CLabelValuesPair;

		typedef std::pair < bool, OpenViBE::float64 > CRocPairValue;

		/**
		 * \class CBoxAlgorithmROCCurve
		 * \author Serrière Guillaume (Inria)
		 * \date Thu May 28 11:49:24 2015
		 * \brief The class CBoxAlgorithmROCCurve describes the box ROC curve.
		 * The roc curve is a graphical plot that represents the performance of a classifier. This curve is created by plotting the true positive
		 * rate against the false positive rate at various threshold settings.
		 *
		 */
		class CBoxAlgorithmROCCurve : virtual public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:
			virtual void release(void) { delete this; }

			virtual bool initialize(void);
			virtual bool uninitialize(void);
			virtual bool processInput(OpenViBE::uint32 ui32InputIndex);
			
			virtual bool process(void);
			

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_ROCCurve)

		private:
			bool computeROCCurves();
			bool computeOneROCCurve(const OpenViBE::CIdentifier& rClassIdentifier, OpenViBE::uint32 ui32ClassIndex);

			// Input decoder:
			OpenViBEToolkit::TStimulationDecoder < CBoxAlgorithmROCCurve > m_oExpectedDecoder;
			OpenViBEToolkit::TStreamedMatrixDecoder < CBoxAlgorithmROCCurve > m_oClassificationValueDecoder;

			std::set < OpenViBE::CIdentifier > m_oClassStimulationSet;
			OpenViBE::CIdentifier m_oComputationTrigger;

			std::vector < CTimestampLabelPair > m_oStimulationTimeline;
			std::vector < CTimestampValuesPair > m_oValueTimeline;

			std::vector< CLabelValuesPair > m_oLabelValueList;

			//Display section
			::GtkWidget* m_pWidget;
			std::vector < CROCCurveDraw* > m_oDrawerList;

		private:
				OpenViBEVisualizationToolkit::IVisualizationContext* m_visualizationContext = nullptr;
		};

		// The box listener can be used to call specific callbacks whenever the box structure changes : input added, name changed, etc.
		// Please uncomment below the callbacks you want to use.
		class CBoxAlgorithmROCCurveListener : public OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >
		{
		public:

			virtual bool onSettingValueChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) {
				if(ui32Index == 1)
				{
					OpenViBE::CString l_sClassCount;
					rBox.getSettingValue(ui32Index, l_sClassCount);
					//Could happen if we rewritte a number
					if(l_sClassCount.length() == 0 )
					{
						return true;
					}

					OpenViBE::int32 l_i32SettingCount;
					std::stringstream l_sStream(l_sClassCount.toASCIIString());
					l_sStream >> l_i32SettingCount;

					//First of all we prevent for the value to goes under 1.
					if(l_i32SettingCount < 1)
					{
						rBox.setSettingValue(ui32Index, "1");
						l_i32SettingCount = 1;
					}

					OpenViBE::int32 l_i32CurrentCount = rBox.getSettingCount()-2;
					//We have two choice 1/We need to add class, 2/We need to remove some
					if(l_i32CurrentCount < l_i32SettingCount)
					{
						while(l_i32CurrentCount < l_i32SettingCount)
						{
							std::stringstream l_sSettingName;
							l_sSettingName << "Class " << (l_i32CurrentCount+1) << " identifier";
							rBox.addSetting(l_sSettingName.str().c_str(), OVTK_TypeId_Stimulation, "");
							++l_i32CurrentCount;
						}
					}
					else
					{
						while(l_i32CurrentCount > l_i32SettingCount)
						{
							rBox.removeSetting(rBox.getSettingCount()-1);
							--l_i32CurrentCount;
						}
					}
				}
				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >, OV_UndefinedIdentifier)
		};

		
		/**
		 * \class CBoxAlgorithmROCCurveDesc
		 * \author Serrière Guillaume (Inria)
		 * \date Thu May 28 11:49:24 2015
		 * \brief Descriptor of the box ROC curve.
		 *
		 */
		class CBoxAlgorithmROCCurveDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("ROC curve"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Serrière Guillaume"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Inria"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Compute the ROC curve for each class."); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("The box computes the ROC curve for each class."); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Evaluation/Classification"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.1"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-yes"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_ROCCurve; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::Evaluation::CBoxAlgorithmROCCurve; }

			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const               { return new CBoxAlgorithmROCCurveListener; }
			virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) const { delete pBoxListener; }

			virtual bool hasFunctionality(OpenViBE::CIdentifier functionalityIdentifier) const
			{
				return functionalityIdentifier == OVD_Functionality_Visualization;
			}
			
			virtual bool getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput("Expected labels",       OV_TypeId_Stimulations);
				rBoxAlgorithmPrototype.addInput("Probability values",    OV_TypeId_StreamedMatrix);

				rBoxAlgorithmPrototype.addSetting("Computation trigger", OV_TypeId_Stimulation,  "OVTK_StimulationId_ExperimentStop");
				rBoxAlgorithmPrototype.addSetting("Number of classes",   OV_TypeId_Integer,      "2");
				rBoxAlgorithmPrototype.addSetting("Class 1 identifier" , OV_TypeId_Stimulation,  "OVTK_StimulationId_Label_01");
				rBoxAlgorithmPrototype.addSetting("Class 2 identifier" , OV_TypeId_Stimulation,  "OVTK_StimulationId_Label_02");

				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanModifySetting);
				
				rBoxAlgorithmPrototype.addFlag(OV_AttributeId_Box_FlagIsUnstable);
				
				return true;
			}
			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_ROCCurveDesc)
		};
	}
}

#endif // TARGET_HAS_ThirdPartyGTK

#endif // __OpenViBEPlugins_BoxAlgorithm_ROCCurve_H__
