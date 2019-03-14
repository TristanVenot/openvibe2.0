#include "ovpCBoxAlgorithmErpPlot.h"
#include <boost/lexical_cast.hpp>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;
using namespace OpenViBEToolkit;
using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SimpleVisualization;


namespace
{
	class _AutoCast_
	{
	public:
		_AutoCast_(const IBox& rBox, IConfigurationManager& rConfigurationManager, const uint32 ui32Index)
			: m_rConfigurationManager(rConfigurationManager)
		{
			rBox.getSettingValue(ui32Index, m_sSettingValue);
			m_sSettingValue = m_rConfigurationManager.expand(m_sSettingValue);
		}
		operator ::GdkColor (void)
		{
			::GdkColor l_oColor;
			int r=0, g=0, b=0;
			sscanf(m_sSettingValue.toASCIIString(), "%i,%i,%i", &r, &g, &b);
			l_oColor.pixel=0;
			l_oColor.red=(r*65535)/100;
			l_oColor.green=(g*65535)/100;
			l_oColor.blue=(b*65535)/100;
			// std::cout << r << " " << g << " " << b << "\n";
			return l_oColor;
		}
	protected:
		IConfigurationManager& m_rConfigurationManager;
		CString m_sSettingValue;
	};
}

static void event_handler(GtkWidget *widget, gint width, gint height, gpointer data)
{

		std::list<Graph*>* l_GraphList = reinterpret_cast<std::list<Graph*>*>(data);
		std::list<Graph*>::iterator l_Iterator;
		for ( l_Iterator=l_GraphList->begin(); l_Iterator!=l_GraphList->end(); l_Iterator++)
		{
			(*l_Iterator)->resizeAxis(width, height, l_GraphList->size());
			(*l_Iterator)->draw(widget);
		}
}

static void on_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
		//std::cout << "on_configure_event"<<event->width<<" "<< event->height<<"on widget "<<widget->allocation.width<<" "<<widget->allocation.height << "\n";
		gtk_widget_queue_draw_area(widget,0, 0,event->width, event->height );
		event_handler(widget, event->width, event->height, data);
}

/*
static gboolean on_resize_event(GtkWidget *widget,  GdkRectangle * event, gpointer data)
{
		//std::cout << "on_resize_event" << "\n";
	event_handler(widget, event->width, event->height, data);
	return TRUE;
}
//*/

static gboolean on_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	//std::cout << "on_expose_event" <<event->area.width<<" "<< event->area.height<<"on widget "<<widget->allocation.width<<" "<<widget->allocation.height<< "\n";
	//gtk_widget_queue_draw_area(widget,0, 0,event->area.width, event->area.height );
	event_handler(widget, event->area.width, event->area.height, data);

	return FALSE;
}

static void rendertext(cairo_t *cr, const char* text, double x, double y) {

		PangoLayout *layout;							// layout for a paragraph of text
		PangoFontDescription *desc;						// this structure stores a description of the style of font you'd most like
		cairo_identity_matrix(cr);
		cairo_translate(cr, x, y);						// set the origin of cairo instance 'cr' to (10,20) (i.e. this is where
																				// drawing will start from).
		layout = pango_cairo_create_layout(cr);					// init pango layout ready for use
		pango_layout_set_text(layout, text, -1);			// sets the text to be associated with the layout (final arg is length, -1
																				// to calculate automatically when passing a nul-terminated string)
		desc = pango_font_description_from_string("Sans Bold 10");		// specify the font that would be ideal for your particular use
		pango_layout_set_font_description(layout, desc);			// assign the previous font description to the layout
		pango_font_description_free(desc);					// free the description

		pango_cairo_update_layout(cr, layout);					// if the target surface or transformation properties of the cairo instance
																				// have changed, update the pango layout to reflect this
		pango_cairo_show_layout(cr, layout);					// draw the pango layout onto the cairo surface // mandatory

		g_object_unref(layout);							// free the layout
}

