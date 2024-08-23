/*
  This file is part of the ArduinoIoTCloud library.

  Copyright 2024 Blues (http://www.blues.com/)

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef ARDUINO_NOTECARD_CONNECTION_HANDLER_H_
#define ARDUINO_NOTECARD_CONNECTION_HANDLER_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <stdint.h>

#include <Arduino.h>
#include <Notecard.h>
#include <Wire.h>

#include "ConnectionHandlerInterface.h"

/******************************************************************************
   DEFINES
 ******************************************************************************/

#define NOTECARD_CONNECTION_HANDLER_VERSION_MAJOR 1
#define NOTECARD_CONNECTION_HANDLER_VERSION_MINOR 0
#define NOTECARD_CONNECTION_HANDLER_VERSION_PATCH 0

#define NOTECARD_CONNECTION_HANDLER_VERSION NOTE_C_STRINGIZE(NOTECARD_CONNECTION_HANDLER_VERSION_MAJOR) "." NOTE_C_STRINGIZE(NOTECARD_CONNECTION_HANDLER_VERSION_MINOR) "." NOTE_C_STRINGIZE(NOTECARD_CONNECTION_HANDLER_VERSION_PATCH)

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

/**
 * @brief The NotecardConnectionHandler class
 *
 * The NotecardConnectionHandler class is a concrete implementation of the
 * ConnectionHandler interface that provides connectivity to the Arduino IoT
 * Cloud using a Notecard.
 */
class NotecardConnectionHandler final : public ConnectionHandler
{
  public:
    /**
     * @brief The manner in which the Notecard is synchronized with Notehub
     *
     * The SyncType enum defines the valid types of synchronization operations
     * that can be performed by the NotecardConnectionHandler class.
     *
     * @par
     * - Full - synchronize both the inbound and outbound queues.
     * - Inbound - synchronize only the inbound queues.
     * - Outbound - synchronize only the outbound queues.
     */
    enum class SyncType : uint8_t {
      Full,
      Inbound,
      Outbound,
    };

    /**
     * @brief The type of topic to be used for R/W operations
     *
     * The Notecard uses topics to identify the target of a read or write
     * operation. The TopicType enum defines the valid types of topics.
     *
     * @par
     * - Command - used to interact with the Arduino IoT Cloud.
     * - Thing - used to send application data to the Arduino IoT Cloud.
     */
    enum class TopicType : uint8_t {
      Invalid = 0,
      Command,
      Thing,
    };

    /**
     * @brief The error codes for communicating with the Notecard
     *
     * The NotecardCommunicationError enum defines the error codes that can be
     * returned by the NotecardConnectionHandler class.
     *
     * @par
     * - NOTECARD_ERROR_NONE - No error occurred.
     * - NOTECARD_ERROR_NO_DATA_AVAILABLE - No data is available.
     * - NOTECARD_ERROR_GENERIC - A generic error occurred.
     * - HOST_ERROR_OUT_OF_MEMORY - The host is out of memory.
     */
    typedef enum {
      NOTECARD_ERROR_NONE                 = 0,
      NOTECARD_ERROR_NO_DATA_AVAILABLE    = -1,
      NOTECARD_ERROR_GENERIC              = -2,
      HOST_ERROR_OUT_OF_MEMORY            = -3,
    } NotecardCommunicationError;

    /**
     * @brief The default timeout for the Notecard to connect to Notehub
     */
    static const uint32_t NOTEHUB_CONN_TIMEOUT_MS = 185000;

    /**
     * @brief The I2C constructor for the Notecard
     *
     * @param project_uid[in] The project UID of the related Notehub account
     * @param i2c_address[in] The I2C address of the Notecard
     * @param i2c_max[in]     The maximum I2C transaction size (MTU)
     * @param wire[in]        The I2C bus to use
     * @param keep_alive[in]  Keep the connection alive if connection to Notehub drops
     */
    NotecardConnectionHandler(
      const String & project_uid,
      uint32_t i2c_address = NOTE_I2C_ADDR_DEFAULT,
      uint32_t i2c_max = NOTE_I2C_MAX_DEFAULT,
      TwoWire & wire = Wire,
      bool keep_alive = true
    );

    /**
     * @brief The UART constructor for the Notecard
     *
     * @param project_uid[in] The project UID of the related Notehub account
     * @param serial[in]      The serial port to use
     * @param baud[in]        The baud rate of the serial port
     * @param keep_alive[in]  Keep the connection alive if connection to Notehub drops
     */
    NotecardConnectionHandler(
      const String & project_uid,
      HardwareSerial & serial,
      uint32_t baud = 9600,
      bool keep_alive = true
    );

    /**
     * @brief Disable hardware interrupts
     *
     * When hardware interrupts are disabled, the `NotecardConnectionHandler`
     * must be polled for incoming data. This is necessary when the host
     * microcontroller is unable to use the ATTN pin of the Notecard.
     */
    inline void disableHardwareInterrupts (void) {
      _en_hw_int = false;
    }

    /**
     * @brief Enable hardware interrupts
     *
     * Hardware interrupts allow the `NotecardConnectionHandler` to leverage the
     * ATTN pin of the Notecard. This improves the responsiveness of the
     * `NotecardConnectionHandler` by eliminating the need for the host
     * microcontroller to poll the Notecard for incoming data.
     */
    inline void enableHardwareInterrupts (void) {
      _en_hw_int = true;
    }

    /**
     * @brief Get the Arduino IoT Cloud Device ID
     *
     * The Arduino IoT Cloud Device ID is set as the serial number of the
     * Notecard when the device is provisioned in Notehub. The serial number is
     * updated on each sync between the Notecard and Notehub and cached by the
     * Notecard. As a result, this value can lag behind the actual value of the
     * Arduino IoT Cloud Device ID used by the Notehub. However, this value is
     * typically unchanged during the life of the Notecard, so this is rarely,
     * if ever, an issue.
     *
     * @return The Arduino IoT Cloud Device ID
     */
    inline const String & getDeviceId (void) {
      check(); // Ensure the connection to the Notecard is initialized
      return _device_id;
    }

