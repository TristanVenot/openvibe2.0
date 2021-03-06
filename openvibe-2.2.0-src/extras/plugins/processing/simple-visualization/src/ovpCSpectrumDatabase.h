#ifndef __OpenViBEPlugins_SimpleVisualization_CSpectrumDatabase_H__
#define __OpenViBEPlugins_SimpleVisualization_CSpectrumDatabase_H__

#include "ovp_defines.h"
#include "ovpCStreamedMatrixDatabase.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#include <vector>
#include <deque>
#include <queue>
#include <string>
#include <cfloat>
#include <iostream>

namespace OpenViBEPlugins
{
	namespace SimpleVisualization
	{
		/**
		* This class is used to store information about the incoming spectrum stream. It can request a IStreamDisplayDrawable
		* object to redraw itself upon changes in its data.
		*/
		class CSpectrumDatabase : public CStreamedMatrixDatabase
		{
		public:
			CSpectrumDatabase(
				OpenViBEToolkit::TBoxAlgorithm<OpenViBE::Plugins::IBoxAlgorithm>& oPlugin);

			virtual ~CSpectrumDatabase();

			virtual bool initialize();

			/**
			 * \brief Set displayed frequency range
			 * \param f64MinimumDisplayedFrequency Minimum frequency to display
			 * \param f64MaximumDisplayedFrequency Maximum frequency to display
			 */
			//TODO (if min/max computation should be restricted to this range)
			/*
			void setDisplayedFrequencyRange(
				OpenViBE::float64 f64MinimumDisplayedFrequency,
				OpenViBE::float64 f64MaximumDisplayedFrequency);*/

			/** \name Frequency bands management */
			//@{

			/**
			 * \brief Get number of frequency bands
			 * \return Number of frequency bands
			 */
			uint32_t getFrequencyAbscissaCount();

			/**
			 * \brief Get width of a frequency band (in Hz)
			 * \return Frequency band width
			 */
//			OpenViBE::float64 getFrequencyBandWidth();

			/**
			 * \brief Get frequency band start frequency
			 * \param ui32FrequencyBandIndex Index of frequency band
			 * \return Frequency band start if it could be retrieved, 0 otherwise
			 */
//			OpenViBE::float64 getFrequencyBandStart(
//				uint32_t ui32FrequencyBandIndex);

			/**
			 * \brief Get frequency band stop frequency
			 * \param ui32FrequencyBandIndex Index of frequency band
			 * \return Frequency band stop if it could be retrieved, 0 otherwise
			 */
//			OpenViBE::float64 getFrequencyBandStop(
//				uint32_t ui32FrequencyBandIndex);

			//@}

		protected:
			virtual bool decodeHeader();

		private:
			std::vector< OpenViBE::float64 > m_FrequencyAbscissa;
		};
	}
}

#endif
