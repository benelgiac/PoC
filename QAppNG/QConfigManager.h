#ifndef INCLUDE_QCONFIGMANAGER_EVO_NG
#define INCLUDE_QCONFIGMANAGER_EVO_NG

// STL includes
#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <list>
#include <set>
#include <fstream>

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

// 3rdParty includes
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/unordered_map.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <pugixml/pugixml.hpp>

// ECUBA includes
#include "core.h"
#include <QAppNG/Singleton.h>

namespace QAppNG
{

    // --------------------------------------------------------------------------------------------------------------------

    /****************************************************************************************************
    * Support Class for automatic output type conversion
    ****************************************************************************************************/

    class CIDRIpAddress {

    public:
        CIDRIpAddress(UInt32 _address = 0, UInt32 _mask = 0xFFFFFFFF ) : m_address(_address), m_mask(_mask)
        {
        }

        void ipAddress( UInt32 _address )
        {
            m_address = _address;
        }

        UInt32 ipAddress( )
        {
            return m_address;
        }

        void subnetMask( UInt32 _mask )
        {
            m_mask = _mask;
        }

        UInt32 subnetMask( )
        {
            return m_mask;
        }

        inline bool isIpInSubnet( UInt32 ip )
        {
            return (ip & m_mask) == m_address;
        }

        std::vector<UInt32> getSubnetAddresses()
        {
            std::vector<UInt32> address_vect;

            if ( m_mask < 0xFFFFFFFF )
            {
                UInt32 _mask = ~m_mask;

                for ( size_t i = 1 ; i < _mask ; ++i )
                {
                    UInt32 _address = m_address | i;
                    address_vect.push_back( _address );
                }
            }
            else
            {
                address_vect.push_back( m_address );
            }

            return address_vect;
        }

        std::string getSubnetIpString()
        {
            static const UInt32 FIRST_OCT_MASK  = 0xFF000000;
            static const UInt32 SECOND_OCT_MASK = 0x00FF0000;
            static const UInt32 THIRD_OCT_MASK  = 0x0000FF00;
            static const UInt32 FOURTH_OCT_MASK = 0x000000FF;

            static UInt32 subnet = m_address & m_mask;

            std::ostringstream subnet_ip_string;

            subnet_ip_string << ( ( subnet & FIRST_OCT_MASK  )  >> 24 )
                << "."       << ( ( subnet & SECOND_OCT_MASK )  >> 16 )
                << "."       << ( ( subnet & THIRD_OCT_MASK  )  >> 8  )
                << "."       << (   subnet & FOURTH_OCT_MASK          );

            return subnet_ip_string.str();
        }

    private:
        UInt32 m_address;
        UInt32 m_mask;
    };

    class ValueConverter
    {
        // Type convertiom template function, if specific specialization does not exist it returns false
    public:
        template <class RETURNED_TYPE>
        bool convert(const std::string &, RETURNED_TYPE &) { return false; }
    };

    // function TEMPLATE SPECIALIZATIONS
    template<> 
    inline bool ValueConverter::convert<std::string>(const std::string &input, std::string &output) { output = input; return true; }

    template<>
    inline bool ValueConverter::convert<UInt64>(const std::string &input, UInt64 &output) { output = strtoul(input.c_str(), NULL, 0); return true; }

    template<>
    inline bool ValueConverter::convert<UInt32>(const std::string &input, UInt32 &output) { output = strtoul(input.c_str(), NULL, 0); return true; }

    template<>
    inline bool ValueConverter::convert<UInt16>(const std::string &input, UInt16 &output) { output = static_cast<UInt16>(strtoul(input.c_str(), NULL, 0)); return true; }

    template<>
    inline bool ValueConverter::convert<UInt8>(const std::string &input, UInt8 &output) { output = static_cast<UInt8>(strtoul(input.c_str(), NULL, 0)); return true; }

    template<>
    inline bool ValueConverter::convert<int>(const std::string &input, int &output) { output = atoi(input.c_str()); return true; }

    template<>
    inline bool ValueConverter::convert<bool>(const std::string &input, bool &output) 
    {
        if ( input == "true" || input == "True" || input == "TRUE" || input == "1" )
        {
            output = true;
            return true;
        }
        else if ( input == "false" || input == "False" || input == "FALSE" || input == "0" )
        {
            output = false;
            return true;
        }

        return false;
    };

