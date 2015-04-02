#if !defined ( CommandlineInterfaceImpl_H_NG )
#define CommandlineInterfaceImpl_H_NG
// ===========================================================================
/// @file
/// @brief This file contains inlined functions for CommandlineInterface
///
/// @copyright
/// @history
/// REF#        Who              When        What
///        Bennett Schneider     24-Jul-2007 Original development
/// 1122        Michael Pearce   21-Aug-2007 Original development contd...
/// @endhistory
///
// ===========================================================================

/*  ...Include files
*/
#include <QAppNG/CommandlineInterface.h>

namespace QAppNG
{

    /// @brief Add a new command to the Command-line Interface
        /// 
        /// @param[in]   a_cmd    The command string
        /// @param[in]   a_desc   A description of the command
        /// @param[in]   a_action The delegate to be used when command is called
        /// @retval      void
    inline
    void CommandlineInterface::AddCommand(const std::string& a_cmd, const std::string& a_desc, StatusDelegate a_action)
        {
        m_commands[a_cmd] = make_pair( (UInt32)m_delegates.size(), a_desc);
        m_delegates.push_back( a_action);

        // add new command to the help output:
        m_help_output.append(" ");
        m_help_output.append(a_cmd);
        for (size_t i = a_cmd.size(); i<20; i++)
        {
            m_help_output.append(" ");
        }

        m_help_output.append(a_desc);
        m_help_output.append("\r\n");
        }

    inline
    void CommandlineInterface::AddDescription(const std::string a_desc)
        {
            // add description of commands below the message
            m_help_output.append("\r\n");
            m_help_output.append(" <");
            m_help_output.append(a_desc);
            m_help_output.append(">\r\n");
        }


}
#endif // CommandlineInterfaceImpl_H_NG

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  ...End of file */


