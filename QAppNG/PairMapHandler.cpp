#include <QAppNG/PairMapHandler.h>
#include <QAppNG/TablesRenderer.h>
#include <QAppNG/CommandlineInterface.h>

namespace QAppNG
{

    using std::set;
    using std::pair;
    using std::vector;

    PairMapHandler::PairMapHandler()
    {
        QAppNG::CommandlineInterface::instance().AddDescription("Show Commands");
        QAppNG::CommandlineInterface::instance().AddCommand("pairShow", "Shows the links type ( optional: [-p=IDprobe] [-o] )", fastdelegate::MakeDelegate(this, &PairMapHandler::CHPairShow));
        QAppNG::CommandlineInterface::instance().AddCommand("dirShow", "Shows the input directions ( optional: [-o] )", fastdelegate::MakeDelegate(this, &PairMapHandler::DirectionShow));
        QAppNG::CommandlineInterface::instance().AddCommand("lacShow", "Shows the monitored lacs for Abis links( optional: [-o] )", fastdelegate::MakeDelegate(this, &PairMapHandler::LacShow));
        QAppNG::CommandlineInterface::instance().AddCommand("pcShow", "Show the monitored point codes for A links( optional: [-o] )", fastdelegate::MakeDelegate(this, &PairMapHandler::PcShow));
   
        QAppNG::CommandlineInterface::instance().AddDescription("Point Codes Commands");
        QAppNG::CommandlineInterface::instance().AddCommand("lacList", "List of monitored lacs ( optional: [-v][-o] )", fastdelegate::MakeDelegate(this, &PairMapHandler::ListLac));
        QAppNG::CommandlineInterface::instance().AddCommand("pcList", "List of monitored point codes ( optional: [-v][-o] )", fastdelegate::MakeDelegate(this, &PairMapHandler::ListPC));
    
        QAppNG::CommandlineInterface::instance().AddDescription("A Interface Dump Commands");
        QAppNG::CommandlineInterface::instance().AddCommand("dumpADirs", "Dump on file directions of all ts.", fastdelegate::MakeDelegate(this, &PairMapHandler::dumpDirectionsMap));
        QAppNG::CommandlineInterface::instance().AddCommand("dumpALACs", "Dump on file discovered LACs of all ts. ", fastdelegate::MakeDelegate(this, &PairMapHandler::dumpALacMap));
        QAppNG::CommandlineInterface::instance().AddCommand("dumpPCs", "Dump on file discovered Point Codes of all ts. -d option shows the PCs in a different notation ( optional [-d] ). ", fastdelegate::MakeDelegate(this, &PairMapHandler::dumpPCMap));

        QAppNG::CommandlineInterface::instance().AddDescription("Balancing Commands");
        QAppNG::CommandlineInterface::instance().AddCommand("linkBal", "Show link balancing", fastdelegate::MakeDelegate(this, &PairMapHandler::linkBal));
    }

    std::string PairMapHandler::convertIntToString(int input_data)
    {
        std::stringstream converter("");
        converter.clear();
        converter << input_data;
        return converter.str();
    };

    std::string PairMapHandler::writeNoDataMessage()
    {
        TablesRenders message_table(25);
        std::string mess = "No data is present into Maps.";
        message_table.table_handler.addRow(mess);
        message_table.setSeparators(TablesHandler::e_NoFormatting, ".", ".");
        return message_table.renderFormattedData();
    };

