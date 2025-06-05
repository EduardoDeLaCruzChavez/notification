/*
 * ESP32_MySQL - An optimized library for ESP32 to directly connect and execute SQL to MySQL database without intermediary.
 *
 * Copyright (c) 2024 Syafiqlim
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

/****************************
  ESP32_MySQL_Connection_Impl.h
  by Syafiqlim @ syafiqlimx
*****************************/
#include <ESP32_MySQL_Encrypt_Sha1.hpp>
#include <ESP32_MySQL_Connection.hpp>
#include <cstdio>

#include <cstdlib>

#define MAX_CONNECT_ATTEMPTS 10
#define CONNECT_DELAY_MS 1000
#define SUCCESS 1

/*
  connect - Connect to a MySQL server.

  This method is used to connect to a MySQL server. It will attempt to
  connect to the server as a client retrying up to MAX_CONNECT_ATTEMPTS.
  This permits the possibility of longer than normal network lag times
  for wireless networks. You can adjust MAX_CONNECT_ATTEMPTS to suit
  your environment.

  server[in]      IP address of the server as IPAddress type
  port[in]        port number of the server
  user[in]        user name
  password[in]    (optional) user password
  db[in]          (optional) default database

  Returns bool - True = connection succeeded
*/

string SQL_IPAddressTostring(const IPAddress &_address)
{
  string str = to_string(_address[0]);
  str += ".";
  str += to_string(_address[1]);
  str += ".";
  str += to_string(_address[2]);
  str += ".";
  str += to_string(_address[3]);

  return str;
}

//////////////////////////////////////////////////////////////

bool ESP32_MySQL_Connection::connect(const char *hostname, const uint16_t port, char *user, char *password, char *db)
{
  int connected = 0;
  int retries = 0;
  bool returnVal = false;

  ESP_LOGI("MySQL", "Connecting to Server: %s, Port = %d", hostname, port);

  if (db)
    ESP_LOGI("MySQL", "Using Database: %s", db);

  // Retry up to MAX_CONNECT_ATTEMPTS times.
  while (retries++ < MAX_CONNECT_ATTEMPTS)
  {
    connected = client->connect(hostname, port);

    ESP_LOGD("MySQL", "connected = %d", connected);

    if (connected != SUCCESS)
    {
      ESP_LOGD("MySQL", "Can't connect. Retry #%d", retries);
      vTaskDelay(pdMS_TO_TICKS(CONNECT_DELAY_MS));
    }
    else
    {
      break;
    }
  }

  if (connected != SUCCESS)
    return false;

  ESP_LOGI("MySQL", "Connect OK. Try reading packets");

  if (!read_packet())
  {
    ESP_LOGE("MySQL", "Can't connect. Error reading packets");
    return false;
  }

  ESP_LOGI("MySQL", "Try parsing packets");

  parse_handshake_packet();

  ESP_LOGI("MySQL", "Try send_authentication packets");

  send_authentication_packet(user, password, db);

  if (!read_packet())
  {
    ESP_LOGE("MySQL", "Can't connect. Error reading auth packets");
  }
  else if (get_packet_type() != ESP32_MYSQL_OK_PACKET)
  {
    parse_error_packet();
  }
  else
  {
    ESP_LOGW("MySQL", "Connected. Server Version = %s", server_version);
    returnVal = true;
  }

  if (server_version)
  {
    free(server_version); // don't need it anymore
    server_version = NULL;
  }

  return returnVal;
}

//////////////////////////////////////////////////////////////

Connection_Result ESP32_MySQL_Connection::connectNonBlocking(const char *hostname, const uint16_t port, char *user, char *password, char *db)
{
  int connected = 0;
  int retries = 0;
  Connection_Result returnVal = RESULT_FAIL;
  TickType_t now = 0;

  ESP_LOGI("MySQL", "Connecting to Server: %s, Port = %d", hostname, port);

  if (db)
    ESP_LOGI("MySQL", "Using Database: %s", db);

  while (retries < MAX_CONNECT_ATTEMPTS)
  {
    if ((now == 0) || (xTaskGetTickCount() - now) > pdMS_TO_TICKS(CONNECT_DELAY_MS))
    {
      now = xTaskGetTickCount();
      connected = client->connect(hostname, port);
      retries++;

      ESP_LOGD("MySQL", "connected = %d", connected);

      if (connected == SUCCESS)
      {
        break;
      }
      else
      {
        continue;
      }
    }
    else
    {
      vTaskDelay(1); // Small delay instead of yield()
    }
  }

  if (connected != SUCCESS)
    return RESULT_FAIL;

  ESP_LOGI("MySQL", "Connect OK. Try reading packets");

  if (!read_packet())
  {
    ESP_LOGE("MySQL", "Can't connect. Error reading packets");
    return RESULT_FAIL;
  }

  ESP_LOGI("MySQL", "Try parsing packets");

  parse_handshake_packet();

  ESP_LOGI("MySQL", "Try send_authentication packets");

  send_authentication_packet(user, password, db);

  if (!read_packet())
  {
    ESP_LOGE("MySQL", "Can't connect. Error reading auth packets");
  }
  else if (get_packet_type() != ESP32_MYSQL_OK_PACKET)
  {
    parse_error_packet();
  }
  else
  {
    ESP_LOGW("MySQL", "Connected. Server Version = %s", server_version);
    returnVal = RESULT_OK;
  }

  if (server_version)
  {
    free(server_version); // don't need it anymore
    server_version = NULL;
  }

  return returnVal;
}

//////////////////////////////////////////////////////////////

bool ESP32_MySQL_Connection::connect(const IPAddress &server, const uint16_t port, char *user, char *password, char *db)
{
  return connect(SQL_IPAddressTostring(server).c_str(), port, user, password, db);
}

//////////////////////////////////////////////////////////////

Connection_Result ESP32_MySQL_Connection::connectNonBlocking(const IPAddress &server, const uint16_t port, char *user, char *password, char *db)
{
  return connectNonBlocking(SQL_IPAddressTostring(server).c_str(), port, user, password, db);
}

//////////////////////////////////////////////////////////////

/*
  close - cancel the connection

  This method closes the connection to the server and frees up any memory
  used in the buffer.
*/
void ESP32_MySQL_Connection::close()
{
  if (connected())
  {
    client->flush();
    client->stop();

    // ESP32_MYSQL_LOGERROR("Disconnected");
  }
}
