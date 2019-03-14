
#pragma once

#include "ovd_base.h"
#include "ovdCLogListenerDesigner.h"

#include <vector>
#include <map>

namespace OpenViBETracker
{

	/**
	 * \class CLogListenerTracker
	 * \brief Log listener wrapper for CLogListenerDesigner that can be called from multiple threads
	 * \author J. T. Lindgren
	 *
	 */
	class CLogListenerTracker : public OpenViBE::Kernel::ILogListener
	{
		public:

			CLogListenerTracker(const OpenViBE::Kernel::IKernelContext& rKernelContext, ::GtkBuilder* pBuilderInterface) : m_oPimpl(rKernelContext, pBuilderInterface) { } ;

			// @note Only call from gtk thread
			virtual void clearMessages() { 
				std::unique_lock<std::mutex> lock(m_oMutex);
				// std::cout << "Intercepted\n"; 
				m_oPimpl.clearMessages();
			}

			virtual OpenViBE::boolean isActive(OpenViBE::Kernel::ELogLevel eLogLevel) { 
				std::unique_lock<std::mutex> lock(m_oMutex);
				return m_oPimpl.isActive(eLogLevel); 
			} ;
			virtual OpenViBE::boolean activate(OpenViBE::Kernel::ELogLevel eLogLevel, OpenViBE::boolean bActive) { 
				std::unique_lock<std::mutex> lock(m_oMutex);
				return m_oPimpl.activate(eLogLevel, bActive); 
			};
			virtual OpenViBE::boolean activate(OpenViBE::Kernel::ELogLevel eStartLogLevel, OpenViBE::Kernel::ELogLevel eEndLogLevel, OpenViBE::boolean bActive) { 
				std::unique_lock<std::mutex> lock(m_oMutex);
				return m_oPimpl.activate(eStartLogLevel, eEndLogLevel, bActive);
			};
			virtual OpenViBE::boolean activate(OpenViBE::boolean bActive) { 
				std::unique_lock<std::mutex> lock(m_oMutex);
				return m_oPimpl.activate(bActive);
			};

			// This function encapsulates CLogListenerDesigner so that its gtk calls will be run from g_idle_add().
			template <class T> void logWrapper(T param) {
				auto ptr = new std::pair<CLogListenerTracker*, T>(this, param);

				auto fun = [](gpointer pUserData) 
				{ 
					auto ptr = static_cast< std::pair<CLogListenerTracker*, T>* >(pUserData);

					// LogListenerDesigner may not be thread safe, so we lock here for safety as activate() and isActive() do not go through the g_idle_add().
					std::unique_lock<std::mutex> lock(ptr->first->getMutex());

					ptr->first->getImpl().log(ptr->second); 
					delete ptr;

					return (gboolean)FALSE;
				};

				g_idle_add( fun, ptr);
			}

			// Specialization for const char*. It frees memory. We don't call it 
                        // template<> logWrapper() to avoid gcc complaining.
			void logWrapperCharPtr(const char* param) {
				auto ptr = new std::pair<CLogListenerTracker*, const char*>(this, param);

				auto fun = [](gpointer pUserData) 
				{ 
					auto ptr = static_cast< std::pair<CLogListenerTracker*, const char*>* >(pUserData);

					// LogListenerDesigner may not be thread safe, so we lock here for safety as activate() and isActive() do not go through the g_idle_add().
					std::unique_lock<std::mutex> lock(ptr->first->getMutex());

					ptr->first->getImpl().log(ptr->second); 
					delete[] ptr->second;
					delete ptr;

					return (gboolean)FALSE;
				};

				g_idle_add( fun, ptr);
			}

			virtual void log(const OpenViBE::time64 time64Value) override { logWrapper(time64Value); };

			virtual void log(const OpenViBE::uint64 ui64Value) override { logWrapper(ui64Value); };
			virtual void log(const OpenViBE::uint32 ui32Value) override { logWrapper(ui32Value); };
			virtual void log(const OpenViBE::uint16 ui16Value) override { logWrapper(ui16Value); };
			virtual void log(const OpenViBE::uint8 ui8Value) override { logWrapper(ui8Value); };

			virtual void log(const OpenViBE::int64 i64Value) override { logWrapper(i64Value); };
			virtual void log(const OpenViBE::int32 i32Value) override { logWrapper(i32Value); };
			virtual void log(const OpenViBE::int16 i16Value) override { logWrapper(i16Value); };
			virtual void log(const OpenViBE::int8 i8Value) override { logWrapper(i8Value); };

			virtual void log(const OpenViBE::float64 f64Value) override { logWrapper(f64Value); };
			virtual void log(const OpenViBE::float32 f32Value) override { logWrapper(f32Value); };

			virtual void log(const OpenViBE::boolean bValue) override { logWrapper(bValue); };

			virtual void log(const OpenViBE::Kernel::ELogLevel eLogLevel) override { logWrapper(eLogLevel); }; 
			virtual void log(const OpenViBE::Kernel::ELogColor eLogColor) override { logWrapper(eLogColor); };

			// With these calls we need to make copies, as by the time the actual calls are run in gtk_idle, the originals may be gone
			virtual void log(const OpenViBE::CIdentifier& rValue) override { OpenViBE::CIdentifier nonRef = rValue; logWrapper(nonRef); };
			virtual void log(const OpenViBE::CString& rValue) override { OpenViBE::CString nonRef = rValue; logWrapper(nonRef); };
			virtual void log(const char* pValue) override { 
				// This one is tricksy, we cannot simply use CString as we'd lose color, but with char* we need to free the memory. Use a specialization.
#if defined(TARGET_OS_Windows)
				const char* valueCopy = _strdup(pValue);
#else
				const char* valueCopy = strdup(pValue);
#endif
				logWrapperCharPtr(valueCopy); 
			};

			// Callbacks need access to these
			virtual OpenViBEDesigner::CLogListenerDesigner& getImpl(void) { return m_oPimpl; }
			virtual std::mutex& getMutex(void) { return m_oMutex; };

			// getClassIdentifier()
			_IsDerivedFromClass_Final_(OpenViBE::Kernel::ILogListener, OV_UndefinedIdentifier);

	protected:

			OpenViBEDesigner::CLogListenerDesigner m_oPimpl;

			std::mutex m_oMutex;

	};
};