    std::string PairMapHandler::pairShow(int selected_probe_ip, SHOW_INFO_TYPE show_type)
    {
        std::ostringstream output;
        TablesRenders data_table(20);

        vector<UInt8> probe_ip_vector = m_pair_manager.getProbeList();

        if( (probe_ip_vector.size() == 1) && (probe_ip_vector[0] == 0) )
        {
            return writeNoDataMessage();
        }

        if (selected_probe_ip!=0)
        {
            probe_ip_vector.clear();
            probe_ip_vector.push_back(selected_probe_ip);
        }

        // Titles definition
        vector< std::string> pairs_title_vector;
        pairs_title_vector.push_back("PAIRS");
        pairs_title_vector.push_back("INP");
    
        switch(show_type)
        {
        case s_TYPE:        
            {
            pairs_title_vector.push_back("TYPE");
            pairs_title_vector.push_back("INP");
            pairs_title_vector.push_back("TYPE");
            }
            break;
        case s_DIRECTION:
            {
                pairs_title_vector.push_back("DIR");
                pairs_title_vector.push_back("INP");
                pairs_title_vector.push_back("DIR");
            }
            break;
        case s_LAC:
            {
            pairs_title_vector.push_back("LAC");
            pairs_title_vector.push_back("INP");
            pairs_title_vector.push_back("LAC");
            }
            break;
        case s_PC:
            {
                pairs_title_vector.push_back("BSC-MSC");
                pairs_title_vector.push_back("INP");
                pairs_title_vector.push_back("BSC-MSC");
            }

            break;
        default:
            break;
        }
    
        vector<TablesHandler::eHorizBarPresence_type> horiz_bar_vect;
        size_t ip_vector_size = probe_ip_vector.size();
        for (size_t i = 0; i < ip_vector_size; i++)
        {
            INPUT_TYPE input_probe_type = m_pair_manager.getInputTypeOfProbe(probe_ip_vector[i]);
            bool show_probe_info = true;
        
            if (show_type == s_LAC)
            {
                if ( ( input_probe_type==MTP2 && m_pair_manager.linkTypeDiscovery(probe_ip_vector[i]) == MULT) )
                    show_probe_info = false;
            }

            if (show_type == s_PC)
            {
                if ( ( input_probe_type==MTP2 && m_pair_manager.linkTypeDiscovery(probe_ip_vector[i]) == MULT) 
                    || input_probe_type == LAPD )
                    show_probe_info = false;
            }

            if (show_probe_info)
            {
                std::ostringstream ip_probe_title;
                ip_probe_title << "PROBE NUMBER: " << data_table.table_handler.convertIntToString(static_cast<int>(probe_ip_vector[i]));
                data_table.table_handler.addRow(ip_probe_title.str(), data_table.table_handler.e_Title);
                data_table.table_handler.addRow(pairs_title_vector, data_table.table_handler.e_Data);

                if( show_type==s_LAC )
                {
                    // Titles have not multiple rows.
                    horiz_bar_vect.push_back(data_table.table_handler.e_HorizBarOnTheBottom);
                    horiz_bar_vect.push_back(data_table.table_handler.e_HorizBarOnTheBottom);
                }

                //data vectors definition
                vector<std::string> pairs;
                vector<std::string> first_input_vector;
                vector<std::string> second_input_vector;
                vector<std::string> first_info_vector;
                vector<std::string> second_info_vector;
                vector< std::pair<UInt8, UInt8> > probe_pairing;

                probe_pairing = m_pair_manager.getProbePairing(probe_ip_vector[i]);

                vector< std::pair<UInt8, UInt8> >::iterator pairing_it;
                int pairs_counter =0;
                int multiple_rows_counter = 0;
                for (pairing_it = probe_pairing.begin(); pairing_it < probe_pairing.end(); pairing_it++)    
                {
                    ++pairs_counter;
                    std::ostringstream first_pairing_el;
                    std::ostringstream second_pairing_el;
                    std::ostringstream first_info_el;
                    std::ostringstream second_info_el;
                    std::ostringstream first_input_info; 
                    std::ostringstream second_input_info;

                    switch(show_type)
                    {
                    case s_TYPE:
                        {
                            first_input_info << m_pair_manager.findTypeStringFromInput(probe_ip_vector[i], pairing_it->first);
                            second_input_info << m_pair_manager.findTypeStringFromInput(probe_ip_vector[i], pairing_it->second);
                        }
                        break;
                    case s_DIRECTION:
                        {
                            first_input_info << m_pair_manager.findDirStringFromInput(probe_ip_vector[i], pairing_it->first);
                            second_input_info << m_pair_manager.findDirStringFromInput(probe_ip_vector[i], pairing_it->second);
                        }
                        break;
                    case s_LAC:
                        {
                            if ( input_probe_type == LAPD )
                            {
                                first_input_info << checkIntValue(static_cast<int>(m_pair_manager.findAbisLacFromInput(probe_ip_vector[i], pairing_it->first)));
                                second_input_info << checkIntValue(static_cast<int>(m_pair_manager.findAbisLacFromInput(probe_ip_vector[i], pairing_it->second)));
                                horiz_bar_vect.push_back(data_table.table_handler.e_HorizBarOnTheBottom);
                            }
                            // We have to manage multiple LACs on the same timeslot
                            else if ( input_probe_type == MTP2 )
                            {
                                set<UInt16> first_lac = (m_pair_manager.findALacFromInput(probe_ip_vector[i], pairing_it->first));
                                set<UInt16> second_lac = (m_pair_manager.findALacFromInput(probe_ip_vector[i], pairing_it->second));
                                int max_set_size;
                                max_set_size = static_cast<int>(first_lac.size() > second_lac.size() ? first_lac.size() : second_lac.size());
                                multiple_rows_counter = max_set_size;
                                if ( max_set_size<=1 )
                                {
                                    horiz_bar_vect.push_back(data_table.table_handler.e_HorizBarOnTheBottom);
                                }
                                else
                                {
                                    for (int k = 1; k < max_set_size; k++)
                                    {
                                        horiz_bar_vect.push_back(data_table.table_handler.e_NoHorizBarOnTheBottom);
                                    }
                                    horiz_bar_vect.push_back(data_table.table_handler.e_HorizBarOnTheBottom);
                                }
                                set<UInt16> :: iterator first_lac_it;
                                for (first_lac_it = first_lac.begin(); first_lac_it != first_lac.end(); first_lac_it++)    
                                {
                                    std::ostringstream input_info;
                                    input_info << checkIntValue(static_cast<int>( *first_lac_it ));
                                    first_info_vector.push_back(input_info.str());
                                }
                                if ( static_cast<int>(first_lac.size()) < max_set_size )
                                {
                                    int diff = max_set_size - static_cast<int>(first_lac.size());
                                    for (int l = 0; l < diff; l++)
                                        first_info_vector.push_back(" ");
                                }
                                else if ( max_set_size == 0 )
                                    first_info_vector.push_back(" ");

                                set<UInt16> :: iterator second_lac_it;
                                for (second_lac_it = second_lac.begin(); second_lac_it != second_lac.end(); second_lac_it++)    
                                {
                                    std::ostringstream input_info;
                                    input_info << checkIntValue(static_cast<int>( *second_lac_it ));
                                    second_info_vector.push_back(input_info.str());
                                }
                                if ( static_cast<int>(second_lac.size()) < max_set_size )
                                {
                                    int diff = max_set_size - static_cast<int>(second_lac.size());
                                    for (int l = 0; l < diff; l++)
                                        second_info_vector.push_back(" ");
                                }
                                else if ( max_set_size == 0 )
                                    second_info_vector.push_back(" ");
                            }
                        
                        }
                        break;
                    case s_PC:
                        {
                            DIR first_direction = m_pair_manager.findDirFromInput(probe_ip_vector[i], pairing_it->first);
                            DIR second_direction = m_pair_manager.findDirFromInput(probe_ip_vector[i], pairing_it->second);

                            if (first_direction == DIR_UPLINK)
                            {
                                UInt16 first_msc = m_pair_manager.findOpcFromInput(probe_ip_vector[i], pairing_it->first);
                                UInt16 first_bsc = m_pair_manager.findDpcFromInput(probe_ip_vector[i], pairing_it->first);
                                first_input_info << checkIntValue(static_cast<int>(first_bsc)) << "-" << checkIntValue(static_cast<int>(first_msc));
                            }
                            else if (first_direction == DIR_DOWNLINK)
                            {
                                UInt16 first_msc = m_pair_manager.findDpcFromInput(probe_ip_vector[i], pairing_it->first);
                                UInt16 first_bsc = m_pair_manager.findOpcFromInput(probe_ip_vector[i], pairing_it->first);
                                first_input_info << checkIntValue(static_cast<int>(first_bsc)) << "-" << checkIntValue(static_cast<int>(first_msc));
                            }

                            if (second_direction == DIR_UPLINK)
                            {
                                UInt16 second_msc = m_pair_manager.findOpcFromInput(probe_ip_vector[i], pairing_it->second);
                                UInt16 second_bsc = m_pair_manager.findDpcFromInput(probe_ip_vector[i], pairing_it->second);
                                second_input_info << checkIntValue(static_cast<int>(second_bsc)) << "-" << checkIntValue(static_cast<int>(second_msc));
                            }
                            else if (second_direction == DIR_DOWNLINK)
                            {
                                UInt16 second_msc = m_pair_manager.findDpcFromInput(probe_ip_vector[i], pairing_it->second);
                                UInt16 second_bsc = m_pair_manager.findOpcFromInput(probe_ip_vector[i], pairing_it->second);
                                second_input_info << checkIntValue(static_cast<int>(second_bsc)) << "-" << checkIntValue(static_cast<int>(second_msc));
                            }
                        }
                        break;
                    default:
                        break;
                    }

                    if (multiple_rows_counter<=1)
                    {
                        pairs.push_back(convertIntToString(static_cast<int>(pairs_counter)));
                        first_pairing_el << static_cast<int>(pairing_it->first);
                        first_input_vector.push_back(first_pairing_el.str());
                        second_pairing_el << static_cast<int>(pairing_it->second);
                        second_input_vector.push_back(second_pairing_el.str());
                        if ( input_probe_type != MTP2 || show_type != s_LAC)
                        {
                            // For the A Interface probes OR show type is not LAC we've already added the info vector
                            first_info_vector.push_back(first_input_info.str());
                            second_info_vector.push_back(second_input_info.str());
                        }
                    }
                    else
                    {
                        pairs.push_back(convertIntToString(static_cast<int>(pairs_counter)));
                        first_pairing_el << static_cast<int>(pairing_it->first);
                        first_input_vector.push_back(first_pairing_el.str());
                        second_pairing_el << static_cast<int>(pairing_it->second);
                        second_input_vector.push_back(second_pairing_el.str());
                        for ( int k = 1; k < multiple_rows_counter; k++ )
                        {
                            pairs.push_back(" ");
                            first_input_vector.push_back(" ");
                            second_input_vector.push_back(" ");
                        }
                    }
                }

                vector< vector<std::string> > probe_info;
                probe_info.push_back(pairs);
                probe_info.push_back(first_input_vector);
                probe_info.push_back(first_info_vector);
                probe_info.push_back(second_input_vector);
                probe_info.push_back(second_info_vector);

                data_table.table_handler.addTableByColumns(probe_info, data_table.table_handler.e_Data, true);
                if( !horiz_bar_vect.empty() )
                    data_table.table_handler.addHorizontalBarVector(horiz_bar_vect);
            }
        
            if ( m_pair_manager.linkTypeDiscovery(probe_ip_vector[i]) == MULT 
                && m_pair_manager.getInputTypeOfProbe(probe_ip_vector[i])==MTP2
                && (show_type == s_PC || show_type == s_LAC))
            {
                std::ostringstream ip_probe_title;
                ip_probe_title << "PROBE NUMBER: " << data_table.table_handler.convertIntToString(static_cast<int>(probe_ip_vector[i]));
                data_table.table_handler.addRow(ip_probe_title.str(), data_table.table_handler.e_Title);

                if(show_type == s_LAC)
                    data_table.table_handler.addRow("Links are Multiplied, please use the dumpALACs command to get monitored LACs.", data_table.table_handler.e_Data);
                else
                    data_table.table_handler.addRow("Links are Multiplied, please use the dumpPCs command to get monitored PCs.", data_table.table_handler.e_Data);
            }
        }
    
        output << data_table.renderFormattedData();
    
        return output.str();
    };

    std::string PairMapHandler::checkIntValue(int value)
    {
        if (value == 0)
        {
            return "-";
        }
        else
        {
            return convertIntToString(value);
        }
    };

