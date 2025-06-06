/*
 * ESP32_MySQL - An optimized library for ESP32 to directly connect and execute SQL to MySQL database without intermediary.
 *
 * Copyright (c) 2024 Syafiqlim
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

/****************************
  ESP32_MySQL_Packet_Impl.h
  by Syafiqlim @ syafiqlimx
*****************************/
#ifndef ESP32_MYSQL_PACKET_IMPL_H
#define ESP32_MYSQL_PACKET_IMPL_H

#include <ESP32_MySQL_Encrypt_Sha1.hpp>
#include <ESP32_MySQL_Packet.hpp>

#include <inttypes.h>
#include <cstring>
#include <stdlib.h>

#if (USING_WIFI_ESP_AT)
#define ESP32_MYSQL_DATA_TIMEOUT 10000
#else
#define ESP32_MYSQL_DATA_TIMEOUT 6000 // Client wait in milliseconds
#endif
//////

#define ESP32_MYSQL_WAIT_INTERVAL 300 // WiFi client wait interval

/*
  Constructor

  Initialize the buffer and store client instance.
*/
MySQL_Packet::MySQL_Packet(Client *client_instance)
{
  buffer = NULL;
  server_version = NULL;
  client = client_instance;
}

/*
  send_authentication_packet

  This method builds a response packet used to respond to the server's
  challenge packet (called the handshake packet). It includes the user
  name and password scrambled using the SHA1 seed from the handshake
  packet. It also sets the character set (default is 8 which you can
  change to meet your needs).

  Note: you can also set the default database in this packet. See
        the code before for a comment on where this happens.

  The authentication packet is defined as follows.

  chars                        Name
  -----                        ----
  4                            client_flags
  4                            max_packet_size
  1                            charset_number
  23                           (filler) always 0x00...
  n (Null-Terminated String)   user
  n (Length Coded Binary)      scramble_buff (1 + x chars)
  n (Null-Terminated String)   databasename (optional)

  user[in]        User name
  password[in]    password
  db[in]          default database
*/

void MySQL_Packet::send_authentication_packet(char *user, char *password, char *db)
{
  char this_buffer[256];
  char scramble[20];

  int size_send = 4;

  // client flags
  this_buffer[size_send] = char(0x0D);
  this_buffer[size_send + 1] = char(0xa6);
  this_buffer[size_send + 2] = char(0x03);
  this_buffer[size_send + 3] = char(0x00);
  size_send += 4;

  // max_allowed_packet
  this_buffer[size_send] = 0;
  this_buffer[size_send + 1] = 0;
  this_buffer[size_send + 2] = 0;
  this_buffer[size_send + 3] = 1;
  size_send += 4;

  // charset - default is 8
  this_buffer[size_send] = char(0x08);
  size_send += 1;

  for (int i = 0; i < 24; i++)
    this_buffer[size_send + i] = 0x00;

  size_send += 23;

  // user name
  memcpy((char *)&this_buffer[size_send], user, strlen(user));
  size_send += strlen(user) + 1;
  this_buffer[size_send - 1] = 0x00;

  if (scramble_password(password, scramble))
  {
    this_buffer[size_send] = 0x14;
    size_send += 1;

    for (int i = 0; i < 20; i++)
      this_buffer[i + size_send] = scramble[i];

    size_send += 20;
    this_buffer[size_send] = 0x00;
  }

  if (db)
  {
    memcpy((char *)&this_buffer[size_send], db, strlen(db));
    size_send += strlen(db) + 1;
    this_buffer[size_send - 1] = 0x00;
  }
  else
  {
    this_buffer[size_send + 1] = 0x00;
    size_send += 1;
  }

  // Write packet size
  int p_size = size_send - 4;
  store_int(&this_buffer[0], p_size, 3);
  this_buffer[3] = char(0x01);

  // Write the packet
  // ESP32_MYSQL_LOGINFO1("Writing this_buffer, size_send =", size_send);

  client->write(this_buffer, size_send);
  client->flush();
}

