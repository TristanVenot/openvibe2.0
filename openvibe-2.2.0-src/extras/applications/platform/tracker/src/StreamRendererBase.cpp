//
// OpenViBE Tracker
//
// @author J.T. Lindgren
// 
// @todo add horizontal scaling support
// @todo add event handlers
// @todo add ruler, stimulations, channel names, a million of other things

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <iostream>

#include <system/ovCTime.h>
#include <openvibe/ovITimeArithmetics.h>

#include <mensia/advanced-visualization.h>
#include <m_GtkGL.hpp>

#include "StreamRendererBase.h"

using namespace Mensia;
using namespace OpenViBE::Kernel;
using namespace OpenViBETracker;

bool StreamRendererBase::initialize(void)
{
	GtkBuilder* pBuilder = gtk_builder_new();
	const OpenViBE::CString l_sFilename = OpenViBE::Directories::getDataDir() + "/applications/tracker/advanced-visualization.ui";
	GError* errorCode = nullptr;
	if (!gtk_builder_add_from_file(pBuilder, l_sFilename, &errorCode)) {
		log() << LogLevel_Error << "Problem loading [" << l_sFilename << "] : "
			<< (errorCode ? errorCode->code : 0) << " " << (errorCode ? errorCode->message : "")
			<< "\n";
		g_object_unref(pBuilder);
		return false;
	}

	::GtkWidget* l_pWindow = GTK_WIDGET(::gtk_builder_get_object(pBuilder, "window"));
	m_pMain = GTK_WIDGET(::gtk_builder_get_object(pBuilder, "table"));
	// ::GtkWidget* l_pToolbar = GTK_WIDGET(::gtk_builder_get_object(pBuilder, "toolbar-window"));

	gtk_widget_ref(m_pMain);
	gtk_container_remove(GTK_CONTAINER(l_pWindow), m_pMain);
	// We keep one ref, the caller can unref after having assigned the widget

	m_pViewport = GTK_WIDGET(::gtk_builder_get_object(pBuilder, "viewport"));
	m_pTop = GTK_WIDGET(::gtk_builder_get_object(pBuilder, "label_top")); // caption
	m_pLeft = GTK_WIDGET(::gtk_builder_get_object(pBuilder, "drawingarea_left"));
	m_pRight = GTK_WIDGET(::gtk_builder_get_object(pBuilder, "drawingarea_right"));
	m_pBottom = GTK_WIDGET(::gtk_builder_get_object(pBuilder, "drawingarea_bottom"));
	m_pCornerLeft = GTK_WIDGET(::gtk_builder_get_object(pBuilder, "label_corner_left"));
	m_pCornerRight = GTK_WIDGET(::gtk_builder_get_object(pBuilder, "label_corner_right"));

	// @note this or something similar is needed or otherwise the widgets will all be crammed to the 
	// same fixed size aperture with no scrolling
	// @todo give users some scaling options
	// @fixme for some reason this causes a mess on the 'message' bar below in the UI, as if it didn't redraw properly
	gtk_widget_set_size_request(m_pMain, 640, 200);

	m_oColor.r = 1;
	m_oColor.g = 1;
	m_oColor.b = 1;

	g_object_unref(pBuilder);
	pBuilder = NULL;

	return true;
}

bool StreamRendererBase::uninitialize(void)
{
	for (size_t i = 0; i < m_vRenderer.size(); i++)
	{
		AdvancedVisualization::IRenderer::release(m_vRenderer[i]);
	}
	m_vRenderer.clear();

	if (m_pRendererContext)
	{
		AdvancedVisualization::IRendererContext::release(m_pRendererContext);
		m_pRendererContext = nullptr;
	}

	if (m_pSubRendererContext)
	{
		AdvancedVisualization::IRendererContext::release(m_pSubRendererContext);
		m_pSubRendererContext = nullptr;
	}

	if (m_pRuler)
	{
		delete m_pRuler;
		m_pRuler = nullptr;
	}

	return true;
}

bool StreamRendererBase::setTitle(const char *title)
{
	if (title)
	{
		::gtk_label_set_text(GTK_LABEL(m_pTop), title);
	}
	else 
	{
		::gtk_label_set_text(GTK_LABEL(m_pTop), "");
	}
	return true;
}