    std::string PairMapHandler::pairShowOfInput(int sel_probe_ip, UInt8 sel_input)
    {
        std::ostringstream output;
        TablesRenders data_table(25);

        //UInt32 key = 0;

        // Titles definition
        // Primary Title
        std::string probe_title = "PROBE NUMBER: ";
        std::string input_title = " - INPUT: ";

        // Secondary titles
        vector< std::string > pairs_title_vector;
        pairs_title_vector.push_back("TS");
        pairs_title_vector.push_back("LAC");

        INPUT_TYPE input_type_probe = m_pair_manager.getInputTypeOfProbe(sel_probe_ip);
        if (input_type_probe == MTP2)
        {
            pairs_title_vector.push_back("BSC");
            pairs_title_vector.push_back("MSC");
        }
        pairs_title_vector.push_back("DIR");

        std::ostringstream title;
        title << probe_title << convertIntToString(static_cast<int>(sel_probe_ip)) << input_title << convertIntToString(static_cast<int>(sel_input));
        data_table.table_handler.addRow(title.str(), data_table.table_handler.e_Title);
        data_table.table_handler.addRow(pairs_title_vector, data_table.table_handler.e_Data);

        int row_counter = 0;
        for (int i = 0; i < MAX_TSS; i++)
        {
            //key = m_pair_manager.GetMapKey(sel_probe_ip, sel_input, i);
            vector< std::string > data_row;
            std::string direction_string;

            if ( m_pair_manager.getInputType(static_cast<UInt8>(sel_probe_ip), sel_input, i) != INP_UNK )
            {
                row_counter++;

                UInt16 lac = 0;
                if (input_type_probe == LAPD)
                    lac = m_pair_manager.getAbisInputLAC(static_cast<UInt8>(sel_probe_ip), sel_input, static_cast<UInt8>(i));
                // To do
                //else if (input_type_probe == MTP2)

                DIR direction = m_pair_manager.getInputDirection(static_cast<UInt8>(sel_probe_ip), sel_input, static_cast<UInt8>(i));
                direction_string = m_pair_manager.convertDirectionToString(direction);
                data_row.push_back(convertIntToString(static_cast<int>(i)));
                data_row.push_back(checkIntValue(static_cast<int>(lac)));

                UInt16 opc = 0;
                UInt16 dpc = 0;
                if (m_pair_manager.getInputTypeOfProbe(sel_probe_ip) == MTP2)
                {
                    opc = m_pair_manager.getInputOPC(static_cast<UInt8>(sel_probe_ip), sel_input, static_cast<UInt8>(i));
                    dpc = m_pair_manager.getInputDPC(static_cast<UInt8>(sel_probe_ip), sel_input, static_cast<UInt8>(i));

                    switch(direction)
                    {
                    case DIR_UPLINK:
                        {
                            data_row.push_back(checkIntValue(static_cast<int>(opc)));
                            data_row.push_back(checkIntValue(static_cast<int>(dpc)));
                        }
                        break;
                    case DIR_DOWNLINK:
                        {
                            data_row.push_back(checkIntValue(static_cast<int>(dpc)));
                            data_row.push_back(checkIntValue(static_cast<int>(opc)));
                        }
                        break;
                    default:
                        {
                            data_row.push_back("-");
                            data_row.push_back("-");
                        }
                        break;
                    }
                }
                data_row.push_back(direction_string);
                data_table.table_handler.addRow(data_row, data_table.table_handler.e_Data, true);
            }
        }

        if (row_counter == 0)
        {
            return writeNoDataMessage();
        }
    
        output << data_table.renderFormattedData();

        return output.str();
    };

    std::string PairMapHandler::ShowLacList()
    {
        std::ostringstream output;
        TablesRenders data_table(15);
        data_table.table_handler.addRow("Lac list", data_table.table_handler.e_Title);

        vector< vector<std::string> > lacs = getLacSetWithInterface();
        vector< std::string > subtitle;
        subtitle.push_back("LACs");
        subtitle.push_back("Interface Type");
        data_table.table_handler.addRow(subtitle, TablesHandler::e_Data);
        data_table.table_handler.addTableByRows(lacs, TablesHandler::e_Data, true);
        output << data_table.renderFormattedData();

        return output.str();

    };

    std::string PairMapHandler::ShowMonitoredLac()
    {
        std::ostringstream output;
        TablesRenders data_table(25);
        data_table.table_handler.addRow("Lacs Info", data_table.table_handler.e_Title);

        set<UInt16> lacs = getLacSet();
        set<UInt16>::iterator lacIt;

        for (lacIt=lacs.begin(); lacIt!=lacs.end(); lacIt++)
        {
            // Lac Title
            std::ostringstream lac_title;
            lac_title << "Lac: " << convertIntToString(static_cast<int>(*lacIt));
            data_table.table_handler.addRow(lac_title.str(), data_table.table_handler.e_Title);

            set<UInt8> probe_set = getProbeSetFromLac((*lacIt));
            set<UInt8>::iterator probeIt;
            for (probeIt=probe_set.begin(); probeIt!=probe_set.end(); probeIt++)
            {
                INPUT_TYPE probe_interface_type = m_pair_manager.getInputTypeOfProbe(*probeIt);
                // Probe Title
                std::ostringstream title;
                title << "Probe: " << convertIntToString(static_cast<int>(*probeIt));
                data_table.table_handler.addRow(title.str(), data_table.table_handler.e_Data);
            
                // Get pairing of current probe
                vector< std::pair<UInt8, UInt8> > probe_pairing;
                probe_pairing = m_pair_manager.getProbePairing(static_cast<int>(*probeIt));

                // Inputs Title
                std::ostringstream inputs_title;
                vector<std::string> inputs_title_vector;
                inputs_title_vector.push_back("Input");
                inputs_title_vector.push_back("Input");
                data_table.table_handler.addRow(inputs_title_vector, data_table.table_handler.e_Data);

                vector< std::pair<UInt8, UInt8> >::iterator pairing_it;
                for (pairing_it = probe_pairing.begin(); pairing_it!=probe_pairing.end(); pairing_it++)    
                {
                    vector<std::string> input_couple;
                    UInt8 first_input = pairing_it->first;
                    UInt8 second_input = pairing_it->second;
                    bool first_inp_is_present = false;
                    bool second_inp_is_present = false;

                    for (size_t ts = 0; ts < MAX_TSS; ts++)
                    {
                        UInt32 current_key = m_pair_manager.GetMapKey(static_cast<int>(*probeIt), first_input, static_cast<UInt8>(ts));
                        if ( probe_interface_type == LAPD)
                        {
                            if ( m_pair_manager.getAbisInputLAC(current_key) == (*lacIt) )
                                first_inp_is_present = true;
                        }
                        else if ( probe_interface_type == MTP2 )
                        {
                            set<UInt16> a_lacs_set = m_pair_manager.getAInputLAC(current_key);
                            set<UInt16>::iterator a_lac_it;
                            for (a_lac_it=a_lacs_set.begin(); a_lac_it!=a_lacs_set.end(); a_lac_it++)
                            {
                                if ( (*a_lac_it) == (*lacIt))
                                    first_inp_is_present = true;
                            }
                        }
                    }

                    for (size_t ts = 0; ts < MAX_TSS; ts++)
                    {
                        UInt32 current_key = m_pair_manager.GetMapKey(static_cast<int>(*probeIt), second_input, static_cast<UInt8>(ts));
                        if ( probe_interface_type == LAPD)
                        {
                            if ( m_pair_manager.getAbisInputLAC(current_key) == (*lacIt) )
                                second_inp_is_present = true;
                        }
                        else if ( probe_interface_type == MTP2 )
                        {
                            set<UInt16> a_lacs_set = m_pair_manager.getAInputLAC(current_key);
                            set<UInt16>::iterator a_lac_it;
                            for (a_lac_it=a_lacs_set.begin(); a_lac_it!=a_lacs_set.end(); a_lac_it++)
                            {
                                if ( (*a_lac_it) == (*lacIt))
                                    second_inp_is_present = true;
                            }
                        }
                    }
                
                    if (first_inp_is_present)
                    {
                        input_couple.push_back(convertIntToString(static_cast<int>(first_input)));
                    }
                    else
                    {
                        input_couple.push_back("-");
                    }

                    if (second_inp_is_present)
                    {
                        input_couple.push_back(convertIntToString(static_cast<int>(second_input)));
                    }
                    else
                    {
                        input_couple.push_back("-");
                    }
                    if ( (first_inp_is_present == true) || (second_inp_is_present == true) )
                    {
                        data_table.table_handler.addRow(input_couple, data_table.table_handler.e_Data, true);
                    }
                }
            }
        }

        output << data_table.renderFormattedData();
        return output.str();

    };

    std::string PairMapHandler::ShowMonitoredPCs()
    {
        std::ostringstream output;
        TablesRenders data_table(30);

        // Add Title
        data_table.table_handler.addRow("Point Codes Info", data_table.table_handler.e_Title);

        vector<UInt8> probe_ip_vector = m_pair_manager.getProbeList();

        set< pair<UInt16, UInt16> > point_codes = getPCSet();
        set< pair<UInt16, UInt16> >::iterator PcIt;

        for (PcIt=point_codes.begin(); PcIt!=point_codes.end(); PcIt++)
        {        
            // Add Secondary Title
            vector< std::string > pc_title_vector;
            std::ostringstream first_column_title;
            first_column_title << "BSC: " << convertIntToString(static_cast<int>(PcIt->first));
            pc_title_vector.push_back(first_column_title.str());
            std::ostringstream second_column_title;
            second_column_title << "MSC: " << convertIntToString(static_cast<int>(PcIt->second));
            pc_title_vector.push_back(second_column_title.str());
            data_table.table_handler.addRow(pc_title_vector, data_table.table_handler.e_Data);
        
            vector<UInt8>::iterator ipIt;
            for (ipIt=probe_ip_vector.begin(); ipIt<probe_ip_vector.end(); ipIt++)
            {
                if (m_pair_manager.getInputTypeOfProbe(*ipIt) == MTP2)
                {
                    // Add Probe Title
                    std::ostringstream ip_probe_title;
                    ip_probe_title << "PROBE: " << convertIntToString(static_cast<int>(*ipIt));

                    // Add Inputs Title
                    vector< std::string > inputs_title_vector;
                    inputs_title_vector.push_back("Inputs from BSC");
                    inputs_title_vector.push_back("Inputs from MSC");

                    UInt16 current_bsc = PcIt->first;
                    UInt16 current_msc = PcIt->second;
                    vector< vector<std::string> > input_pairs;
                    LINK_TYPE link_type = m_pair_manager.linkTypeDiscovery(*ipIt);
                    input_pairs = getInputsSetWithPCs(current_bsc, current_msc, *ipIt, link_type);
                    if ((!input_pairs[0].empty()) && (!input_pairs[1].empty()))
                    {
                        // Add Titles and data
                        data_table.table_handler.addRow(ip_probe_title.str(), data_table.table_handler.e_Data);
                        data_table.table_handler.addRow(inputs_title_vector, data_table.table_handler.e_Data);
                        data_table.table_handler.addTableByColumns(input_pairs, data_table.table_handler.e_Data, true);
                    }
                }
            }
        }
        output << data_table.renderFormattedData();
        return output.str();

    };

