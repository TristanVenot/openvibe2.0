/*
 * This file implements various contexts we need in order to run a box.
 *
 * @fixme most of the implementation is very primitive and may be even missing altogether
 *
 */
#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <stack>
#include <map>
#include <vector>

#include <openvibe/ov_all.h>
#include <openvibe/ovCMatrix.h>

#include <fs/Files.h>

#include <ebml/CReader.h>
#include <ebml/CReaderHelper.h>

#include <system/ovCMemory.h>

#include <algorithm> // std::find on Ubuntu

#include "Contexted.h"
#include "EncodedChunk.h"

namespace OpenViBETracker
{

/**
 * \class TrackerPlayerContext 
 * \brief Implements OpenViBE::Kernel::IPlayerContext
 * \author J. T. Lindgren
 *
 */
class TrackerPlayerContext : protected Contexted, public OpenViBE::Kernel::IPlayerContext
{
public:
	TrackerPlayerContext(const OpenViBE::Kernel::IKernelContext& ctx) : Contexted(ctx) { };

	virtual bool sendSignal(
		const OpenViBE::Kernel::IMessageSignal& rMessageSignal) override {
		return true;
	}

	virtual bool sendMessage(
		const OpenViBE::Kernel::IMessageEvent& rMessageEvent,
		const OpenViBE::CIdentifier& rTargetIdentifier) override {
		return true;
	}

	virtual bool sendMessage(
		const OpenViBE::Kernel::IMessageEvent& rMessageEvent,
		const OpenViBE::CIdentifier* pTargetIdentifier,
		const OpenViBE::uint32 ui32TargetIdentifierCount) override {
		return true;
	}


	virtual OpenViBE::uint64 getCurrentTime(void) const override { return 0; }

	virtual OpenViBE::uint64 getCurrentLateness(void) const override { return 0; }

	virtual OpenViBE::float64 getCurrentCPUUsage(void) const override { return 0; }

	virtual OpenViBE::float64 getCurrentFastForwardMaximumFactor(void) const override {
		return 0;
	}

	virtual bool stop(void) override { return true; }

	virtual bool pause(void) override
	{
		return true;
	}

	virtual bool play(void) override { return true; }

	virtual bool forward(void) override { return true; }

	virtual OpenViBE::Kernel::EPlayerStatus getStatus(void) const override {
		return OpenViBE::Kernel::PlayerStatus_Stop;
	}

	virtual OpenViBE::Kernel::IConfigurationManager& getConfigurationManager(void) const override
	{
		return m_KernelContext.getConfigurationManager(); 
	}

	virtual OpenViBE::Kernel::IAlgorithmManager& getAlgorithmManager(void) const override { return m_KernelContext.getAlgorithmManager();  }

	virtual OpenViBE::Kernel::ILogManager& getLogManager(void) const override { return m_KernelContext.getLogManager();  }

	virtual OpenViBE::Kernel::IErrorManager& getErrorManager(void) const override { return m_KernelContext.getErrorManager(); }

	virtual OpenViBE::Kernel::IScenarioManager& getScenarioManager(void) const override {
		return m_KernelContext.getScenarioManager();
	}

	virtual OpenViBE::Kernel::ITypeManager& getTypeManager(void) const override { return m_KernelContext.getTypeManager(); }

	virtual bool canCreatePluginObject(const OpenViBE::CIdentifier& pluginIdentifier) const override { return false; }
	virtual OpenViBE::Plugins::IPluginObject* createPluginObject(const OpenViBE::CIdentifier& pluginIdentifier) const override
	{
		return nullptr;
	}
	virtual bool releasePluginObject(OpenViBE::Plugins::IPluginObject* pluginObject) const override { return true;  }

	OpenViBE::CIdentifier getClassIdentifier(void) const override { return OpenViBE::CIdentifier(); }

};

/**
 * \class TrackerBox 
 * \brief Implements OpenViBE::Kernel::IBox
 * \author J. T. Lindgren
 *
 */
class TrackerBox : protected Contexted, public OpenViBE::Kernel::IBox
{
public:
	TrackerBox(const OpenViBE::Kernel::IKernelContext& ctx) : Contexted(ctx) { };

	virtual OpenViBE::CIdentifier getIdentifier(void) const override { return m_Identifier; }

	virtual OpenViBE::CString getName(void) const override { return m_Name; };

	/*
	virtual OpenViBE::CIdentifier getAlgorithmClassIdentifier(void) override
	{ 
		return m_AlgorithmClassIdentifier; 
	};
	*/

	// @f
	virtual OpenViBE::CIdentifier getUnusedSettingIdentifier(const OpenViBE::CIdentifier& rIdentifier = OV_UndefinedIdentifier) const  override
	{ 
		return rIdentifier; 
	};
	virtual OpenViBE::CIdentifier getUnusedInputIdentifier(const OpenViBE::CIdentifier& rIdentifier = OV_UndefinedIdentifier) const  override
	{ 
		return rIdentifier; 
	};
	virtual OpenViBE::CIdentifier getUnusedOutputIdentifier(const OpenViBE::CIdentifier& rIdentifier = OV_UndefinedIdentifier) const  override
	{ 
		return rIdentifier; 
	};

