#ifndef JPEG_WRITTER_SQL_HPP__
#define JPEG_WRITTER_SQL_HPP__
#include <sstream>
#include <date.h>
#include <random>

#include <opencv2/opencv.hpp>

#include <boost/application.hpp>
#include <boost/filesystem.hpp>
#include <boost/noncopyable.hpp>

#include "opencv_frame.hpp"

#include <soci/soci.h>
#include <soci/mysql/soci-mysql.h>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/loglevel.h>
#include <log4cplus/configurator.h>

using namespace log4cplus;
using namespace log4cplus::helpers;

class jpeg_writter_sql
{
    public:
        jpeg_writter_sql(const std::string& dbip)
            : lg_(Logger::getInstance("jpgsql"))
              , dbip_(dbip)
              , sql_("mysql", std::string("host=") + dbip_ + std::string(" port=3306 user=pizza password=pizza1 db=pj"))
              , di_(6, 10)
    {
        LOG4CPLUS_INFO(lg_, std::string("Result will be stored on mysql host=") << dbip_ << " port=3306 db=pj");
    }

        void operator()(opencv_frame_t d)
        {
            if (d.frame_ && !d.frame_->empty())
            {
                if (d.squares_ && !(d.squares_->empty()))
                {
                    draw_objects(d);
                    // detected image
                    std::string fname = get_fname(d);
                    if (write_image(d, fname))
                    {
                        LOG4CPLUS_INFO(lg_, "Writted image: " << fname);
                    }
                    else
                    {
                        LOG4CPLUS_WARN(lg_, "Error writting fail: " << fname);
                    }
                }
            }
        }

    private:

        void draw_objects(opencv_frame_t& d)
        {
#if 0
            if (d.circles_ && !(d.circles_->empty()))
            {
                // draw circles
                const auto& ci = *(d.circles_);
                for (const auto& elm : ci)
                {
                    cv::Point center(cvRound(elm[0]), cvRound(elm[1]));
                    int radius = cvRound(elm[2]);
                    circle(*(d.frame_), center, 3, cv::Scalar(0, 255, 0), -1, 8, 0);
                    // draw the circle outline
                    circle(*(d.frame_), center, radius, cv::Scalar(0, 0, 255), 3, 8, 0);
                }
            }
#endif
            if (d.squares_ && !(d.squares_->empty()))
            {
                const auto& sq = *(d.squares_);
                for (const auto& elm : sq)
                {
                    cv::rectangle(*(d.frame_), elm, cv::Scalar(0, 0, 255), 4);
                }
            }
        }

        std::string get_time_captured(const opencv_frame_t& d)
        {
            using namespace date;
            std::ostringstream ss;
            ss << d.time_captured_;
            return ss.str();
        }

        bool write_image(const opencv_frame_t& d, const std::string& fname)
        {
            using namespace soci;

            std::vector<unsigned char> buf;
            buf.reserve(1024*200); // reserve 200K
            std::string fn {get_fname(d)};

            if(cv::imencode(".jpg", *(d.frame_), buf))
            {
                //            CREATE TABLE IF NOT EXISTS `PJ`.`PIZZAS`(
                //                `ID` INTEGER UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
                //                `PRODUCT_ID` INTEGER UNSIGNED NOT NULL,
                //                `RESTAURANT_ID` INTEGER UNSIGNED NOT NULL,
                //                `CAPTURE_DT` DATETIME NOT NULL COMMENT 'дата/время захвата пиццы',
                //                `DIAMETER` INTEGER(2) UNSIGNED,
                //                `CHEESE` INTEGER(2) UNSIGNED,
                //                `COLOR` INTEGER(2) UNSIGNED,
                //                `COATING` INTEGER(2) UNSIGNED,
                //                `WELL` INTEGER(2) UNSIGNED,
                //                `EDGE` INTEGER(2) UNSIGNED,
                //                `RES` INTEGER(2) UNSIGNED NOT NULL,
                //                `VER` VARCHAR(32) NOT NULL COMMENT 'версия детектора и распознователя качетсва пиццы',
                //                `PARAMS` VARCHAR(256) COMMENT 'параметры для/от детектора и распознователя качетсва пиццы',
                //                `UPLOAD_DT` TIMESTAMP default now() COMMENT 'дата/время загрузки данных в БД',
                //                `FILENAME` VARCHAR(256) COMMENT 'имя файла',
                //                `PHOTO_SRC` LONGBLOB,
                //              CHECK (`DIAMETER` is NULL or `DIAMETER` between 1 and 10),
                //              CHECK (`CHEESE` is NULL or `CHEESE` between 1 and 10),
                //              CHECK (`COLOR` is NULL or `COLOR` between 1 and 10),
                //              CHECK (`WELL` is NULL or `WELL` between 1 and 10),
                //              CHECK (`EDGE` is NULL or `EDGE` between 1 and 10),
                //              CHECK (`RES` between 1 and 10),
                //              FOREIGN KEY (`PRODUCT_ID`) REFERENCES `PJ`.`PRODUCTS` (`id`) ON UPDATE CASCADE ON DELETE RESTRICT,
                //              FOREIGN KEY (`RESTAURANT_ID`) REFERENCES `PJ`.`RESTAURANTS` (`id`) ON UPDATE CASCADE ON DELETE RESTRICT
                //            );

                auto cdt = get_time_captured(d);
                std::string ver {"v0.1"};
                std::string params;
                std::string udt;
                auto fname = get_fname(d);
                std::string jpeg_data(buf.begin(), buf.end());
                long id = 0;

                LOG4CPLUS_INFO(lg_, "imencode successful, try to insert into db");
                int cheese  = get_random_int_val();
                int color   = get_random_int_val();
                int coating = get_random_int_val();
                int well    = get_random_int_val();
                int edge    = get_random_int_val();
                int res     = get_random_int_val();
                sql_ << "insert into pizzas(PRODUCT_ID, RESTAURANT_ID, CAPTURE_DT, DIAMETER, CHEESE, COLOR, COATING, WELL, EDGE, RES, VER, FILENAME)"
                    "values(    4,          1,           :cdt,         35, :cheese, :color,  :coating,  :well,    :edge, :res,  :ver, :fname)",
                    use(cdt), use(cheese), use(color), use(coating), use(well), use(edge), use(res) ,use(ver), use(fname);
                if(sql_.get_last_insert_id("pizzas", id))
                {
                    sql_ << "insert into photos(id, capture_dt, filename, photo_src)"
                        "values( :id, :cdt, :fname, :jpeg_data)", use(id), use(cdt), use(fname), use(jpeg_data);
                }
                else
                {
                    LOG4CPLUS_ERROR(lg_, "get_last_insert_id returned error");
                }
                LOG4CPLUS_INFO(lg_, "done... fname is " << fname);

            }
            else
            {
                LOG4CPLUS_ERROR(lg_, "imencode return false");
            }

        }


        std::string get_fname(const opencv_frame_t& d)
        {
            auto ret = get_time_captured(d);

            auto pred = [](char& ch)
            { return (ch == ' ' || ch == ':' || ch == '.' || ch == ','); };
            std::replace_if(ret.begin(), ret.end(), pred, '-');
            ret += ".jpg";
            return ret;
        }

        int get_random_int_val()
        {
            return di_(dre_);
        }

        log4cplus::Logger lg_;
        std::string dbip_;
        soci::session sql_;
        std::uniform_int_distribution<int> di_;
        std::default_random_engine dre_;
};

#endif // JPEG_WRITTER_SQL_HPP__