    vector< vector<std::string> > PairMapHandler::getInputsSetWithPCs(UInt16 bsc, UInt16 msc, UInt8 ip, LINK_TYPE type)
    {
        vector< vector<std::string> > inputs;
        vector<std::string> first_inputs_vector;
        vector<std::string> second_inputs_vector;


    
        switch(type)
        {
        case STD:
            {
                vector< std::pair<UInt8, UInt8> > probe_pairing;
                probe_pairing = m_pair_manager.getProbePairing(ip);
                vector< std::pair<UInt8, UInt8> >::iterator pairing_it;

                for (pairing_it = probe_pairing.begin(); pairing_it < probe_pairing.end(); pairing_it++)    
                {
                    UInt16 bsc_from_map = 0;
                    UInt16 msc_from_map= 0;
                    UInt8 first_input = pairing_it->first;
                    UInt8 second_input = pairing_it->second;

                    bool first_input_is_present = false;
                    bool second_input_is_present = false;


                    for (int ts=0; ts<MAX_TSS; ts++)
                    {
                        // First Input
                        UInt32 first_key = m_pair_manager.GetMapKey(ip, first_input, static_cast<UInt8>(ts));
                        DIR first_direction = m_pair_manager.getInputDirection(first_key);
                        if ( first_direction == DIR_DOWNLINK)
                        {
                            bsc_from_map = m_pair_manager.getInputDPC(first_key);
                            msc_from_map = m_pair_manager.getInputOPC(first_key);
                        }
                        else if( first_direction == DIR_UPLINK )
                        {
                            msc_from_map = m_pair_manager.getInputDPC(first_key);
                            bsc_from_map = m_pair_manager.getInputOPC(first_key);
                        }
                        if ( (bsc_from_map == bsc) && (msc_from_map == msc) )
                        {
                            first_input_is_present = true;
                        }

                        // Second Input
                        UInt32 second_key = m_pair_manager.GetMapKey(ip, second_input, static_cast<UInt8>(ts));
                        DIR second_direction = m_pair_manager.getInputDirection(second_key);
                        if ( second_direction == DIR_DOWNLINK)
                        {
                            bsc_from_map = m_pair_manager.getInputDPC(second_key);
                            msc_from_map = m_pair_manager.getInputOPC(second_key);
                        }
                        else if( first_direction == DIR_UPLINK )
                        {
                            msc_from_map = m_pair_manager.getInputDPC(second_key);
                            bsc_from_map = m_pair_manager.getInputOPC(second_key);
                        }
                        if ( (bsc_from_map == bsc) && (msc_from_map == msc) )
                        {
                            second_input_is_present = true;
                        }
                    }
                    if ( (first_input_is_present) || (second_input_is_present) )
                    {
                        if (first_input_is_present)
                        {
                            first_inputs_vector.push_back(convertIntToString(static_cast<int>(first_input)));
                        }
                        else
                        {
                            first_inputs_vector.push_back("-");
                        }

                        if (second_input_is_present)
                        {
                            second_inputs_vector.push_back(convertIntToString(static_cast<int>(second_input)));
                        }
                        else
                        {
                            second_inputs_vector.push_back("-");
                        }
                    }
                }
            }
            break;
        case MULT:
            {
                for (int input = 0; input < MAX_INPUTS; input++)
                {
                    bool input_is_present = false;

                    for (int ts = 0; ts < MAX_TSS; ts++)
                    {
                        UInt16 bsc_from_map = 0;
                        UInt16 msc_from_map= 0;

                        UInt32 key = m_pair_manager.GetMapKey(ip, input, static_cast<UInt8>(ts));
                        DIR direction = m_pair_manager.getInputDirection(key);

                        if ( direction == DIR_DOWNLINK )
                        {
                            bsc_from_map = m_pair_manager.getInputDPC(key);
                            msc_from_map = m_pair_manager.getInputOPC(key);
                        }
                        else if( direction == DIR_UPLINK )
                        {
                            msc_from_map = m_pair_manager.getInputDPC(key);
                            bsc_from_map = m_pair_manager.getInputOPC(key);
                        }
                        if ( ( bsc_from_map == bsc && msc_from_map == msc ) && ( bsc!=0 && msc!=0) )
                        {
                            input_is_present = true;
                        }
                    }

                    if (input_is_present)
                    {
                        first_inputs_vector.push_back(convertIntToString(static_cast<int>(input)));
                        second_inputs_vector.push_back(convertIntToString(static_cast<int>(input)));
                    }
                }
            }
            break;
        default:
            break;
        }


        inputs.push_back(first_inputs_vector);
        inputs.push_back(second_inputs_vector);

        return inputs;
    };

    std::string PairMapHandler::ShowPCList()
    {
        std::ostringstream output;
        TablesRenders data_table(20);
    
        // Add Title
        data_table.table_handler.addRow("Point Codes list", data_table.table_handler.e_Title);

        // Add Secondary Title
        vector< std::string > title_vector;
        title_vector.push_back("BSC");
        title_vector.push_back("MSC");
        data_table.table_handler.addRow(title_vector, data_table.table_handler.e_Data);
    
        set< pair<UInt16, UInt16> > point_codes = getPCSet();
        set< pair<UInt16, UInt16> >::iterator PcIt;
        vector< vector<UInt16> > bsc_msc_vector;
        UInt16 bsc = 0;
        UInt16 msc = 0;

        for (PcIt=point_codes.begin(); PcIt!=point_codes.end(); PcIt++)
        {
            vector<UInt16> pc_row;
            bsc = (*PcIt).first;
            msc = (*PcIt).second;
            if ( (bsc != 0) && (msc != 0) )
            {
                pc_row.push_back(bsc);
                pc_row.push_back(msc);
                bsc_msc_vector.push_back(pc_row);
            }
        }

        data_table.table_handler.addTableByRows(bsc_msc_vector, data_table.table_handler.e_Data, true);

        output << data_table.renderFormattedData();

        return output.str();

    };

    set<UInt16> PairMapHandler::getLacSet()
    {
        set<UInt16> lac_set;
        vector<UInt32> keys_vector = m_pair_manager.getSortedKeysVector();
        for (size_t i = 0; i < keys_vector.size(); i++)
        {
            UInt8 probe_ip = 0;
            m_pair_manager.getIpFromKey( keys_vector[i], probe_ip );
            INPUT_TYPE probe_link_type = m_pair_manager.getInputTypeOfProbe(probe_ip);
            if ( probe_link_type == LAPD )
            {
                UInt16 abis_lac = m_pair_manager.getAbisInputLAC(keys_vector[i]);
                if (abis_lac != 0)
                {
                    lac_set.insert(abis_lac);
                }
            }
            else if( probe_link_type == MTP2 )
            {
                set<UInt16> a_lac = m_pair_manager.getAInputLAC( keys_vector[i] );
                set<UInt16>::iterator a_lac_it;
                for ( a_lac_it=a_lac.begin() ; a_lac_it != a_lac.end(); a_lac_it++ )
                {
                    lac_set.insert(*a_lac_it);
                }
            }
        }
    
        return lac_set;
    };


