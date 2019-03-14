#include "ovasCConfigurationDriverGenericOscillator.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBEAcquisitionServer;
using namespace std;

CConfigurationDriverGenericOscillator::CConfigurationDriverGenericOscillator(IDriverContext& rDriverContext, 
	const char* sGtkBuilderFileName, 
	boolean& rSendPeriodicStimulations,
	double& rStimulationInterval)
	:CConfigurationBuilder(sGtkBuilderFileName)
	 ,m_rDriverContext(rDriverContext)
	 ,m_rSendPeriodicStimulations(rSendPeriodicStimulations)
	 ,m_rStimulationInterval(rStimulationInterval)
{
}

boolean CConfigurationDriverGenericOscillator::preConfigure(void)
{
	if (!CConfigurationBuilder::preConfigure())
	{
		return false;
	}

	::GtkToggleButton* l_pToggleSendPeriodicStimulations = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "checkbutton_send_periodic_stimulations"));
	::GtkSpinButton* l_pStimulationInterval  = GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "spinbutton_stimulation_interval"));

	gtk_toggle_button_set_active(l_pToggleSendPeriodicStimulations, m_rSendPeriodicStimulations);
	gtk_spin_button_set_value(l_pStimulationInterval, m_rStimulationInterval);

	return true;
}

boolean CConfigurationDriverGenericOscillator::postConfigure(void)
{
	if (m_bApplyConfiguration)
	{
		::GtkToggleButton* l_pToggleSendPeriodicStimulations = GTK_TOGGLE_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "checkbutton_send_periodic_stimulations"));
		::GtkSpinButton* l_pStimulationInterval  = GTK_SPIN_BUTTON(gtk_builder_get_object(m_pBuilderConfigureInterface, "spinbutton_stimulation_interval"));

		m_rSendPeriodicStimulations = (::gtk_toggle_button_get_active(l_pToggleSendPeriodicStimulations)>0);
		
		gtk_spin_button_update(l_pStimulationInterval);
		m_rStimulationInterval = gtk_spin_button_get_value(l_pStimulationInterval);
	}

	if (!CConfigurationBuilder::postConfigure())
	{
		return false;
	}

	return true;
}