void Graph::resizeAxis(gint width, gint height, int nrOfGraphs)
{
	uint32 l_ui32NrOfRows = (uint32)ceil(sqrt((double)nrOfGraphs));
	uint32 l_ui32NrOfColumns = l_ui32NrOfRows;
	if ( (uint32)nrOfGraphs<=(l_ui32NrOfRows-1)*l_ui32NrOfRows)
		l_ui32NrOfRows--;
	this->m_dGraphWidth = ((double)(width))/(double)l_ui32NrOfColumns;
	this->m_dGraphHeight = ((double)(height))/(double)l_ui32NrOfRows;
	this->m_dGraphOriginX = this->m_dGraphWidth*((double)this->colIndex);
	this->m_dGraphOriginY = this->m_dGraphHeight*((double)this->rowIndex);

		//std::cout << "resizeAxis: origin x: " << m_dGraphOriginX << ", origin y: " << m_dGraphOriginY << ", width: " << m_dGraphWidth << ", height: " << m_dGraphHeight << "\n";
}

void Graph::draw(GtkWidget* widget)//cairo_t * cairoContext)
{
	cairo_t * l_pCairoContext;
	l_pCairoContext = gdk_cairo_create(widget->window);

	cairo_set_line_width (l_pCairoContext, 1);
	cairo_translate(l_pCairoContext, m_dGraphOriginX+20, m_dGraphOriginY+20);
	cairo_scale(l_pCairoContext, m_dGraphWidth-40, m_dGraphHeight-40);

	cairo_save(l_pCairoContext);
	drawAxis(l_pCairoContext);
	cairo_restore(l_pCairoContext);

	cairo_save(l_pCairoContext);
	drawLegend(l_pCairoContext);
	cairo_restore(l_pCairoContext);

	cairo_save(l_pCairoContext);
	drawVar(l_pCairoContext);
	cairo_restore(l_pCairoContext);

	cairo_save(l_pCairoContext);
	drawCurves(l_pCairoContext);
	cairo_restore(l_pCairoContext);

	cairo_save(l_pCairoContext);
	drawAxisLabels(l_pCairoContext);
	cairo_restore(l_pCairoContext);

	cairo_destroy(l_pCairoContext);

}

void Graph::drawAxis(cairo_t * cairoContext)
{
	//make background white by drawing white rectangle
	cairo_set_source_rgb(cairoContext, 1.0, 1.0, 1.0);	
		cairo_rectangle(cairoContext, 0, 0, 1, 1);
	cairo_fill(cairoContext);
	
	double ux=1, uy=1;
	cairo_device_to_user_distance (cairoContext, &ux, &uy);
	if (ux < uy)
		ux = uy;
	cairo_set_line_width (cairoContext, ux);

	cairo_set_source_rgb(cairoContext, 0, 0, 0);

	//cairo_save(cairoContext);
	//draw the horizontal line at zero if its inside the plotting region
	double zeroLevel = adjustValueToScale(0);
	double Xo = 0;
	double Yo = zeroLevel;
	double Xe = 1.0;
	double Ye = zeroLevel;
	if(std::fabs(zeroLevel)<=1)
	{
		drawLine(cairoContext, &Xo, &Yo, &Xe, &Ye);
	}

	// Draw y axis
	double dXo = 0;
	double dYo = 0;
	double dXe = 0;
	double dYe = 1.0;
	drawLine(cairoContext, &dXo, &dYo, &dXe, &dYe);
}

void Graph::drawLine(cairo_t *cairoContext, double* Xo, double* Yo, double* Xe, double* Ye)
{
	cairo_save(cairoContext);

	snapCoords(cairoContext, Xo, Yo);
	snapCoords(cairoContext, Xe, Ye);

	cairo_identity_matrix(cairoContext);
	cairo_set_line_width (cairoContext, 1.0);
	cairo_move_to(cairoContext, *Xo, *Yo);
	cairo_line_to(cairoContext, *Xe, *Ye);
	cairo_stroke(cairoContext);
	cairo_restore(cairoContext);

}

void Graph::snapCoords(cairo_t * cairoContext, double* x, double* y )
{
	cairo_user_to_device(cairoContext, x, y);
	*x = ceil(*x) + 0.5;
	*y = ceil(*y) + 0.5;
}