    /**
     * @brief Get the Notecard object
     *
     * The Notecard object is used to interact with the Notecard. This object
     * provides methods to read and write data to the Notecard, as well as
     * methods to configure the Notecard.
     *
     * @return The Notecard object
     */
    inline const Notecard & getNotecard (void) {
      return _notecard;
    }

    /**
     * @brief Get the Notecard Device ID
     *
     * The Notecard Device ID is the unique identifier of the Notecard. This
     * value is set at time of manufacture, and is used to identify the Notecard
     * in Notehub.
     *
     * @return The Notecard Device ID
     */
    inline const String & getNotecardUid (void) {
      check(); // Ensure the connection to the Notecard is initialized
      return _notecard_uid;
    }

    /**
     * @brief Get the topic type of the most recent R/W operations
     *
     * @return The current topic type
     *
     * @see TopicType
     */
    TopicType getTopicType (void) const {
      return _topic_type;
    }

    /**
     * @brief Initiate a synchronization operation with Notehub
     *
     * The Notecard maintains two queues: an inbound queue and an outbound
     * queue. The inbound queue is used to receive data from Notehub, while the
     * outbound queue is used to send data to Notehub. This method initiates a
     * synchronization operation between the Notecard and Notehub.
     *
     * As the name implies, this method is asynchronous and will only initiate
     * the synchronization operation. The actual synchronization operation will
     * be performed by the Notecard in the background.
     *
     * @param type[in] The type of synchronization operation to perform
     * @par
     * - SyncType::Full - synchronize both the inbound and outbound queues (default)
     * - SyncType::Inbound - synchronize only the inbound queues.
     * - SyncType::Outbound - synchronize only the outbound queues.
     *
     * @return 0 if successful, otherwise an error code
     *
     * @see SyncType
     * @see NotecardCommunicationError
     */
    int initiateNotehubSync (SyncType type = SyncType::Full) const;

    /**
     * @brief Set the inbound polling interval (in minutes)
     *
     * A cellular Notecard will receive inbound traffic from the Arduino IoT
     * Cloud in real-time. As such, the polling interval is used as a fail-safe
     * to ensure the Notecard is guaranteed to receive inbound traffic at the
     * interval specified by this method.
     *
     * Alternatively, a LoRa (or Satellite) Notecard does not maintain a
     * continuous connection, and therefore must rely on the polling interval to
     * establish the maximum acceptable delay before receiving any unsolicited,
     * inbound traffic from the Arduino IoT Cloud. The polling interval must
     * balance the needs of the application against the regulatory limitations
     * of LoRa (or bandwidth limitations and cost of Satellite).
     *
     * LoRaWAN Fair Use Policy:
     * https://www.thethingsnetwork.org/forum/t/fair-use-policy-explained/1300
     *
     * @param interval_min[in] The inbound polling interval (in minutes)
     *
     * @note Set the interval to 0 to disable inbound polling.
     */
    inline void setNotehubPollingInterval (int32_t interval_min) {
      _inbound_polling_interval_min = (interval_min ? interval_min : -1);
    }

    /**
     * @brief Set the topic type for R/W operations
     *
     * @param topic[in] The topic type
     * @par
     * - TopicType::Command - used to interact with the Arduino IoT Cloud.
     * - TopicType::Thing - used to send application data to the Arduino IoT Cloud.
     *
     * @see TopicType
     */
    void setTopicType (TopicType topic) {
      _topic_type = topic;
    }

    /**
     * @brief Set the WiFi credentials to be used by the Notecard
     *
     * @param ssid[in] The SSID of the WiFi network
     * @param pass[in] The password of the WiFi network
     *
     * @return 0 if successful, otherwise an error code
     *
     * @note This method is only applicable when using a Wi-Fi capable Notecard,
     *       and is unnecessary when using a Notecard with cellular connectivity.
     *       If the Notecard is not Wi-Fi capable, this method will be a no-op.
     *
     * @see NotecardCommunicationError
     */
    int setWiFiCredentials (const String & ssid, const String & pass);

    // ConnectionHandler interface
    virtual bool available() override;
    virtual unsigned long getTime() override;
    virtual int read() override;
    virtual int write(const uint8_t *buf, size_t size) override;

  protected:

    virtual NetworkConnectionState update_handleInit         () override;
    virtual NetworkConnectionState update_handleConnecting   () override;
    virtual NetworkConnectionState update_handleConnected    () override;
    virtual NetworkConnectionState update_handleDisconnecting() override;
    virtual NetworkConnectionState update_handleDisconnected () override;

  private:

    // Private members
    Notecard _notecard;
    String _device_id;
    String _notecard_uid;
    String _project_uid;
    HardwareSerial * _serial;
    TwoWire * _wire;
    uint8_t * _inbound_buffer;
    uint32_t _conn_start_ms;
    uint32_t _i2c_address;
    uint32_t _i2c_max;
    uint32_t _inbound_buffer_index;
    uint32_t _inbound_buffer_size;
    int32_t _inbound_polling_interval_min;
    uint32_t _uart_baud;
    bool _en_hw_int;
    TopicType _topic_type;

    // Private methods
    bool armInterrupt (void) const;
    bool configureConnection (bool connect) const;
    uint_fast8_t connected (void) const;
    J * getNote (bool pop = false) const;
    bool updateUidCache (void);
};

#endif /* ARDUINO_NOTECARD_CONNECTION_HANDLER_H_ */
