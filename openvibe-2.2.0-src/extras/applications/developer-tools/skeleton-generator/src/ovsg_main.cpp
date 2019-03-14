#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <iostream>
#include <clocale> // std::setlocale

#include "ovsgCDriverSkeletonGenerator.h"
#include "ovsgCBoxAlgorithmSkeletonGenerator.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBESkeletonGenerator;

using namespace std;

int main(int argc, char** argv)
{

	CKernelLoader l_oKernelLoader;

	cout<<"[  INF  ] Created kernel loader, trying to load kernel module"<<endl;
	CString l_sError;
#if defined TARGET_OS_Windows
	CString l_sKernelFile = OpenViBE::Directories::getLibDir() + "/openvibe-kernel.dll";
#else
	CString l_sKernelFile = OpenViBE::Directories::getLibDir() + "/libopenvibe-kernel.so";
#endif
	if(!l_oKernelLoader.load(l_sKernelFile, &l_sError))
	{
		cout<<"[ FAILED ] Error loading kernel ("<<l_sError<<")" << " from [" << l_sKernelFile << "]\n";
	}
	else
	{
		cout<<"[  INF  ] Kernel module loaded, trying to get kernel descriptor"<<endl;
		IKernelDesc* l_pKernelDesc= nullptr;
		IKernelContext* l_pKernelContext= nullptr;
		l_oKernelLoader.initialize();
		l_oKernelLoader.getKernelDesc(l_pKernelDesc);
		if(!l_pKernelDesc)
		{
			cout<<"[ FAILED ] No kernel descriptor"<<endl;
		}
		else
		{
			cout<<"[  INF  ] Got kernel descriptor, trying to create kernel"<<endl;
			l_pKernelContext=l_pKernelDesc->createKernel("skeleton-generator", OpenViBE::Directories::getDataDir() + "/kernel/openvibe.conf");
			if(!l_pKernelContext || !l_pKernelContext->initialize())
			{
				cout<<"[ FAILED ] No kernel created by kernel descriptor"<<endl;
			}
			else
			{
				l_pKernelContext->getConfigurationManager().addConfigurationFromFile(OpenViBE::Directories::getDataDir() + "/kernel/openvibe.conf");
				l_pKernelContext->getConfigurationManager().addConfigurationFromFile(OpenViBE::Directories::getDataDir() + "/applications/skeleton-generator/skeleton-generator.conf");
				OpenViBEToolkit::initialize(*l_pKernelContext);
				IConfigurationManager& l_rConfigurationManager = l_pKernelContext->getConfigurationManager();
				l_pKernelContext->getPluginManager().addPluginsFromFiles(l_rConfigurationManager.expand("${Kernel_Plugins}"));
				gtk_init(&argc, &argv);

				// We rely on this with 64bit/gtk 2.24, to roll back gtk_init() sometimes switching
				// the locale to one where ',' is needed instead of '.' for separators of floats, 
				// causing issues, for example getConfigurationManager.expandAsFloat("0.05") -> 0; 
				// due to implementation by std::stod().
				std::setlocale(LC_ALL, "C");

				::GtkBuilder * l_pBuilderInterface = gtk_builder_new();
				const OpenViBE::CString l_sFilename = OpenViBE::Directories::getDataDir() + "/applications/skeleton-generator/generator-interface.ui";

				OV_ERROR_UNLESS(
					gtk_builder_add_from_file(l_pBuilderInterface, l_sFilename, NULL),
					"Problem loading [" << l_sFilename << "]",
					OpenViBE::Kernel::ErrorType::BadFileRead,
					-1, l_pKernelContext->getErrorManager(), l_pKernelContext->getLogManager());

				//gtk_builder_connect_signals(l_pBuilderInterface, NULL);

				::GtkWidget * l_pDialog = GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterface, "sg-selection-dialog"));

				gtk_dialog_add_button (GTK_DIALOG (l_pDialog),
					GTK_STOCK_OK,
					GTK_RESPONSE_OK);

				gtk_dialog_add_button (GTK_DIALOG (l_pDialog),
					GTK_STOCK_CANCEL,
					GTK_RESPONSE_CANCEL);

				::GtkWidget * l_pRadioDriver = GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterface, "sg-driver-selection-radio-button"));
				::GtkWidget * l_pRadioAlgo = GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterface, "sg-algo-selection-radio-button"));
				::GtkWidget * l_pRadioBox = GTK_WIDGET(gtk_builder_get_object(l_pBuilderInterface, "sg-box-selection-radio-button"));


				CBoxAlgorithmSkeletonGenerator l_BoxGenerator(*l_pKernelContext,l_pBuilderInterface);
				CDriverSkeletonGenerator l_DriverGenerator(*l_pKernelContext,l_pBuilderInterface);

				gint resp = gtk_dialog_run(GTK_DIALOG(l_pDialog));
				
				if(resp== GTK_RESPONSE_OK)
				{
					gtk_widget_hide(GTK_WIDGET(l_pDialog));

					if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(l_pRadioDriver)))
					{
						l_DriverGenerator.initialize();
					}
					OV_ERROR_UNLESS(
						!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(l_pRadioAlgo)),
						"NOT YET AVAILABLE.",
						OpenViBE::Kernel::ErrorType::Internal,
						0, l_pKernelContext->getErrorManager(), l_pKernelContext->getLogManager());

					if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(l_pRadioBox)))
					{
						l_BoxGenerator.initialize();
					}
					gtk_main();
				}
				else
				{
					std::cout<< "User cancelled. Exit." <<std::endl;
					return 0 ;
				}
			}
		}
	}

	l_oKernelLoader.uninitialize();
	l_oKernelLoader.unload();
	return 0;
}