	virtual bool addInterfacor(OpenViBE::Kernel::BoxInterfacorType interfacorType, const OpenViBE::CString& newName, const OpenViBE::CIdentifier& typeIdentifier, const OpenViBE::CIdentifier& identifier, bool shouldNotify = true)   override
	{
		return false;
	};
	virtual bool removeInterfacor(OpenViBE::Kernel::BoxInterfacorType interfacorType, const uint32_t index, bool shouldNotify = true)  override
	{
		return false;
	};

	virtual uint32_t getInterfacorCount(OpenViBE::Kernel::BoxInterfacorType interfacorType) const  override
	{
		if(interfacorType==OpenViBE::Kernel::Setting)
		{
			return getSettingCount();
		} 
		else if(interfacorType==OpenViBE::Kernel::Input)
		{
			return getInputCount();
		} 
		else if(interfacorType==OpenViBE::Kernel::Output)
		{
			return getOutputCount();	
		}
		
		return 0;
	};
	virtual uint32_t getInterfacorCountIncludingDeprecated(OpenViBE::Kernel::BoxInterfacorType interfacorType) const override
	{
		return getInterfacorCount(interfacorType);
	};

	virtual bool getInterfacorIdentifier(OpenViBE::Kernel::BoxInterfacorType interfacorType, uint32_t index, OpenViBE::CIdentifier& identifier) const  override
	{ 
		return false;
	};

	virtual bool getInterfacorIndex(OpenViBE::Kernel::BoxInterfacorType interfacorType, const OpenViBE::CIdentifier& identifier, uint32_t& index) const  override 
	{
		return false;
	};
	virtual bool getInterfacorIndex(OpenViBE::Kernel::BoxInterfacorType interfacorType, const OpenViBE::CString& name, uint32_t& index) const  override
	{
		return false;
	};

	virtual bool getInterfacorType(OpenViBE::Kernel::BoxInterfacorType interfacorType, uint32_t index, OpenViBE::CIdentifier& typeIdentifier) const  override
	{
		return false;
	};
	virtual bool getInterfacorType(OpenViBE::Kernel::BoxInterfacorType interfacorType, const OpenViBE::CIdentifier& identifier, OpenViBE::CIdentifier& typeIdentifier) const  override
	{
		return false;
	};
	virtual bool getInterfacorType(OpenViBE::Kernel::BoxInterfacorType interfacorType, const OpenViBE::CString& name, OpenViBE::CIdentifier& typeIdentifier) const  override
	{
		return false;
	};

	virtual bool getInterfacorName(OpenViBE::Kernel::BoxInterfacorType interfacorType, uint32_t index, OpenViBE::CString& name) const  override
	{
		return false;
	};
	bool getInterfacorName(OpenViBE::Kernel::BoxInterfacorType interfacorType, const OpenViBE::CIdentifier& identifier, OpenViBE::CString& name) const override
	{
		return false;
	};

	virtual bool getInterfacorDeprecatedStatus(OpenViBE::Kernel::BoxInterfacorType interfacorType, uint32_t index, bool& value) const  override
	{
		return false;
	};
	virtual bool getInterfacorDeprecatedStatus(OpenViBE::Kernel::BoxInterfacorType interfacorType, const OpenViBE::CIdentifier& identifier, bool& value) const override 
	{
		return false;
	};

	virtual bool hasInterfacorWithIdentifier(OpenViBE::Kernel::BoxInterfacorType interfacorType, const OpenViBE::CIdentifier& identifier) const override
	{
		return false;
	};
	virtual bool hasInterfacorWithType(OpenViBE::Kernel::BoxInterfacorType interfacorType, uint32_t index, const OpenViBE::CIdentifier& typeIdentifier) const override
	{
		return false;	
	};
	virtual bool hasInterfacorWithNameAndType(OpenViBE::Kernel::BoxInterfacorType interfacorType, const OpenViBE::CString& name, const OpenViBE::CIdentifier& typeIdentifier) const override
	{
		return false;
	};

	virtual bool setInterfacorType(OpenViBE::Kernel::BoxInterfacorType interfacorType, uint32_t index, const OpenViBE::CIdentifier& typeIdentifier) override
	{
		return false;
	};
	virtual bool setInterfacorType(OpenViBE::Kernel::BoxInterfacorType interfacorType, const OpenViBE::CIdentifier& identifier, const OpenViBE::CIdentifier& typeIdentifier) override
	{
		return false;
	};
	virtual bool setInterfacorType(OpenViBE::Kernel::BoxInterfacorType interfacorType, const OpenViBE::CString& name, const OpenViBE::CIdentifier& typeIdentifier) override
	{
		return false;
	};

	virtual bool setInterfacorName(OpenViBE::Kernel::BoxInterfacorType interfacorType, uint32_t index, const OpenViBE::CString& name) override
	{
		return false;
	};
	virtual bool setInterfacorName(OpenViBE::Kernel::BoxInterfacorType interfacorType, const OpenViBE::CIdentifier& identifier, const OpenViBE::CString& name) override
	{
		return false;
	};

	virtual bool setInterfacorDeprecatedStatus(OpenViBE::Kernel::BoxInterfacorType interfacorType, uint32_t index, bool value) override
	{
		return false;
	};
	virtual bool setInterfacorDeprecatedStatus(OpenViBE::Kernel::BoxInterfacorType interfacorType, const OpenViBE::CIdentifier& identifier, bool value) override
	{
		return false;
	};

	virtual bool updateInterfacorIdentifier(OpenViBE::Kernel::BoxInterfacorType interfacorType, uint32_t index, const OpenViBE::CIdentifier& newIdentifier) override
	{
		return false;
	};