void Graph::drawAxisLabels(cairo_t * cairoContext)
{
	cairo_set_source_rgb(cairoContext, 0, 0, 0);

	// If we haven't received any data yet, bail out
	if(!(m_pMinimum < FLT_MAX && m_pMaximum > -FLT_MAX))
	{
		cairo_move_to(cairoContext, 0, 1);

		double cx, cy;
		cairo_get_current_point(cairoContext, &cx, &cy);
		cairo_user_to_device(cairoContext, &cx, &cy);

		cairo_save(cairoContext);
		rendertext(cairoContext, "No data", cx, cy);
		cairo_restore(cairoContext);
		return;
	}
	
	// Note the scaling here should be compatible with adjustValueToScale.
	float64	GraphMin = m_pMinimum - m_vVariance[m_pArgMinimum.first][m_pArgMinimum.second];	
	float64 GraphMax = m_pMaximum + m_vVariance[m_pArgMaximum.first][m_pArgMaximum.second];


	// Including headroom may not be necessary as we are using adjustValueToScale to query where the Cairo drawing locations are, it'll take it into account
	// GraphMin = GraphMin-0.10f*std::fabs(GraphMin);
	// GraphMax = GraphMax+0.10f*std::fabs(GraphMax);

	const uint32_t numSteps = 10;
	float64 dataRange = GraphMax-GraphMin;
	float64 stepSize = dataRange / numSteps;

	// Find a starting point in y so that stepping will pass through 0
	float64 startY = floor(GraphMin/stepSize)*stepSize;

	for(uint32_t i=0 ; i<=numSteps; i++)
	{
		float64 valueAtTick = startY + i*stepSize;

		float64 Y = adjustValueToScale(valueAtTick);

		cairo_move_to(cairoContext, 0, Y);

		double cx, cy;
		cairo_get_current_point(cairoContext, &cx, &cy);
		//std::cout<<"current point "<<cx<<" "<<cy<<"\n";
		cairo_user_to_device(cairoContext, &cx, &cy);
		//std::cout<<"device current point "<<cx<<" "<<cy<<"\n";

		std::stringstream ss; ss.precision(2);
		ss << valueAtTick;

		cairo_save(cairoContext);
		rendertext(cairoContext, ss.str().c_str(), cx, cy);
		cairo_restore(cairoContext);

	}

	uint64 XBegin = this->StartTime;
	uint64 XEnd = this->EndTime;


	for(float64 X=0;X<=1;X+=0.2)
	{
		cairo_move_to(cairoContext, X, 1);

		double cx, cy;
		cairo_get_current_point(cairoContext, &cx, &cy);
		//std::cout<<"current point "<<cx<<" "<<cy<<"\n";
		cairo_user_to_device(cairoContext, &cx, &cy);


		//X value to print range from XBegin to XEnd
		float64 dataLengthSecs = ITimeArithmetics::timeToSeconds(XEnd - XBegin);
		float64 dataStart =  ITimeArithmetics::timeToSeconds(XBegin);

		std::stringstream ss; ss.precision(2); 
		ss << (dataLengthSecs * X + dataStart);

		cairo_save(cairoContext);
		rendertext(cairoContext, ss.str().c_str(), cx, cy);
		cairo_restore(cairoContext);
	}

}

void Graph::drawCurves(cairo_t * cairoContext)
{
	double ux=1, uy=1;
	cairo_device_to_user_distance (cairoContext, &ux, &uy);
	if (ux < uy)
		ux = uy;
	cairo_set_line_width (cairoContext, ux);

	for(uint32 gi=0; gi<m_vCurves.size(); gi++)
	{
		cairo_set_source_rgb(cairoContext, (double)m_cLineColor[gi].red/65535.0, 
				(double)m_cLineColor[gi].green/65535.0, 
				(double)m_cLineColor[gi].blue/65535.0);
		const std::vector<float64>& l_vCurve = m_vCurves[gi];

		//center
		float64 l_f64Y = adjustValueToScale( l_vCurve[0] );
		float64 l_f64X = 0.0;
		cairo_move_to(cairoContext, l_f64X, l_f64Y);

		for(int si = 1; si<curveSize; si++)
		{
			l_f64Y = adjustValueToScale( l_vCurve[si] );
			l_f64X = ((double)si)/((double)curveSize);
			cairo_line_to(cairoContext, l_f64X, l_f64Y);
		}

		cairo_save(cairoContext);
		cairo_identity_matrix(cairoContext);
		cairo_set_line_width (cairoContext, 1.0);
		cairo_stroke(cairoContext);
		cairo_restore(cairoContext);
	}
}

