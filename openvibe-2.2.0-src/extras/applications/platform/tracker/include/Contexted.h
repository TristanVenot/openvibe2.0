
#pragma once

#include <openvibe/ov_all.h>

namespace OpenViBETracker
{

/**
 * \class Contexted 
 * \brief This class can be used to provide KernelContext and related getters for derived classes without overly complicating the derived class interface in their headers. 
 * \author J. T. Lindgren
 * @fixme actually this class does pretty much the same thing as OpenViBE::Kernel::TKernelObject
 *
 */
class Contexted {
public:
	Contexted(const OpenViBE::Kernel::IKernelContext& ctx) 
		: 
		m_KernelContext(ctx) { };
	virtual ~Contexted(void) { };

	virtual const OpenViBE::Kernel::IKernelContext& getKernelContext(void) const { return m_KernelContext; }

	virtual OpenViBE::Kernel::IAlgorithmManager& getAlgorithmManager(void) const { return m_KernelContext.getAlgorithmManager(); }
	virtual OpenViBE::Kernel::IErrorManager& getErrorManager(void) const { return m_KernelContext.getErrorManager(); }
	virtual OpenViBE::Kernel::ILogManager& getLogManager(void) const { return m_KernelContext.getLogManager(); }
	virtual OpenViBE::Kernel::ITypeManager& getTypeManager(void) const { return m_KernelContext.getTypeManager(); }

	// Convenience handle to getLogManager()
	virtual OpenViBE::Kernel::ILogManager& log(void) const { return getLogManager(); }

	Contexted() = delete;

	// Needed if Contexted is used as a base class for TAttributable
	// virtual OpenViBE::boolean isDerivedFromClass(
	//	const OpenViBE::CIdentifier& rClassIdentifier) const 
	// { 
	// 	return false;
	// }

protected:

	const OpenViBE::Kernel::IKernelContext& m_KernelContext;



};

};

