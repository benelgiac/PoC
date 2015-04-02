#ifndef QSERIALIZER_H_NG
#define QSERIALIZER_H_NG

/** =================================================================================================================
* @file    QSerializer.h
*
* @brief   QSerializer Template-Class encloses a 
*          primitive types structure to send it
*          (by libComm) over the Network.
*
* @history
*  REF#        Who             When           What
* #5409        I. Velasco      Sett-2010      Original development
*
* @endhistory
* ===================================================================================================================
*/

#include <cal/DataBlock.h>
#include <cal/StreamDataReader.h>

namespace QAppNG
{

    /// ...HowTo...
    ///
    /// ************************ Host TX **********************
    /// struct MY_STRUCT {...} s1;    // structure we want send by libComm
    /// UInt32 ID = hash(MY_STRUCT);  // identifier for the struct Type (it could be an Enum)
    ///
    /// QSerializer<MY_STRUCT> ToSerialize(s1, ID);
    /// cal::CommConnector::Instance().SendData( &CommParams, ToSerialize ); // send data with libcomm
    ///
    ///
    /// ************************ Host RX **********************
    /// here get the 'std::shared_ptr<cal::DataBlock> db'
    /// from the libcomm receiver ( see. QLibCommServer.h )
    /// ...
    /// switch(QSerializer::ExtractID(db))
    /// {
    ///    case hash(MY_STRUCT):
    ///    {
    ///         QSerializer<MY_STRUCT> ToUnSerialize(db);
    ///         struct MY_STRUCT  s2 = ToUnSerialize.GetData();  // here we have the original struct.
    ///         break;
    ///    case hash(MY_STRUCT_2):
    ///         QSerializer<MY_STRUCT_2> ToUnSerialize(db);
    ///         struct MY_STRUCT_2  s3 = ToUnSerialize.GetData();  // here we have the original struct.
    ///         break;
    ///    ...
    /// }

    template <typename STRUCT>
    class QSerializer : public cal::DataBlock 
    {
    public:
        /// Default Constructor for TX struct
        /// @input 
        ///     STRUCT _val          : (mandatory) when we want sending the structure.
        ///     UInt32 _id           : (mandatory) structure type identifier 
        ///     cal::Timestamp _time : Timestamp when the class is created
        ///
        QSerializer( STRUCT _val, UInt32 _id, cal::Timestamp _time = cal::Timestamp(0,0));

        /// Default Constructor for RX struct
        /// @input 
        ///     std::shared_ptr<cal::DataBlock> _db : (mandatory)
        ///
        QSerializer( std::shared_ptr<cal::DataBlock> _db );

        /// Default Destructor
        virtual ~QSerializer();

        /// Static method to retrive the Struct identifier before Unserilized it
        static UInt32 ExtractID(std::shared_ptr<cal::DataBlock> _db);
    
        /// Getter functions
        inline UInt32             getId( void );
        inline cal::Timestamp     getTime( void );
        inline STRUCT             getData( void );

        /// to check if serialization/unserialization is ok
        bool  is_serialized( void );
        bool  is_unserialized( void );

        /// Debug Purpose
        std::shared_ptr<cal::DataBlock> getDataBlock( void );

    private:
        UInt32          m_id;
        UInt64          m_time_sec;
        UInt64          m_time_nsec;
        STRUCT          m_value;
        bool            m_is_serialized;
        bool            m_is_unserialized;

        /// Arrange the object before send it with libComm
        bool          serialize( void );

        /// Retrieve the Structure from std::shared_ptr<cal::DataBlock>
        bool          unserialize(std::shared_ptr<cal::DataBlock> _db);
    };


    //-------------------------------------------------------------------------------
    /* UNUSED
    static UInt32 ExtractID(std::shared_ptr<cal::DataBlock> _db)
    {
        return *((UInt32*)_db->begin()->m_pDataSection);
    }
    */

    //-------------------------------------------------------------------------------