void Graph::drawVar(cairo_t *cairoContext)
{
	double ux=1, uy=1;
	cairo_device_to_user_distance (cairoContext, &ux, &uy);
	if (ux < uy)
		ux = uy;
	cairo_set_line_width (cairoContext, ux);

	for(uint32 gi=0; gi<m_vCurves.size(); gi++)
	{
		cairo_set_source_rgba(cairoContext, (double)m_cLineColor[gi].red/65535.0,
						(double)m_cLineColor[gi].green/65535.0,
						(double)m_cLineColor[gi].blue/65535.0,
							 0.5);
		const std::vector<float64>& l_vCurve = m_vCurves[gi];
		const std::vector<float64>& l_vVariances = m_vVariance[gi];
		// Test first if we have any variance at all, if not, don't bother drawing as cairo slows down with tiny apertures
		if(std::none_of(l_vVariances.begin(), l_vVariances.end(), [](float64 a) { return a>0; })) 
		{
			continue;
		}

		float64 Var = l_vVariances[0];

		float64 l_f64Y = adjustValueToScale( l_vCurve[0]-Var );
		float64 l_f64X = 0.0;
		cairo_move_to(cairoContext, l_f64X, l_f64Y);

		// Draw variance below the data points
		for(int si = 1; si<curveSize; si++)
		{
			Var = l_vVariances[si];

			l_f64Y = adjustValueToScale( l_vCurve[si]-Var );
			l_f64X = ((double)si)/((double)curveSize);
			cairo_line_to(cairoContext, l_f64X, l_f64Y);
		}

		// Draw the last point separately
		cairo_line_to(cairoContext, l_f64X, l_f64Y+2*Var);

		// Draw variance above the points, including the first point
		for(int si = curveSize-1; si>=0; si--)
		{
			Var = l_vVariances[si];
			l_f64Y = adjustValueToScale( l_vCurve[si]+Var );
			l_f64X = ((double)si)/((double)curveSize);
			cairo_line_to(cairoContext, l_f64X, l_f64Y);
		}

		// Fill the surrounded region?
		cairo_fill(cairoContext);
		
	}

}

void Graph::drawLegend(cairo_t * cairoContext)
{
	double ux=1, uy=1;
	cairo_device_to_user_distance (cairoContext, &ux, &uy);
	cairo_select_font_face(cairoContext, "Sans Bold 10",
		  CAIRO_FONT_SLANT_NORMAL,
		  CAIRO_FONT_WEIGHT_BOLD);

	cairo_set_font_size(cairoContext, 0.04);

	float64 yTotal = 0;
	for(uint32 gi=0; gi<m_vCurves.size(); gi++)
	{
		cairo_set_source_rgb(cairoContext, (double)m_cLineColor[gi].red/65535.0,
						(double)m_cLineColor[gi].green/65535.0,
						(double)m_cLineColor[gi].blue/65535.0);

		cairo_text_extents_t extents;
		cairo_text_extents(cairoContext, m_sLineText[gi].toASCIIString(), &extents);

		yTotal += extents.height + 0.02;

		// size_t len = m_sLineText[gi].length();
		cairo_move_to(cairoContext, (1.0-extents.width-0.01), yTotal);

		double cx, cy;
		cairo_get_current_point(cairoContext, &cx, &cy);
		//std::cout<<"current point "<<cx<<" "<<cy<<"\n";
		cairo_user_to_device(cairoContext, &cx, &cy);
		//std::cout<<m_sLineText[gi]<<"\n";
		cairo_save(cairoContext);
		cairo_show_text(cairoContext, m_sLineText[gi].toASCIIString() );
		cairo_restore(cairoContext);

	}
}

float64 Graph::adjustValueToScale(OpenViBE::float64 value)
{

	//std::cout<<m_pVariance[m_pArgMinimum.first][m_pArgMinimum.second] <<" "<<m_pVariance[m_pArgMaximum.first][m_pArgMaximum.second]<<"\n";
	float64 GraphMin = m_pMinimum - m_vVariance[m_pArgMinimum.first][m_pArgMinimum.second];
	float64 GraphMax = m_pMaximum + m_vVariance[m_pArgMaximum.first][m_pArgMaximum.second];

	GraphMin = GraphMin-0.10f*std::fabs(GraphMin);
	GraphMax = GraphMax+0.10f*std::fabs(GraphMax);
	return (GraphMin - value)/(GraphMax-GraphMin)+ 1.0f;
}

