#if !defined ( TABLESHANDLER_H_NG )
#define TABLESHANDLER_H_NG
/** ===================================================================================================================
* @file    TablesHandler HEADER FILE
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

#include <vector>
#include <string>
#include <sstream>

namespace QAppNG
{

    using std::vector;

    typedef vector< vector< std::string > > TableType;

    class TablesHandler 
    {

    public:
    
        enum eData_type
        {
            e_NoFormatting = 0,
            e_Title,
            e_Data
        };

        enum eHorizBarPresence_type
        {
            e_HorizBarOnTheBottom = 0,
            e_NoHorizBarOnTheBottom
        };

        TablesHandler();
        ~TablesHandler();

        // Get Methods
        TableType getRowsContainer(){return m_table_type_container;};
        vector< eData_type > getFormattingVector(){return m_data_type_container;};
        vector< int > getFormattedBlocks(){return m_formatted_blocks_container;};
        vector< eHorizBarPresence_type > getMultipleRowsVector( size_t input_table_size );

        void addHorizontalBarVector(vector< eHorizBarPresence_type > a_horiz_bar)
        {
            m_horizontal_bar_vector = a_horiz_bar;
        };

        template< class VECTORTYPE>
        int getMaxNumberOfColumns( vector< vector< VECTORTYPE > > double_vector)
        {
            int max_row_size = 0;
            if (double_vector.empty())
            {
                return 0;
            }
            else
            {
                for (size_t i=0; i<double_vector.size(); i++)
                {
                    if (max_row_size < static_cast<int>(double_vector[0].size()))
                    {
                        max_row_size = static_cast<int>(double_vector[0].size());
                    }
                }
            }
            return max_row_size;
        };

        // Convertion to string Methods
        static std::string convertIntToString(int input_data)
        {
            std::stringstream converter("");
            converter.clear();
            converter << input_data;
            return converter.str();
        };

        template <class SOURCETYPE>
        vector< std::string > convertVectorTypeToString(vector< SOURCETYPE > input_data)
        {
            vector < std::string > string_vector;
            for (size_t i=0; i<input_data.size(); i++)
            {
                std::stringstream converter("");
                converter.clear();
                converter << input_data[i];
                string_vector.push_back(converter.str());
            }
            return string_vector;
        };

        template <class SOURCETYPE>
        vector< vector< std::string > > convertTableTypeToString(vector< vector < SOURCETYPE > > input_data)
        {
            vector < vector< std::string > > string_vector;
            for (size_t i=0; i<input_data.size(); i++)
            {
                string_vector.push_back(convertVectorTypeToString(input_data[i]));
            }
            return string_vector;
        };

        //Add objects
        void addRow(vector< std::string > row_data, eData_type formatting_type = e_NoFormatting, bool keep_formatting = false);
        void addRow( std::string row_data, eData_type formatting_type = e_NoFormatting, bool keep_formatting = false);

        void addColumn(vector< std::string > row_data, eData_type formatting_type = e_NoFormatting, bool keep_formatting = false);
    
        void addRow( int  row_data, eData_type formatting_type = e_NoFormatting, bool keep_formatting = false)
        {
            if ( keep_formatting == false )
            {
                m_formatted_blocks_counter++;
            }

            vector< std::string > vector_of_strings;
            std::string row_data_string = convertIntToString(row_data);
            vector_of_strings.push_back(row_data_string);
            m_table_type_container.push_back(vector_of_strings);
            m_data_type_container.push_back(formatting_type);
            m_formatted_blocks_container.push_back(m_formatted_blocks_counter-1);
        };

        template <class VECTORTYPE>
        void addRow( vector< VECTORTYPE > row_data, eData_type formatting_type = e_NoFormatting, bool keep_formatting = false)
        {
            if ( keep_formatting == false )
            {
                m_formatted_blocks_counter++;
            }

            vector< std::string > vector_of_strings = convertVectorTypeToString<VECTORTYPE>(row_data);
            m_table_type_container.push_back(vector_of_strings);
            m_data_type_container.push_back(formatting_type);
            m_formatted_blocks_container.push_back(m_formatted_blocks_counter-1);
        };

        template <class VECTORTYPE>
        void addColumn( vector< VECTORTYPE > row_data, eData_type formatting_type = e_NoFormatting, bool keep_formatting = false)
        {
            if ( keep_formatting == false )
            {
                m_formatted_blocks_counter++;
            }

            vector< std::string > vector_of_strings = convertVectorTypeToString<VECTORTYPE>(row_data);
            vector<std::string> row_data_vector;

            for (size_t i=0; i<vector_of_strings.size(); i++)
            {
                row_data_vector.push_back(vector_of_strings[i]);

                m_table_type_container.push_back(row_data_vector);
                m_data_type_container.push_back(formatting_type);
                m_formatted_blocks_container.push_back(m_formatted_blocks_counter-1);
            
                row_data_vector.pop_back();
            }
        };

        /*
        addTableByRows Method:
        It adds table consisting of rows of vectors
        v1[0]    v1[1]    v1[2]
        v2[0]    v2[1]    v2[2]
        v3[0]    v3[1]    v3[2]
        */
    
        void addTableByRows(vector< vector< std::string > > table_data, eData_type formatting_type = e_NoFormatting, bool keep_formatting = false);

        template < class VECTORTYPE >
        void addTableByRows( vector< vector< VECTORTYPE > > table_data, eData_type formatting_type = e_NoFormatting, bool keep_formatting = false)
        {
            if ( keep_formatting == false )
            {
                m_formatted_blocks_counter++;
            }

            vector< vector< std::string > > vector_of_strings = convertTableTypeToString<VECTORTYPE>(table_data);
            for (size_t i=0; i<table_data.size(); i++)
            {
                m_table_type_container.push_back(vector_of_strings[i]);
                m_data_type_container.push_back(formatting_type);
                m_formatted_blocks_container.push_back(m_formatted_blocks_counter-1);
            }
        };

        /*
        addTableByColumns Method:
        Add table consisting of columns of vectors
        v1[0]    v2[0]    v3[0]
        v1[1]    v2[1]    v3[1]
        v1[2]    v2[2]    v3[2]
        */

        void addTableByColumns(vector< vector< std::string > > table_data, eData_type formatting_type = e_NoFormatting, bool keep_formatting = false);

        template < class VECTORTYPE >
        void addTableByColumns( vector< vector< VECTORTYPE > > table_data, eData_type formatting_type = e_NoFormatting, bool keep_formatting = false)
        {    
            if ( keep_formatting == false )
            {
                m_formatted_blocks_counter++;
            }

            int max_size_rows = getMaxNumberOfColumns(table_data);
            vector< vector< std::string > > vector_of_strings = convertTableTypeToString<VECTORTYPE>(table_data);
            for (int i=0; i<max_size_rows; i++)
            {
                vector< std::string > row;
                for (size_t j=0; j<vector_of_strings.size(); j++)
                {
                    row.push_back(vector_of_strings[j][i]);
                }
                addRow(row, formatting_type, true);
            }
        };

    protected:

    private:

        TableType m_table_type_container;
        vector< eData_type > m_data_type_container;
        vector< int > m_formatted_blocks_container;
        int m_formatted_blocks_counter;

        vector< eHorizBarPresence_type > m_horizontal_bar_vector;
        /* 
        m_horizontal_bar_vector Vector
    
        Add this vector to TableHandler object to enable/disable the horizontal bar
                                           ---------
        v[0] = e_HorizBarOnTheBottom       |   1   |
                                           ---------
        v[1] = e_NoHorizBarOnTheBottom     |  2.1  |
        v[2] = e_HorizBarOnTheBottom       |  2.2  |
                                           ---------
        v[3] = e_HorizBarOnTheBottom       |   3   |
                                           ---------
        */



    };
    
}
#endif // TABLESHANDLER_H_NG