    template <typename STRUCT>
    bool QSerializer<STRUCT>::is_serialized( void )
    {
        return m_is_serialized;
    }

    //-------------------------------------------------------------------------------

    template <typename STRUCT>
    bool QSerializer<STRUCT>::is_unserialized( void )
    {
        return m_is_unserialized;
    }

    //-------------------------------------------------------------------------------

    template <typename STRUCT>
    std::shared_ptr<cal::DataBlock> QSerializer<STRUCT>::getDataBlock( void )
    {
        cal::DataBlock * db = (cal::DataBlock *)(this);
        return std::shared_ptr<cal::DataBlock>(db);
    }

    //-------------------------------------------------------------------------------

    template <typename STRUCT>
    UInt32 QSerializer<STRUCT>::getId( void )
    {
        return m_id;
    }

    //-------------------------------------------------------------------------------

    template <typename STRUCT>
    cal::Timestamp QSerializer<STRUCT>::getTime( void )
    {
        return cal::Timestamp(m_time_sec, m_time_nsec);
    }

    //-------------------------------------------------------------------------------

    template <typename STRUCT>
    STRUCT QSerializer<STRUCT>::getData( void )
    {
        return m_value;
    }

    //-------------------------------------------------------------------------------

    template <typename STRUCT>
    QSerializer<STRUCT>::QSerializer(STRUCT _Val, UInt32 _id, cal::Timestamp _time)
    : cal::DataBlock()
    {
        memset(&m_value, 0, sizeof(STRUCT));
        m_is_serialized = false;
        m_is_unserialized = false;

        m_value = _Val;
        m_id = _id;
        m_time_sec = _time.GetTime();
        m_time_nsec = _time.GetFraction();

        // Serialize the structure
        if ( serialize() )
               m_is_serialized = true;
    }

    //-------------------------------------------------------------------------------

    template <typename STRUCT>
    QSerializer<STRUCT>::QSerializer(std::shared_ptr<cal::DataBlock> _db)
    : cal::DataBlock()
    {
        memset(&m_value, 0, sizeof(STRUCT));
        m_is_serialized = false;
        m_is_unserialized = false;

        m_id = 0;
        m_time_sec = 0;
        m_time_nsec = 0;

        // Unserialize the structure
        if ( unserialize(_db) )
            m_is_unserialized = true;
    }

    //-------------------------------------------------------------------------------

    template <typename STRUCT>
    QSerializer<STRUCT>::~QSerializer() {
    }

    //-------------------------------------------------------------------------------

    template <typename STRUCT>
    bool QSerializer<STRUCT>::serialize( void )
    {
        try {
            addNode(sizeof(UInt32), &m_id);
            addNode(sizeof(UInt64), &m_time_sec); 
            addNode(sizeof(UInt64), &m_time_nsec);

            UInt8* byte_buffer = (UInt8*)(&m_value);
            for (unsigned int i=0; i< sizeof(STRUCT); i++)
                 addNode(sizeof(UInt8), byte_buffer+i);

        }
        catch(std::exception _e)
        {
            return false;
        }
        return true;
    }

    //-------------------------------------------------------------------------------

    template <typename STRUCT>
    bool QSerializer<STRUCT>::unserialize( std::shared_ptr<cal::DataBlock> _db )
    {
        try {
            cal::StreamDataReader _datastream(_db);

            _datastream >> m_id;
            _datastream >> m_time_sec;
            _datastream >> m_time_nsec;

            UInt8* byte_buffer = (UInt8*) malloc(sizeof(STRUCT));
            if( byte_buffer != NULL)
            {
                UInt8* cursor = byte_buffer;
                for (unsigned int i=0; i< sizeof(STRUCT); i++)
                    _datastream >> *(cursor+i);

                m_value = *((STRUCT*)byte_buffer);
                free(byte_buffer);
                return true;
            }
            else
                return false;
        }
        catch(std::exception _e)
        {
            return false;
        }
    }
}
#endif //QSERIALIZER_H_NG

