#ifndef INCLUDE_SENSITIVEINFODEFAULTCONFIG_NG
#define INCLUDE_SENSITIVEINFODEFAULTCONFIG_NG
/** ===================================================================================================================
  * @file    Sensitive Info HEADER FILE
  *
  * @brief   Sensitive Info Default Configuration
  *
  * @copyright
  *
  * @history
  * REF#        Who                                             When          What
  * #6376       F. Gragnani                                     Oct-2011      Add Default Condig
  *
  * @endhistory
  * ===================================================================================================================
  */

// STL includes
#include <string>
#include <map>

namespace QAppNG
{

    // --------------------------------------------------------------------------------------------------------------------

    class SensitiveInfoDefaultConfig : public std::map<std::string, std::string>
    {
    public:

        SensitiveInfoDefaultConfig() : std::map<std::string, std::string>()
        {
            insert(std::make_pair( "SuppressData",                         "deny_store_and_forward"));
            insert(std::make_pair( "DoNotWriteSensitivePduBufferToSlab",                     "true"));
            insert(std::make_pair( "Enable_All_IMSI_IMEI_Maps",                                 "0"));
        };
    };

}
#endif // INCLUDE_SENSITIVEINFODEFAULTCONFIG_NG
// --------------------------------------------------------------------------------------------------------------------
// End of file