void Graph::updateCurves(const OpenViBE::float64* curve, size_t howMany, unsigned int curveIndex)
{
	m_vCurves[curveIndex].assign(curve,curve+howMany);

	m_pMaximum = -FLT_MAX;
	m_pMinimum = FLT_MAX;
	for (uint32 j=0; j<m_vCurves.size(); j++)
	{
		for(int i = 0; i<curveSize; i++)
		{
			//m_pMaximum = m_lCurves[j][i]>m_pMaximum ? m_lCurves[j][i]:m_pMaximum;
			if (m_vCurves[j][i]>m_pMaximum)
			{
				m_pMaximum = m_vCurves[j][i];
				m_pArgMaximum.first = j;
				m_pArgMaximum.second = i;
			}
			//m_pMinimum = m_lCurves[j][i]<m_pMinimum ? m_lCurves[j][i]:m_pMinimum;
			if (m_vCurves[j][i]<m_pMinimum)
			{
				m_pMinimum = m_vCurves[j][i];
				m_pArgMinimum.first = j;
				m_pArgMinimum.second = i;
			}
		}
		//m_pMaximum = m_pMaximum+0.01f*std::fabs(m_pMaximum);
		//m_pMinimum = m_pMinimum-0.01f*std::fabs(m_pMinimum);
	}
}

bool CBoxAlgorithmErpPlot::initialize(void)
{	
	// If you need to retrieve setting values, use the FSettingValueAutoCast function.
	const IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	m_sFigureFileName = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	m_sTriggerToSave = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);
	m_bXStartsAt0 = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 2);
	
	const uint32_t inputParamsStartAt = 3;

	m_lGraphList = new std::list<Graph*>;

	//should be a Graph per channel/electrode not per input (should be done when first header is received)
	for (uint32 i=1; i<l_rStaticBoxContext.getInputCount(); i++)
	{
		if ((i%2)==1)
		{
			uint32 l_ui32Class = i/2;
			m_oLegendColors.push_back(_AutoCast_(l_rStaticBoxContext, this->getConfigurationManager(), inputParamsStartAt + 2*l_ui32Class+0));
			m_oLegend.push_back(FSettingValueAutoCast(*this->getBoxAlgorithmContext(),                 inputParamsStartAt + 2*l_ui32Class+1));
			m_vDecoders.push_back( new TStreamedMatrixDecoder<CBoxAlgorithmErpPlot>(*this,i) );
		}
		else
		{
			m_vVarianceDecoders.push_back( new TStreamedMatrixDecoder<CBoxAlgorithmErpPlot>(*this,i) );
		}
	}

	m_oStimulationDecoder = new TStimulationDecoder<CBoxAlgorithmErpPlot>(*this,0);

	//*
	//initialize graphic component
	::GtkBuilder* m_pMainWidgetInterface=gtk_builder_new(); // glade_xml_new(m_sInterfaceFilename.toASCIIString(), "p300-speller-toolbar", NULL);
	GError* error = nullptr;
	this->getLogManager() << LogLevel_Trace << "Path to erp.ui " << OpenViBE::Directories::getDataDir() + CString("/plugins/simple-visualization/erp-plot.ui\n");
	gtk_builder_add_from_file(m_pMainWidgetInterface, OpenViBE::Directories::getDataDir() + "/plugins/simple-visualization/erp-plot.ui", &error);

	m_pDrawWindow=GTK_WIDGET(gtk_builder_get_object(m_pMainWidgetInterface, "plot-window"));

	m_visualizationContext = dynamic_cast<OpenViBEVisualizationToolkit::IVisualizationContext*>(this->createPluginObject(OVP_ClassId_Plugin_VisualizationContext));
	m_visualizationContext->setWidget(*this, m_pDrawWindow);

	g_signal_connect(m_pDrawWindow, "expose-event", G_CALLBACK (on_expose_event), m_lGraphList);
	g_signal_connect(m_pDrawWindow, "configure-event", G_CALLBACK (on_configure_event), m_lGraphList);


	gtk_widget_show_all(m_pDrawWindow);

	//*/
	m_bFirstHeaderReceived = false;



	return true;
}