    vector< vector<std::string> > PairMapHandler::getLacSetWithInterface()
    {
        set<UInt16> abis_lac_set;
        set<UInt16> a_lac_set;
        set<UInt16> tot_lac_set;

        vector< vector<std::string> > lacs_with_interf;
        vector<UInt32> keys_vector = m_pair_manager.getSortedKeysVector();
        for (size_t i = 0; i < keys_vector.size(); i++)
        {
            UInt8 probe_ip = 0;
            m_pair_manager.getIpFromKey( keys_vector[i], probe_ip );
            INPUT_TYPE probe_link_type = m_pair_manager.getInputTypeOfProbe(probe_ip);
            if ( probe_link_type == LAPD )
            {
                UInt16 abis_lac = m_pair_manager.getAbisInputLAC(keys_vector[i]);
                if (abis_lac != 0)
                {
                    abis_lac_set.insert(abis_lac);
                    tot_lac_set.insert(abis_lac);
                }
            }
            else if( probe_link_type == MTP2 )
            {
                set<UInt16> a_lac = m_pair_manager.getAInputLAC( keys_vector[i] );
                set<UInt16>::iterator a_lac_it;
                for ( a_lac_it=a_lac.begin() ; a_lac_it != a_lac.end(); a_lac_it++ )
                {
                    a_lac_set.insert(*a_lac_it);
                    tot_lac_set.insert(*a_lac_it);
                }
            }
        }

        set<UInt16>::iterator tot_lac_it;
        for ( tot_lac_it=tot_lac_set.begin() ; tot_lac_it != tot_lac_set.end(); tot_lac_it++ )
        {
            vector< std::string > single_lac_row;
            int abis_lac = static_cast<int>(abis_lac_set.count(*tot_lac_it));
            int a_lac =static_cast<int>( a_lac_set.count(*tot_lac_it));
            if ( abis_lac > 0 && a_lac > 0 )
            {
                single_lac_row.push_back(convertIntToString(static_cast<int>(*tot_lac_it)));
                single_lac_row.push_back("Abis and A");
            }
            else if( abis_lac == 0 && a_lac > 0 )
            {
                single_lac_row.push_back(convertIntToString(static_cast<int>(*tot_lac_it)));
                single_lac_row.push_back("A");
            }
            else if( abis_lac > 0 && a_lac == 0 )
            {
                single_lac_row.push_back(convertIntToString(static_cast<int>(*tot_lac_it)));
                single_lac_row.push_back("Abis");
            }

            lacs_with_interf.push_back(single_lac_row);
        }

        return lacs_with_interf;
    };

    set< pair<UInt16, UInt16> > PairMapHandler::getPCSet()
    {
        set< pair<UInt16, UInt16> > pc_set;

        vector<UInt8> probe_ip_vector = m_pair_manager.getProbeList();

        for (size_t i = 0; i < probe_ip_vector.size(); i++)
        {
            if (m_pair_manager.getInputTypeOfProbe(probe_ip_vector[i]) == MTP2)
            {
                DIR direction; 
                for (int input=0; input < MAX_INPUTS; input++)    
                {
                    UInt16 bsc = 0;
                    UInt16 msc = 0;

                    for (int k = 0; k<(MAX_TSS-1); k++)
                    {
                        direction = m_pair_manager.getInputDirection(probe_ip_vector[i], static_cast<UInt8>(input), k);
                        if ( direction == DIR_DOWNLINK)
                        {
                            bsc = m_pair_manager.getInputDPC(probe_ip_vector[i], static_cast<UInt8>(input), k);
                            msc = m_pair_manager.getInputOPC(probe_ip_vector[i], static_cast<UInt8>(input), k);
                        }
                        else if( direction == DIR_UPLINK )
                        {
                            msc = m_pair_manager.getInputDPC(probe_ip_vector[i], static_cast<UInt8>(input), k);
                            bsc = m_pair_manager.getInputOPC(probe_ip_vector[i], static_cast<UInt8>(input), k);
                        }

                        if ( (bsc != 0) && (msc != 0) )
                        {
                            pc_set.insert(std::make_pair(bsc,msc));
                        }
                    }
                }
            }
        }
        return pc_set;
    };

    set<UInt8> PairMapHandler::getProbeSetFromLac(UInt16 lac)
    {
        set<UInt8> probe_set;
        UInt8 ip;
        vector<UInt32> keys_vector = m_pair_manager.getSortedKeysVector();
        for (size_t i=0; i<keys_vector.size(); i++)
        {
            if ( m_pair_manager.getAbisInputLAC(keys_vector[i])== lac)
            {
                m_pair_manager.getIpFromKey(keys_vector[i], ip);
                probe_set.insert(ip);
            }
            set<UInt16> a_lacs = m_pair_manager.getAInputLAC(keys_vector[i]);
            set<UInt16> :: iterator lac_it;
            for (lac_it = a_lacs.begin(); lac_it != a_lacs.end(); lac_it++)
            {
                if ( (*lac_it) == lac )
                {
                    m_pair_manager.getIpFromKey(keys_vector[i], ip);
                    probe_set.insert(ip);
                    break;
                }
            }

        }
        return probe_set;
    };

    std::string PairMapHandler::decto383(int val)
    {
        std::ostringstream  stringa;
        stringa << ((val&0x7800)>>11) << "-" << ((val&0x7F8)>>3) << "-" << (val&0x7) ;
        return stringa.str();
    };

    std::string PairMapHandler::dumpPCsPerProbe( bool pc_to_show, bool show_decto )
    {
        // Definition of variables about the file to write.
        //const char *filename;
        std::ofstream    probe_fp;
        if (!pc_to_show)
        {
            probe_fp.open("status/dumpADirections.txt", std::ios::out);
        }
        else
        {
            probe_fp.open("status/dumpPCs.txt", std::ios::out);
        }
    

        // Definition of variables about the CLI
        std::ostringstream output;
        TablesRenders data_table(90);

        // Add Title
        data_table.table_handler.addRow("DUMPING OF PROBE INFO", data_table.table_handler.e_Title);

        vector<UInt8> probe_ip_vector = m_pair_manager.getProbeList();

        for (size_t i = 0; i < probe_ip_vector.size(); i++)
        {
            if ( m_pair_manager.getInputTypeOfProbe( probe_ip_vector[i]) == MTP2)
            {
        
                std::ostringstream ip_probe_title;
                ip_probe_title << "PROBE NUMBER: " << data_table.table_handler.convertIntToString(static_cast<int>(probe_ip_vector[i]));
                data_table.table_handler.addRow(ip_probe_title.str(), data_table.table_handler.e_Title);
        
                // time slots Title
                vector<std::string> ts_vector_title;
                for (int ts=0; ts<MAX_TSS; ts++)
                {
                    if (ts==0)
                    {
                        ts_vector_title.push_back("Inp/ts");
                    }
                    else
                    {
                        ts_vector_title.push_back(convertIntToString(ts-1));
                    }
                }
                data_table.table_handler.addRow(ts_vector_title, data_table.table_handler.e_Data);

                vector< vector< std::string > > probe_info_table;
                // data ordered by inputs
                LINK_TYPE link_type = m_pair_manager.linkTypeDiscovery(probe_ip_vector[i]);
                switch(link_type)
                {
                case STD:
                    {

                        vector< std::pair<UInt8, UInt8> > probe_pairing;
                        probe_pairing = m_pair_manager.getProbePairing(probe_ip_vector[i]);

                        vector< std::pair<UInt8, UInt8> >::iterator pairing_it;
                        for (pairing_it = probe_pairing.begin(); pairing_it < probe_pairing.end(); pairing_it++)    
                        {
                            vector< std::string > first_data_row;
                            vector< std::string > second_data_row;
                            UInt32 key;
                            DIR direction;
                        
                            // First INPUT of pair
                            first_data_row.push_back(convertIntToString(static_cast<int>(pairing_it->first)));
                            for (int ts=0; ts<MAX_TSS; ts++)
                            {
                                key = m_pair_manager.GetMapKey(probe_ip_vector[i], pairing_it->first, static_cast<UInt8>(ts));
                                direction = m_pair_manager.getInputDirection(key);

                                if (!pc_to_show)
                                {
                                    if (direction == DIR_UNK)
                                    {
                                        first_data_row.push_back("-");
                                    }
                                    else
                                    {
                                        first_data_row.push_back(m_pair_manager.convertDirectionToString(direction));
                                    }
                                }
                                else
                                {
                                    UInt16 bsc = 0;
                                    UInt16 msc = 0;
                                    if ( direction == DIR_DOWNLINK)
                                    {
                                        bsc = m_pair_manager.getInputDPC(key);
                                        msc = m_pair_manager.getInputOPC(key);
                                    }
                                    else if( direction == DIR_UPLINK )
                                    {
                                        msc = m_pair_manager.getInputDPC(key);
                                        bsc = m_pair_manager.getInputOPC(key);
                                    }

                                    if ( (bsc != 0) && (msc != 0) )
                                    {
                                        std::ostringstream pc_string;
                                        //decto383() to do
                                        if ( show_decto )
                                        {
                                            pc_string << decto383(static_cast<int>(bsc)) << "-" << decto383(static_cast<int>(msc));
                                        }
                                        else
                                        {
                                            pc_string << bsc << "-" << msc;
                                        }

                                        first_data_row.push_back(pc_string.str());
                                    }
                                }
                            }

                            // Second INPUT of pair
                            second_data_row.push_back(convertIntToString(static_cast<int>(pairing_it->second)));
                            for (int ts=0; ts<MAX_TSS; ts++)
                            {
                                key = m_pair_manager.GetMapKey(probe_ip_vector[i], pairing_it->second, static_cast<UInt8>(ts));
                                direction = m_pair_manager.getInputDirection(key);

                                if (!pc_to_show)
                                {
                                    if (direction == DIR_UNK)
                                    {
                                        second_data_row.push_back("-");
                                    }
                                    else
                                    {
                                        second_data_row.push_back(m_pair_manager.convertDirectionToString(direction));
                                    }
                                }
                                else
                                {
                                    UInt16 bsc = 0;
                                    UInt16 msc = 0;
                                    if ( direction == DIR_DOWNLINK)
                                    {
                                        bsc = m_pair_manager.getInputDPC(key);
                                        msc = m_pair_manager.getInputOPC(key);
                                    }
                                    else if( direction == DIR_UPLINK )
                                    {
                                        msc = m_pair_manager.getInputDPC(key);
                                        bsc = m_pair_manager.getInputOPC(key);
                                    }

                                    if ( (bsc != 0) && (msc != 0) )
                                    {
                                        std::ostringstream pc_string;
                                        //decto383() to do
                                        if ( show_decto )
                                        {
                                            pc_string << decto383(static_cast<int>(bsc)) << "-" << decto383(static_cast<int>(msc));
                                        }
                                        else
                                        {
                                            pc_string << bsc << "-" << msc;
                                        }

                                        second_data_row.push_back(pc_string.str());
                                    }
                                }
                            }

                            probe_info_table.push_back(first_data_row);
                            probe_info_table.push_back(second_data_row);
                        }
                    
                    }
                    break;
                case MULT:
                    {
                        for (int input=0; input < MAX_INPUTS; input++)
                        {
                            vector< std::string> data_row;
                            data_row.push_back(convertIntToString(input));
                        
                            for (int ts=0; ts<MAX_TSS; ts++)
                            {
                                UInt32 key = m_pair_manager.GetMapKey(probe_ip_vector[i], static_cast<UInt8>(input), static_cast<UInt8>(ts));
                                DIR direction = m_pair_manager.getInputDirection(key);
                            
                                if (!pc_to_show)
                                {
                                    if (direction == DIR_UNK)
                                    {
                                        data_row.push_back("-");
                                    }
                                    else
                                    {
                                        data_row.push_back(m_pair_manager.convertDirectionToString(direction));
                                    }
                                }
                                else
                                {
                                    UInt16 bsc = 0;
                                    UInt16 msc = 0;
                                    if ( direction == DIR_DOWNLINK)
                                    {
                                        bsc = m_pair_manager.getInputDPC(key);
                                        msc = m_pair_manager.getInputOPC(key);
                                    }
                                    else if( direction == DIR_UPLINK )
                                    {
                                        msc = m_pair_manager.getInputDPC(key);
                                        bsc = m_pair_manager.getInputOPC(key);
                                    }

                                    if ( (bsc != 0) && (msc != 0) )
                                    {
                                        std::ostringstream pc_string;
                                        //decto383() to do
                                        if ( show_decto )
                                        {
                                            pc_string << decto383(static_cast<int>(bsc)) << "-" << decto383(static_cast<int>(msc));
                                        }
                                        else
                                        {
                                            pc_string << bsc << "-" << msc;
                                        }

                                        data_row.push_back(pc_string.str());
                                    }
                                }

                            }

                            probe_info_table.push_back(data_row);
                        }
                    }
                    break;
                default:
                    break;
                
                }

                data_table.table_handler.addTableByRows(probe_info_table, data_table.table_handler.e_Data, true);

            }
        }

        probe_fp << data_table.renderFormattedData();

        // Definition of title for command line interface.
        TablesRenders cli_data_table(40);

        if (!probe_fp.is_open())
        {
            cli_data_table.table_handler.addRow("Cannot open file!", cli_data_table.table_handler.e_Data);
        }
        else if (probe_fp.is_open() )
        {
            cli_data_table.table_handler.addRow("File correctly saved into status dir!", cli_data_table.table_handler.e_Data);
            probe_fp.close();
        }

        output << cli_data_table.renderFormattedData();
        return output.str();

    };

