/* 
 * File:   main.cpp
 * Author: giacomo_benelli
 *
 * Created on 2 aprile 2015, 15.48
 */

#include <iostream>
#include <string>
#include "QAppNG/QTcpSocket.h"

int echo(QAppNG::QTcpSocket & sock)
{
    sock.send(sock.m_io_buffer, sock.m_io_buffer_current_used_bytes);
    return sock.m_io_buffer_current_used_bytes;
}

int main(int argc, char * argv[])
{
    QAppNG::QTcpSocket sock(3200);
    std::string ip_address ("0.0.0.0");
    std::string welcome_message ("Welcome to New Generation\n");
    
    sock.startSingleThreadSingleClientReceiving(ip_address, 8888, welcome_message, &echo);
    std::cout << "See you soon" << std::endl;
    return 0;   
}
