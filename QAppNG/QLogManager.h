#ifndef INCLUDE_QLOGMANAGER_EVO_NG
#define INCLUDE_QLOGMANAGER_EVO_NG


#include <QAppNG/Singleton.h>
#include <QAppNG/QConfigManager.h>
#include <log4cpp/Category.h>
#include <log4cpp/PropertyConfigurator.h>
#include <map>

namespace QAppNG
{

    class QLogManager : public Singleton<QLogManager>
    {
    public:

        typedef log4cpp::Priority Priority;

        // Load log4cpp configuration file from QLogManager
        QLogManager()
        {
            if (!QAppNG::QConfigManager::instance().getValue<std::string>("main", "logCfgFile", m_log_configuration_file))
                std::cout << "QLogManager error: error while reading main log configuration file:" << m_log_configuration_file  << std::endl;
        }

        // Configure log4cpp
        void init()
        {
			if (!m_log_configuration_file.empty())
				log4cpp::PropertyConfigurator::configure(m_log_configuration_file);
        }

        // Add a log4cpp Category to QLogManager
        void addCategoryInstance( const std::string &category_name )
        {
            log4cpp::Category::getInstance( category_name );
        }

        // Log to file
        // If no Category Instance has been set up, log messages are written to main log
        void log( const std::string &category_name, log4cpp::Priority::PriorityLevel priority_level,  const std::string &message )
        {
            log4cpp::Category* category = &log4cpp::Category::getInstance( category_name );
            if(category) category->log(priority_level, message);
        }

        void log( const std::string &category_name, log4cpp::Priority::PriorityLevel priority_level, const char* stringFormat, ... )
        {
            va_list args;
            va_start(args, stringFormat);
            log4cpp::Category* category = &log4cpp::Category::getInstance( category_name );
			if (category) category->logva(priority_level, stringFormat, args);
            va_end(args);   
        }

        void flush( const std::string &category_name, log4cpp::Priority::PriorityLevel priority_level )
        {
            log4cpp::Category* category = &log4cpp::Category::getInstance( category_name );
			if (category) category->getStream(priority_level).flush();
        }


		void setConfigurationPath(const std::string& fname)
		{
			m_log_configuration_file = fname;
			log4cpp::PropertyConfigurator::configure(m_log_configuration_file);
		}

    private:
        std::string m_log_configuration_file;
    };

}
#endif //INCLUDE_QLOGMANAGER_EVO_NG
