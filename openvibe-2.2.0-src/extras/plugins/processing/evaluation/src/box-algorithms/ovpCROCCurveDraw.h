#ifndef __OpenViBEPlugins_BoxAlgorithm_ROCCurveDraw_H__
#define __OpenViBEPlugins_BoxAlgorithm_ROCCurveDraw_H__

#if defined(TARGET_HAS_ThirdPartyGTK)

#include "../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <gtk/gtk.h>
#include <vector>

namespace OpenViBEPlugins
{
	namespace Evaluation
	{
		typedef std::pair < OpenViBE::float64, OpenViBE::float64 > CCoordinate;

		//The aim of the class is to handle the graphical part of a RocCurve
		class CROCCurveDraw{

		public:
			CROCCurveDraw(::GtkNotebook* pNotebook, OpenViBE::uint32 ui32ClassIndex, OpenViBE::CString& rClassName);
			virtual ~CROCCurveDraw();
			std::vector < CCoordinate >& getCoordinateVector();

			void generateCurve();

			//Callbak functions, should not be called
			void resizeEvent(::GdkRectangle *pRectangle);
			void exposeEnvent();

			//This function is called when the cruve should be redraw for an external reason
			void forceRedraw(void);

		private:
			OpenViBE::uint32 m_ui32Margin;
			OpenViBE::uint32 m_ui32ClassIndex;
			std::vector <GdkPoint> m_oPointList;
			std::vector < CCoordinate > m_oCoordinateList;
			OpenViBE::uint64 m_ui64PixelsPerLeftRulerLabel;

			::GtkWidget *m_pDrawableArea;
			OpenViBE::boolean m_bHasBeenInit;

			//For a mytical reason, gtk says that the DrawableArea is not a DrawableArea unless it's been exposed at least once...
			// So we need to if the DrawableArea as been exposed
			OpenViBE::boolean m_bHasBeenExposed;

			void redraw();
			void drawLeftMark(OpenViBE::uint32 ui32W, OpenViBE::uint32 ui32H, const char* sLabel);
			void drawBottomMark(OpenViBE::uint32 ui32W, OpenViBE::uint32 ui32H, const char *sLabel);

		};
	}
}


#endif // TARGET_HAS_ThirdPartyGTK

#endif // OVPCROCCURVEDRAW_H
