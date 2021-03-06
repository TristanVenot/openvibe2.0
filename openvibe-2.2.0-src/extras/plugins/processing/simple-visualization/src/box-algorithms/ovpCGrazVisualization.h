#ifndef __OpenViBEPlugins_SimpleVisualization_CGrazVisualization_H__
#define __OpenViBEPlugins_SimpleVisualization_CGrazVisualization_H__

#include "../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <visualization-toolkit/ovviz_all.h>

#include <gtk/gtk.h>

#include <vector>
#include <string>
#include <map>
#include <deque>

namespace TCPTagging
{
	class IStimulusSender; // fwd declare
};

namespace OpenViBEPlugins
{
	namespace SimpleVisualization
	{

		enum EArrowDirection
		{
			EArrowDirection_None	= 0,
			EArrowDirection_Left,
			EArrowDirection_Right,
			EArrowDirection_Up,
			EArrowDirection_Down,
		};

		enum EGrazVisualizationState
		{
			EGrazVisualizationState_Idle,
			EGrazVisualizationState_Reference,
			EGrazVisualizationState_Cue,
			EGrazVisualizationState_ContinousFeedback
		};

		/**
		*/
		class CGrazVisualization :
			virtual public OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>
		{
		public:

			CGrazVisualization(void);

			virtual void release(void) { delete this; }

			virtual bool initialize();
			virtual bool uninitialize();
			virtual bool processInput(OpenViBE::uint32 ui32InputIndex);
			virtual bool process();

			virtual void redraw();
			virtual void resize(OpenViBE::uint32 ui32Width, OpenViBE::uint32 ui32Height);

		public:

			void flushQueue(void);					// Sends all accumulated stimuli to the TCP Tagging

		protected:

			virtual void setStimulation(const OpenViBE::uint32 ui32StimulationIndex, const OpenViBE::uint64 ui64StimulationIdentifier, const OpenViBE::uint64 ui64StimulationDate);

			virtual void setMatrixBuffer(const OpenViBE::float64* pBuffer);

			virtual void processState();


			virtual void drawReferenceCross();
			virtual void drawArrow(EArrowDirection eDirection);
			virtual void drawBar();
			virtual void drawAccuracy();
			virtual void updateConfusionMatrix(OpenViBE::float64 f64Prediction);
			virtual OpenViBE::float64 aggregatePredictions(bool bIncludeAll);

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithm, OVP_ClassId_GrazVisualization)

		public:
			//! The Builder handler used to create the interface
			::GtkBuilder* m_pBuilderInterface;

			GtkWidget * m_pMainWindow;

			GtkWidget * m_pDrawingArea;

			//ebml
			OpenViBEToolkit::TStimulationDecoder<CGrazVisualization> m_oStimulationDecoder;
			OpenViBEToolkit::TStreamedMatrixDecoder<CGrazVisualization> m_oMatrixDecoder;

			EGrazVisualizationState m_eCurrentState;
			EArrowDirection m_eCurrentDirection;

			OpenViBE::float64 m_f64MaxAmplitude;
			OpenViBE::float64 m_f64BarScale;

			//Start and end time of the last buffer
			OpenViBE::uint64 m_ui64StartTime;
			OpenViBE::uint64 m_ui64EndTime;

			bool m_bTwoValueInput;

			GdkPixbuf * m_pOriginalBar;
			GdkPixbuf * m_pLeftBar;
			GdkPixbuf * m_pRightBar;

			GdkPixbuf * m_pOriginalLeftArrow;
			GdkPixbuf * m_pOriginalRightArrow;
			GdkPixbuf * m_pOriginalUpArrow;
			GdkPixbuf * m_pOriginalDownArrow;

			GdkPixbuf * m_pLeftArrow;
			GdkPixbuf * m_pRightArrow;
			GdkPixbuf * m_pUpArrow;
			GdkPixbuf * m_pDownArrow;

			GdkColor m_oBackgroundColor;
			GdkColor m_oForegroundColor;

			std::deque<OpenViBE::float64> m_vAmplitude; // predictions for the current trial

			bool m_bShowInstruction;
			bool m_bShowFeedback;
			bool m_bDelayFeedback;
			bool m_bShowAccuracy;
			bool m_bPositiveFeedbackOnly;

			OpenViBE::uint64 m_i64PredictionsToIntegrate;

			OpenViBE::CMatrix m_oConfusion;
			
			// For queuing stimulations to the TCP Tagging
			std::vector< OpenViBE::uint64 > m_vStimuliQueue;
			guint m_uiIdleFuncTag;
			TCPTagging::IStimulusSender* m_pStimulusSender;

			OpenViBE::uint64 m_ui64LastStimulation;

		private:
			OpenViBEVisualizationToolkit::IVisualizationContext* m_visualizationContext = nullptr;
		};

		/**
		* Plugin's description
		*/
		class CGrazVisualizationDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:
			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Graz visualization"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Bruno Renier, Jussi T. Lindgren"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA/IRISA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Visualization plugin for the Graz experiment"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Visualization/Feedback plugin for the Graz experiment"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Visualization/Presentation"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.2"); }
			virtual void release(void)                                   { }
			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_GrazVisualization; }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-fullscreen"); }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SimpleVisualization::CGrazVisualization(); }

			virtual bool hasFunctionality(OpenViBE::CIdentifier functionalityIdentifier) const
			{
				return functionalityIdentifier == OVD_Functionality_Visualization;
			}

			virtual bool getBoxPrototype(OpenViBE::Kernel::IBoxProto& rPrototype) const
			{
				rPrototype.addInput("Stimulations", OV_TypeId_Stimulations);
				rPrototype.addInput("Amplitude", OV_TypeId_StreamedMatrix);

				rPrototype.addSetting("Show instruction", OV_TypeId_Boolean,            "true");
				rPrototype.addSetting("Show feedback", OV_TypeId_Boolean,               "false");
				rPrototype.addSetting("Delay feedback", OV_TypeId_Boolean,              "false");
				rPrototype.addSetting("Show accuracy", OV_TypeId_Boolean,               "false");
				rPrototype.addSetting("Predictions to integrate", OV_TypeId_Integer,    "5");
				rPrototype.addSetting("Positive feedback only", OV_TypeId_Boolean,      "false");

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_GrazVisualizationDesc)
		};

	};
};

#endif
