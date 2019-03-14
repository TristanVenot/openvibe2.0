#pragma once

#include "defines.h"
#include "ovCMessaging.h"

#include "socket/IConnectionClient.h"

namespace Communication
{
	class Communication_API MessagingClient : public CMessaging
	{
	public:

		/**
		 * \brief	Default constructor.
		 */
		MessagingClient();

		/**
		 * \brief	Destructor.
		 */
		~MessagingClient();

		/**
		 * \brief	Connect to a server.
		 *
		 * \param	URI		URI of the server.
		 * \param	port	The port.
		 *
		 * \retval True if it succeeds.
		 * \retval False there is no available error.
		 * 		Library errors:
		 * 			- FailedToConnect
		 * 			- NotConnected
		 * 			- NotReadyToSend
		 *
		 * \sa close
		 */
		bool connect(const std::string& URI, const unsigned int port);

		/**
		 * \brief Closes the connection to the server.
		 *
		 * \retval True if it succeeds.
		 * \retval False there is no available error.
		 * 		Library errors:
		 * 			- NotConnected
		 * 			- FailedToCloseConnection
		 *
		 * \sa getLastError
		 */
		bool close();

		/**
		 * \brief Return the number of  box information's parameters available.
		 *
		 * \return The number of parameters available.
		 *
		 * \sa getParameter
		 */
		uint32_t getParameterCount() const;

		/**
		 * \brief Return the number of box information's input available.
		 *
		 * \return The number of input available.
		 *
		 * \sa getInput
		 */
		uint32_t getInputCount() const;

		/**
		 * \brief Return the number of output of the box information, available.
		 *
		 * \return The number of output available.
		 *
		 * \sa getOutput
		 */
		uint32_t getOutputCount() const;

		/**
		 * \brief Get parameter information.
		 *
		 * \param		i		The index.
		 * \param[out]	id		The id
		 * \param[out]	type	The type (corresponding to the OpenViBE CIdentifier)
		 * \param[out]	name	Name
		 * \param[out]	value	Value (to convert accordinf to the type)
		 *
		 * \retval True if it succeeds.
		 * \retval False if the index is out of range.
		 */
		bool getParameter(const size_t i, uint32_t& id, uint64_t& type, std::string& name, std::string& value) const;

		/**
		 * \brief Get input information.
		 *
		 * \param		i		The index.
		 * \param[out]	id		The id
		 * \param[out]	type	The type (corresponding to the OpenViBE CIdentifier)
		 * \param[out]	name	Name
		 *
		 * \retval True if it succeeds.
		 * \retval False if the index is out of range.
		 *
		 * \sa getInputCount
		 */
		bool getInput(const size_t i, uint32_t& id, uint64_t& type, std::string& name) const;

		/**
		 * \brief Get input information.
		 *
		 * \param		i		The index.
		 * \param[out]	id		The id
		 * \param[out]	type	The type (corresponding to the OpenViBE CIdentifier)
		 * \param[out]	name	Name
		 *
		 * \retval True if it succeeds.
		 * \retval False if the index is out of range.
		 *
		 * \sa getOutputCount
		 */
		bool getOutput(const size_t i, uint32_t& id, uint64_t& type, std::string& name) const;

		/**
		 * \brief Get the oldest error message, if available.
		 *
		 * \param[out]	type		The error's type.
		 * \param[out]	guiltyId	Identifier of the guilty message
		 *
		 * \retval True if it succeeds.
		 * \retval False there is no available error.
		 */
		bool popError(uint64_t& packetId, EError& type, uint64_t& guiltyId);

		/**
		 * \brief Get the oldest EBML message, if available.
		 *
		 * \param[out]	index	 	Box input index.
		 * \param[out]	startTime	The start time.
		 * \param[out]	endTime  	The end time.
		 * \param[out]	ebml	 	The EBML vector.
		 *
		 * \retval True if it succeeds.
		 * \retval False there is no available error.
		 */
		bool popEBML(uint64_t& packetId, uint32_t& index, uint64_t& startTime, uint64_t& endTime, std::shared_ptr<const std::vector<uint8_t>>& ebml);

		/**
		 * \brief	Push a log message to the server.
		 *
		 * \param	logLevel	The log level.
		 * \param	log			The log string message.
		 *
		 * \retval True if it succeeds.
		 * \retval False there is no available error.
		 * 		Library errors:
		 * 			- NotConnected
		 * 			- NotReadyToSend
		 *
		 * \sa getLastError
		 */
		bool pushLog(const ELogLevel logLevel, const std::string& log);

		/**
		 * \brief	Pushes an ebml.
		 *
		 * \param	index	 	Index of the box output.
		 * \param	startTime	The start time.
		 * \param	endtime  	The endtime.
		 * \param	ebml	 	The ebml vector.
		 *
		 * \retval True if it succeeds.
		 * \retval False there is no available error.
		 * 		Library errors:
		 * 			- NotConnected
		 * 			- NotReadyToSend
		 *
		 * \sa getLastError
		 */
		bool pushEBML(const uint32_t index, const uint64_t startTime, const uint64_t endtime, std::shared_ptr<const std::vector<uint8_t>> ebml);

		/**
		 * \brief Push Sync message to the server.
		 *
		 * \retval True if it succeeds.
		 * \retval False if the library is in error state.
		 */
		bool pushSync();

		/**
		 * \brief Check if a sync message is received.
		 *
		 * \retval True if a sync message is received.
		 * \retval False if no sync message was received.
		 */
		bool waitForSyncMessage();

	private:

		/**
		 * \brief	Pushes an authentication message).
		 *
		 * \param	connectionID	The connection identifier.
		 *
		 * \retval True if it succeeds.
		 * \retval False there is no available error.
		 * 		Library errors:
		 * 			- NotConnected
		 * 			- NotReadyToSend
		 */
		bool pushAuthentication(std::string connectionID);

	private:
		Socket::IConnectionClient* m_Client;

		bool m_BoxDescriptionReceived;
	};
}