    std::string PairMapHandler::showLinkBal()
    {

        std::ostringstream output;

        vector<UInt8> probe_ip_vector = m_pair_manager.getProbeList();

        for (size_t i = 0; i < probe_ip_vector.size(); i++)
        {
            TablesRenders data_table(40);
            bool write_title = true;
            // Add Title
            if( i==0 )
                data_table.table_handler.addRow("Check of Links Balancing", data_table.table_handler.e_Title);

            std::ostringstream title;
            title << "Probe Number" << convertIntToString(static_cast<int>(probe_ip_vector[i]));
            data_table.table_handler.addRow(title.str(), data_table.table_handler.e_Data);

            bool links_are_balanced = true;
            LINK_TYPE link_type = m_pair_manager.linkTypeDiscovery(probe_ip_vector[i]);
            INPUT_TYPE probe_input_type = m_pair_manager.getInputTypeOfProbe(probe_ip_vector[i]);

            vector<std::string> first_input_vector;
            vector<std::string> second_input_vector;
            vector<std::string> first_ts_vector;
            vector<std::string> first_dir_vector;
            vector<std::string> second_dir_vector;
            vector<std::string> first_lac_vector;
            vector<std::string> second_lac_vector;

            int first_input;
            int second_input;
            DIR first_dir;
            DIR second_dir;
            UInt32 first_key;
            UInt32 second_key;
            std::string first_lac;
            std::string second_lac;

            if ( link_type == STD )
            {
                //data vectors definition
                vector<std::string> pairs;

                vector< std::pair<UInt8, UInt8> > probe_pairing;

                probe_pairing = m_pair_manager.getProbePairing(probe_ip_vector[i]);

                vector< std::pair<UInt8, UInt8> >::iterator pairing_it;
                for (pairing_it = probe_pairing.begin(); pairing_it < probe_pairing.end(); pairing_it++)    
                {
                    for (int ts = 0; ts < MAX_TSS; ts++)
                    {
                        first_input = pairing_it->first;
                        second_input = pairing_it->second;
                        first_key = m_pair_manager.GetMapKey(probe_ip_vector[i],pairing_it->first, ts);
                        second_key = m_pair_manager.GetMapKey(probe_ip_vector[i],pairing_it->second, ts);
                        first_dir = m_pair_manager.getInputDirection(first_key);
                        second_dir = m_pair_manager.getInputDirection(second_key);
                        if (probe_input_type == LAPD)
                        {
                            first_lac = checkIntValue(static_cast<int>(m_pair_manager.getAbisInputLAC(first_key)));
                            second_lac = checkIntValue(static_cast<int>(m_pair_manager.getAbisInputLAC(second_key)));
                        }

                        bool dir_is_unbalanced = false;
                        if ( (first_dir== DIR_UPLINK && second_dir!= DIR_DOWNLINK) || (first_dir== DIR_DOWNLINK && second_dir!= DIR_UPLINK) )
                            dir_is_unbalanced = true;

                        bool lac_is_unbalanced = true;
                        if(probe_input_type == MTP2 || first_lac == second_lac)
                            lac_is_unbalanced = false;

                        if( dir_is_unbalanced || lac_is_unbalanced)
                        {
                            if (write_title) 
                            {    
                                // Add title of unbalanced probe.
                                first_input_vector.push_back("INPUT");
                                if (probe_input_type == MTP2)
                                    first_ts_vector.push_back("TS");
                                first_dir_vector.push_back("DIR");
                                if (probe_input_type == LAPD)
                                    first_lac_vector.push_back("LAC");
                                second_input_vector.push_back("INPUT");
                                second_dir_vector.push_back("DIR");
                                if (probe_input_type == LAPD)
                                    second_lac_vector.push_back("LAC");
                            }
                            links_are_balanced = false;
                            write_title = false;

                            first_input_vector.push_back(convertIntToString(first_input));
                            if (probe_input_type == MTP2)
                                first_ts_vector.push_back(convertIntToString(ts));
                            first_dir_vector.push_back(m_pair_manager.convertDirectionToString(first_dir));
                            if (probe_input_type == LAPD)
                                first_lac_vector.push_back(first_lac);
                            second_input_vector.push_back(convertIntToString(second_input));
                            second_dir_vector.push_back(m_pair_manager.convertDirectionToString(second_dir));
                            if (probe_input_type == LAPD)
                                second_lac_vector.push_back(second_lac);
                        }
                    }
                }

                if (links_are_balanced)
                {
                    data_table.table_handler.addRow("Links are BALANCED.", data_table.table_handler.e_Data);
                }
                else
                {
                    data_table.table_handler.addRow("Links are UNBALANCED, Details:", data_table.table_handler.e_Data);
                    vector< vector< std::string> > columns_data;
                    columns_data.push_back(first_input_vector);
                    if (probe_input_type == MTP2)
                        columns_data.push_back(first_ts_vector);
                    columns_data.push_back(first_dir_vector);
                    if (probe_input_type == LAPD)
                        columns_data.push_back(first_lac_vector);
                    columns_data.push_back(second_input_vector);
                    columns_data.push_back(second_dir_vector);
                    if (probe_input_type == LAPD)
                        columns_data.push_back(second_lac_vector);

                    data_table.table_handler.addTableByColumns(columns_data, data_table.table_handler.e_Data);
                }
            }
            else if ( link_type == MULT )
            {
            
                for ( int input = 0; input < MAX_INPUTS; input++)
                {
                    for (int ts=1; ts < MAX_TSS/2; ts+=2)
                    {
                        first_key = m_pair_manager.GetMapKey(probe_ip_vector[i], static_cast<UInt8>(input), static_cast<UInt8>(ts) );
                        second_key = m_pair_manager.GetMapKey(probe_ip_vector[i], static_cast<UInt8>(input), static_cast<UInt8>(ts+1) );
                        first_dir = m_pair_manager.getInputDirection(first_key);
                        second_dir = m_pair_manager.getInputDirection(second_key);
                        first_lac = checkIntValue(static_cast<int>(m_pair_manager.getAbisInputLAC(first_key)));
                        second_lac = checkIntValue(static_cast<int>(m_pair_manager.getAbisInputLAC(second_key)));

                        bool dir_is_unbalanced = false;
                        if ( (first_dir== DIR_UPLINK && second_dir!= DIR_DOWNLINK) || (first_dir== DIR_DOWNLINK && second_dir!= DIR_UPLINK) )
                            dir_is_unbalanced = true;

                        bool lac_is_unbalanced = true;
                        if(probe_input_type == MTP2 || first_lac == second_lac)
                            lac_is_unbalanced = false;

                        if( dir_is_unbalanced || lac_is_unbalanced)
                        {
                            if (write_title) 
                            {    
                                // Add title of unbalanced probe.
                                first_input_vector.push_back("INPUT");
                                first_ts_vector.push_back("TS");
                                first_dir_vector.push_back("DIR");
                                first_lac_vector.push_back("LAC");
                                second_dir_vector.push_back("DIR");
                                second_lac_vector.push_back("LAC");
                            }
                            links_are_balanced = false;
                            write_title = false;

                            first_input_vector.push_back(convertIntToString(input));
                            first_ts_vector.push_back(convertIntToString(ts));
                            first_dir_vector.push_back(m_pair_manager.convertDirectionToString(first_dir));
                            first_lac_vector.push_back(first_lac);
                            second_dir_vector.push_back(m_pair_manager.convertDirectionToString(second_dir));
                            second_lac_vector.push_back(second_lac);
                        }
                    }
                }
                if (links_are_balanced)
                {
                    data_table.table_handler.addRow("Links are BALANCED.", data_table.table_handler.e_Data);
                }
                else
                {
                    data_table.table_handler.addRow("Links are UNBALANCED, Details:", data_table.table_handler.e_Data);
                    vector< vector< std::string> > columns_data;
                    columns_data.push_back(first_input_vector);
                    columns_data.push_back(first_ts_vector);
                    columns_data.push_back(first_dir_vector);
                    columns_data.push_back(first_lac_vector);
                    columns_data.push_back(second_dir_vector);
                    columns_data.push_back(second_lac_vector);

                    data_table.table_handler.addTableByColumns(columns_data, data_table.table_handler.e_Data);
                }
            }

            output << data_table.renderFormattedData();
        }



        return output.str();
    };