	virtual bool addInterfacorTypeSupport(OpenViBE::Kernel::BoxInterfacorType interfacorType, const OpenViBE::CIdentifier& typeIdentifier) override
	{
		return false;
	};
	virtual bool hasInterfacorTypeSupport(OpenViBE::Kernel::BoxInterfacorType interfacorType, const OpenViBE::CIdentifier& typeIdentifier) const override
	{
		return false;
	};
			
	virtual bool setIdentifier(const OpenViBE::CIdentifier& rIdentifier) override {
		m_Identifier = rIdentifier;
		return true;
	};

	virtual bool setName(const OpenViBE::CString& sName) override {
		m_Name = sName;
		return true;
	}

	virtual bool setAlgorithmClassIdentifier(
		const OpenViBE::CIdentifier& rAlgorithmClassIdentifier) override {
		m_AlgorithmClassIdentifier = rAlgorithmClassIdentifier;
		return true; 
	}

	virtual bool initializeFromAlgorithmClassIdentifier(
		const OpenViBE::CIdentifier& rAlgorithmClassIdentifier) override {
		return false; 
	}

	virtual bool initializeFromExistingBox(
		const OpenViBE::Kernel::IBox& rExistingBox) override {

	//	this->clearBox();
		this->setName(rExistingBox.getName());
		this->setAlgorithmClassIdentifier(rExistingBox.getAlgorithmClassIdentifier());

		for(uint32_t i = 0; i < rExistingBox.getInputCount(); ++i)
		{
			OpenViBE::CIdentifier l_oType;
			OpenViBE::CString l_sName;
			rExistingBox.getInputType(i, l_oType);
			rExistingBox.getInputName(i, l_sName);
			addInput(l_sName, l_oType);
		}

		for (uint32_t i = 0; i < rExistingBox.getOutputCount(); ++i)
		{
			OpenViBE::CIdentifier l_oType;
			OpenViBE::CString l_sName;
			rExistingBox.getOutputType(i, l_oType);
			rExistingBox.getOutputName(i, l_sName);
			addOutput(l_sName, l_oType);
		}

		for (uint32_t i = 0; i < rExistingBox.getSettingCount(); ++i)
		{
			OpenViBE::CIdentifier l_oType;
			OpenViBE::CString l_sName;
			OpenViBE::CString l_sValue;
			OpenViBE::CString l_sDefaultValue;
			bool l_bModifiability;
			bool l_bNotify = false;
			rExistingBox.getSettingType(i, l_oType);
			rExistingBox.getSettingName(i, l_sName);
			rExistingBox.getSettingValue(i, l_sValue);
			rExistingBox.getSettingDefaultValue(i, l_sDefaultValue);
			rExistingBox.getSettingMod(i, l_bModifiability);
			addSetting(l_sName, l_oType, l_sDefaultValue, -1 ,l_bModifiability);
			setSettingValue(i, l_sValue, l_bNotify);
		}

		OpenViBE::CIdentifier l_oIdentifier=rExistingBox.getNextAttributeIdentifier(OV_UndefinedIdentifier);
		while(l_oIdentifier!=OV_UndefinedIdentifier)
		{
			this->addAttribute(l_oIdentifier, rExistingBox.getAttributeValue(l_oIdentifier));
			l_oIdentifier=rExistingBox.getNextAttributeIdentifier(l_oIdentifier);
		}

		OpenViBE::CIdentifier l_oStreamTypeIdentifier = OV_UndefinedIdentifier;
		while((l_oStreamTypeIdentifier=this->getKernelContext().getTypeManager().getNextTypeIdentifier(l_oStreamTypeIdentifier))!=OV_UndefinedIdentifier)
		{
			if(this->getKernelContext().getTypeManager().isStream(l_oStreamTypeIdentifier))
			{
				//First check if it is a stream
				if(rExistingBox.hasInputSupport(l_oStreamTypeIdentifier))
				{
					this->addInputSupport(l_oStreamTypeIdentifier);
				}
				if(rExistingBox.hasOutputSupport(l_oStreamTypeIdentifier))
				{
					this->addOutputSupport(l_oStreamTypeIdentifier);
				}
			}
		}

//		this->enableNotification();

//		this->notify(BoxModification_Initialized);

		return true; 
	}

	virtual bool addInput(
		const OpenViBE::CString& sName,
		const OpenViBE::CIdentifier& rTypeIdentifier,
		const OpenViBE::CIdentifier& oIdentifier = OV_UndefinedIdentifier,
		const bool bNotify=true	) override 
	{
		IOEntry tmp;
		tmp.name = sName;
		tmp.id = rTypeIdentifier;
		m_Inputs.push_back(tmp);

		return true; 
	}

	virtual bool removeInput(
		const OpenViBE::uint32 ui32InputIndex,
		const bool bNotify=true) override
	{
		return false; 
	}

	virtual OpenViBE::uint32 getInputCount(void) const override { return m_Inputs.size(); };

	virtual bool getInputType(
		const OpenViBE::uint32 ui32InputIndex,
		OpenViBE::CIdentifier& rTypeIdentifier) const override 
	{
		rTypeIdentifier = m_Inputs[ui32InputIndex].id;
		return true;
	}

	virtual bool getInputName(
		const OpenViBE::uint32 ui32InputIndex,
		OpenViBE::CString& rName) const override
	{
		rName = m_Inputs[ui32InputIndex].name;
		return true; 
	}

