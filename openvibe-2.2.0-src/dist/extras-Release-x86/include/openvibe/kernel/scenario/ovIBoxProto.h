#ifndef __OpenViBE_Kernel_Scenario_IBoxProto_H__
#define __OpenViBE_Kernel_Scenario_IBoxProto_H__

#include "../ovIKernelObject.h"

namespace OpenViBE
{
	namespace Kernel
	{
		/**
		 * \brief This enum lists all the flags a box can be have
		 * \sa OpenViBE::Kernel::IBoxProto::addFlag
		 */
		enum EBoxFlag
		{
			BoxFlag_CanAddInput,
			BoxFlag_CanModifyInput,
			BoxFlag_CanAddOutput,
			BoxFlag_CanModifyOutput,
			BoxFlag_CanAddSetting,
			BoxFlag_CanModifySetting,
			BoxFlag_IsDeprecated,
			BoxFlag_ManualUpdate
		};

		/**
		 * \class IBoxProto
		 * \author Yann Renard (INRIA/IRISA)
		 * \date 2006-07-05
		 * \brief OpenViBE box prototype
		 * \ingroup Group_Scenario
		 * \ingroup Group_Kernel
		 * \ingroup Group_Extend
		 *
		 * This class is used by a plugin algorithm descriptor
		 * to let the OpenViBE platform know what an algorithm
		 * box looks like. It declares several things, like
		 * it input types, output types and settings.
		 *
		 * \sa OpenViBE::Kernel::IBoxAlgorithmDesc
		 */
		class OV_API IBoxProto : public OpenViBE::Kernel::IKernelObject
		{
		public:

			/**
			 * \brief Adds an input to the box
			 * \param sName [in] : the name of the input to add
			 * \param rTypeIdentifier [in] : the type of the input
			 * \param oIdentifier [in] : The input identifier
			 * \param bNotify [in]: if true, activate notification callback (true by default)
			 * \return true if successful
			 */
			virtual bool addInput(
				const OpenViBE::CString& sName,
				const OpenViBE::CIdentifier& rTypeIdentifier,
				const OpenViBE::CIdentifier& oIdentifier = OV_UndefinedIdentifier,
				const OpenViBE::boolean bNotify=true)=0;

			/**
			 * \brief Adds an output to the box
			 * \param sName [in] : the name of the output to add
			 * \param rTypeIdentifier [in] : the type of the output
			 * \param oIdentifier [in] : The output identifier
			 * \param bNotify [in]: if true, activate notification callback (true by default)
			 * \return true if successful
			 */
			virtual bool addOutput(
				const OpenViBE::CString& sName,
				const OpenViBE::CIdentifier& rTypeIdentifier,
				const OpenViBE::CIdentifier& rIdentifier = OV_UndefinedIdentifier,
				const OpenViBE::boolean bNotify=true)=0;
				
			/**
			 * \brief Add an setting to the box
			 * \param sName [in] : the name of the setting to add
			 * \param rTypeIdentifier [in] : the type of the setting
			 * \param sDefaultValue [in] : the default value of this
			 *        setting (used to initialize the box itself)
			 * \param bModifiability [in] : true if modifiable setting 
			 * \param oIdentifier [in] : The setting identifier
			 * \param bNotify [in]: if true, activate notification callback (true by default)
			 * \return true if successful
			 */
			virtual bool addSetting(
				const OpenViBE::CString& sName,
				const OpenViBE::CIdentifier& rTypeIdentifier,
				const OpenViBE::CString& sDefaultValue,
				const bool bModifiable = false,
				const OpenViBE::CIdentifier& rIdentifier = OV_UndefinedIdentifier,
				const OpenViBE::boolean bNotify=true)=0;
			/**
			 * \brief Adds a flag to the box
			 * \param eBoxFlag [in] : the flag to add to the box
			 * \return \e true in case of success.
			 * \return \e false in case of error.
			 */
			virtual bool addFlag(const OpenViBE::Kernel::EBoxFlag eBoxFlag)=0;

			/**
			 * \brief Adds a flag to the box
			 * \param cIdentifierFlag [in] : the flag to add to the box
			 * \return \e true in case of success.
			 * \return \e false in case of error.
			 */
			virtual bool addFlag(const OpenViBE::CIdentifier& cIdentifierFlag)=0;

			/**
			 * \brief Adds a new type supported by inputs of the box
			  * \param rTypeIdentifier [in] : The type identifier
			  * \return \e true in case of success.
			  * \return \e false in case of error.
			  */
			virtual bool addInputSupport(const OpenViBE::CIdentifier &rTypeIdentifier)=0;
			/**
			 * \brief Adds a new type supported by outputs of the box
			  * \param rTypeIdentifier [in] : The type identifier
			  * \return \e true in case of success.
			  * \return \e false in case of error.
			  */
			virtual bool addOutputSupport(const OpenViBE::CIdentifier &rTypeIdentifier)=0;

			_IsDerivedFromClass_(OpenViBE::Kernel::IKernelObject, OV_ClassId_Kernel_Scenario_BoxProto)
		};
	};
};

#endif // __OpenViBE_Kernel_Scenario_IBoxProto_H__
