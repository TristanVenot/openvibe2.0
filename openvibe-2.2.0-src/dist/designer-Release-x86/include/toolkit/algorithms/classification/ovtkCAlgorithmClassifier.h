#pragma once

#include "../ovtkTAlgorithm.h"
#include "../../ovtkIVector.h"
#include <openvibe/ov_all.h>
#include "../../ovtkIFeatureVector.h"
#include "../../ovtkIFeatureVectorSet.h"

#include <xml/IXMLNode.h>

#define OVTK_ClassId_Algorithm_Classifier                                OpenViBE::CIdentifier(0x3B910935, 0xE4DBACC4)
#define OVTK_ClassId_Algorithm_ClassifierDesc                            OpenViBE::CIdentifier(0xFDB84F2F, 0x2F5C510D)

#define OVTK_Algorithm_Classifier_InputParameterId_FeatureVector         OpenViBE::CIdentifier(0x6D69BF98, 0x1EB9EE66)  // Single vector to classify
#define OVTK_Algorithm_Classifier_InputParameterId_FeatureVectorSet      OpenViBE::CIdentifier(0x27C05927, 0x5DE9103A)	// Training set
#define OVTK_Algorithm_Classifier_InputParameterId_Configuration         OpenViBE::CIdentifier(0xA705428E, 0x5BB1CADD)  // The model
#define OVTK_Algorithm_Classifier_InputParameterId_NumberOfClasses       OpenViBE::CIdentifier(0x1B95825A, 0x24F2E949)  
#define OVTK_Algorithm_Classifier_InputParameterId_ExtraParameter        OpenViBE::CIdentifier(0x42AD6BE3, 0xF483DE3F)  // Params specific to classifier type

#define OVTK_Algorithm_Classifier_OutputParameterId_Class                OpenViBE::CIdentifier(0x8A39A7EA, 0xF2EE45C4)
#define OVTK_Algorithm_Classifier_OutputParameterId_ClassificationValues OpenViBE::CIdentifier(0xDA77D7E4, 0x766B48EA)
#define OVTK_Algorithm_Classifier_OutputParameterId_ProbabilityValues    OpenViBE::CIdentifier(0xDA77D7E4, 0x766B48EB)
#define OVTK_Algorithm_Classifier_OutputParameterId_Configuration        OpenViBE::CIdentifier(0x30590936, 0x61CE5971)

#define OVTK_Algorithm_Classifier_InputTriggerId_Train                   OpenViBE::CIdentifier(0x34684752, 0x78A46DE2)
#define OVTK_Algorithm_Classifier_InputTriggerId_Classify                OpenViBE::CIdentifier(0x843A87D8, 0x566E85A1)
#define OVTK_Algorithm_Classifier_InputTriggerId_SaveConfiguration       OpenViBE::CIdentifier(0x79750528, 0x6CC85FC1)
#define OVTK_Algorithm_Classifier_InputTriggerId_LoadConfiguration       OpenViBE::CIdentifier(0xF346BBE0, 0xADAFC735)

#define OVTK_Algorithm_Classifier_OutputTriggerId_Success                OpenViBE::CIdentifier(0x24FAB755, 0x78868782)
#define OVTK_Algorithm_Classifier_OutputTriggerId_Failed                 OpenViBE::CIdentifier(0x6E72B255, 0x317FAA04)


namespace OpenViBEToolkit
{
	class OVTK_API CAlgorithmClassifier : public OpenViBEToolkit::TAlgorithm < OpenViBE::Plugins::IAlgorithm >
	{
	public:

		virtual OpenViBE::boolean initialize();
		virtual OpenViBE::boolean uninitialize();

		virtual void release(void) { delete this; }

		virtual OpenViBE::boolean process(void);

		virtual OpenViBE::boolean train(const OpenViBEToolkit::IFeatureVectorSet& featureVectorSet) = 0;
		virtual OpenViBE::boolean classify(const OpenViBEToolkit::IFeatureVector& featureVector
		                                   ,OpenViBE::float64& estimatedClass
		                                   ,OpenViBEToolkit::IVector& distanceValue
		                                   ,OpenViBEToolkit::IVector& probabilityValue) = 0;

		virtual XML::IXMLNode* saveConfiguration(void)=0;
		virtual OpenViBE::boolean loadConfiguration(XML::IXMLNode * configurationRoot)=0;

		virtual OpenViBE::uint32 getOutputProbabilityVectorLength(void) =0;
		virtual OpenViBE::uint32 getOutputDistanceVectorLength(void) =0;

