
#pragma once

#include <openvibe/ov_all.h>

#include "TypeMatrix.h"

namespace OpenViBETracker
{

/**
 * \class TypeFeatureVector
 * \brief Abstact class defining chunk types for Feature Vector streams
 * \author J. T. Lindgren
 *
 */
class TypeFeatureVector
{
public:
	static OpenViBE::CIdentifier getTypeIdentifier(void) { return OV_TypeId_FeatureVector; };

	class Header : public TypeMatrix::Header
	{
	public:
	};

	class Buffer : public TypeMatrix::Buffer
	{
	public:
	};

	class End : public TypeMatrix::End { };

	// Prevent constructing
	TypeFeatureVector()=delete;
	TypeFeatureVector(const TypeFeatureVector&)=delete;
	TypeFeatureVector(TypeFeatureVector&&)=delete;
};


};
