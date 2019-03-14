#pragma once

#include "../ovIKernelObject.h"
#include "../../metaboxes/ovIMetaboxObjectDesc.h"

namespace OpenViBE
{
	namespace Kernel
	{
		/**
		 * \class IMetaboxManager
		 * \brief Metabox manager
		 * \author Thierry Gaugry (INRIA/Mensia)
		 * \date 2017-04-12
		 * \ingroup Group_Metabox
		 * \ingroup Group_Kernel
		 *
		 * The metabox manager is in charge of loading/unloading metaboxes
		 * modules (defined in OV_ScenarioImportContext_OnLoadMetaboxImport import context)
		 * containing OpenViBE metaboxes.
		 * It also provides functions in order to list metabox descriptors,
		 * create or release metabox objects...
		 */
		class OV_API IMetaboxManager : public OpenViBE::Kernel::IKernelObject
		{
		public:

			/** \name Metabox modules/descriptors management */
			//@{

			/**
			 * \brief Loads new metaboxes module file(s)
			 * \param[in] rFileNameWildCard  : a wild card with the file(s) to search metaboxes in
			 * \return \e true in case of success.
			 * \return \e false in case of error.
			 */
			virtual bool addMetaboxesFromFiles(const OpenViBE::CString& fileNameWildCard) = 0;

			/**
			 * \brief Gets next metabox object descriptor identifier
			 * \param[in] previousIdentifier : The identifier
			 *        for the preceeding metabox object descriptor
			 * \return The identifier of the next metabox object descriptor in case of success.
			 * \retval \c OV_UndefinedIdentifier on error.
			 * \note Giving \c OV_UndefinedIdentifier as \c previousIdentifier
			 *       will cause this function to return the first metabox object
			 *       descriptor identifier.
			 */
			virtual OpenViBE::CIdentifier getNextMetaboxObjectDescIdentifier(
				const OpenViBE::CIdentifier& previousIdentifier) const = 0;

			/**
			 * \brief Gets details on a specific metabox object descriptor
			 * \param[in] metaboxIdentifier : the metabox object descriptor identifier which details should be returned
			 * \return the corresponding metabox object descriptor pointer.
			 */
			virtual const OpenViBE::Plugins::IPluginObjectDesc* getMetaboxObjectDesc(
				const OpenViBE::CIdentifier& metaboxIdentifier) const = 0;

			/**
			 * \brief Sets details on a specific metabox object descriptor
			 * \param[in] metaboxIdentifier : the metabox object descriptor identifier
			 * \param[in] metaboxDescriptor : the metabox object descriptor
			 */
			virtual void setMetaboxObjectDesc(const OpenViBE::CIdentifier& metaboxIdentifier,
						OpenViBE::Plugins::IPluginObjectDesc* metaboxDescriptor) = 0;


			/**
			 * \brief Gets the path of the scenario of a specific metabox
			 * \param[in] metaboxIdentifier : the metabox object descriptor identifier which path should be returned
			 * \return the path to the scenario file of the metabox.
			 */
			virtual OpenViBE::CString getMetaboxFilePath(const OpenViBE::CIdentifier& metaboxIdentifier) const = 0;
			/**
			 * \brief Sets the path of the scenario of a specific metabox
			 * \param[in] metaboxIdentifier : the metabox object descriptor identifier
			 * \param[in] filePath : the metabox scenario path
			 */
			virtual void setMetaboxFilePath(const OpenViBE::CIdentifier& metaboxIdentifier, const OpenViBE::CString& filePath) = 0;

			/**
			 * \brief Gets the hash of the metabox
			 * \param[in] metaboxIdentifier : the metabox object descriptor identifier which hash should be returned
			 * \return the hash of the metabox.
			 */
			virtual OpenViBE::CIdentifier getMetaboxHash(const OpenViBE::CIdentifier& metaboxIdentifier) const = 0;
			/**
			 * \brief Sets the hash of the metabox
			 * \param[in] metaboxIdentifier : the metabox object descriptor identifier
			 * \param[in] hash : the metabox hash
			 */
			virtual void setMetaboxHash(const OpenViBE::CIdentifier& metaboxIdentifier, const OpenViBE::CIdentifier& hash) = 0;

			_IsDerivedFromClass_(OpenViBE::Kernel::IKernelObject, OV_ClassId_Kernel_Metabox_MetaboxManager)

		};
	};
};