bool StreamRendererBase::setRulerVisibility(bool isVisible)
{
	m_bIsScaleVisible = isVisible;
	return updateRulerVisibility();
}

bool StreamRendererBase::updateRulerVisibility(void)
{

	/*
	if ((::gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_pScaleVisible)) ? true : false) != m_bIsScaleVisible)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pScaleVisible), m_bIsScaleVisible);
	}
	*/

	void(*l_fpAction)(::GtkWidget*) = m_bIsScaleVisible ? gtk_widget_show : gtk_widget_hide;
	(*l_fpAction)(this->m_pTop);
	(*l_fpAction)(this->m_pLeft);
	(*l_fpAction)(this->m_pRight);
	(*l_fpAction)(this->m_pBottom);
	(*l_fpAction)(this->m_pCornerLeft);
	(*l_fpAction)(this->m_pCornerRight);

	return true;
}
bool StreamRendererBase::realize(void)
{
	gtk_widget_realize(m_pTop);
	gtk_widget_realize(m_pLeft);
	gtk_widget_realize(m_pRight);
	gtk_widget_realize(m_pBottom);
	gtk_widget_realize(m_pViewport);
	gtk_widget_realize(m_pMain);

	return true;
}


bool StreamRendererBase::redraw(bool bImmediate /* = false */)
{
	m_oGtkGLWidget.redraw(bImmediate);
	m_oGtkGLWidget.redrawLeft(bImmediate);
	m_oGtkGLWidget.redrawRight(bImmediate);
	m_oGtkGLWidget.redrawBottom(bImmediate);

	return true;
}

bool StreamRendererBase::reshape(uint32_t width, uint32_t height)
{
	m_Width = uint32_t(width);
	m_Height = uint32_t(height);
	m_pRendererContext->setAspect(width*1.f / height);

	return true;
}

bool StreamRendererBase::draw(void)
{
	StreamRendererBase::preDraw();

	::glPushAttrib(GL_ALL_ATTRIB_BITS);
	//::glClearColor(1.0,1.0,1.0,1.0);
	::glColor4f(m_oColor.r, m_oColor.g, m_oColor.b, m_pRendererContext->getTranslucency());

	if (m_bRotate)
	{
		::glScalef(1, -1, 1);
		::glRotatef(-90, 0, 0, 1);
	}

	m_vRenderer[0]->render(*m_pRendererContext);
	::glPopAttrib();

	StreamRendererBase::postDraw();

	return true;
}

void StreamRendererBase::drawLeft(void)
{
	if (m_pRuler) m_pRuler->doRenderLeft(m_pLeft);
}

void StreamRendererBase::drawRight(void)
{
	if (m_pRuler) m_pRuler->doRenderRight(m_pRight);
}

void StreamRendererBase::drawBottom(void)
{
	if (m_pRuler) m_pRuler->doRenderBottom(m_pBottom);
}
bool StreamRendererBase::preDraw(void)
{
	this->updateRulerVisibility();

	//	auto m_sColorGradient=OpenViBE::CString("0:0,0,0; 100:100,100,100");

	auto m_sColorGradient = OpenViBE::CString("0:100, 100, 100; 12:50, 100, 100; 25:0, 50, 100; 38:0, 0, 50; 50:0, 0, 0; 62:50, 0, 0; 75:100, 50, 0; 88:100, 100, 50; 100:100, 100, 100");

	if (!m_TextureId)
	{
		m_TextureId = m_oGtkGLWidget.createTexture(m_sColorGradient.toASCIIString());
	}
	::glBindTexture(GL_TEXTURE_1D, m_TextureId);

	m_pRendererContext->setAspect(m_pViewport->allocation.width * 1.f / m_pViewport->allocation.height);

	return true;
}


bool StreamRendererBase::postDraw(void)
{
	::glPushAttrib(GL_ALL_ATTRIB_BITS);
	if(m_pRuler) m_pRuler->doRender();
	::glPopAttrib();

	return true;
}