	virtual bool setInputType(
		const OpenViBE::uint32 ui32InputIndex,
		const OpenViBE::CIdentifier& rTypeIdentifier) override
	{
		if (m_Inputs.size() <= ui32InputIndex) {
			m_Inputs.resize(ui32InputIndex+1);
		}
		m_Inputs[ui32InputIndex].id = rTypeIdentifier;
		return true; 
	}

	virtual bool setInputName(
		const OpenViBE::uint32 ui32InputIndex,
		const OpenViBE::CString& rName) override 
	{
		if (m_Inputs.size() <= ui32InputIndex)
		{
			m_Inputs.resize(ui32InputIndex+1);
		}
		m_Inputs[ui32InputIndex].name = rName;
		return true; 
	}


	virtual bool addOutput(
		const OpenViBE::CString& sName,
		const OpenViBE::CIdentifier& rTypeIdentifier,
		const OpenViBE::CIdentifier& rIdentifier = OV_UndefinedIdentifier,
		const bool bNotify = true) override
	{
		IOEntry tmp;
		tmp.name = sName;
		tmp.id = rTypeIdentifier;
		m_Outputs.push_back(tmp);
		return true; 
	}

	virtual bool removeOutput(
		const OpenViBE::uint32 ui32OutputIndex,
		const bool bNotify=true) override
	{
		return false;
	}

	virtual OpenViBE::uint32 getOutputCount(void) const override { return m_Outputs.size(); }

	virtual bool getOutputType(
		const OpenViBE::uint32 ui32OutputIndex,
		OpenViBE::CIdentifier& rTypeIdentifier) const override {
		rTypeIdentifier = m_Outputs[ui32OutputIndex].id;
		return true; 
	}

	virtual bool getOutputName(
		const OpenViBE::uint32 ui32OutputIndex,
		OpenViBE::CString& rName) const override {
		rName = m_Outputs[ui32OutputIndex].name;
		return true; 
	}

	virtual bool setOutputType(
		const OpenViBE::uint32 ui32OutputIndex,
		const OpenViBE::CIdentifier& rTypeIdentifier) override
	{
		if (m_Outputs.size() <= ui32OutputIndex)  {
			m_Outputs.resize(ui32OutputIndex+1);
		}
		m_Outputs[ui32OutputIndex].id = rTypeIdentifier;
		return true;
	}

	virtual bool setOutputName(
		const OpenViBE::uint32 ui32OutputIndex,
		const OpenViBE::CString& rName) override
	{
		if (m_Outputs.size() <= ui32OutputIndex) {
			m_Outputs.resize(ui32OutputIndex+1);
		}
		m_Outputs[ui32OutputIndex].name = rName;
		return true;
	}

	virtual bool addSetting(
		const OpenViBE::CString& sName,
		const OpenViBE::CIdentifier& rTypeIdentifier,
		const OpenViBE::CString& sDefaultValue,
		const OpenViBE::uint32 ui32Index = -1,
		const bool bModifiability = false,
		const OpenViBE::CIdentifier& rIdentifier = OV_UndefinedIdentifier,
		const bool bNotify=true) override
	{

		OpenViBE::Kernel::ITypeManager& typeManager = m_KernelContext.getTypeManager();

		OpenViBE::CString l_sValue(sDefaultValue);
		if (typeManager.isEnumeration(rTypeIdentifier))
		{
			if (typeManager.getEnumerationEntryValueFromName(rTypeIdentifier, sDefaultValue) == OV_UndefinedIdentifier)
			{
				if (typeManager.getEnumerationEntryCount(rTypeIdentifier) != 0)
				{
					// get value to the first enum entry
					// and eventually correct this after
					uint64_t l_ui64Value = 0;
					typeManager.getEnumerationEntry(rTypeIdentifier, 0, l_sValue, l_ui64Value);

					// Find if the default value string actually is an identifier, otherwise just keep the zero index name as default.
					OpenViBE::CIdentifier l_oIdentifier;
					l_oIdentifier.fromString(sDefaultValue);

					// Finally, if it is an identifier, then a name should be found
					// from the type manager ! Otherwise l_sValue is left to the default.
					OpenViBE::CString l_sCandidateValue = typeManager.getEnumerationEntryNameFromValue(rTypeIdentifier, l_oIdentifier.toUInteger());
					if (l_sCandidateValue != OpenViBE::CString(""))
					{
						l_sValue = l_sCandidateValue;
					}
				}
			}
		}

		Setting tmp;
		tmp.name = sName;
		tmp.id = rTypeIdentifier;
		tmp.value = l_sValue;
		tmp.defaultValue = l_sValue;

		uint32_t l_ui32Index = ui32Index;

#if 0
		uint32_t slot = (i32Index >= 0 ? i32Index : m_Settings.size());
		OV_ERROR_UNLESS_KRF(
			i32Index <= static_cast<int32_t>(m_vSetting.size()),
			"Tried to push '" << sName << "' to slot " << i32Index << " with the array size being " << static_cast<int32>(m_vSetting.size()),
			ErrorType::OutOfBound
			);
#endif

//		int32_t l_i32InsertLocation;

		if (ui32Index == OV_Value_UndefinedIndexUInt || ui32Index == static_cast<uint32_t>(m_Settings.size()))
		{
			m_Settings.push_back(tmp);
//			l_i32InsertLocation = (static_cast<int32_t>(m_Settings.size())) - 1;
		}
		else
		{
			std::vector<Setting>::iterator l_it = m_Settings.begin();
			l_it += l_ui32Index;
			m_Settings.insert(l_it, tmp);
//			l_i32InsertLocation = i32Index;
		}

#if 0
		this->getLogManager() << LogLevel_Debug << "Pushed '" << m_vSetting[l_i32InsertLocation].m_sName << "' : '" << m_vSetting[l_i32InsertLocation].m_sValue << "' to slot " << l_i32InsertLocation << " with the array size now " << static_cast<int32_t>(m_vSetting.size()) << "\n";

		this->notify(BoxModification_SettingAdded, l_i32InsertLocation);
		this->notifySettingChange(SettingAdd, l_i32InsertLocation);
#endif

		return true; 
	}