    // convert cidr notation ip address string into CIDRIpAddress object in host byte order
    template<> 
    inline bool ValueConverter::convert<CIDRIpAddress>(const std::string &input, CIDRIpAddress &output)
    {
        typedef boost::tokenizer< boost::char_separator<char> > tokenizer;
        boost::char_separator<char> sep("/"); // separators here
        tokenizer tokens(input, sep);

        for ( tokenizer::iterator iter = tokens.begin() ; iter != tokens.end() ; ++iter )
        {
            //first token is ip address
            if ( iter == tokens.begin() )
            {
                Int32 address;
                address = inet_addr( (*iter).c_str() );

                if ( address == -1 )
                    return false;

                output.ipAddress( ntohl( address ) );
            }
            //last is bits of subnet mask
            else
            {
                UInt32 mask = static_cast<UInt32>(strtoul( (*iter).c_str(), NULL, 0 ) );
                UInt32 temp_mask = 0;

                for(unsigned int i = 0; i < mask; ++i)
                {
                    temp_mask = temp_mask << 1;
                    temp_mask |= 1;
                }

                output.subnetMask( ntohl( temp_mask ) );
            }
        }

        return true;
    }

    // parse string of "x,y,z" to vector<int>
    template<>
    inline bool ValueConverter::convert< std::vector< UInt16 > >(const std::string &input, std::vector<UInt16> &output)
    {
        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        boost::char_separator<char> sep(", \t \n \r"); // separators here
        tokenizer tokens(input, sep);

        for (tokenizer::iterator tok_iter = tokens.begin();
            tok_iter != tokens.end(); ++tok_iter)
        {
            output.push_back(static_cast<UInt16>(strtoul((*tok_iter).c_str(), NULL, 0)));
        }
        return true; 
    };

    // parse list of CIDR ip addresses separated by ", \t \n \r" to vector< CIDRIpAddress >
    template<>
    inline bool ValueConverter::convert< std::vector< CIDRIpAddress > >(const std::string &input, std::vector< CIDRIpAddress > &output)
    {
        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        boost::char_separator<char> sep(", \t \n \r"); // separators here
        tokenizer tokens(input, sep);

        for (tokenizer::iterator tok_iter = tokens.begin();
            tok_iter != tokens.end(); ++tok_iter)
        {
            CIDRIpAddress val;

            if ( convert( (*tok_iter).c_str(), val ) )
            {
                output.push_back(val);
            }
        }
        return true; 
    };

    // parse string of "x,y,z" to list<UInt16>
    template<>
    inline bool ValueConverter::convert< std::list< UInt16 > >(const std::string &input, std::list<UInt16> &output)
    {
        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        boost::char_separator<char> sep(", ; - \t \n \r"); // separators here
        tokenizer tokens(input, sep);

        for (tokenizer::iterator tok_iter = tokens.begin();
            tok_iter != tokens.end(); ++tok_iter)
        {
            output.push_back(static_cast<UInt16>(strtoul((*tok_iter).c_str(), NULL, 0)));
        }
        return true; 
    };

    // parse string of "x,y,z" to list<UInt32>
    template<>
    inline bool ValueConverter::convert< std::list< UInt32 > >(const std::string &input, std::list<UInt32> &output)
    {
        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        boost::char_separator<char> sep(", ; - \t \n \r"); // separators here
        tokenizer tokens(input, sep);

        for (tokenizer::iterator tok_iter = tokens.begin();
            tok_iter != tokens.end(); ++tok_iter)
        {
            output.push_back(static_cast<UInt32>(strtoul((*tok_iter).c_str(), NULL, 0)));
        }
        return true; 
    };

    // parse string of "x,y,z" to list<UInt64>
    template<>
    inline bool ValueConverter::convert< std::list< UInt64 > >(const std::string &input, std::list<UInt64> &output)
    {
        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        boost::char_separator<char> sep(", ; - \t \n \r"); // separators here
        tokenizer tokens(input, sep);

        for (tokenizer::iterator tok_iter = tokens.begin();
            tok_iter != tokens.end(); ++tok_iter)
        {
            output.push_back(static_cast<UInt64>(strtod((*tok_iter).c_str(), NULL)));
        }
        return true; 
    };
    // parse string of "x,y,z" to list<std::string>
    template<>
    inline bool ValueConverter::convert< std::list< std::string > >(const std::string &input, std::list<std::string> &output)
    {
        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        boost::char_separator<char> sep(", ; - \t \n \r"); // separators here
        tokenizer tokens(input, sep);

        for (tokenizer::iterator tok_iter = tokens.begin();
            tok_iter != tokens.end(); ++tok_iter)
        {
            output.push_back(*tok_iter);
        }
        return true; 
    };

