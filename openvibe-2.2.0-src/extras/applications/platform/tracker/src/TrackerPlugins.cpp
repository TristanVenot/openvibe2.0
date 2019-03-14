
#include <algorithm> // std::sort

#include "TrackerPlugins.h"
#include "TrackerPluginCountStimulations.h"
#include "TrackerPluginChannelCheck.h"

using namespace OpenViBETracker;

TrackerPlugins::TrackerPlugins(const OpenViBE::Kernel::IKernelContext& ctx) : Contexted(ctx)
{
	// Declare all plugins here
	m_PluginCreateCalls.push_back( [&ctx](void) { return new TrackerPluginCountStimulations(ctx); } );
	m_PluginCreateCalls.push_back( [&ctx](void) { return new TrackerPluginChannelCheck(ctx); } );

	// Create example instances of the plugins so the GUI can display a list
	for(auto fun : m_PluginCreateCalls)
	{
		auto ptr = fun();
		m_TrackerPlugins.push_back(ptr);
	}

	if(m_TrackerPlugins.size()==0)
	{
		return;
	}

	// get both arrays sorted. No code beauty contest winners here...
	std::vector< size_t > indexes;
	for(size_t i=0;i<m_TrackerPlugins.size();i++)
	{
		indexes.push_back(i);
	}

	auto& pluginRef = m_TrackerPlugins;
	auto& callRef = m_PluginCreateCalls;

	std::sort(indexes.begin(), indexes.end(), 
		[&pluginRef](size_t a, size_t b) { return (pluginRef[a]->getName()) < (pluginRef[b]->getName()); } );

	std::vector<ITrackerPlugin*> sortedPlugins; 
	std::transform(indexes.begin(), indexes.end(), std::back_inserter(sortedPlugins), [pluginRef](size_t i) { return pluginRef[i]; });
	m_TrackerPlugins = sortedPlugins;

	std::vector< std::function<ITrackerPlugin*(void)> > sortedCalls;
	std::transform(indexes.begin(), indexes.end(), std::back_inserter(sortedCalls), [callRef](size_t i) { return callRef[i]; });
	m_PluginCreateCalls = sortedCalls;
}

TrackerPlugins::~TrackerPlugins()
{
	for (auto f : m_TrackerPlugins) 
	{
		delete f;
	}
	m_TrackerPlugins.clear();
	m_PluginCreateCalls.clear();
}

ITrackerPlugin* TrackerPlugins::getPluginCopy(size_t index) const
{
	if(index>=m_PluginCreateCalls.size())
	{
		return nullptr;
	}

	auto fun = m_PluginCreateCalls[index];
	return fun();
}

