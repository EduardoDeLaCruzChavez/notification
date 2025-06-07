#pragma once

#include <lwip/sockets.h>
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define ESP32_MYSQL_GENERIC_VERSION "0.1.0"

// Clase IPAddress de ayuda (similar a Arduino)
class IPAddress
{
private:
    uint8_t octets[4];

public:
    IPAddress() : octets{0, 0, 0, 0} {}
    IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d) : octets{a, b, c, d} {}

    uint8_t operator[](int index) const
    {
        if (index >= 0 && index < 4)
        {
            return octets[index];
        }
        return 0;
    }
};

class Client
{
private:
    int sockfd = -1;
    bool bConnected = false;
    struct sockaddr_in server_addr;

public:
    // Constructor
    Client() {}

    // Constructor con descriptor de socket (para accept())
    Client(int fd)
    {
        sockfd = fd;
        bConnected = true;
    }

    // Destructor
    ~Client()
    {
        stop();
    }

    // Conectar a un servidor
    int connect(const char *host, uint16_t port)
    {
        if (bConnected)
        {
            stop();
        }
        int pt = (int)port;
        // Configurar dirección del servidor
        bzero(&server_addr, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(pt);

        if (inet_pton(AF_INET, host, &server_addr.sin_addr) == 0)
        {
            struct addrinfo tInfo = {}, *tServer = NULL;
            struct sockaddr_in *tSvAddr = NULL;
            char acPort[8] = {0};
            tInfo.ai_family = AF_INET;       // Solo IPv4
            tInfo.ai_socktype = SOCK_STREAM; // TCP
            itoa(pt, acPort, 10);

            int err = getaddrinfo(host, acPort, &tInfo, &tServer);
            if (err != 0 || tServer == NULL)
            {
                return -1;
            }

            tSvAddr = (struct sockaddr_in *)tServer->ai_addr; // tSvAddr->sin_addr
            inet_pton(AF_INET, inet_ntoa(tSvAddr->sin_addr), &server_addr.sin_addr);
            freeaddrinfo(tServer); // Liberar memoria
        }

        // Crear socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
        {
            printf("Error al crear socket: %s\n", strerror(errno));
            return 0;
        }
        struct timeval timeout;
        timeout.tv_sec = 10; // 10 segundos
        timeout.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

        socklen_t addr_len = sizeof(server_addr);
        // Conectar
        if (::connect(sockfd, (struct sockaddr *)&server_addr, addr_len) < 0)
        {
            printf("Error al conectar: %s\n", strerror(errno));
            close(sockfd);
            sockfd = -1;
            return 0;
        }

        bConnected = true;
        return 1;
    }

    // Conectar con dirección IP
    int connect(IPAddress ip, uint16_t port)
    {
        char ip_str[16];
        snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
        return connect(ip_str, port);
    }

    // Verificar si está conectado
    bool connected()
    {
        if (!bConnected)
            return false;

        // Verificar si el socket sigue activo
        char buf;
        int ret = recv(sockfd, &buf, 1, MSG_PEEK | MSG_DONTWAIT);
        if (ret == 0)
        {
            // bConnected = false;
            return false;
        }
        else if (ret < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
        {
            // bConnected = false;
            return false;
        }

        return true;
    }
    bool cconnected()
    {
        return bConnected;
    }
    // Cerrar conexión
    void stop()
    {
        if (sockfd >= 0)
        {
            close(sockfd);
            sockfd = -1;
        }
        bConnected = false;
    }

    // Escribir datos
    size_t write(const char *buf, size_t size)
    {
        if (!bConnected)
            return 0;

        ssize_t ret = send(sockfd, buf, size, 0);
        if (ret < 0)
        {
            printf("Error al escribir: %s\n", strerror(errno));
            stop();
            return 0;
        }
        return ret;
    }

    size_t write(char b)
    {
        return write(&b, 1);
    }

    // Leer datos
    int read(uint8_t *buf, size_t size)
    {
        if (!bConnected)
            return -1;

        ssize_t ret = recv(sockfd, buf, size, 0);
        if (ret < 0)
        {
            printf("Error al leer: %s\n", strerror(errno));
            stop();
            return -1;
        }
        else if (ret == 0)
        {
            // stop();
            return -1;
        }
        return ret;
    }

    int read()
    {
        uint8_t b;
        if (read(&b, 1) == 1)
        {
            return b;
        }
        return -1;
    }

    int available()
    {
        if (!bConnected)
            return 0;

        int count = 0;
        lwip_ioctl(sockfd, FIONREAD, &count);
        return count;
    }

    int peek()
    {
        if (!bConnected)
            return -1;

        uint8_t b;
        ssize_t ret = recv(sockfd, &b, 1, MSG_PEEK);
        if (ret <= 0)
        {
            return -1;
        }
        return b;
    }

    void flush()
    {
        // No hay implementación necesaria para sockets TCP
    }

    // Operador para verificar conexión
    operator bool()
    {
        return bConnected;
    }

    // Obtener descriptor de socket (para uso avanzado)
    int fd() const
    {
        return sockfd;
    }
};