    // parse string of "x,y,z" to set<UInt16>
    template<>
    inline bool ValueConverter::convert< std::set< UInt16 > >(const std::string &input, std::set<UInt16> &output)
    {
        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        boost::char_separator<char> sep(", ; - \t \n \r"); // separators here
        tokenizer tokens(input, sep);

        for (tokenizer::iterator tok_iter = tokens.begin();
            tok_iter != tokens.end(); ++tok_iter)
        {
            output.insert(static_cast<UInt16>(strtoul((*tok_iter).c_str(), NULL, 0)));
        }
        return true; 
    };

    // parse string of "x,y,z" to set<UInt32>
    template<>
    inline bool ValueConverter::convert< std::set< UInt32 > >(const std::string &input, std::set<UInt32> &output)
    {
        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        boost::char_separator<char> sep(", ; - \t \n \r"); // separators here
        tokenizer tokens(input, sep);

        for (tokenizer::iterator tok_iter = tokens.begin();
            tok_iter != tokens.end(); ++tok_iter)
        {
            output.insert(static_cast<UInt32>(strtoul((*tok_iter).c_str(), NULL, 0)));
        }
        return true; 
    };

    // parse string of "x,y,z" to set<UInt64>
    template<>
    inline bool ValueConverter::convert< std::set< UInt64 > >(const std::string &input, std::set<UInt64> &output)
    {
        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        boost::char_separator<char> sep(", ; - \t \n \r"); // separators here
        tokenizer tokens(input, sep);

        for (tokenizer::iterator tok_iter = tokens.begin();
            tok_iter != tokens.end(); ++tok_iter)
        {
            output.insert(static_cast<UInt64>(strtod((*tok_iter).c_str(), NULL)));
        }
        return true; 
    };

    // parse string of "x,y,z" to set<std::string>
    template<>
    inline bool ValueConverter::convert< std::set< std::string > >(const std::string &input, std::set<std::string> &output)
    {
        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        boost::char_separator<char> sep(", ; - \t \n \r"); // separators here
        tokenizer tokens(input, sep);

        for (tokenizer::iterator tok_iter = tokens.begin();
            tok_iter != tokens.end(); ++tok_iter)
        {
            output.insert(*tok_iter);
        }
        return true; 
    };

    //--------------------------------------------------------------------------------------------------------------------

    class QConfigManager : public Singleton<QConfigManager>
    {
    public:


        struct CNestedValue
        {
            void clear()
            {
                m_name.clear();
                m_value.clear();
                m_attributes.clear();
            }

            std::string     m_name;
            std::string     m_value;

            std::vector< std::pair< std::string, std::string > > m_attributes;
        };

    
        /****************************************************************************************************
        * Custom class for iterating over nested (and normal) xml tags.
        * Supported tags types:
        *
        * - Repeated tags
        *       <tag_name>val1</tag_name>
        *       <tag_name>val2</tag_name>
        *
        * ---------------------------------------------------------------------------------------------------
        *
        * - One level nested tags:
        *       <PointCodes>
        *           <PointCode> 1 </PointCode>
        *           <PointCode> 2 </PointCode>
        *           <PointCode> 3 </PointCode>
        *       </PointCodes>
        *       <PointCodes>
        *           <PointCode> 4 </PointCode>
        *           <PointCode> 5 </PointCode>
        *           <PointCode> 6 </PointCode>
        *       </PointCodes>
        *
        * ---------------------------------------------------------------------------------------------------
        *
        * - One level repeated nested tags:
        *       <kpmRuleInterface>
        *           <host>      127.0.0.1   </host>
        *           <port>      9001        </port>
        *           <mailbox>   1           </mailbox>
        *       </kpmRuleInterface>
        *       <kpmRuleInterface>
        *           <host>      127.0.0.2   </host>
        *           <port>      9002        </port>
        *           <mailbox>   3           </mailbox>
        *       </kpmRuleInterface>
        *
        * ---------------------------------------------------------------------------------------------------
        *
        * - One level repeated nested tags with attributes:
        *       <Operator ExtendedName="AFRICELL" Country="Gambia" TAG="GM02" MCC="607" MNC="02">
        *           <CodeE164>22070</CodeE164>
        *           <CodeE164>22077</CodeE164>
        *       </Operator>
        *       <Operator ExtendedName="AIRCEL CELLULAR (Chennai)" Country="India" TAG="IN41" MCC="404" MNC="41">
        *           <CodeE164>919841</CodeE164>
        *       </Operator>
        *
        * ---------------------------------------------------------------------------------------------------
        * Sample usage:
        * itc allows to iterate over all repeated elements. For every element we can query for
        * - attribute: itc.getAttribute("ExtendedName",output)
        * - nested_values: a vector of pair (name, value), itc.getNestedValues(nested_values)
        *
        * itc = QAppNG::QConfigManager::instance().getIterator("main","Operator");
        * while (itc.isValid()) // iterate over all repeated elements
        * {
        *     std::vector<QAppNG::QConfigManager::CNestedValue> nested_values;
        *     std::string output;
        *     itc.getAttribute("ExtendedName",output);
        *     itc.getNestedValues(nested_values);
        *     for (UInt8 i = 0; i!=nested_values.size(); ++i)
        *         atoi((nested_values[i].m_value).c_str());
        *     itc.next();
        * }
        *
        * ---------------------------------------------------------------------------------------------------
        *
        ****************************************************************************************************/
        class QConfigManagerIterator
        {
        public:

            QConfigManagerIterator(const std::shared_ptr<pugi::xml_document>& a_xml_document, const std::string& parameter_name) 
            {
                if (!a_xml_document)
                {
                    m_current_position = 0;
                    return;
                }
                pugi::xml_node iterator_node;

                /*if ( std::string("config").compare( a_xml_document->first_child().name() ) == 0 )
                {*/
                iterator_node = a_xml_document->first_child().child( parameter_name.c_str() );
                /*}
                else
                {
                iterator_node = a_xml_document->child( parameter_name.c_str() );
                }*/

                while( iterator_node )
                {
                    if ( parameter_name == iterator_node.name() )
                    {
                        m_node_vector.push_back( iterator_node );
                    }

                    iterator_node = iterator_node.next_sibling();
                }

                m_current_position = 0;
            }

            bool isValid() 
            {
                return m_current_position != m_node_vector.size();
            }

            /* TODO: TEMPLETIZED VERSION
            * template <class RETURNED_TYPE>
            * bool getNestedValues( std::vector< std::pair< std::string, typename RETURNED_TYPE > > &output )
            */
            bool getNestedValues( std::vector< CNestedValue > &output )
            {
                bool            rval(false);
                CNestedValue    value;

                if ( !isValid() )
                    return false;

                pugi::xml_node child_node = m_node_vector[ m_current_position ].first_child();

                // Check if iterator points to a NON-NESTED tag
                if ( !child_node )
                {
                    value.m_name = m_node_vector[ m_current_position ].name();
                    rval = vc.convert<std::string>( m_node_vector[ m_current_position ].value(), value.m_value );
                    if ( rval )
                        output.push_back( value );
                }
                else // If points to a NESTED type element return a vector of nested elements
                {
                    while ( child_node )    
                    {
                        value.m_name = child_node.name();
                        std::string node_value  = child_node.child_value();
                        boost::algorithm::trim( node_value );

                        rval = vc.convert<std::string>( node_value, value.m_value );

                        pugi::xml_attribute attr_node = child_node.first_attribute();

                        while( attr_node )
                        {
                            std::string attr_name = attr_node.name();
                            std::string attr_value = attr_node.value();

                            value.m_attributes.push_back( make_pair( attr_name, attr_value ) );

                            attr_node = attr_node.next_attribute();

                            rval = true;
                        }

                        if ( rval )
                            output.push_back( value );

                        value.clear();
                        child_node = child_node.next_sibling();    
                    }
                }

                return rval;
            }

            bool getNestedValuesByName(const std::string& tag_name, std::vector< CNestedValue > &output)
            {
                bool            rval(false);
                CNestedValue    value;

                if (!isValid())
                    return false;

                pugi::xml_node child_node = m_node_vector[m_current_position].first_child();

                // Check if iterator points to a NON-NESTED tag
                if (!child_node)
                {
                    if ( tag_name != m_node_vector[m_current_position].name() )
                    {
                        return false;
                    }

                    value.m_name = m_node_vector[m_current_position].name();
                    rval = vc.convert<std::string>(m_node_vector[m_current_position].value(), value.m_value);
                    if (rval)
                        output.push_back(value);
                }
                else // If points to a NESTED type element return a vector of nested elements
                {
                    while (child_node)
                    {
                        if ( tag_name == child_node.name() )
                        {
                            value.m_name = child_node.name();
                            std::string node_value = child_node.child_value();
                            boost::algorithm::trim(node_value);

                            rval = vc.convert<std::string>(node_value, value.m_value);

                            pugi::xml_attribute attr_node = child_node.first_attribute();

                            while (attr_node)
                            {
                                std::string attr_name = attr_node.name();
                                std::string attr_value = attr_node.value();

                                value.m_attributes.push_back(make_pair(attr_name, attr_value));

                                attr_node = attr_node.next_attribute();

                                rval = true;
                            }

                            if (rval)
                                output.push_back(value);

                            value.clear();
                        }

                        child_node = child_node.next_sibling();
                    }
                }

                return rval;
            }

