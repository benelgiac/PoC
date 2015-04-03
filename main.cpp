/* 
 * File:   main.cpp
 * Author: giacomo_benelli
 *
 * Created on 2 aprile 2015, 15.48
 */

#include <iostream>
#include <string>
#include "QAppNG/QTcpSocket.h"
#include "QAppNG/CCassClient.h"

int main(int argc, char * argv[])
{
    
    
    Cassandra::SimpleClient cass_client;
    cass_client.connect("127.0.0.1");
        
    std::cout << "See you soon" << std::endl;
    return 0;   
}