/*
  scramble_password - Build a SHA1 scramble of the user password

  This method uses the password hash seed sent from the server to
  form a SHA1 hash of the password. This is used to send back to
  the server to complete the challenge and response step in the
  authentication handshake.

  password[in]    User's password in clear text
  pwd_hash[in]    Seed from the server

  Returns bool - True = scramble succeeded
*/
bool MySQL_Packet::scramble_password(char *password, char *pwd_hash)
{
  char *digest;
  char hash1[20];
  char hash2[20];
  char hash3[20];
  char pwd_buffer[40];
  Encrypt_SHA1 Sha1;
  if (strlen(password) == 0)
    return false;

  // hash1
  Sha1.init();
  Sha1.write((uint8_t *)password, strlen(password));
  digest = (char *)Sha1.result();
  memcpy(hash1, digest, 20);

  // hash2
  Sha1.init();
  Sha1.write((uint8_t *)hash1, 20);
  digest = (char *)Sha1.result();
  memcpy(hash2, digest, 20);

  // hash3 of seed + hash2
  Sha1.init();
  memcpy(pwd_buffer, &seed, 20);
  memcpy(pwd_buffer + 20, hash2, 20);
  Sha1.write((uint8_t *)pwd_buffer, 40);
  digest = (char *)Sha1.result();
  memcpy(hash3, digest, 20);

  // XOR for hash4
  for (int i = 0; i < 20; i++)
    pwd_hash[i] = hash1[i] ^ hash3[i];

  return true;
}

/*
  wait_for_chars - Wait until data is available for reading

  This method is used to permit the connector to respond to servers
  that have high latency or execute long queries. The timeout is
  set by ESP32_MYSQL_DATA_TIMEOUT. Adjust this value to match the performance of
  your server and network.

  It is also used to read how many chars in total are available from the
  server. Thus, it can be used to know how large a data burst is from
  the server.

  chars_need[in]    chars count to wait for

  Returns integer - Number of chars available to read.
*/
int MySQL_Packet::wait_for_chars(const int &chars_need)
{
  const TickType_t wait_till = xTaskGetTickCount() + pdMS_TO_TICKS(ESP32_MYSQL_DATA_TIMEOUT * 2);
  int num = 0;

  do
  {
    // if ((now == 0) || ((xTaskGetTickCount() - now) > pdMS_TO_TICKS(ESP32_MYSQL_WAIT_INTERVAL)))
    //{
    // now = xTaskGetTickCount();
    num = client->available();

    ESP_LOGD("MySQL", "wait_for_chars: Num chars= %d, need chars= %d", num, chars_need);

    if (num >= chars_need)
      break;
    //}

    vTaskDelay(10 / portTICK_PERIOD_MS);
  } while (xTaskGetTickCount() < wait_till * 2);

  if (num == 0 && xTaskGetTickCount() >= wait_till)
  {
    ESP_LOGE("MySQL", "wait_for_chars: Timeout waiting for data");
  }

  ESP_LOGD("MySQL", "wait_for_chars: OK, Num chars= %d", num);
  return num;
}

/*
  read_packet - Read a packet from the server and store it in the buffer

  This method reads the chars sent by the server as a packet. All packets
  have a packet header defined as follows.

  chars                 Name
  -----                 ----
  3                     Packet Length
  1                     Packet Number

  Thus, the length of the packet (not including the packet header) can
  be found by reading the first 4 chars from the server then reading
  N chars for the packet payload.
*/

// TODO: Pass buffer pointer instead of using global buffer

bool MySQL_Packet::read_packet()
{
#define PACKET_HEADER_SZ 4
  char local[PACKET_HEADER_SZ];

  if (largest_buffer_size > 0)
    memset(buffer, 0, largest_buffer_size);

  // Read packet header
  if (wait_for_chars(PACKET_HEADER_SZ) < PACKET_HEADER_SZ)
  {
    packet_len = 0;
    ESP_LOGD("MySQL", "read_packet: Timeout reading header");
    return false;
  }

  packet_len = 0;

  for (int i = 0; i < PACKET_HEADER_SZ; i++)
    local[i] = client->read();

  // Get packet length
  packet_len = local[0];
  packet_len += (local[1] << 8);
  packet_len += ((uint32_t)local[2] << 16);

  ESP_LOGD("MySQL", "read_packet: packet_len= %d", packet_len);

  // Check for valid packet
  if ((packet_len < 0) || (packet_len > MAX_TRANSMISSION_UNIT))
  {
    ESP_LOGE("MySQL", "read_packet: Invalid packet length");
    packet_len = 0;
    return false;
  }

  if (largest_buffer_size < packet_len + PACKET_HEADER_SZ)
  {
    if (largest_buffer_size == 0)
    {
      largest_buffer_size = packet_len + PACKET_HEADER_SZ;
      ESP_LOGD("MySQL", "read_packet: First time allocate buffer, size = %d", largest_buffer_size);
      buffer = (char *)malloc(largest_buffer_size);
    }
    else
    {
      largest_buffer_size = packet_len + PACKET_HEADER_SZ;
      ESP_LOGD("MySQL", "read_packet: Reallocate buffer, size = %d", largest_buffer_size);
      buffer = (char *)realloc(buffer, largest_buffer_size);
    }
  }

  if (buffer == NULL)
  {
    ESP_LOGE("MySQL", "read_packet: Memory allocation failed");
    largest_buffer_size = 0;
    return false;
  }
  else
  {
    memset(buffer, 0, largest_buffer_size);
  }

  for (int i = 0; i < PACKET_HEADER_SZ; i++)
    buffer[i] = local[i];

  for (int i = PACKET_HEADER_SZ; i < packet_len + PACKET_HEADER_SZ; i++)
    buffer[i] = client->read();

  return true;
}
/*
  parse_handshake_packet - Decipher the server's challenge data

  This method reads the server version string and the seed from the
  server. The handshake packet is defined as follows.

   chars                        Name
   -----                        ----
   1                            protocol_version
   n (Null-Terminated String)   server_version
   4                            thread_id
   8                            scramble_buff
   1                            (filler) always 0x00
   2                            server_capabilities
   1                            server_language
   2                            server_status
   2                            server capabilities (two upper chars)
   1                            length of the scramble seed
  10                            (filler)  always 0
   n                            rest of the plugin provided data
                                (at least 12 chars)
   1                            \0 char, terminating the second part of
                                 a scramble seed
*/