            /*
            *  #6493
            *  TEMPLATIZED VERSION
            */
            template <class RETURNED_TYPE>
            bool getNestedValue( const std::string &nested_name, RETURNED_TYPE &nested_value )
            {
                for ( pugi::xml_node child_node = m_node_vector[ m_current_position ].first_child() ; child_node; child_node = child_node.next_sibling() )
                {
                    std::string name = child_node.name();
                    if ( name == nested_name )
                    {
                        // check if the node has children
                        if ( child_node.child_value() )
                        {
                            std::string node_value  = child_node.child_value();
                            boost::algorithm::trim( node_value );

                            // no child present then return node value
                            return vc.convert<RETURNED_TYPE>( node_value, nested_value );
                        }
                        else
                        {
                            // node has children then return false
                            return false;
                        }
                    }
                }

                // node not found
                return false;
            }

            /* TEMPLETIZED VERSION
            template <class RETURNED_TYPE>
            bool getAttributes( std::vector< std::pair< std::string, typename RETURNED_TYPE > > &output )
            */

            bool getAttribute(const std::string &attribute_name, std::string &attribute_value )
            {
                pugi::xml_attribute attribute_handle = m_node_vector[ m_current_position ].attribute( attribute_name.c_str() );

                if ( attribute_handle )
                {   
                    attribute_value = attribute_handle.value();
                    return true;
                }

                return false;
            }

            template <class RETURNED_TYPE>
            bool getAttribute( const std::string &attribute_name, RETURNED_TYPE &attribute_value )
            {

                pugi::xml_attribute attribute_handle = m_node_vector[ m_current_position ].attribute( attribute_name.c_str() );

                if ( attribute_handle )
                {
                    return vc.convert( attribute_handle.value(), attribute_value );
                }

                return false;
            }

            /*
            bool getAttributes( std::map< std::string, std::string > &output )
            {
            cal::xml::AttributeSet &attribute_set = xml_handles_vector[current_index]->ToElement()->GetAttributes();
            for ( cal::xml::AttributeSet::iterator it = attribute_set.begin(); it!= attribute_set.end(); ++it)
            {
            output.insert(make_pair((*it)->Name(), (*it)->Value()));
            }
            return true;
            }
            */

            bool getValue(std::string &output)
            {
                if ( !m_node_vector[ m_current_position ].text().empty() )
                {
                    output = m_node_vector[ m_current_position ].text().get();
                    boost::algorithm::trim( output );
                    if (output.c_str())
                        return true;
                }

                return false;
            }

            template <class RETURNED_TYPE>
            bool getValue( RETURNED_TYPE &node_value )
            {
                if ( !m_node_vector[ m_current_position ].text().empty() )
                {
                    std::string output = m_node_vector[ m_current_position ].text().get();
                    boost::algorithm::trim( output );
                    if (output.c_str())
                        return vc.convert( output, node_value );
                }

                return false;
            }

            void next()
            {
                if ( isValid() ) 
                    ++m_current_position; 
            }

        private:
            std::vector< pugi::xml_node > m_node_vector;
            size_t m_current_position;

            ValueConverter vc;
        };

        /****************************************************************************************************
        * Load xml file and indexes it with a configuration alias name
        ****************************************************************************************************/
        bool loadConfigurationFile( const std::string &configuration_alias, const std::string &file_name )
        {
            boost::recursive_mutex::scoped_lock lock(m_mutex);
            bool rv( false );

            std::string line;
            std::string xml_buf;
            std::ifstream config_file(file_name);
            if(config_file.is_open())
            {
                while(getline(config_file,line))
                {
                    xml_buf += line;
                    xml_buf += "\n";
                }
                config_file.close();
            }

            std::shared_ptr<pugi::xml_document> xml_doc_ptr(new pugi::xml_document());
            pugi::xml_parse_result  result = xml_doc_ptr->load_buffer(xml_buf.c_str(),xml_buf.length());
            if (result)
            {
                    // store the xml in the map
                if (!m_xml_documents_map.count(configuration_alias))
                {
                    m_xml_documents_map[configuration_alias] = xml_doc_ptr;
                }

                    rv = true;
            }
            else
            {
                std::cout << "QAppNG::QConfigManager: " + file_name + " " + std::string(result.description()) << std::endl;
                return false;
            }

            return rv;
        }
        /****************************************************************************************************/