bool CBoxAlgorithmErpPlot::uninitialize(void)
{
	for (uint32 inputi=0; inputi<m_vDecoders.size(); inputi++)
	{
		m_vDecoders[inputi]->uninitialize();
		m_vVarianceDecoders[inputi]->uninitialize();
	}

	m_oStimulationDecoder->uninitialize();

	if (m_pDrawWindow)
	{
		gtk_widget_destroy(m_pDrawWindow);
		m_pDrawWindow = nullptr;
	}

	while(!m_lGraphList->empty()) delete m_lGraphList->front(), m_lGraphList->pop_front();

	if(m_visualizationContext)
	{
		this->releasePluginObject(m_visualizationContext);
		m_visualizationContext = nullptr;
	}

	return true;
}

bool CBoxAlgorithmErpPlot::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}

//saving the graph in png images
bool CBoxAlgorithmErpPlot::save(void)
{
	//main context
	cairo_t * cairoContext;
	cairoContext = gdk_cairo_create(m_pDrawWindow->window);

	//the main surface
	cairo_surface_t * surface = cairo_get_target(cairoContext);

	//building filename
	OpenViBE::CString CSFilename = m_sFigureFileName;
	std::string extension = ".png";


	std::list<Graph*>::iterator l_Iterator;
	for ( l_Iterator=m_lGraphList->begin(); l_Iterator!=m_lGraphList->end(); l_Iterator++)
	{
		//cutting this graph
		cairo_surface_t * subsurface = cairo_surface_create_for_rectangle  (surface, (*l_Iterator)->m_dGraphOriginX, (*l_Iterator)->m_dGraphOriginY, (*l_Iterator)->m_dGraphWidth, (*l_Iterator)->m_dGraphHeight);

		std::stringstream filename;
		//creating filename
		filename << m_sFigureFileName << ((*l_Iterator)->rowIndex) <<"_"<< ((*l_Iterator)->colIndex) << extension;
		std::string Sfilename;
		filename >> Sfilename;
		this->getLogManager() << LogLevel_Info << "Saving [" << Sfilename.c_str() <<"] \n";
		cairo_surface_write_to_png ( subsurface, Sfilename.c_str() );//m_sFigureFileName.toASCIIString() );
	}

	return true;

}

