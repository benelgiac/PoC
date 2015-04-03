/* 
 * File:   CCassClient.h
 * Author: giacomo_benelli
 *
 * Created on 3 aprile 2015, 14.57
 */

#ifndef CCASSCLIENT_H
#define	CCASSCLIENT_H

#include "cassandra.h"
#include <string>
#include <iostream>

namespace Cassandra 
{
    inline CassError printError(CassError error) 
    {
        std::cout << cass_error_desc(error) << "\n";
        return error;
    }
    
    class SimpleClient 
    {
    private:
        CassCluster* cluster;
        CassSession* session;
    public:
        inline CassSession* getSession() { return session; }
        CassError connect(const std::string nodes);
        void close();
    }; // end class
} // e

#endif	/* CCASSCLIENT_H */