	virtual bool removeSetting(
		const OpenViBE::uint32 ui32SettingIndex,
		const bool bNotify=true) override
	{
		auto it = m_Settings.begin() + ui32SettingIndex;
		if (it == m_Settings.end())
		{
			getLogManager() << OpenViBE::Kernel::LogLevel_Error << "Error: No setting found\n";
			return false;
		}
		it = m_Settings.erase(it);
		return true;
	}

	virtual OpenViBE::uint32 getSettingCount(void) const override { return m_Settings.size();  }

	virtual bool hasSettingWithName(
		const OpenViBE::CString& rName) const override {
		return false;
	}

	/*
	virtual OpenViBE::int32 getSettingIndex(
		const OpenViBE::CString& rName) const override {
		return -1; 
	}
	*/

	virtual bool getSettingType(
		const OpenViBE::uint32 ui32SettingIndex,
		OpenViBE::CIdentifier& rTypeIdentifier) const override 
	{
		rTypeIdentifier = m_Settings[ui32SettingIndex].id;
		return true; 
	}

	virtual bool getSettingName(
		const OpenViBE::uint32 ui32SettingIndex,
		OpenViBE::CString& rName) const override 
	{
		rName = m_Settings[ui32SettingIndex].name;
		return true;
	}

	virtual bool getSettingDefaultValue(
		const OpenViBE::CIdentifier &rIdentifier,
		OpenViBE::CString& rDefaultValue) const override
	{
		return false;
	}
				
	virtual bool getSettingDefaultValue(
		const OpenViBE::uint32 ui32SettingIndex,
		OpenViBE::CString& rDefaultValue) const override
	{
		rDefaultValue = m_Settings[ui32SettingIndex].defaultValue;
		return true;
	}

	virtual bool getSettingDefaultValue(
		const OpenViBE::CString &sName,
		OpenViBE::CString& rDefaultValue) const override
	{
			
		return false;
	}
				
	virtual bool getSettingValue(
		const OpenViBE::uint32 ui32SettingIndex,
		OpenViBE::CString& rValue) const override
	{
	
		if (m_Settings.size() < ui32SettingIndex) { return false; }

		rValue = m_Settings[ui32SettingIndex].value;
		return true;
	}

	virtual bool setSettingType(
		const OpenViBE::uint32 ui32SettingIndex,
		const OpenViBE::CIdentifier& rTypeIdentifier) override
	{
		if (m_Settings.size() <= ui32SettingIndex) {
			m_Settings.resize(ui32SettingIndex+1);
		}
		m_Settings[ui32SettingIndex].id = rTypeIdentifier;
		return true;
	}

	virtual bool setSettingName(
		const OpenViBE::uint32 ui32SettingIndex,
		const OpenViBE::CString& rName) override
	{
		if (m_Settings.size() <= ui32SettingIndex) {
			m_Settings.resize(ui32SettingIndex+1);
		}
		m_Settings[ui32SettingIndex].name = rName;
		return true;
	};

	virtual bool setSettingDefaultValue(
		const OpenViBE::uint32 ui32SettingIndex,
		const OpenViBE::CString& rDefaultValue) override
	{
		if (m_Settings.size() <= ui32SettingIndex) {
			m_Settings.resize(ui32SettingIndex+1);
		}
		m_Settings[ui32SettingIndex].defaultValue = rDefaultValue;
		return true; 
	}

	virtual bool setSettingValue(
		const OpenViBE::uint32 ui32SettingIndex,
		const OpenViBE::CString& rValue,
		const bool bNotify=true) override
	{
		if (m_Settings.size() <= ui32SettingIndex) {
			m_Settings.resize(ui32SettingIndex+1);
		}
		m_Settings[ui32SettingIndex].value = rValue;
		return true;
	}

	virtual bool getSettingMod(
		const OpenViBE::uint32 ui32SettingIndex,
		bool& rValue) const override
	{
		return false; 
	}

	virtual bool setSettingMod(
		const OpenViBE::uint32 ui32SettingIndex,
		const bool rValue) override
	{
		return false;
	}

	virtual bool getSettingValue(
		const OpenViBE::CIdentifier &rIdentifier,
		OpenViBE::CString& rValue) const override
	{ 
		return false; 
	};

	virtual bool getSettingValue(
		const OpenViBE::CString &sName,
		OpenViBE::CString& rValue) const override
	{ 
		return false; 
	};

	virtual bool setSettingDefaultValue(
		const OpenViBE::CIdentifier &rIdentifier,
		const OpenViBE::CString& rDefaultValue) override { return false; };
		
