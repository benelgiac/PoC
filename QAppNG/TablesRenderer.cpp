
/** ===================================================================================================================
* @file    TablesRenderer IMPLEMENTATION FILE
*
* @brief   Utility class used to format data passed as vectors (or multiple vectors)
*          
*
* @copyright
*
* @history
* REF#        Who                             When          What
* #3341       R. Buti                         Apr-2009      Added file
*
* @endhistory
* ===================================================================================================================
**/

#include <QAppNG/TablesRenderer.h>

#include <iomanip>

namespace QAppNG
{

    TablesRenders::TablesRenders( int row_length )
    {
        TablesHandler table_handler_input;
        table_handler = table_handler_input;
        m_table_to_render_row_length = row_length;

        m_vertical_sep_map[TablesHandler::e_NoFormatting] = " ";
        m_vertical_sep_map[TablesHandler::e_Title] = "*";
        m_vertical_sep_map[TablesHandler::e_Data] = "|";

        m_horiz_sep_map[TablesHandler::e_NoFormatting] = " ";
        m_horiz_sep_map[TablesHandler::e_Title] = "*";
        m_horiz_sep_map[TablesHandler::e_Data] = "-";

    };

    TablesRenders::~TablesRenders()
    {
    };

    std::string TablesRenders::renderFormattedData()
    {

        TableType input_table = table_handler.getRowsContainer();
        vector< TablesHandler::eData_type > rows_formatting_vector = table_handler.getFormattingVector();
        vector< int > formatted_blocks = table_handler.getFormattedBlocks();
        vector< TablesHandler::eHorizBarPresence_type > horizontal_bar_vector = table_handler.getMultipleRowsVector(input_table.size());

        int max_row_length = getMaxRowLength(input_table);
    
        if ( max_row_length > m_table_to_render_row_length )
        {
            m_table_to_render_row_length = max_row_length;
        }

        std::ostringstream output;
        int current_block = 0;
        TableType block_table;
        vector< TablesHandler::eData_type > rows_formatt_block;
        vector< TablesHandler::eHorizBarPresence_type > horizontal_bar_block_vector;
        //vector< TablesHandler::eHorizBarPresence_type > horizontal_bar_vector = table_handler.getMultipleRowsVector();
        std::string vert_sep = "";
        std::string horiz_sep = "";

        for (size_t i=0; i<formatted_blocks.size(); i++)
        {
            if( !horizontal_bar_vector.empty())
                horizontal_bar_block_vector.push_back( horizontal_bar_vector[i] );

            if (current_block == formatted_blocks[i])
            {
                block_table.push_back(input_table[i]);
                rows_formatt_block.push_back(rows_formatting_vector[i]);

                int j = static_cast<int>(i)+1;
                int max_index = static_cast<int>(formatted_blocks.size());
                if (j < max_index) // It avoid to have an index out of the maximum value
                {
                    if ( current_block != formatted_blocks[j] ) 
                    {
                        current_block++;
                        output << formatBlock(block_table, rows_formatt_block, horizontal_bar_block_vector);
                        // empty the vector
                        while (!block_table.empty())
                        {
                            block_table.pop_back();
                            rows_formatt_block.pop_back();
                        }
                        while (!horizontal_bar_block_vector.empty())
                            horizontal_bar_block_vector.pop_back();
                    }
                }
                else if(j == max_index)
                {
                    output << formatBlock(block_table, rows_formatt_block, horizontal_bar_block_vector);
                }
            }
            else
            {
                current_block++;
            }
        }
        return output.str();
    };