    /********************    #4044 Command line processing functions *********************/
    std::string PairMapHandler::GetShowCommandResult(int _argc, char *_argv[], SHOW_INFO_TYPE _show_type, std::string _outfile)
    {
        FILE * fp;
        std::string _result;
        std::ostringstream ostring;
        bool _write_to_file = false;
        std::string _bad_arg_type ="Bad arguments type.";
        std::string param0 = "";
        std::string value0 = "";
        std::string param1 = "";
        std::string value1 = "";
        std::string param2 = "";
        std::string value2 = "";
        int probe=0;

        if(_argc == 0)
        {
            _result = pairShow(0, _show_type);
        }
        else if(_argc == 1) // -p= OR -o= option
        {
            param0 = string(_argv[0]);
            if(param0.size() > 3)
            {
                value0 = param0.substr(3);
                param0 = param0.substr(0,3);
            }

            if(param0 == "-p=")
            {
                probe = atoi(value0.c_str());
                _result = pairShow(probe, _show_type);
            }
            else if(param0 == "-o")
            {
                _result = pairShow(0, _show_type);
                _write_to_file = true;
            }

        }    
        else if(_argc == 2) 
        {    
            param0 = string(_argv[0]);
            if(param0.size() > 3)
            {
                value0 = param0.substr(3);
                param0 = param0.substr(0,3);
            }

            if(param0 != "-p=")
                return _bad_arg_type;
            if(!probe)
            {
                probe = atoi(value0.c_str());
                _result = pairShow(probe, _show_type);
            }

            param1 = string(_argv[1]);
            if(param1 == "-o")
            {
                _write_to_file = true;
            }
            else
                return _bad_arg_type;
        }

        else
            return _bad_arg_type; 

        if(_write_to_file)
        {
            ostring << _result << "\n" << _outfile << " file is saved into status dir ";
            fp=fopen(_outfile.c_str(), "w");
            fputs(ostring.str().c_str(),fp);
            fclose(fp);

            _result = ostring.str();
        }

        return _result;
    }

    std::string PairMapHandler::CHPairShow(int argc, char *argv[])
    {
        CAL_UNUSED_ARG (argc);
        CAL_UNUSED_ARG (argv);
        std::ostringstream ostring;
        std::string output;

        SHOW_INFO_TYPE show_type = s_TYPE;
        std::string outfile = "status/pairShow.txt";

        return GetShowCommandResult(argc, argv, show_type, outfile);
    }

    std::string PairMapHandler::DirectionShow(int argc, char *argv[])
    {
        CAL_UNUSED_ARG (argc);
        CAL_UNUSED_ARG (argv);
        std::ostringstream ostring;
        std::string output;

        SHOW_INFO_TYPE show_type = s_DIRECTION;
        std::string outfile = "status/dirShow.txt";

        return GetShowCommandResult(argc, argv, show_type, outfile);

    }

    std::string PairMapHandler::LacShow(int argc, char *argv[])
    {
        CAL_UNUSED_ARG (argc);
        CAL_UNUSED_ARG (argv);
        std::ostringstream ostring;
        std::string output;

        SHOW_INFO_TYPE show_type = s_LAC;
        std::string outfile = "status/lacShow.txt";

        return GetShowCommandResult(argc, argv, show_type, outfile);

    }

    std::string PairMapHandler::PcShow(int argc, char *argv[])
    {
        CAL_UNUSED_ARG (argc);
        CAL_UNUSED_ARG (argv);
        std::ostringstream ostring;
        std::string output;

        SHOW_INFO_TYPE show_type = s_PC;
        std::string outfile = "status/pcShow.txt";

        return GetShowCommandResult(argc, argv, show_type, outfile);
    }

    std::string PairMapHandler::ListLac(int argc, char *argv[])
    {
        CAL_UNUSED_ARG (argc);
        CAL_UNUSED_ARG (argv);
        FILE * fp;
        std::ostringstream ostring;
        std::string output;
        std::string param0 = "";
        std::string value0 = "";
        std::string param1 = "";
        std::string param2 = "";

        std::string bad_arg_type = "Bad arguments type.";
        bool write_to_file = false;
        std::string outfile = "status/lacList.txt";

        if(argc == 0)
        {
            output = ShowLacList();
            return output;
        }
        else if(argc == 1) // -v option shows a list of Lacs with additional info
        {
            param0 = string(argv[0]);
            if(param0.size() == 2)
            {
                param1 = param0.substr(0,2);
                if(param1 == "-v")
                {
                    outfile = "status/lacListVerbose.txt";
                    output = ShowMonitoredLac();
                }
                else if(param1 == "-o")
                {
                    output = ShowLacList();
                    write_to_file = true;
                }
                else
                {
                    return bad_arg_type;
                }
            }
            else if (param0.size() == 3)
            {
                param1 = param0.substr(0,3);
                if(param1 == "-vo")
                {
                    outfile = "status/lacListVerbose.txt";
                    output = ShowMonitoredLac();
                    write_to_file = true;
                }
            }
        }

        if(write_to_file)
        {
            ostring << output << "\n" << outfile << " file is saved into status dir ";
            fp=fopen(outfile.c_str(), "w");
            fputs(ostring.str().c_str(),fp);
            fclose(fp);

            output = ostring.str();
        }

        return output;
    }

