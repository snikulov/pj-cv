////////////////////////////////////////////////////////////////////////////////////////////////
///	Copyright (C) 2016-2017, Sergei Nikulov (sergey.nikulov@gmail.com)
///	All rights reserved.
///
///	Redistribution and use in source and binary forms, with or without
///	modification, are permitted provided that the following conditions are met:
///	* Redistributions of source code must retain the above copyright
///	notice, this list of conditions and the following disclaimer.
///	* Redistributions in binary form must reproduce the above copyright
///	notice, this list of conditions and the following disclaimer in the
///	documentation and/or other materials provided with the distribution.
///	* Neither the name of the <organization> nor the
///	names of its contributors may be used to endorse or promote products
///	derived from this software without specific prior written permission.
///
///	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
///	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
///	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
///	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
///	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
///	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
///	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
///	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
///	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
///	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
////////////////////////////////////////////////////////////////////////////////////////////////


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

std::string get_time_captured()
{
   return "2016.12.13 08:47:22";
}

int main(int argc, char** argv)
{
    init_logger();
    auto lg = Logger::getInstance("main");
    try
    {
        using namespace soci;

        soci::session sql(backEnd, "host=87.255.232.104 port=3306 user=pizza password=pizza1 db=pj");
        soci::mysql_session_backend *sessionBackEnd = static_cast<soci::mysql_session_backend *>(sql.get_backend());
        std::string version = mysql_get_server_info(sessionBackEnd->conn_);

        std::cout << "Version is " << version << std::endl;

        auto cdt = get_time_captured();
        std::string ver {"1"};
        std::string params;
        std::string udt;
        std::string fname{"2016-12-13-08-47-22-266033653-140171534657280.jpg"};
        std::vector<unsigned char> buf(2000, 1);
        std::string strblob(buf.begin(), buf.end());

        LOG4CPLUS_ERROR(lg, "imencode successful, try to insert into db");

        sql << "insert into pizzas(PRODUCT_ID, RESTAURANT_ID, CAPTURE_DT, DIAMETER, CHEESE, COLOR, COATING, WELL, EDGE, RES, VER, PARAMS, FILENAME, PHOTO_SRC)"
                           "values(    4,          1,           :cdt,         9,      9,      9,      9,     9,    9,    9,  :ver, :params, :fname, :photo)",
                              use(cdt), use(ver), use(params), use(fname), use(strblob);
        LOG4CPLUS_ERROR(lg, "done... fname is " << fname);

    }
    catch(soci::mysql_soci_error const &e)
    {
	std::cout << "error " << e.what() << std::endl;
    }

    return 0;
}