    std::string TablesRenders::formatBlock(vector< vector< std::string > > data_block, vector < TablesHandler::eData_type > rows_format_block, vector< TablesHandler::eHorizBarPresence_type > horizontal_bar_block_vector)
    {
        fillTable(data_block);
        std::ostringstream output;
        std::string vert_sep = " ";
        std::string horiz_sep = " ";

        /* 
                    - vector<int> column_size_vect -
        Vector containing the max lenght of each string
        */
        //vector<int> column_size_vect = CalculateColumnsSize(data_block,elements );
        vector<int> column_size_vect = calculateColumnsSize(data_block );

        /* 
        - vector<int> column_adj_size -
        Vector containing the max lenght of each data_block column
        Those values fixes the length of each columns
        */
        vector<int> column_adj_size = getColumnSpacingVector(column_size_vect);

        for (size_t j=0; j<data_block.size(); j++)
        {
            vert_sep = getVerticalSeparator(rows_format_block[j]);
            horiz_sep = getHorizontalSeparator(rows_format_block[j]);
        
            if ( j >=1 )
            {
                if ( horizBarHasToPrint(rows_format_block[j-1], rows_format_block[j]) )
                {
                    output << stampRow(horiz_sep, vert_sep, rows_format_block[j], column_adj_size , e_Top);
                }
            }
            else
            {
                output << stampRow(horiz_sep, vert_sep, rows_format_block[j], column_adj_size, e_Top);
            }

            output << vert_sep;
            int col_left_spacing = 0;

            vector<int> columns_left_spacing_vector = getLeftSpacingColVect(column_size_vect, column_adj_size);

            int col_right_spacing = 0;
            int current_row_size_sum = 0;

            //cicle on all the rows
            for (int i=0; i<static_cast<int>(data_block[j].size()); i++)
            {
                col_left_spacing = columns_left_spacing_vector[i];
                output << std::setw(col_left_spacing) << " ";
                current_row_size_sum += (col_left_spacing +1);
                output << data_block[j][i];
                current_row_size_sum += static_cast<int>(data_block[j][i].length());
                col_right_spacing = getRightSpacingCell(col_left_spacing, data_block[j][i], column_adj_size[i]);
                output << std::setw(col_right_spacing) << " ";
                current_row_size_sum += col_right_spacing + 1;
                output << vert_sep;
            }
            output << "\n";

            if( !horizontal_bar_block_vector.empty() )
                if( horizontal_bar_block_vector[j]!= TablesHandler::e_NoHorizBarOnTheBottom )
                    output << stampRow(horiz_sep,vert_sep, rows_format_block[j], column_adj_size, e_Short);
        }
        return output.str();
    }

    bool TablesRenders::horizBarHasToPrint(TablesHandler::eData_type previous_format_value, TablesHandler::eData_type current_format_value)
    {

        switch(previous_format_value)
        {
            case TablesHandler::e_NoFormatting:
                {
                    if (current_format_value != TablesHandler::e_NoFormatting)
                    {
                        return true;
                    }
                }
                break;
            case TablesHandler::e_Data:
                {
                    if ((current_format_value == TablesHandler::e_NoFormatting) || (current_format_value == TablesHandler::e_Title) )
                    {
                        return true;
                    }
                }
                break;
            default:
                return false;
                break;
        }
        return false;
    }

    vector<int> TablesRenders::getColumnSpacingVector(vector<int> column_size_vect)
    {
        int sum = 0;
        int space_for_cell_to_add =0;

        vector<int> column_adj_size;
        for (size_t k=0; k<column_size_vect.size(); k++)
        {
            sum += (column_size_vect[k]);
        }

        for (size_t k=0; k<column_size_vect.size(); k++)
        {
            column_adj_size.push_back(column_size_vect[k]);
        }

        space_for_cell_to_add = (m_table_to_render_row_length - static_cast<int>(column_size_vect.size()) - 1 - sum);
        int cycle = space_for_cell_to_add;
        for (int k=0; k<cycle; k++)
        {
            for (size_t i=0; i<column_size_vect.size(); i++)
            {
                if (space_for_cell_to_add > 0)
                {
                    column_adj_size[i] += 1;
                    space_for_cell_to_add -= 1;
                }
                else
                {
                    return column_adj_size;
                }
            }
        }
        return column_adj_size;
    }

    void TablesRenders::fillTable(vector< vector< std::string > > &data_block)
    {
        int max_size = 0;
        for(size_t j=0; j<data_block.size(); j++)
        {
            for (size_t i=0; i<data_block[j].size(); i++)
            {
                if (static_cast<int>(data_block[j].size()) > max_size)
                {
                    max_size = static_cast<int>(data_block[j].size());
                }
            }
        }

        for(size_t j=0; j<data_block.size(); j++)
        {
            if (static_cast<int>(data_block[j].size()) < max_size)
            {
                int cell_to_add = max_size - static_cast<int>(data_block[j].size());
                for (int i=0; i<cell_to_add; i++)
                {
                    data_block[j].push_back("-");
                }
            }
        }
    }