        /****************************************************************************************************
        * Load xml file and indexes it with a configuration alias name
        ****************************************************************************************************/
        bool loadConfigurationDir( const std::string &configuration_alias, const boost::filesystem::path & directory, const std::string &file_name )
        {
            // get lock
            boost::recursive_mutex::scoped_lock lock(m_mutex);
            bool rv( false );

            std::list<std::string> configfiles;

            if( boost::filesystem::exists( directory ) )
            {
                boost::filesystem::directory_iterator end;
                for( boost::filesystem::directory_iterator iter(directory) ; iter != end ; ++iter )
                {
                    if(boost::filesystem::is_regular_file(*iter))
                    {
                        boost::filesystem::path pathfile = *iter;

                        std::string xml_filename = pathfile.string().substr(directory.string().length(),std::string::npos);
                        
                        if(xml_filename.compare(file_name))
                        {
                            size_t xml_filename_length = xml_filename.length();

                            if(xml_filename.find(".xml", xml_filename_length-4) != std::string::npos)
                            {
                                configfiles.push_back(xml_filename);
                            }
                        }
                    
                    }
                }

                if(configfiles.size() > 0 )
                {
                    std::string xml_container_filename = directory.string()+file_name;
                    std::ofstream xmlcontainer( xml_container_filename.c_str(), std::ios_base::binary);
                    xmlcontainer << "<config>" << std::endl;
                    xmlcontainer << std::endl;

                    std::list<std::string>::iterator itcfg = configfiles.begin();
                    while(itcfg != configfiles.end())
                    {
                        //xmlcontainer << "<include href=\"" << *itcfg << "\"/>" << std::endl;
                        
                        std::string tmpstring("");
                        if(getAllXmlContextToString(directory.string()+(*itcfg),tmpstring))
                        {
                            xmlcontainer << tmpstring << std::endl;
                        }

                        itcfg++;
                    }
                    xmlcontainer << std::endl;
                    xmlcontainer << "</config>" << std::endl;
                    xmlcontainer.close();



                    rv = loadConfigurationFile(configuration_alias, xml_container_filename);
                }
            }

            return rv;
        }
        /****************************************************************************************************/
        
        /****************************************************************************************************
        * Check if xml file was already loaded
        ****************************************************************************************************/
        bool hasConfigurationFile( const std::string &configuration_alias )
        {
            boost::recursive_mutex::scoped_lock lock(m_mutex);

            bool rv;
            m_xml_documents_map.count(configuration_alias) 
                ? rv = true
                : rv = false;

            return rv;
        }
        /****************************************************************************************************/

        /****************************************************************************************************
        * Set default values with provided map (std::map<std::string, std::string>)
        ****************************************************************************************************/
        void setDefaultValues( const std::string &configuration_alias, std::map<std::string, std::string> _default_values ) 
        {
            boost::recursive_mutex::scoped_lock lock(m_mutex);

            // create new map for the configuration_alias if no one is present
            if ( !m_default_values_map.count(configuration_alias) )
            {
                m_default_values_map.insert( make_pair( configuration_alias, std::map<std::string, std::string>() ) );
            }

            // copy the map
            m_default_values_map[configuration_alias].insert( _default_values.begin(), _default_values.end() );
        }
        /****************************************************************************************************/

        /****************************************************************************************************
        * Get and store (output) a value ( parameter_name ) from an xml file (configuration_alias)
        * performing type convertion:
        *
        * Example:
        *   Load ProbePairType tag value into std::string probe_pair_type 
        *   <ProbePairType>0</ProbePairType>
        *   QAppNG::QConfigManager::instance().getValue<std::string>("main", "ProbePairType", probe_pair_type);
        *  
        ****************************************************************************************************/
        template <class RETURNED_TYPE>
        bool getValue( const std::string &configuration_alias, const std::string &parameter_name, RETURNED_TYPE &output )
            //bool getValue( const char &configuration_alias, const char &parameter_name[], RETURNED_TYPE &output )
        {
            bool rv( false );

            if ( getValueFromCustom<RETURNED_TYPE> ( configuration_alias, parameter_name, output ) )
                rv = true;
            else if ( getValueFromXmlDocument<RETURNED_TYPE>( configuration_alias, parameter_name, output ) )
                rv = true;
            else if ( getValueFromDefault<RETURNED_TYPE>( configuration_alias, parameter_name, output ) )
                rv = true;

            return rv;
        }
        /****************************************************************************************************/

        /****************************************************************************************************
        * Get and store (output) a value ( parameter_name ) from an xml file (configuration_alias)
        * performing type convertion:
        *
        * Example:
        *   Load ProbePairType tag value into std::string probe_pair_type 
        *   <ProbePairType>0</ProbePairType>
        *   QAppNG::QConfigManager::instance().getValue<std::string>("main", "ProbePairType", probe_pair_type);
        *  
        ****************************************************************************************************/
        template <class RETURNED_TYPE>
        bool getAttribute( const std::string &configuration_alias, const std::string &parameter_name, const std::string &attribute_name, RETURNED_TYPE &output )
        {
            bool rv( false );

            if ( getAttributeFromXmlDocument<RETURNED_TYPE>( configuration_alias, parameter_name, attribute_name, output ) )
                rv = true;

            return rv;
        }
        /****************************************************************************************************/

