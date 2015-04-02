
/** ===================================================================================================================
* @file    TablesHandler IMPLEMENTATION FILE
*
* @brief   Utility class used to store:
*            
*            - data passed as vectors (or multiple vectors) of strings (and various types as UIntX, int)
*            - formatting info of each row
*            - info about which rows must have the same columns size.
* @copyright
*
* @history
* REF#        Who                             When          What
* #3341       R. Buti                         Apr-2009      Added file
*
* @endhistory
* ===================================================================================================================
**/

#include <QAppNG/TablesHandler.h>

namespace QAppNG
{


    TablesHandler::TablesHandler()
    {
        m_formatted_blocks_counter = 0;
    };

    TablesHandler::~TablesHandler()
    {
    };


    void TablesHandler::addRow( vector< std::string > row_data, eData_type row_formatting_type, bool keep_formatting )
    {
        if ( keep_formatting == false )
            m_formatted_blocks_counter++;

        m_table_type_container.push_back(row_data);
        m_data_type_container.push_back(row_formatting_type);
        m_formatted_blocks_container.push_back(m_formatted_blocks_counter-1);
    };

    void TablesHandler::addRow( std::string row_data, eData_type row_formatting_type, bool keep_formatting )
    {
        if ( keep_formatting == false )
            m_formatted_blocks_counter++;

        vector<std::string> row_data_vector;
        row_data_vector.push_back(row_data);
        m_table_type_container.push_back(row_data_vector);
        m_data_type_container.push_back(row_formatting_type);
        m_formatted_blocks_container.push_back(m_formatted_blocks_counter-1);
    };

    void TablesHandler::addColumn( vector< std::string > row_data, eData_type row_formatting_type, bool keep_formatting )
    {
        if ( keep_formatting == false || m_formatted_blocks_counter==0 )
        {
            m_formatted_blocks_counter++;
        }

        vector<std::string> row_data_vector;

        for (size_t i=0; i<row_data.size(); i++)
        {
            row_data_vector.push_back(row_data[i]);

            m_table_type_container.push_back(row_data);
            m_data_type_container.push_back(row_formatting_type);
            m_formatted_blocks_container.push_back(m_formatted_blocks_counter-1);
        
            row_data_vector.pop_back();
        }
    };


    void TablesHandler::addTableByRows( vector< vector< std::string > > table_data, eData_type table_formatting_type, bool keep_formatting )
    {
        if ( keep_formatting == false )
            m_formatted_blocks_counter++;

        for (size_t i=0; i<table_data.size(); i++)
        {
            m_table_type_container.push_back(table_data[i]);
            m_data_type_container.push_back(table_formatting_type);
            m_formatted_blocks_container.push_back(m_formatted_blocks_counter-1);
        }
    };

    void TablesHandler::addTableByColumns( vector< vector< std::string > > table_data, eData_type table_formatting_type, bool keep_formatting )
    {    
        if ( keep_formatting == false )
            m_formatted_blocks_counter++;

        int max_size_rows = getMaxNumberOfColumns(table_data);
        for (int i=0; i<max_size_rows; i++) 
        {
            vector< std::string > row;
            for (size_t j=0; j<table_data.size(); j++)
            {
                if(static_cast<int>(table_data[j].size()) > i)
                    row.push_back(table_data[j][i]);
                else
                    row.push_back("-");
            }
            addRow(row, table_formatting_type, true);
        }
    };

    vector< TablesHandler::eHorizBarPresence_type > TablesHandler::getMultipleRowsVector( size_t input_table_size )
    {
        if(m_horizontal_bar_vector.empty())
        {
            for(size_t j=0; j<input_table_size; j++)
            {
                m_horizontal_bar_vector.push_back( e_HorizBarOnTheBottom );
            }
        }
        else if ( m_horizontal_bar_vector.size() < input_table_size )
        {
            size_t diff = input_table_size - m_horizontal_bar_vector.size();
            for ( size_t k = 0; k < diff; k++)
            {
                m_horizontal_bar_vector.push_back( e_HorizBarOnTheBottom );
            }
        }

        return m_horizontal_bar_vector;
    };


}
