#ifndef MOCK_UDP_H
#define MOCK_UDP_H

const int MOCK_UDP_PORT(13000);

sockaddr_in get_server_address(uint16_t udp_port)
{
    sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(udp_port);

    return server_address;
}

#endif