        /****************************************************************************************************
        * Get and store single nested_name value (output) inside a parameter_name from an xml file (configuration_alias)
        ****************************************************************************************************/
        template <class RETURNED_TYPE>
        bool getNestedValue( const std::string &configuration_alias, const std::string &parameter_name, const std::string &nested_name, RETURNED_TYPE &output )
        {
            bool rv( false );

            if ( getNestedValueFromXmlDocument<RETURNED_TYPE>( configuration_alias, parameter_name, nested_name, output ) )
                rv = true;

            return rv;
        }
        /****************************************************************************************************/

        /****************************************************************************************************
        * Get and store a list of nested_name value (output) inside a parameter_name from an xml file (configuration_alias)
        ****************************************************************************************************/
        template <class RETURNED_TYPE>
        bool getNestedValueList( const std::string &configuration_alias, const std::string &parameter_name, const std::string &nested_name, std::list<RETURNED_TYPE> &output )
        {
            bool rv( false );

            if ( getNestedValueListFromXmlDocument<RETURNED_TYPE>( configuration_alias, parameter_name, nested_name, output ) )
                rv = true;

            return rv;
        }
        /****************************************************************************************************/

        /****************************************************************************************************
        * Set Custom Values ( used by application command line argument )
        ****************************************************************************************************/
        void setValue( const std::string &configuration_alias, const std::string &parameter_name, std::string input )
        {
            boost::recursive_mutex::scoped_lock lock(m_mutex);

            if ( !m_custom_values_map.count(configuration_alias) )
            {
                m_custom_values_map.insert( make_pair( configuration_alias, std::map<std::string, std::string>() ) );
            }

            m_custom_values_map[configuration_alias].insert( make_pair( parameter_name, input ) );
        }
        /****************************************************************************************************/

        /****************************************************************************************************
        * Get Iterator of tags
        *****************************************************************************************************/
        QConfigManagerIterator getIterator( const std::string& configuration_alias, const std::string& parameter_name )
        {
            if (m_xml_documents_map.find(configuration_alias) != m_xml_documents_map.end())
            {
                std::shared_ptr<pugi::xml_document> xml_doc_ptr = m_xml_documents_map[configuration_alias];
                return QConfigManagerIterator( xml_doc_ptr, parameter_name );
            }
            else
            {
                return QConfigManagerIterator( std::shared_ptr<pugi::xml_document>(), parameter_name );
            }
        }
        /****************************************************************************************************/

    private:

        std::map< std::string, std::map< std::string, std::string > >   m_default_values_map;
        std::map< std::string, std::map< std::string, std::string > >   m_custom_values_map;
        std::map< std::string, std::shared_ptr<pugi::xml_document> >  m_xml_documents_map;

        ValueConverter vc;

        // LOCK
        boost::recursive_mutex m_mutex;

        template <class RETURNED_TYPE>
        bool getValueFromXmlDocument( const std::string &configuration_alias, const std::string &parameter_name, RETURNED_TYPE &output )
        {
            if ( !m_xml_documents_map.count(configuration_alias) ) return false;

            std::shared_ptr<pugi::xml_document> xml_doc_ptr = m_xml_documents_map[configuration_alias];

            pugi::xml_node root_handle( xml_doc_ptr->first_child() );

            if ( !root_handle.empty() && root_handle.child(parameter_name.c_str()) )
            {
                std::string node_value = root_handle.child(parameter_name.c_str()).child_value();
                boost::algorithm::trim( node_value );

                if( node_value.empty() )
                {
                    return false;
                }

                return vc.convert<RETURNED_TYPE>(node_value, output );
            }

            return false;
        };

        template <class RETURNED_TYPE>
        bool getValueFromDefault( const std::string &configuration_alias, const std::string &parameter_name, RETURNED_TYPE &output )
        {
            if ( m_default_values_map.count(configuration_alias) && m_default_values_map[configuration_alias].count(parameter_name) )
                return vc.convert<RETURNED_TYPE>( m_default_values_map[configuration_alias][parameter_name], output );
            else
                return false;
        };