bool CBoxAlgorithmErpPlot::process(void)
{
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();
	const IBox& l_rStaticBoxContext=this->getStaticBoxContext();

	//listen for stimulation input
	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++)
	{
		m_oStimulationDecoder->decode(i);
		if(m_oStimulationDecoder->isBufferReceived())
		{
			IStimulationSet * l_sStimSet = m_oStimulationDecoder->getOutputStimulationSet();
			for (uint32 j=0; j<l_sStimSet->getStimulationCount(); j++)
			{
				if (l_sStimSet->getStimulationIdentifier(j)==m_sTriggerToSave)
				{
					this->getLogManager() << LogLevel_Trace << "Saving\n";
					save();
				}
			}
		}
	}

	bool dataChanged = false;

	for (uint32 inputi=1; inputi<l_rStaticBoxContext.getInputCount(); inputi++)
	{
		for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(inputi); i++)
		{
			if( (inputi)%2 ==1 )
			{
				m_vDecoders[inputi/2]->decode(i);

				if(m_vDecoders[inputi/2]->isHeaderReceived() && !m_bFirstHeaderReceived)
				{
					uint32 l_ui32NrOfElectrodes = m_vDecoders[inputi/2]->getOutputMatrix()->getDimensionSize(0);
					uint32 l_ui32NrOfColumns = (uint32)ceil(sqrt((double)l_ui32NrOfElectrodes));

					//create list of graph subplots
					for (uint32 dimi=0; dimi<l_ui32NrOfElectrodes; dimi++)
					{
						Graph* l_pGraph = (Graph*) new Graph(m_oLegendColors, m_oLegend, (uint32)floor((float)dimi/l_ui32NrOfColumns),
												(uint32)(dimi%l_ui32NrOfColumns), m_vDecoders[inputi/2]->getOutputMatrix()->getDimensionSize(1));
						m_lGraphList->push_back(l_pGraph);
					}

					//draw the empty graphs
					std::list<Graph*>::iterator l_Iterator;
					for ( l_Iterator=m_lGraphList->begin(); l_Iterator!=m_lGraphList->end(); l_Iterator++)
					{

						(*l_Iterator)->StartTime = 0;
						(*l_Iterator)->EndTime = 1;
						//(*l_Iterator)->m_pVariance = nullptr;

						cairo_t * l_pCairoContext;
						l_pCairoContext = gdk_cairo_create(m_pDrawWindow->window);
						(*l_Iterator)->resizeAxis(400, 400, m_lGraphList->size());//default init size
						(*l_Iterator)->drawAxis(l_pCairoContext);
						cairo_destroy(l_pCairoContext);

						cairo_t * l_pCairoContext2;
						l_pCairoContext2 = gdk_cairo_create(m_pDrawWindow->window);
						(*l_Iterator)->drawAxisLabels(l_pCairoContext2);
						cairo_destroy(l_pCairoContext2);

					}
					m_bFirstHeaderReceived = true;

					dataChanged = true;
				}
				if(m_vDecoders[inputi/2]->isBufferReceived())
				{
					uint64 ChunkStartTime = l_rDynamicBoxContext.getInputChunkStartTime (inputi, i);
					uint64 ChunkEndTime = l_rDynamicBoxContext.getInputChunkEndTime (inputi, i);

					//redraw all
					//gtk_widget_queue_draw(m_pDrawWindow);

					IMatrix* l_pMatrix = m_vDecoders[inputi/2]->getOutputMatrix();
					uint32 l_ui32NrOfElectrodes = l_pMatrix->getDimensionSize(0);
					uint32 l_ui32NrOfSamples = l_pMatrix->getDimensionSize(1);
					std::list<Graph*>::iterator l_iGraphIterator = m_lGraphList->begin();
					for (uint32 dimi=0; dimi<l_ui32NrOfElectrodes; dimi++, l_iGraphIterator++)
					{
						const float64* l_pPtr = l_pMatrix->getBuffer()+dimi*l_ui32NrOfSamples;
						(*l_iGraphIterator)->updateCurves(l_pPtr, l_ui32NrOfSamples, inputi/2);
						//std::cout << "update curve " << inputi/2 << " beginning value " << l_pDestinationMatrix[0] << ", second value " << l_pDestinationMatrix[42] << "\n";

						(*l_iGraphIterator)->StartTime = (m_bXStartsAt0 ? 0 : ChunkStartTime);
						(*l_iGraphIterator)->EndTime = (m_bXStartsAt0 ? (ChunkEndTime - ChunkStartTime) : ChunkEndTime);

						//(*l_iGraphIterator)->draw(m_pDrawWindow);
					}

					dataChanged = true;

					l_rDynamicBoxContext.markInputAsDeprecated(inputi,i);

				}
				//if(m_vDecoders[inputi/2]->isEndReceived())
				//{
				//}

			}
			else
			{
				//std::cout<<" variance input"<<(inputi/2-1)<<"\n";
				m_vVarianceDecoders[inputi/2-1]->decode(i);


				if(m_vVarianceDecoders[inputi/2-1]->isBufferReceived())
				{
					IMatrix* l_pMatrix = m_vVarianceDecoders[inputi/2-1]->getOutputMatrix();

					uint32 l_ui32NrOfSamples = l_pMatrix->getDimensionSize(1);
					uint32 l_ui32NrOfElectrodes = l_pMatrix->getDimensionSize(0);

					std::list<Graph*>::iterator l_iGraphIterator = m_lGraphList->begin();
					for (uint32 dimi=0; dimi<l_ui32NrOfElectrodes; dimi++, l_iGraphIterator++)
					{
						const float64* l_pPtr = l_pMatrix->getBuffer()+dimi*l_ui32NrOfSamples;
						
						(*l_iGraphIterator)->m_vVariance[inputi/2-1].assign(l_pPtr,l_pPtr + l_ui32NrOfSamples);
					}

					dataChanged = true;

					l_rDynamicBoxContext.markInputAsDeprecated(inputi,i);

				}
			}
		}
	}


	//redraw all?
	if(dataChanged)
	{
		gtk_widget_queue_draw(m_pDrawWindow);
		std::list<Graph*>::iterator l_Iterator;
		for ( l_Iterator=m_lGraphList->begin(); l_Iterator!=m_lGraphList->end(); l_Iterator++)
		{
			(*l_Iterator)->draw(m_pDrawWindow);
		}
	}

	return true;
}