    std::string PairMapHandler::ListPC(int argc, char *argv[])
    {
        CAL_UNUSED_ARG (argc);
        CAL_UNUSED_ARG (argv);
        FILE * fp;
        std::ostringstream ostring;
        std::string output;
        std::string param0 = "";
        std::string value0 = "";
        std::string param1 = "";
        std::string param2 = "";

        std::string bad_arg_type = "Bad arguments type.";
        bool write_to_file = false;
        std::string outfile = "status/pcList.txt";

        if(argc == 0)
        {
            output = ShowPCList();
            return output;
        }
        else if(argc == 1) // -v option shows a list of Lacs with additional info
        {
            param0 = string(argv[0]);
            if(param0.size() == 2)
            {
                param1 = param0.substr(0,2);
                if(param1 == "-v")
                {
                    outfile = "status/pcListVerbose.txt";
                    output =ShowMonitoredPCs();
                }
                else if(param1 == "-o")
                {
                    output = ShowPCList();
                    write_to_file = true;
                }
                else
                {
                    return bad_arg_type;
                }
            }
            else if (param0.size() == 3)
            {
                outfile = "status/pcListVerbose.txt";
                param1 = param0.substr(0,3);
                if(param1 == "-vo")
                {
                    output = ShowMonitoredPCs();
                    write_to_file = true;
                }
            }
        }

        if(write_to_file)
        {
            ostring << output << "\n" << outfile << " file is saved into status dir ";
            fp=fopen(outfile.c_str(), "w");
            fputs(ostring.str().c_str(),fp);
            fclose(fp);

            output = ostring.str();
        }

        return output;
    }

    std::string PairMapHandler::linkBal(int argc, char *argv[])
    {
        CAL_UNUSED_ARG (argc);
        CAL_UNUSED_ARG (argv);
        std::string output;

        output = showLinkBal();
        return output;
    }

    std::string PairMapHandler::dumpDirectionsMap(int argc, char *argv[])
    {
        CAL_UNUSED_ARG (argc);
        CAL_UNUSED_ARG (argv);
        std::string output;
        std::string param0 = "";
        //int lac = 0;
        if(argc == 0)
        {
            bool show_pc = false;
            output = dumpPCsPerProbe(show_pc);
            return output;
        }
        else
        {
            return "Incomplete command.";
        }
    }

    std::string PairMapHandler::dumpPCMap(int argc, char *argv[])
    {
        CAL_UNUSED_ARG (argc);
        CAL_UNUSED_ARG (argv);
        std::string output;
        std::string value0 = "";
        std::string param0 = "";
        //int lac = 0;
        bool show_pc = true;

        if(argc == 0)
        {
            output = dumpPCsPerProbe(show_pc);
            return output;
        }
        else if(argc == 1) // -i option is not permitted without -p option 
        {
            param0 = string(argv[0]);
            if(param0.size() > 2)
            {
                value0 = param0.substr(2);
                param0 = param0.substr(0,2);
            }

            if(param0 == "-d")
            {
                bool show_decto_pc = true;
                output = dumpPCsPerProbe(show_pc, show_decto_pc);
                return output;
            }
        }    
    
        return "Incomplete command.";
    
    }

    std::string PairMapHandler::dumpALacMap(int argc, char *argv[])
    {
        CAL_UNUSED_ARG (argc);
        CAL_UNUSED_ARG (argv);
        std::string output;
        std::string value0 = "";
        std::string param0 = "";

        if(argc == 0)
        {
            output = dumpALacsPerProbe();
            return output;
        }

        return "Incomplete command.";

    }

    std::string PairMapHandler::dumpALacsPerProbe()
    {
        // Definition of variables about the file to write.
        std::ofstream    probe_fp;
        probe_fp.open("status/dumpALacs.txt", std::ios::out);

        // Definition of variables about the CLI
        std::ostringstream output;
        TablesRenders data_table(90);

        // Add Title
        data_table.table_handler.addRow("DUMPING OF PROBE INFO", data_table.table_handler.e_Title);

        vector<UInt8> probe_ip_vector = m_pair_manager.getProbeList();

        for (size_t i = 0; i < probe_ip_vector.size(); i++)
        {
            if ( m_pair_manager.getInputTypeOfProbe( probe_ip_vector[i]) == MTP2)
            {

                std::ostringstream ip_probe_title;
                ip_probe_title << "PROBE NUMBER: " << data_table.table_handler.convertIntToString(static_cast<int>(probe_ip_vector[i]));
                data_table.table_handler.addRow(ip_probe_title.str(), data_table.table_handler.e_Title);

                // time slots Title
                vector<std::string> ts_vector_title;
                for (int ts=0; ts<MAX_TSS+1; ts++)
                {
                    if (ts==0)
                    {
                        ts_vector_title.push_back("Inp/ts");
                    }
                    else
                    {
                        ts_vector_title.push_back(convertIntToString(ts-1));
                    }
                }

                data_table.table_handler.addRow(ts_vector_title, data_table.table_handler.e_Data);

                vector< vector< std::string > > probe_info_table;
                // data ordered by inputs
                LINK_TYPE link_type = m_pair_manager.linkTypeDiscovery(probe_ip_vector[i]);
                switch(link_type)
                {
                case STD:
                    {
                        vector< std::pair<UInt8, UInt8> > probe_pairing;
                        probe_pairing = m_pair_manager.getProbePairing(probe_ip_vector[i]);

                        vector< std::pair<UInt8, UInt8> >::iterator pairing_it;
                        for (pairing_it = probe_pairing.begin(); pairing_it < probe_pairing.end(); pairing_it++)    
                        {
                            vector< std::string > first_data_row;
                            UInt32 key;

                            // First INPUT of pair
                            first_data_row.push_back(convertIntToString(static_cast<int>(pairing_it->first)));
                            for (int ts=0; ts<MAX_TSS; ts++)
                            {
                                key = m_pair_manager.GetMapKey(probe_ip_vector[i], pairing_it->first, static_cast<UInt8>(ts));
                                set<UInt16> lac_set = m_pair_manager.getAInputLAC(key);
                                set<UInt16>::iterator lac_it;
                                std::ostringstream lacs_string;
                                int counter = 0;
                                for ( lac_it=lac_set.begin() ; lac_it != lac_set.end(); lac_it++ )
                                {
                                    if (counter > 0)
                                        lacs_string << "- " << (*lac_it) << " ";
                                    else
                                        lacs_string << (*lac_it) << " ";
                                    counter++;
                                }
                                first_data_row.push_back(lacs_string.str());
                            }

                            // Second INPUT of pair
                            vector< std::string > second_data_row;

                            second_data_row.push_back(convertIntToString(static_cast<int>(pairing_it->second)));
                            for (int ts=0; ts<MAX_TSS; ts++)
                            {
                                key = m_pair_manager.GetMapKey(probe_ip_vector[i], pairing_it->second, static_cast<UInt8>(ts));
                                set<UInt16> lac_set = m_pair_manager.getAInputLAC(key);
                                set<UInt16>::iterator lac_it;
                                std::ostringstream lacs_string;
                                int counter = 0;
                                for ( lac_it=lac_set.begin() ; lac_it != lac_set.end(); lac_it++ )
                                {
                                    if (counter > 0)
                                        lacs_string << "- " << (*lac_it) << " ";
                                    else
                                        lacs_string << (*lac_it) << " ";
                                    counter++;                            }
                                second_data_row.push_back(lacs_string.str());
                            }

                            probe_info_table.push_back(first_data_row);
                            probe_info_table.push_back(second_data_row);

                        }
                    }
                    break;
                case MULT:
                    {
                    
                        for (int input=0; input<MAX_INPUTS; input++)    
                        {
                            vector< std::string > first_data_row;
                            std::ostringstream input_title;
                            input_title << input;
                            first_data_row.push_back(input_title.str());

                            for (int ts=0; ts<MAX_TSS; ts++)    
                            {
                                UInt32 key = m_pair_manager.GetMapKey(probe_ip_vector[i], input, static_cast<UInt8>(ts));
                                set<UInt16> a_lacs_set = m_pair_manager.getAInputLAC(key);
                                set<UInt16>::iterator a_lac_it;
                                std::ostringstream lac_string;
                                int counter = 0;
                                for (a_lac_it=a_lacs_set.begin(); a_lac_it!=a_lacs_set.end(); a_lac_it++)
                                {
                                    if(counter ==0)
                                        lac_string << (*a_lac_it);
                                    else
                                        lac_string << " - " << (*a_lac_it);
                                }
                                first_data_row.push_back(lac_string.str());
                            }
                            probe_info_table.push_back(first_data_row);
                        }
                    }
                    break;
                default:
                    break;
                }

                data_table.table_handler.addTableByRows(probe_info_table, data_table.table_handler.e_Data, true);
            }
        }
        probe_fp << data_table.renderFormattedData();

        // Definition of title for command line interface.
        TablesRenders cli_data_table(40);

        if (!probe_fp.is_open())
        {
            cli_data_table.table_handler.addRow("Cannot open file!", cli_data_table.table_handler.e_Data);
        }
        else if (probe_fp.is_open() )
        {
            cli_data_table.table_handler.addRow("File correctly saved into status dir!", cli_data_table.table_handler.e_Data);
            probe_fp.close();
        }

        output << cli_data_table.renderFormattedData();
        return output.str();
    };

    /*************************************************************************************/

}
