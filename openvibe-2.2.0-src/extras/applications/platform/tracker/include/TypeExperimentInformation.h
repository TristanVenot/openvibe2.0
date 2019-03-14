
#pragma once

#include <openvibe/ov_all.h>

#include "Chunk.h"

namespace OpenViBETracker
{

/**
 * \class TypeExperimentInformation
 * \brief Abstact class defining chunk types for Experiment Information streams
 * \author J. T. Lindgren
 *
 */
class TypeExperimentInformation
{
public:
	static OpenViBE::CIdentifier getTypeIdentifier(void) { return OV_TypeId_ExperimentInformation; };

	class Header : public Chunk
	{
	public:
		// Experiment
		uint64_t    m_Experiment_Identifier;
		std::string m_Experiment_Date;

		// Subject
		uint64_t    m_Subject_Identifier;
		std::string m_Subject_Name;
		uint64_t    m_Subject_Age;
		uint64_t    m_Subject_Gender;

		// Context
		uint64_t    m_Context_LaboratoryIdentifier;
		std::string m_Context_LaboratoryName;
		uint64_t    m_Context_TechnicianIdentifier;
		std::string m_Context_TechnicianName;
	};

	class Buffer : public Chunk
	{
	public:
		// Payload
	};

	class End : public Chunk { };

	// Prevent constructing
	TypeExperimentInformation()=delete;
	TypeExperimentInformation(const TypeExperimentInformation&)=delete;
	TypeExperimentInformation(TypeExperimentInformation&&)=delete;
};


};
