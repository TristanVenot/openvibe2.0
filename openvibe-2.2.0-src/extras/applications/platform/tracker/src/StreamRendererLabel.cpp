//
// OpenViBE Tracker
//
// @author J.T. Lindgren
// 

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <iostream>

#include <system/ovCTime.h>
#include <openvibe/ovITimeArithmetics.h>

#include "StreamRendererLabel.h"

using namespace OpenViBETracker;

bool StreamRendererLabel::initialize(void)
{
	m_pMain = gtk_frame_new("");

	m_pLabel = gtk_label_new("");
	gtk_label_set_justify(GTK_LABEL(m_pLabel), GTK_JUSTIFY_LEFT);

	gtk_container_add(GTK_CONTAINER(m_pMain), m_pLabel);
	
	gtk_widget_ref(m_pMain);

	// @fixme hardcoded numbers likely not such a great idea as different font sizes etc
	gtk_widget_set_size_request(m_pMain, 640, 40);

	gtk_widget_show(m_pMain);
	gtk_widget_show(m_pLabel);

	return true;
}

bool StreamRendererLabel::setTitle(const char* title)
{
	gtk_label_set(GTK_LABEL(m_pLabel), title);

	return true;
}

// Instead of showing chunk list, this prints the stream structure information
// in a window. How it works is that a derived class can implement a text render
// routine which this function then calls. This way each derived class
// doesn't need to copy-paste the identical window code.
bool StreamRendererLabel::showChunkList(const char* title)
{
	// @fixme Not very pretty but better than nothing; here we have the benefit
	// that since we make a new builder here, reusing the workspace information
	// window does not mess up the real one

	GtkBuilder* pBuilder = gtk_builder_new();
	const OpenViBE::CString l_sFilename = OpenViBE::Directories::getDataDir() + "/applications/tracker/tracker.ui";
	if (!gtk_builder_add_from_file(pBuilder, l_sFilename, NULL)) {
		std::cout << "Problem loading [" << l_sFilename << "]\n";
		return false;
	}

	GtkWidget* window = GTK_WIDGET(gtk_builder_get_object(pBuilder, "tracker-workspace_information"));
	GtkTextBuffer* buffer = GTK_TEXT_BUFFER(gtk_builder_get_object(pBuilder, "tracker-textbuffer-workspace_information"));

	gtk_window_set_title(GTK_WINDOW(window), title);

	// Hide instead of destroy on closing the window
	g_signal_connect(window, "delete_event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);

	OpenViBE::CString details = renderAsText(0);

	gtk_text_buffer_set_text(buffer, details.toASCIIString(), -1);

	gtk_widget_show_all(window);

	gtk_window_present(GTK_WINDOW(window));

	g_object_unref(pBuilder);

	return true;
}