	virtual bool setSettingDefaultValue(
		const OpenViBE::CString &sName,
		const OpenViBE::CString& rDefaultValue) override { return false; };
		
	virtual bool setSettingValue(
		const OpenViBE::CIdentifier& rIdentifier,
		const OpenViBE::CString& rValue)  override
	{
		return false; 
	};
				
	virtual bool setSettingValue(
		const OpenViBE::CString& rsName,
		const OpenViBE::CString& rValue) override
	{ 
		return false; 
	};

	virtual bool getSettingMod(
		const OpenViBE::CIdentifier &rIdentifier,
		bool& rValue) const override { return false; };

	virtual bool getSettingMod(
		const OpenViBE::CString &sName,
		bool& rValue) const override { return false; };

	virtual bool setSettingMod(
		const OpenViBE::CIdentifier &rIdentifier,
		const bool rValue) override { return false; };

	virtual bool setSettingMod(
		const OpenViBE::CString &sName,
		const bool rValue) override { return false; };
				
	virtual bool swapSettings(unsigned int indexA, unsigned int indexB) override { return false; };
	virtual bool swapInputs(unsigned int indexA, unsigned int indexB) override { return false; };
	virtual bool swapOutputs(unsigned int indexA, unsigned int indexB) override { return false; };
	
	virtual bool hasModifiableSettings(void) const override { return false;  }

	virtual std::vector<OpenViBE::CIdentifier> getInputSupportTypes() const override { std::vector<OpenViBE::CIdentifier> tmp; return tmp; };
	virtual std::vector<OpenViBE::CIdentifier> getOutputSupportTypes() const override { std::vector<OpenViBE::CIdentifier> tmp; return tmp; };
	
	virtual OpenViBE::uint32* getModifiableSettings(OpenViBE::uint32& rCount) const override {
		rCount = 0;
		return nullptr;
	}

	virtual bool addInputSupport(
		const OpenViBE::CIdentifier& rTypeIdentifier) override
	{
		m_InputSupports.push_back(rTypeIdentifier);
		return true;
	} 

	virtual bool hasInputSupport(
		const OpenViBE::CIdentifier& rTypeIdentifier) const override
	{
		if (m_InputSupports.size() == 0 || std::find(m_InputSupports.begin(), m_InputSupports.end(), rTypeIdentifier) != m_InputSupports.end())
		{
			return true;
		}
		return false;
	}

	virtual bool addOutputSupport(
		const OpenViBE::CIdentifier& rTypeIdentifier) override
	{
		m_OutputSupports.push_back(rTypeIdentifier);
		return true;
	}

	virtual bool hasOutputSupport(
		const OpenViBE::CIdentifier& rTypeIdentifier) const override
	{
		if (m_OutputSupports.size() == 0 || std::find(m_OutputSupports.begin(), m_OutputSupports.end(), rTypeIdentifier) != m_OutputSupports.end())
		{
			return true;
		}
		return false;
	}

	virtual bool setSupportTypeFromAlgorithmIdentifier(
		const OpenViBE::CIdentifier& rTypeIdentifier) override
	{
		return true;
	}

	OpenViBE::CIdentifier getClassIdentifier(void) const override { return OpenViBE::CIdentifier(); }

	_IsDerivedFromClass_(OpenViBE::Kernel::IAttributable, OV_ClassId_Kernel_Scenario_Box)

	struct Setting
	{
		OpenViBE::CIdentifier id;
		OpenViBE::CString name;
		OpenViBE::CString value;
		OpenViBE::CString defaultValue;
	};

	struct IOEntry
	{
		OpenViBE::CIdentifier id;
		OpenViBE::CString name;
	};

	std::vector<Setting> m_Settings;
	std::vector<IOEntry> m_Inputs;
	std::vector<IOEntry> m_Outputs;
	std::vector<OpenViBE::CIdentifier> m_InputSupports;
	std::vector<OpenViBE::CIdentifier> m_OutputSupports;

	// This box
	OpenViBE::CString m_Name;
	OpenViBE::CIdentifier m_Identifier;
	OpenViBE::CIdentifier m_AlgorithmClassIdentifier;

	// Attributable
	virtual bool addAttribute(
	const OpenViBE::CIdentifier& rAttributeIdentifier,
	const OpenViBE::CString& sAttributeValue)  {
		return true;
	}

	virtual bool removeAttribute(
		const OpenViBE::CIdentifier& rAttributeIdentifier) {
		return true; 
	}
	virtual bool removeAllAttributes(void) { return true;  }

	virtual OpenViBE::CString getAttributeValue(
		const OpenViBE::CIdentifier& rAttributeIdentifier) const {
		return OpenViBE::CString(""); 
	}
	virtual bool setAttributeValue(
		const OpenViBE::CIdentifier& rAttributeIdentifier,
		const OpenViBE::CString& sAttributeValue) {
		return true; 
	}

	virtual bool hasAttribute(
		const OpenViBE::CIdentifier& rAttributeIdentifier) const {
		return false; 
	}

	virtual bool hasAttributes(void) const { return false; }

	virtual OpenViBE::CIdentifier getNextAttributeIdentifier(
		const OpenViBE::CIdentifier& rPreviousIdentifier) const {
		return OpenViBE::CIdentifier(); 
	}

	// @f
	virtual void clearOutputSupportTypes() { };
	virtual void clearInputSupportTypes() { };
			
