#include <QAppNG/CCassClient.h>

namespace Cassandra
{

    CassError SimpleClient::connect(const std::string nodes) 
    {
        
        CassError rc = CASS_OK;
        std::cout << "Connecting to " << nodes << "\n";
   
        cluster = cass_cluster_new();
        session = cass_session_new();

        
        CassFuture* connect_future = NULL;
        cass_cluster_set_contact_points(cluster, "127.0.0.1");
        connect_future = cass_session_connect(session, cluster);
        cass_future_wait(connect_future);
        
        rc = cass_future_error_code(connect_future);
        if ( rc == CASS_OK )
        {
            std::cout << "Connected." << "\n";
        }
        else
        {
            printError(rc);
        }
        
        cass_future_free(connect_future);
        
        return rc;
    }
    
    void SimpleClient::close() 
    {
        std::cout << "Closing down cluster connection." << "\n";
        cass_session_close(session);
        cass_cluster_free(cluster);
    }

}