#if !defined ( TABLESRENDERER_H_NG )
#define TABLESRENDERER_H_NG
/** ===================================================================================================================
* @file    TablesRenderer HEADER FILE
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

#include <QAppNG/TablesHandler.h>
#include <string>
#include <vector>
#include <map>

namespace QAppNG
{

    class TablesRenders 
    {
    public:

        enum eStampEmptyRow
        {
            e_Top = 0,
            e_Bottom,
            e_Short
        };

        TablesHandler table_handler;

        TablesRenders( int row_length );

        ~TablesRenders();

        // Get Methods:
        std::string getVerticalSeparator(TablesHandler::eData_type formatting_type);
        std::string getHorizontalSeparator(TablesHandler::eData_type formatting_type);

        //Replace the default separators
        void setSeparators(TablesHandler::eData_type formatting, std::string horiz_sep, std::string vert_sep);

        std::string renderFormattedData();
    
    protected:

    private:

        //convert an mxn table in a nxn table filling empty cell with "-"
        void fillTable(vector< vector< std::string > > &data_block);

        // It manage a group of rows (block) that has the same formatting
        std::string formatBlock(vector< vector< std::string > > data_block, vector < TablesHandler::eData_type > rows_format_block, vector< TablesHandler::eHorizBarPresence_type > horizontal_bar_block_vector);
    
        //Print a single formatted row
        std::string stampRow(std::string input, std::string vert_sep, TablesHandler::eData_type data_type, vector<int> column_adj_size, eStampEmptyRow empty_row_type);
    
        // It returns true if the horizontal bar has to be printed
        bool horizBarHasToPrint(TablesHandler::eData_type previous_format_value, TablesHandler::eData_type current_format_value);
    
        //Calculation of double vector size
        int getMaxNumberOfColumns( vector< vector< std::string > > double_vector);
        int getMaxRowLength( vector< vector< std::string > > double_vector);
        vector<int> calculateColumnsSize(vector< vector< std::string > > data_block);

        //Calculation of formatting parameter
        int getRightSpacingCell(int col_left_spacing, std::string cell_data, int column_size);
        vector<int> getLeftSpacingColVect(vector<int> column_size_vect, vector<int> column_adj_size);
        vector<int> getColumnSpacingVector(vector<int> column_size_vect);
    
        int m_table_to_render_row_length;
        std::map<TablesHandler::eData_type, std::string> m_horiz_sep_map;
        std::map<TablesHandler::eData_type, std::string> m_vertical_sep_map;
    
    };

}
#endif // TABLESRENDERER_H_NG
    