	OpenViBE::CIdentifier getAlgorithmClassIdentifier(void) const override { return OpenViBE::CIdentifier(); }

};

/**
 * \class TrackerBoxProto 
 * \brief Implements OpenViBE::Kernel::IBoxProto
 * \author J. T. Lindgren
 *
 */
class TrackerBoxProto : public TrackerBox, public OpenViBE::Kernel::IBoxProto
{
public:
	virtual bool addSetting(
		const OpenViBE::CString& sName,
		const OpenViBE::CIdentifier& rTypeIdentifier,
		const OpenViBE::CString& sDefaultValue,
		const bool bModifiable = false)
	{
		return TrackerBox::addSetting(sName, rTypeIdentifier, sDefaultValue, -1, bModifiable);
	}

};


/**
 * \class TrackerBoxIO 
 * \brief Implements OpenViBE::Kernel::IBoxIO
 * \author J. T. Lindgren
 *
 */
class TrackerBoxIO : public OpenViBE::Kernel::IBoxIO
{
public:

	~TrackerBoxIO()
	{
		for (size_t i = 0; i < m_OutputChunks.size(); i++)
		{
			delete m_OutputChunks[i];
		}
	}

	struct MemoryBufferWithStamps {
		OpenViBE::CMemoryBuffer* m_Buffer;
		ovtime_t m_StartTime;
		ovtime_t m_EndTime;
	};

	virtual bool initialize(const OpenViBE::Kernel::IBox* staticBoxContext) 
	{
		for (size_t i = 0; i < m_OutputChunks.size(); i++)
		{
			delete m_OutputChunks[i];
		}

		m_InputChunks.resize(staticBoxContext->getInputCount());
		m_OutputChunks.resize(staticBoxContext->getOutputCount());
		m_Ready.resize(staticBoxContext->getOutputCount(), 0);
		m_StartTime.resize(staticBoxContext->getOutputCount());
		m_EndTime.resize(staticBoxContext->getOutputCount());

		for (size_t i = 0; i < m_OutputChunks.size(); i++)
		{
			m_OutputChunks[i] = new OpenViBE::CMemoryBuffer();
		}

		return true;
	}

	virtual OpenViBE::uint32 getInputChunkCount(const OpenViBE::uint32 ui32InputIndex) const override {
		if (ui32InputIndex >= m_InputChunks.size())
		{
			return 0;
		}
		return m_InputChunks[ui32InputIndex].size();
	}

	virtual OpenViBE::boolean getInputChunk(
		const OpenViBE::uint32 ui32InputIndex,
		const OpenViBE::uint32 ui32ChunkIndex,
		OpenViBE::uint64& rStartTime,
		OpenViBE::uint64& rEndTime,
		OpenViBE::uint64& rChunkSize,
		const OpenViBE::uint8*& rpChunkBuffer) const override  {
	
		const std::vector<MemoryBufferWithStamps>& chunks = m_InputChunks[ui32InputIndex];
		const MemoryBufferWithStamps& chk = chunks[ui32ChunkIndex];

		rStartTime = chk.m_StartTime;
		rEndTime = chk.m_EndTime;
		rChunkSize = chk.m_Buffer->getSize(); 
		rpChunkBuffer = chk.m_Buffer->getDirectPointer();

		return true;
	}

	// Essentially this function being const and requiring its output as IMemoryBuffer prevents us from simply carrying our data as std::vector<EncodedChunk>. 
	virtual const OpenViBE::IMemoryBuffer* getInputChunk(
		const OpenViBE::uint32 ui32InputIndex,
		const OpenViBE::uint32 ui32ChunkIndex) const override {
		const std::vector<MemoryBufferWithStamps>& chunks = m_InputChunks[ui32InputIndex];
		const MemoryBufferWithStamps& chk = chunks[ui32ChunkIndex];

		return chk.m_Buffer;
	}

	virtual OpenViBE::uint64 getInputChunkStartTime(
		const OpenViBE::uint32 ui32InputIndex,
		const OpenViBE::uint32 ui32ChunkIndex) const override {
		const std::vector<MemoryBufferWithStamps>& chunks = m_InputChunks[ui32InputIndex];
		const MemoryBufferWithStamps& chk = chunks[ui32ChunkIndex];

		return chk.m_StartTime;
	}

	virtual OpenViBE::uint64 getInputChunkEndTime(
		const OpenViBE::uint32 ui32InputIndex,
		const OpenViBE::uint32 ui32ChunkIndex) const override {
		const std::vector<MemoryBufferWithStamps>& chunks = m_InputChunks[ui32InputIndex];
		const MemoryBufferWithStamps& chk = chunks[ui32ChunkIndex];

		return chk.m_EndTime;
	}

	virtual OpenViBE::boolean markInputAsDeprecated(
		const OpenViBE::uint32 ui32InputIndex,
		const OpenViBE::uint32 ui32ChunkIndex) override {
		return true; 
	}

	virtual OpenViBE::uint64 getOutputChunkSize(
		const OpenViBE::uint32 ui32OutputIndex) const override 
	{
		if (ui32OutputIndex >= m_OutputChunks.size())
		{
			return false;
		}
		return m_OutputChunks[ui32OutputIndex]->getSize();
	}

