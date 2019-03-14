#include "ovp_defines.h"

#include "algorithms/ovpCAlgorithmLevelMeasure.h"

//Presentation
#include "box-algorithms/ovpCGrazVisualization.h"
#include "box-algorithms/ovpCBoxAlgorithmP300SpellerVisualization.h"
#include "box-algorithms/ovpCBoxAlgorithmP300MagicCardVisualization.h"
#include "box-algorithms/ovpCBoxAlgorithmP300IdentifierCardVisualization.h"
#include "box-algorithms/ovpCDisplayCueImage.h"
//#include "box-algorithms/ovpCBoxAlgorithmJeuPong.h"
//2D plugins
#include "box-algorithms/ovpCSignalDisplay.h"
#include "box-algorithms/ovpCBoxAlgorithmLevelMeasure.h"

#include "ovpCBoxAlgorithmErpPlot.h"

OVP_Declare_Begin()

	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OV_TypeId_BoxAlgorithmFlag, OV_AttributeId_Box_FlagIsUnstable.toString(), OV_AttributeId_Box_FlagIsUnstable.toUInteger());

	rPluginModuleContext.getTypeManager().registerEnumerationType (OVP_TypeId_SphericalLinearInterpolationType, "Spherical linear interpolation type");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_SphericalLinearInterpolationType, "Spline (potentials)", OVP_TypeId_SphericalLinearInterpolationType_Spline);
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_SphericalLinearInterpolationType, "Spline laplacian (currents)",  OVP_TypeId_SphericalLinearInterpolationType_Laplacian);

	rPluginModuleContext.getTypeManager().registerEnumerationType (OVP_TypeId_SignalDisplayMode, "Signal display mode");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_SignalDisplayMode, "Scroll", OVP_TypeId_SignalDisplayMode_Scroll.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_SignalDisplayMode, "Scan",  OVP_TypeId_SignalDisplayMode_Scan.toUInteger());

	rPluginModuleContext.getTypeManager().registerEnumerationType (OVP_TypeId_SignalDisplayScaling, "Signal display y scaling");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_SignalDisplayScaling, OpenViBEPlugins::SimpleVisualization::CSignalDisplayView::m_vScalingModes[OVP_TypeId_SignalDisplayScaling_PerChannel], OVP_TypeId_SignalDisplayScaling_PerChannel);
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_SignalDisplayScaling, OpenViBEPlugins::SimpleVisualization::CSignalDisplayView::m_vScalingModes[OVP_TypeId_SignalDisplayScaling_Global]    , OVP_TypeId_SignalDisplayScaling_Global);
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_SignalDisplayScaling, OpenViBEPlugins::SimpleVisualization::CSignalDisplayView::m_vScalingModes[OVP_TypeId_SignalDisplayScaling_None]      , OVP_TypeId_SignalDisplayScaling_None);

	OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CAlgorithmLevelMeasureDesc)

	OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CGrazVisualizationDesc)
	OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CBoxAlgorithmP300SpellerVisualizationDesc)
	OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CBoxAlgorithmP300MagicCardVisualizationDesc)
	OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CDisplayCueImageDesc)

	//OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CBoxAlgorithmJeuPong)

	OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CSignalDisplayDesc)
//	OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CTimeFrequencyMapDisplayDesc)
//	OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CPowerSpectrumDisplayDesc)
	OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CBoxAlgorithmLevelMeasureDesc)

//	OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CSimple3DDisplayDesc)
//	OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CTopographicMap3DDisplayDesc)
//	OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CVoxelDisplayDesc)
	OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CBoxAlgorithmP300IdentifierCardVisualizationDesc)

	OVP_Declare_New(OpenViBEPlugins::SimpleVisualization::CBoxAlgorithmErpPlotDesc)

OVP_Declare_End()