void MySQL_Packet::parse_handshake_packet()
{
  if (!buffer)
  {
    ESP_LOGE("MySQL", "parse_handshake_packet: NULL buffer");
    return;
  }

  int i = 5;

  do
  {
    i++;
  } while (buffer[i - 1] != 0x00);

  if (i > 5)
  {
    server_version = (char *)malloc(i - 5);

    if (server_version)
    {
      strncpy(server_version, (char *)&buffer[5], i - 5);
      server_version[i - 5 - 1] = 0;
      ESP_LOGD("MySQL", "Server version: %s", server_version);
    }
  }

  // Capture the first 8 characters of seed
  i += 4; // Skip thread id

  for (int j = 0; j < 8; j++)
  {
    seed[j] = buffer[i + j];
  }

  // Capture rest of seed
  i += 27; // skip ahead

  for (int j = 0; j < 12; j++)
  {
    seed[j + 8] = buffer[i + j];
  }
}

/*
  parse_error_packet - Display the error returned from the server

  This method parses an error packet from the server and displays the
  error code and text via Serial.print. The error packet is defined
  as follows.

  Note: the error packet is already stored in the buffer since this
        packet is not an expected response.

  chars                       Name
  -----                       ----
  1                           field_count, always = 0xff
  2                           errno
  1                           (sqlstate marker), always '#'
  5                           sqlstate (5 characters)
  n                           message
*/

void MySQL_Packet::parse_error_packet()
{
  if (!buffer)
  {
    ESP_LOGE("MySQL", "parse_error_packet: NULL buffer");
    return;
  }

  int __errno = read_int(5, 2);
  ESP_LOGE("MySQL", "Server error: %d", __errno);

  // Print error message if needed
  char *msg = (char *)&buffer[13];
  ESP_LOGE("MySQL", "Error message: %s", msg);
}

/*
  get_packet_type - Returns the packet type received from the server.

   chars                       Name
   -----                       ----
   1   (Length Coded Binary)   field_count, always = 0
   1-9 (Length Coded Binary)   affected_rows
   1-9 (Length Coded Binary)   insert_id
   2                           server_status
   2                           warning_count
   n   (until end of packet)   message

  Returns integer - 0 = successful parse, packet type if not an Ok packet
*/

int MySQL_Packet::get_packet_type()
{
  if (!buffer)
  {
    ESP_LOGE("MySQL", "get_packet_type: NULL buffer");
    return -1;
  }

  int type = buffer[4];
  ESP_LOGD("MySQL", "Packet type: %d", type);
  return type;
}

/*
  get_lcb_len - Retrieves the length of a length coded binary value

  This reads the first char from the offset into the buffer and returns
  the number of chars (size) that the integer consumes. It is used in
  conjunction with read_int() to read length coded binary integers
  from the buffer.

  Returns integer - number of chars integer consumes
*/

int MySQL_Packet::get_lcb_len(const int &offset)
{
  if (!buffer)
  {
    // ESP32_MYSQL_LOGERROR("MySQL_Packet::get_lcb_len: NULL buffer");
    return 0;
  }

  int read_len = buffer[offset];

  if (read_len > 250)
  {
    // read type:
    char type = buffer[offset + 1];

    if (type == 0xfc)
      read_len = 2;
    else if (type == 0xfd)
      read_len = 3;
    else if (type == 0xfe)
      read_len = 8;
  }
  else
  {
    read_len = 1;
  }

  // ESP32_MYSQL_LOGDEBUG1("MySQL_Packet::get_lcb_len: read_len= ", read_len);

  return read_len;
}

