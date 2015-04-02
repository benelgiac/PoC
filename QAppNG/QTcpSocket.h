#pragma once

#include "CalTypes.h"
#include <future>
#include <time.h>
#include "string.h" //for memcpy

// SOCKETs
#include <stdio.h>
#ifdef WIN32
#include "Winsock2.h"
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <arpa/inet.h>
static void closesocket( int socket ) { ::close( socket ); }
#endif

// --------------------------------------------------------------------------------------------------------------------
namespace QAppNG
{
    class QTcpSocket
    {
    public:
        static const UInt32 INTERVAL_BETWEEN_CONNECTION_ATTEMPT_SECONDS = 5;
        static const UInt32 MAX_IO_BUFFER_LENGTH_DEFAULT = 4096;

        QTcpSocket( UInt32 max_io_buffer_length )
            : m_remote_ip_address( "" )
            , m_remote_port( 0 )
            , m_local_ip_address( "" )
            , m_local_port( 0 )
            , m_socket_fd( socket( AF_INET, SOCK_STREAM, 0 ) )
            , m_error( -1 )
            , m_total_byte_sent( 0 )
            , m_previous_byte_sent( 0 )
            , m_error_byte_sent( 0 )
            , m_total_byte_received( 0 )
            , m_previous_byte_received( 0 )
            , m_error_byte_received( 0 )
            , m_is_connected( false )
            , m_first_pdu_received( 0 )
            , m_shutdown( false )
            , m_max_io_buffer_length( max_io_buffer_length )
        {
            m_io_buffer = (UInt8*)malloc( max_io_buffer_length );
            resetIoBuffer();
        }

        // end points data
        std::string m_remote_ip_address;
        UInt16      m_remote_port;
        std::string m_local_ip_address;
        UInt16      m_local_port;

        // IO Buffer
        UInt8 *m_io_buffer;

        // Socket FD
        int m_socket_fd;

        // error
        int m_error;

        // addData 
        // attempts to copy "length" bytes from input_data_ptr to I/O buffer. If no
        // room is available, I/O buffer is emptied (all data sent to network), then 
        // the copy is made
        bool addData( UInt8* input_data_ptr, size_t length )
        {
            if (m_io_buffer_current_used_bytes + length >= m_max_io_buffer_length)
            {
                return false;
            }

            memcpy( m_io_buffer_current_position_ptr, input_data_ptr, length );

            m_io_buffer_current_position_ptr += length;

            m_io_buffer_current_used_bytes += length;

            return true;
        }

        //returns true if I/O buffer has room for num_bytes_to_write_in_io_buffer bytes more.
        bool checkCapacity( size_t num_bytes_to_write_in_io_buffer )
        {
            return ((m_io_buffer_current_used_bytes + num_bytes_to_write_in_io_buffer) < m_max_io_buffer_length);
        }

        //returns number of bytes in use in the I/O buffer
        size_t getIoBufferUsedBytes() { return m_io_buffer_current_used_bytes; }

        //returns size of I/O buffer
        UInt32 getMaxIoBufferLength() { return m_max_io_buffer_length; }

        void resetIoBuffer()
        {
            // reset IO buffer handling values
            m_io_buffer_current_used_bytes = 0;
            m_io_buffer_current_position_ptr = m_io_buffer;
        }

        //counter for stats and rate
        // SEND
        UInt64 m_total_byte_sent;
        UInt64 m_previous_byte_sent;
        UInt64 m_error_byte_sent;

        // RECEIVE
        UInt64 m_total_byte_received;
        UInt64 m_previous_byte_received;
        UInt64 m_error_byte_received;
        // COMMN
        bool   m_is_connected;

        bool m_first_pdu_received;

        bool m_shutdown;

        typedef std::function<int( QTcpSocket& )> SingleThreadSingleClientReceivingDelegate;

        int echo() 
        { 
            return send(m_io_buffer, getIoBufferUsedBytes()); 
        };
        int send( UInt8 * buf, size_t len ) { return ::send( m_socket_fd, (char*)(buf), len, 0 ); };
        size_t getReceivedData( char * buf, UInt32 buf_len ) 
        {
            if (getIoBufferUsedBytes() < buf_len)
            {
                memcpy( (void*)buf, (void*)m_io_buffer, getIoBufferUsedBytes() );
                return getIoBufferUsedBytes();
            }
            else
            { 
                memcpy( (void*)buf, m_io_buffer, buf_len );
                return buf_len;
            }
        };

    //protected:
        size_t m_io_buffer_current_used_bytes;
        UInt8* m_io_buffer_current_position_ptr;
        UInt32 m_max_io_buffer_length;
        

        //this will start reception on a single socket, handling only
        // one connection. No threads will be spawned, so call this method
        // from a thread which is not the main one or you will block
        // everything. The provided method will be invoked everytime
        // data is available on the socket. The delegate is responsible to
        // copy the data elsewhere if needed. After the delegate closure, data
        // will not be valid!
        void startSingleThreadSingleClientReceiving( std::string ip_address, 
                                                     UInt16 port, 
                                                     std::string & welcome_message,
                                                     SingleThreadSingleClientReceivingDelegate process_read )
        {
            // CREATE RECEIVING SOCKET
            int accepting_connection_socket = socket( AF_INET, SOCK_STREAM, 0 );

            sockaddr_in serv_addr;
            memset( &serv_addr, 0, sizeof( serv_addr ) );
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_addr.s_addr = inet_addr( ip_address.c_str() );
            serv_addr.sin_port = htons( port );

            // BIND
            if (bind( accepting_connection_socket, (sockaddr *)&serv_addr, sizeof( serv_addr ) ) < 0)
            {
                perror( "ERROR on binding" );
                exit( 1 );
            }

            // LISTEN
            listen( accepting_connection_socket, 5 );
            while (!m_shutdown)
            {
                m_is_connected = false;
                // ACCEPT
                sockaddr_in cli_addr;
                socklen_t cli_len = sizeof( cli_addr );
                memset( &cli_addr, 0, sizeof( cli_addr ) );
                //blocking call 
                m_socket_fd = accept( accepting_connection_socket, (struct sockaddr *)&cli_addr, &cli_len );

                //connection received from client
                m_is_connected = true;
                m_remote_ip_address = inet_ntoa( cli_addr.sin_addr );
                m_remote_port = ntohs( cli_addr.sin_port );

                //reset IO buffer
                resetIoBuffer();
                UInt32 bytes_to_read = m_max_io_buffer_length;

                send( (UInt8 *)welcome_message.c_str(), welcome_message.length() );

                while (!m_shutdown)
                {
                    int recv_result_value = recv
                        ( m_socket_fd
                        , (char *)m_io_buffer_current_position_ptr
                        , bytes_to_read
                        , 0 );

                    if (recv_result_value <= 0)
                    {
                        //let's make a short pause to spare cpu. If there is data, it will come
                        std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
                    }
                    else
                    {
                        m_io_buffer_current_used_bytes += recv_result_value;
                        m_io_buffer_current_position_ptr += recv_result_value;
                        //invoke delegate passing pointer to buffer and len received
                        int rv = process_read( *this );

                        //reset IO buffer: the delegate has already taken care of copying the 
                        // received bytes elsewhere
                        resetIoBuffer();
                        bytes_to_read = m_max_io_buffer_length;

                        if (rv == 0)
                        { 
                            closesocket( m_socket_fd );
                            break;
                        }    
                    }
                }

            }
        };
    };
}

// --------------------------------------------------------------------------------------------------------------------
