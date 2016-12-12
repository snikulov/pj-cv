#include "soci/soci.h"
#include "soci/mysql/soci-mysql.h"

#include <boost/application.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/loglevel.h>
#include <log4cplus/configurator.h>

namespace app = boost::application;
namespace fs = boost::filesystem;

using namespace log4cplus;
using namespace log4cplus::helpers;



static void init_logger()
{
    using namespace log4cplus;
    using namespace log4cplus::helpers;
    try
    {
        PropertyConfigurator::doConfigure("log4cplus.properties");
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        exit(0);
    }

}

soci::backend_factory const &backEnd = *soci::factory_mysql();

int main(int argc, char** argv)
{
    init_logger();
    try
    {
        soci::session sql(backEnd, "host=87.255.232.104 port=3306 user=pizza password=pizza1 db=pj");
        soci::mysql_session_backend *sessionBackEnd = static_cast<soci::mysql_session_backend *>(sql.get_backend());
        std::string version = mysql_get_server_info(sessionBackEnd->conn_);
        
        std::cout << "Version is " << version << std::endl;
    }
    catch(soci::mysql_soci_error const &e)
    {
	std::cout << "error " << e.what() << std::endl;
    }

    return 0;
}