    int TablesRenders::getRightSpacingCell(int col_left_spacing, std::string cell_data, int column_size)
    {
        int right_spacing = 0;
        right_spacing = column_size - col_left_spacing - static_cast<int>(cell_data.length());
        return right_spacing;
    }

    vector<int> TablesRenders::getLeftSpacingColVect(vector<int> column_size_vect, vector<int> column_adj_size)
    {
        vector<int> left_spacing_vect;
        int left_sp = 0;
        for (size_t j=0; j<column_size_vect.size(); j++)
        {
            left_sp = (column_adj_size[j] - column_size_vect[j])/2;
            left_spacing_vect.push_back(left_sp);
        }
        return left_spacing_vect;
    }

    vector<int> TablesRenders::calculateColumnsSize(vector< vector< std::string > > data_block)
    {
        vector<int> max_columns_lenghts;
        int max_numb_of_col = getMaxNumberOfColumns(data_block);
        for (int j=0; j<max_numb_of_col; j++)
        {
            max_columns_lenghts.push_back(0);
        }
        if(data_block.size() == 1)
        {
            for (size_t i=0; i<data_block[0].size(); i++)
            {
                int a = static_cast<int>(data_block[0][i].length());
                if(max_columns_lenghts[i] < a)
                {
                    max_columns_lenghts[i] = a;
                }
            }
        }
        else
        {
            for (size_t j=0; j<data_block.size(); j++)
            {
                for (size_t i=0; i<data_block[j].size(); i++)
                {
                    if(max_columns_lenghts[i] < static_cast<int>(data_block[j][i].length()))
                    {
                        max_columns_lenghts[i] = static_cast<int>(data_block[j][i].length());
                    }
                }
            }
        }
        return max_columns_lenghts;
    }

    int TablesRenders::getMaxNumberOfColumns( vector< vector< std::string > > double_vector)
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
                if (max_row_size < static_cast<int>(double_vector[i].size()))
                {
                    max_row_size = static_cast<int>(double_vector[i].size());
                }
            }
        }
        return max_row_size;
    };

    int TablesRenders::getMaxRowLength( vector< vector< std::string > > double_vector)
    {
        int max_size = 0;
        int max_numb_of_col = getMaxNumberOfColumns(double_vector);
        vector<int> column_max_size = calculateColumnsSize(double_vector);
        if (double_vector.empty())
        {
            return 0;
        }
        else
        {
            for (int i=0; i<max_numb_of_col; i++)
            {
                max_size += (column_max_size[i] +3);
            }
        }
        max_size += 1;

        return max_size;
    };

    std::string TablesRenders::getVerticalSeparator(TablesHandler::eData_type formatting_type)
    {
        return m_vertical_sep_map.find(formatting_type)->second;
    };

    std::string TablesRenders::getHorizontalSeparator(TablesHandler::eData_type formatting_type)
    {
        return m_horiz_sep_map.find(formatting_type)->second;
    };

    std::string TablesRenders::stampRow(std::string horiz_sep, std::string /*vert_sep*/, TablesHandler::eData_type data_type, vector<int> column_adj_size, eStampEmptyRow empty_row_type)
    {
    
        std::ostringstream output;
        if ( (data_type == TablesHandler::e_Title) && (empty_row_type == e_Bottom) )
        {
            for (size_t i=0; i<column_adj_size.size(); i++)
            {
                output << horiz_sep << std::setw(column_adj_size[i]) << " ";
            }
            output << horiz_sep << "\n";
        }

        for (int j=0; j<(m_table_to_render_row_length); j++)
        {
            output << horiz_sep;
        }
        output << "\n";


        if ( (data_type == TablesHandler::e_Title) && (empty_row_type == e_Top) )
        {
            for (size_t i=0; i<column_adj_size.size(); i++)
            {
                output << horiz_sep << std::setw(column_adj_size[i]) << " ";
            }
            output << horiz_sep << "\n";
        }
        return output.str();
    };

    void TablesRenders::setSeparators(TablesHandler::eData_type formatting, std::string horiz_sep, std::string vert_sep)
    {
        m_horiz_sep_map[formatting] = horiz_sep;
        m_vertical_sep_map[formatting] = vert_sep;
    };

}