	virtual OpenViBE::boolean setOutputChunkSize(
		const OpenViBE::uint32 ui32OutputIndex,
		const OpenViBE::uint64 ui64Size,
		const OpenViBE::boolean bDiscard = true) override 
	{
		if (m_OutputChunks.size() <= ui32OutputIndex)
		{
			m_OutputChunks.resize(ui32OutputIndex + 1);
		}
		m_OutputChunks[ui32OutputIndex]->setSize(ui64Size, bDiscard);
		return true;
	}

	virtual OpenViBE::uint8* getOutputChunkBuffer(
		const OpenViBE::uint32 ui32OutputIndex) override
	{
		if (m_OutputChunks.size() >= ui32OutputIndex)
		{
			return nullptr;
		}
		return m_OutputChunks[ui32OutputIndex]->getDirectPointer();

	}

	virtual OpenViBE::boolean appendOutputChunkData(
		const OpenViBE::uint32 ui32OutputIndex,
		const OpenViBE::uint8* pBuffer,
		const OpenViBE::uint64 ui64BufferSize) override
	{
		
		return m_OutputChunks[ui32OutputIndex]->append(pBuffer, ui64BufferSize);

		return true; 
	}

	virtual OpenViBE::IMemoryBuffer* getOutputChunk(
		const OpenViBE::uint32 ui32OutputIndex) override
	{
		return m_OutputChunks[ui32OutputIndex];
	}

	virtual bool getOutputChunk(const OpenViBE::uint32 ui32OutputIndex, EncodedChunk& chk)
	{	
	
		chk.bufferData.assign(m_OutputChunks[ui32OutputIndex]->getDirectPointer(),
			m_OutputChunks[ui32OutputIndex]->getDirectPointer() + m_OutputChunks[ui32OutputIndex]->getSize());
		chk.m_startTime = m_StartTime[ui32OutputIndex];
		chk.m_endTime = m_EndTime[ui32OutputIndex];

		return true;
	}


	virtual OpenViBE::boolean markOutputAsReadyToSend(
		const OpenViBE::uint32 ui32OutputIndex,
		const OpenViBE::uint64 ui64StartTime,
		const OpenViBE::uint64 ui64EndTime) override 
	{
		m_StartTime[ui32OutputIndex] = ui64StartTime;
		m_EndTime[ui32OutputIndex] = ui64EndTime;
		m_Ready[ui32OutputIndex] = true;
		return true; 
	}

	std::vector< std::vector<MemoryBufferWithStamps> > m_InputChunks; // Queue per input. See code for comments.
	std::vector<OpenViBE::IMemoryBuffer*>  m_OutputChunks;   // Just one buffer per output
	std::vector<uint32_t> m_Ready;
	std::vector<ovtime_t> m_StartTime;
	std::vector<ovtime_t> m_EndTime;

	OpenViBE::CIdentifier getClassIdentifier(void) const override { return OpenViBE::CIdentifier(); }

	///

	bool addInputChunk(uint32_t inputIndex, const EncodedChunk& chk)
	{
		if (m_InputChunks.size() <= inputIndex)
		{
			m_InputChunks.resize(inputIndex + 1);
		}
		MemoryBufferWithStamps buf; 
		buf.m_StartTime = chk.m_startTime;
		buf.m_EndTime = chk.m_endTime;
		buf.m_Buffer = new OpenViBE::CMemoryBuffer(&chk.bufferData[0], chk.bufferData.size()); // We cannot copy CMemoryBuffer, so have to use new() @fixme SDK should implement working copy or prevent
		m_InputChunks[inputIndex].push_back(buf);
		return true;
	}

	bool clearInputChunks(void) 
	{
		for (size_t i = 0; i < m_InputChunks.size(); i++)
		{
			for(auto& chk : m_InputChunks[i])
			{
				delete chk.m_Buffer;
			}
			m_InputChunks[i].clear();
		}

		return true;
	}

	bool isReadyToSend(uint32_t outputIndex)
	{
		return (m_Ready[outputIndex] != 0);
	}
	bool deprecateOutput(uint32_t outputIndex) 
	{
		m_OutputChunks[outputIndex]->setSize(0, true);
		m_Ready[outputIndex] = false;
		return true;
	}
};

/**
 * \class TrackerBoxAlgorithmContext 
 * \brief Implements OpenViBE::Kernel::IBoxAlgorithmContext
 * \author J. T. Lindgren
 *
 */
class TrackerBoxAlgorithmContext : protected Contexted, public OpenViBE::Kernel::IBoxAlgorithmContext
{
public:

	TrackerBoxAlgorithmContext(const OpenViBE::Kernel::IKernelContext& ctx) : Contexted(ctx), m_StaticBoxContext(ctx), m_PlayerContext(ctx) { };

	virtual const OpenViBE::Kernel::IBox* getStaticBoxContext(void) override { return &m_StaticBoxContext; }

	virtual OpenViBE::Kernel::IBoxIO* getDynamicBoxContext(void) override { return &m_DynamicBoxContext; }

	virtual OpenViBE::Kernel::IPlayerContext* getPlayerContext(void) override { return &m_PlayerContext; }

	virtual OpenViBE::boolean markAlgorithmAsReadyToProcess(void) override { return true; };

	OpenViBE::CIdentifier getClassIdentifier(void) const override { return OpenViBE::CIdentifier(); }

	///

	virtual TrackerBoxIO* getTrackerBoxIO(void) { return &m_DynamicBoxContext; }

	TrackerBox m_StaticBoxContext;
	TrackerBoxIO m_DynamicBoxContext;
	TrackerPlayerContext m_PlayerContext;

};

};

