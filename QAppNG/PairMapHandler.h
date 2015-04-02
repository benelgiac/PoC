#ifndef INCLUDE_PAIRMAPHANDLER_NG
#define INCLUDE_PAIRMAPHANDLER_NG

//include
#include <QAppNG/CPairMngEvolution.h>
#include "CNlPdu.h"
#include <QAppNG/QConfigManager.h>
#include <set>

namespace QAppNG
{

    enum SHOW_INFO_TYPE 
    {
        s_TYPE=0,
        s_DIRECTION,
        s_LAC,
        s_PC
    };

    class PairMapHandler : public QAppNG::Singleton<PairMapHandler>
    {
    public:
        PairMapHandler();
        virtual ~PairMapHandler(){};

        // Commands
        std::string pairShow( int selected_probe_ip, SHOW_INFO_TYPE show_type );
        std::string pairShowOfInput(int sel_probe_ip, UInt8 sel_input);
        std::string ShowLacList();
        std::string ShowMonitoredLac();
        std::string ShowPCList();
        std::string ShowMonitoredPCs();
        std::string showLinkBal();
        std::string dumpPCsPerProbe( bool pc_to_show, bool show_decto = false );
        std::string dumpALacsPerProbe();


    protected:

    private:

        std::string convertIntToString(int input_data);
        std::string checkIntValue(int value);
        std::string writeNoDataMessage();
        std::string decto383(int val);
        std::set<UInt16> getLacSet();
        std::vector< std::vector<std::string> > getLacSetWithInterface();
        std::set< std::pair<UInt16, UInt16> > getPCSet();
        std::set<UInt8> getProbeSetFromLac(UInt16 lac);
        std::vector< std::vector<std::string> > getInputsSetWithPCs(UInt16 bsc, UInt16 msc, UInt8 ip, LINK_TYPE type);

        std::string GetShowCommandResult(int _argc, char *_argv[], SHOW_INFO_TYPE _show_type, std::string _outfile);
        std::string CHPairShow(int argc, char *argv[]);
        std::string DirectionShow(int argc, char *argv[]);
        std::string LacShow(int argc, char *argv[]);
        std::string PcShow(int argc, char *argv[]);
        std::string ListLac(int argc, char *argv[]);
        std::string ListPC(int argc, char *argv[]);
        std::string linkBal(int argc, char *argv[]);
        std::string dumpDirectionsMap(int argc, char *argv[]);
        std::string dumpPCMap(int argc, char *argv[]);
        std::string dumpALacMap(int argc, char *argv[]);

        CPairMngEvolution m_pair_manager;
    };

}
#endif //INCLUDE_PAIRMAPHANDLER_NG