        template <class RETURNED_TYPE>
        bool getValueFromCustom( const std::string &configuration_alias, const std::string &parameter_name, RETURNED_TYPE &output )
        {
            if ( m_custom_values_map.count(configuration_alias) && m_custom_values_map[configuration_alias].count(parameter_name) )
                return vc.convert<RETURNED_TYPE>( m_custom_values_map[configuration_alias][parameter_name], output );
            else
                return false;
        };

        template <class RETURNED_TYPE>
        bool getAttributeFromXmlDocument( const std::string &configuration_alias, const std::string &parameter_name, const std::string &attibute_name, RETURNED_TYPE &output )
        {
            if ( !m_xml_documents_map.count(configuration_alias) ) return false;

            std::shared_ptr<pugi::xml_document> xml_doc_ptr = m_xml_documents_map[configuration_alias];

            pugi::xml_node root_handle( xml_doc_ptr->first_child() );

            if ( !root_handle.empty() && root_handle.child( parameter_name.c_str() ) )
            {
                pugi::xml_node param_handle=root_handle.child(parameter_name.c_str() );

                if ( !param_handle.empty() && param_handle.attribute( attibute_name.c_str() ) )

                {
                    std::string attribute_value = param_handle.attribute(attibute_name.c_str()).value();
                    if( attribute_value.empty() )
                    {
                        return false;
                    }
                    return vc.convert<RETURNED_TYPE>( attribute_value, output );
                }
            }

            return false;
        };

        template <class RETURNED_TYPE>
        bool getNestedValueListFromXmlDocument( const std::string &configuration_alias, const std::string &parameter_name, const std::string &nested_name, std::list<RETURNED_TYPE> &output )
        {
            bool found(false);

            if ( !m_xml_documents_map.count(configuration_alias) ) return false;

            std::shared_ptr<pugi::xml_document> xml_doc_ptr = m_xml_documents_map[configuration_alias];

            pugi::xml_node root_handle( xml_doc_ptr->first_child() );

            if ( !root_handle.empty() && root_handle.child( parameter_name.c_str() ) )
            {
                if ( !root_handle.empty() && root_handle.child( parameter_name.c_str() ) )
                {
                    pugi::xml_node param_handle=root_handle.child(parameter_name.c_str() );



                    pugi::xml_node_iterator itp = param_handle.begin();
                    while ( itp != param_handle.end() )
                    {
                        std::string it_nestname = itp->name();
                        std::string it_nestval  = itp->child_value();

                        if ( nested_name.compare(it_nestname) == 0 )
                        {
                            found=true;
                            boost::algorithm::trim( it_nestval );
                            RETURNED_TYPE tmp;
                            vc.convert<RETURNED_TYPE>(it_nestval, tmp );
                            output.push_back(tmp);
                        }
                        ++itp;
                    }
                }
            }

            return found;
        };


        template <class RETURNED_TYPE>
        bool getNestedValueFromXmlDocument( const std::string &configuration_alias, const std::string &parameter_name, const std::string &nested_name, RETURNED_TYPE &output )
        {
            bool found(false);

            if ( !m_xml_documents_map.count(configuration_alias) ) return false;

            std::shared_ptr<pugi::xml_document> xml_doc_ptr = m_xml_documents_map[configuration_alias];

            pugi::xml_node root_handle( xml_doc_ptr->first_child() );

            if ( !root_handle.empty() && root_handle.child( parameter_name.c_str() ) )
            {
                if ( !root_handle.empty() && root_handle.child( parameter_name.c_str() ) )
                {
                    pugi::xml_node param_handle=root_handle.child(parameter_name.c_str() );

                    pugi::xml_node_iterator itp = param_handle.begin();
                    while ( itp != param_handle.end() )
                    {
                        std::string it_nestname = itp->name();
                        std::string it_nestval  = itp->child_value();

                        if ( nested_name.compare(it_nestname) == 0 )
                        {
                            found=true;
                            boost::algorithm::trim( it_nestval );
                            return vc.convert<RETURNED_TYPE>(it_nestval, output );
                        }
                        ++itp;
                    }
                }
            }

            return found;
        };

        bool getAllXmlContextToString(const std::string &file_name, std::string &xmlstring)
        {
            if( boost::filesystem::exists( file_name ) )
            {
                std::ostringstream xmlcontent;
                std::string line;
                std::ifstream myfile (file_name);
                if (myfile.is_open())
                {
                    while ( myfile.good() )
                    {
                        getline (myfile,line);
                        xmlcontent << line << std::endl;
                    }
                    myfile.close();
                    xmlstring = xmlcontent.str();
                    return true;
                }
            }
            
            return false;
        }
    };


}
#endif // INCLUDE_QCONFIGMANAGER_EVO_NG
// --------------------------------------------------------------------------------------------------------------------
// End of file

