#include "ovp_defines.h"

#include "box-algorithms/ovpCBoxAlgorithmNoiseGenerator.h"
#include "box-algorithms/ovpCSinusSignalGenerator.h"
#include "box-algorithms/ovpCBoxAlgorithmChannelUnitsGenerator.h"

OVP_Declare_Begin();

	OVP_Declare_New(OpenViBEPlugins::DataGeneration::CNoiseGeneratorDesc);
	rPluginModuleContext.getTypeManager().registerEnumerationType (OVP_TypeId_NoiseType, "Noise type");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_NoiseType, "Uniform",           OVP_TypeId_NoiseType_Uniform.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_NoiseType, "Gaussian",          OVP_TypeId_NoiseType_Gaussian.toUInteger());

	OVP_Declare_New(OpenViBEPlugins::DataGeneration::CSinusSignalGeneratorDesc);
	OVP_Declare_New(OpenViBEPlugins::DataGeneration::CChannelUnitsGeneratorDesc);



OVP_Declare_End();