bool StreamRendererBase::mouseButton(int32_t x, int32_t y, int32_t button, int status)
{
	m_vButton[button] = status;

	/*
	if (button == 1 && status == 1)
	{
		m_bIsScaleVisible = !m_bIsScaleVisible;
		m_pRendererContext->setScaleVisibility(m_bIsScaleVisible);
	}
	*/

	return true;
}

bool StreamRendererBase::mouseMotion(int32_t x, int32_t y)
{
	if (!m_bMouseInitialized)
	{
		m_i32MouseX = x;
		m_i32MouseY = y;
		m_bMouseInitialized = true;
	}

	if (m_vButton[3])
	{
		const float value = ::powf(.99f, float(y - m_i32MouseY));
//		std::cout << "scale " << value << "\n";
		m_pRendererContext->scaleBy(value);
		redraw();
	}
	if (m_vButton[2])
	{
		const float value = ::powf(.99f, float(y - m_i32MouseY));
//		std::cout << "zoom " << value << "\n";
		m_pRendererContext->zoomBy(value);
	}
	if (m_vButton[1])
	{
//		std::cout << "Rotate\n";
		m_pRendererContext->rotateByY((x - m_i32MouseX)*.1f);
		m_pRendererContext->rotateByX((y - m_i32MouseY)*.1f);
	}

	m_i32MouseX = x;
	m_i32MouseY = y;

	return true;
}

bool StreamRendererBase::finalize(void)
{
	for (size_t i = 0; i < m_vRenderer.size(); i++)
	{
		m_vRenderer[i]->rebuild(*m_pRendererContext);
		m_vRenderer[i]->refresh(*m_pRendererContext);
	}
	redraw(true);

	return true;
}

bool StreamRendererBase::restoreSettings(const std::string& prefix)
{
	if (!m_pRendererContext)
	{
		return false;
	}

	// Lets see if we have a scale token
	std::string token = std::string("${") + prefix + "_Scale}";
	float newScale = static_cast<float>(m_KernelContext.getConfigurationManager().expandAsFloat(token.c_str(), m_pRendererContext->getScale()));
	m_pRendererContext->setScale(newScale);

	return true;
}

bool StreamRendererBase::storeSettings(const std::string& prefix)
{
	if (!m_pRendererContext)
	{
		return false;
	}

	std::string token = prefix + "_Scale";
	std::stringstream value;
	value << m_pRendererContext->getScale();
	m_KernelContext.getConfigurationManager().addOrReplaceConfigurationToken(token.c_str(), value.str().c_str());

	return true;
}


OpenViBE::CString StreamRendererBase::renderAsText(uint32_t indent /* = 0 */) const
{
	std::stringstream ss; ss << std::string(indent, ' ') << "Detail printing unimplemented for stream type or placeholder renderer in use\n";
	return OpenViBE::CString(ss.str().c_str());
};

void OpenViBETracker::add_column(GtkTreeView* treeView, const char* name, uint32_t id, uint32_t minWidth)
{
	::GtkTreeViewColumn* l_pTreeViewColumnTokenName = gtk_tree_view_column_new();
	::GtkCellRenderer* l_pCellRendererTokenName = gtk_cell_renderer_text_new();
	gtk_tree_view_column_set_title(l_pTreeViewColumnTokenName, name);
	gtk_tree_view_column_pack_start(l_pTreeViewColumnTokenName, l_pCellRendererTokenName, TRUE);
	gtk_tree_view_column_set_attributes(l_pTreeViewColumnTokenName, l_pCellRendererTokenName, "text", id, NULL);
	gtk_tree_view_column_set_sort_column_id(l_pTreeViewColumnTokenName, id);
	gtk_tree_view_column_set_sizing(l_pTreeViewColumnTokenName, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_expand(l_pTreeViewColumnTokenName, TRUE);
	gtk_tree_view_column_set_resizable(l_pTreeViewColumnTokenName, TRUE);
	gtk_tree_view_column_set_min_width(l_pTreeViewColumnTokenName, minWidth);
	gtk_tree_view_append_column(treeView, l_pTreeViewColumnTokenName);
	gtk_tree_view_column_set_sort_indicator(l_pTreeViewColumnTokenName, TRUE);
};