		_IsDerivedFromClass_(OpenViBEToolkit::TAlgorithm < OpenViBE::Plugins::IAlgorithm >, OVTK_ClassId_Algorithm_Classifier);

	protected:
		OpenViBE::boolean initializeExtraParameterMechanism();
		OpenViBE::boolean uninitializeExtraParameterMechanism();

		OpenViBE::uint64 getUInt64Parameter(const OpenViBE::CIdentifier& parameterIdentifier);
		OpenViBE::int64 getInt64Parameter(const OpenViBE::CIdentifier& parameterIdentifier);
		OpenViBE::float64 getFloat64Parameter(const OpenViBE::CIdentifier& parameterIdentifier);
		OpenViBE::boolean getBooleanParameter(const OpenViBE::CIdentifier& parameterIdentifier);
		OpenViBE::CString* getCStringParameter(const OpenViBE::CIdentifier& parameterIdentifier);
		OpenViBE::uint64 getEnumerationParameter(const OpenViBE::CIdentifier& parameterIdentifier, const OpenViBE::CIdentifier& enumerationIdentifier);

	private:
		OpenViBE::CString& getParameterValue(const OpenViBE::CIdentifier& parameterIdentifier);
		void setMatrixOutputDimension(OpenViBE::Kernel::TParameterHandler < OpenViBE::IMatrix* > &matrix, OpenViBE::uint32 length);

		OpenViBE::Kernel::IAlgorithmProxy *m_AlgorithmProxy;
		void* m_ExtraParametersMap;


	};

	class OVTK_API CAlgorithmClassifierDesc : public OpenViBE::Plugins::IAlgorithmDesc
	{
	public:

		virtual OpenViBE::boolean getAlgorithmPrototype(
			OpenViBE::Kernel::IAlgorithmProto& algorithmPrototype) const
		{
			algorithmPrototype.addInputParameter (OVTK_Algorithm_Classifier_InputParameterId_FeatureVector,         "Feature vector",        OpenViBE::Kernel::ParameterType_Matrix);
			algorithmPrototype.addInputParameter (OVTK_Algorithm_Classifier_InputParameterId_FeatureVectorSet,      "Feature vector set",    OpenViBE::Kernel::ParameterType_Matrix);
			algorithmPrototype.addInputParameter (OVTK_Algorithm_Classifier_InputParameterId_Configuration,         "Configuration",         OpenViBE::Kernel::ParameterType_Pointer);
			algorithmPrototype.addInputParameter (OVTK_Algorithm_Classifier_InputParameterId_NumberOfClasses,       "Number of classes",     OpenViBE::Kernel::ParameterType_UInteger);
			algorithmPrototype.addInputParameter (OVTK_Algorithm_Classifier_InputParameterId_ExtraParameter,        "Extra parameter",       OpenViBE::Kernel::ParameterType_Pointer);

			algorithmPrototype.addOutputParameter(OVTK_Algorithm_Classifier_OutputParameterId_Class,                "Class",                 OpenViBE::Kernel::ParameterType_Float);
			algorithmPrototype.addOutputParameter(OVTK_Algorithm_Classifier_OutputParameterId_ClassificationValues, "Hyperplane distance",   OpenViBE::Kernel::ParameterType_Matrix);
			algorithmPrototype.addOutputParameter(OVTK_Algorithm_Classifier_OutputParameterId_ProbabilityValues,    "Probability values",    OpenViBE::Kernel::ParameterType_Matrix);
			algorithmPrototype.addOutputParameter(OVTK_Algorithm_Classifier_OutputParameterId_Configuration,        "Configuration",         OpenViBE::Kernel::ParameterType_Pointer);

			algorithmPrototype.addInputTrigger   (OVTK_Algorithm_Classifier_InputTriggerId_Train,                   "Train");
			algorithmPrototype.addInputTrigger   (OVTK_Algorithm_Classifier_InputTriggerId_Classify,                "Classify");
			algorithmPrototype.addInputTrigger   (OVTK_Algorithm_Classifier_InputTriggerId_LoadConfiguration,       "Load configuration");
			algorithmPrototype.addInputTrigger   (OVTK_Algorithm_Classifier_InputTriggerId_SaveConfiguration,       "Save configuration");

			algorithmPrototype.addOutputTrigger  (OVTK_Algorithm_Classifier_OutputTriggerId_Success,                "Success");
			algorithmPrototype.addOutputTrigger  (OVTK_Algorithm_Classifier_OutputTriggerId_Failed,                 "Failed");


			return true;
		}

		_IsDerivedFromClass_(OpenViBE::Plugins::IAlgorithmDesc, OVTK_ClassId_Algorithm_ClassifierDesc);
	};
};