/*
  read_int - Retrieve an integer from the buffer in size chars.

  This reads an integer from the buffer at offset position indicated for
  the number of chars specified (size).

  offset[in]      offset from start of buffer
  size[in]        number of chars to use to store the integer

  Returns integer - integer from the buffer
*/

int MySQL_Packet::read_int(const int &offset, const int &size)
{
  int value = 0;
  int new_size = 0;

  if (!buffer)
  {
    // ESP32_MYSQL_LOGERROR("MySQL_Packet::read_int: NULL buffer");
    return -1;
  }

  if (size == 0)
    new_size = get_lcb_len(offset);

  if (size == 1)
    return buffer[offset];

  new_size = size;
  int shifter = (new_size - 1) * 8;

  for (int i = new_size; i > 0; i--)
  {
    value += (buffer[i - 1] << shifter);
    shifter -= 8;
  }

  return value;
}

/*
  store_int - Store an integer value into a char array of size chars.

  This writes an integer into the buffer at the current position of the
  buffer. It will transform an integer of size to a length coded binary
  form where 1-3 chars are used to store the value (set by size).

  buff[in]        pointer to location in internal buffer where the
                  integer will be stored
  value[in]       integer value to be stored
  size[in]        number of chars to use to store the integer
*/
void MySQL_Packet::store_int(char *buff, const long &value, const int &size)
{
  if (!buff)
  {
    // ESP32_MYSQL_LOGERROR("MySQL_Packet::store_int: NULL buffer");
    return;
  }

  memset(buff, 0, size);

  if (value <= 0xff)
    buff[0] = (char)value;
  else if (value <= 0xffff)
  {
    buff[0] = (char)value;
    buff[1] = (char)(value >> 8);
  }
  else if (value <= 0xffffff)
  {
    buff[0] = (char)value;
    buff[1] = (char)(value >> 8);
    buff[2] = (char)(value >> 16);
  }
  else if (value > 0xffffff)
  {
    buff[0] = (char)value;
    buff[1] = (char)(value >> 8);
    buff[2] = (char)(value >> 16);
    buff[3] = (char)(value >> 24);
  }
}

/*
  read_lcb_int - Read an integer with len encoded char

  This reads an integer from the buffer looking at the first char in the offset
  as the encoded length of the integer.

  offset[in]      offset from start of buffer

  Returns integer - integer from the buffer
*/

int MySQL_Packet::read_lcb_int(const int &offset)
{
  int len_size = 0;
  int value = 0;

  if (!buffer)
  {
    // ESP32_MYSQL_LOGERROR("MySQL_Packet::read_lcb_int: NULL buffer");
    return -1;
  }

  len_size = buffer[offset];

  if (len_size < 252)
  {
    return buffer[offset];
  }
  else if (len_size == 252)
  {
    len_size = 2;
  }
  else if (len_size == 253)
  {
    len_size = 3;
  }
  else
  {
    len_size = 8;
  }

  int shifter = (len_size - 1) * 8;

  for (int i = len_size; i > 0; i--)
  {
    value += (buffer[offset + i] << shifter);
    shifter -= 8;
  }

  return value;
}

/*
  print_packet - Print the contents of a packet via Serial.print

  This method is a diagnostic method. It is best used to decipher a
  packet from the server (or before being sent to the server). If you
  are looking for additional program memory space, you can safely
  delete this method.
*/

void MySQL_Packet::print_packet()
{
  if (!buffer)
  {
    // ESP32_MYSQL_LOGERROR("MySQL_Packet::print_packet: NULL buffer");
    return;
  }

  // ESP32_MYSQL_LOGDEBUG3("Packet: ", buffer[3], " contains no. chars = ", packet_len + 3);

  // ESP32_MYSQL_LOGDEBUG0("  HEX: ");

  for (int i = 0; i < packet_len + 3; i++)
  {
    continue;
    // ESP32_MYSQL_LOGDEBUG0(String(buffer[i], HEX));
    // ESP32_MYSQL_LOGDEBUG0(" ");
  }

  // ESP32_MYSQL_LOGDEBUG0LN("");

  // ESP32_MYSQL_LOGDEBUG0("ASCII: ");

  //  for (int i = 0; i < packet_len + 3; i++)
  //  ESP32_MYSQL_LOGDEBUG0((char)buffer[i]);

  // ESP32_MYSQL_LOGDEBUG0LN("");
}

#endif // ESP32_MYSQL_PACKET_IMPL_H
