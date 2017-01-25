#ifndef JPEG_WRITTER_SQL_HPP__
#define JPEG_WRITTER_SQL_HPP__
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

#include <date.h>
#include <random>
#include <sstream>

#include <boost/application.hpp>
#include <boost/filesystem.hpp>
#include <boost/noncopyable.hpp>

#include <soci/mysql/soci-mysql.h>
#include <soci/soci.h>

#include <log4cplus/configurator.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/loglevel.h>

#include <tiny_dnn/tiny_dnn.h>

#include "opencv_frame.hpp"

using namespace log4cplus;
using namespace log4cplus::helpers;

using tiny_dnn::vec_t;

class jpeg_writter_sql
{
public:
    jpeg_writter_sql(const std::string& dbip)
        : lg_(Logger::getInstance("jpgsql"))
        , dbip_(dbip)
        , sql_("mysql", std::string("host=") + dbip_ + std::string(" port=3306 user=pizza password=pizza1 db=pj"))
        , di_(6, 10)
    {
        std::string model_file = "group.model";
        LOG4CPLUS_INFO(lg_, "Result will be stored on mysql host=" << dbip_ << " port=3306 db=pj");
        LOG4CPLUS_INFO(lg_, "Loading model from " << model_file);

        nn.load(model_file);
    }

    void operator()(opencv_frame_t d)
    {
        if (d.frame_ && !d.frame_->empty())
        {
            if (d.squares_ && !(d.squares_->empty()))
            {
#if 0
                    // draw found objects on frame
                    draw_objects(d);
#endif
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
        if (d.squares_ && !(d.squares_->empty()))
        {
            const auto& sq = *(d.squares_);
            for (const auto& elm : sq)
            {
                cv::rectangle(*(d.frame_), elm, cv::Scalar(0, 0, 255), 4);
            }
        }
#endif
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
        buf.reserve(1024 * 200); // reserve 200K
        std::string fn{ get_fname(d) };

        if (cv::imencode(".jpg", *(d.frame_), buf))
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
            std::string ver{ "v0.1" };
            std::string params = get_json_params(d);
            std::string udt;
            auto fname = get_fname(d);
            std::string jpeg_data(buf.begin(), buf.end());
            long id = 0;

            LOG4CPLUS_INFO(lg_, "imencode successful, try to insert into db");

            vec_t res = nn.predict(Mat2vec_t(*(d.frame_)));

            double product_id, razmer, bort, cheezlok, cheezgot, cheezvnesh, cheezvnutr, kvadrvnesh, kvadrvnutr, cvetverh, cvetnijn, itogo;
            double vmin = 0.0, vmax = 0.8;
            int vn = 9;

            std::vector<double> params_values = { product_id, razmer, bort, cheezlok, cheezgot, cheezvnesh, cheezvnutr, kvadrvnesh, kvadrvnutr, cvetverh, cvetnijn };
            vec_t::const_iterator res_first = res.begin();
            vec_t::const_iterator res_last = res.begin();

            for (int index = 0; index < params_values.size(); index++)
            {
                if (index == 0) // product_id
                {
                    //  1 Super Papa, 2 Pepperoni, 3 Little Italy, 4	Vegetarian, 5	Meat, 6	Hawaiian, 7	Margherita, 8	Chicken Ranch
                    vmax = 8.0;
                    vmin = 1.0;
                    vn = 8;
                    res_last = res_first + vn;
                    vec_t res_subvector(res_first, res_last);
                    product_id = vec_t2value(res_subvector, vmin, vmax);
                }
                else if (index == 1) //razmer
                {
                    vmin = 0.0;
                    vmax = 0.8;
                    vn = 9;
                    res_last = res_first + vn;
                    vec_t res_subvector(res_first, res_last);
                    razmer = vec_t2value(res_subvector, vmin, vmax);
                }
                else if (index == 2) //bort
                {
                    vmin = 0.0;
                    vmax = 0.8;
                    vn = 9;
                    res_last = res_first + vn;
                    vec_t res_subvector(res_first, res_last);
                    bort = vec_t2value(res_subvector, vmin, vmax);
                }
                else if (index == 3) //cheezlok
                {
                    vmin = 0.0;
                    vmax = 0.8;
                    vn = 9;
                    res_last = res_first + vn;
                    vec_t res_subvector(res_first, res_last);
                    cheezlok = vec_t2value(res_subvector, vmin, vmax);
                }
                else if (index == 4) //cheezgot
                {
                    vmin = 0.0;
                    vmax = 0.8;
                    vn = 9;
                    res_last = res_first + vn;
                    vec_t res_subvector(res_first, res_last);
                    cheezgot = vec_t2value(res_subvector, vmin, vmax);
                }
                else if (index == 5) //cheezvnesh
                {
                    vmin = 0.0;
                    vmax = 0.8;
                    vn = 9;
                    res_last = res_first + vn;
                    vec_t res_subvector(res_first, res_last);
                    cheezvnesh = vec_t2value(res_subvector, vmin, vmax);
                }
                else if (index == 6) //cheezvnutr
                {
                    vmin = 0.0;
                    vmax = 0.8;
                    vn = 9;
                    res_last = res_first + vn;
                    vec_t res_subvector(res_first, res_last);
                    cheezvnutr = vec_t2value(res_subvector, vmin, vmax);
                }
                else if (index == 7) //kvadrvnesh
                {
                    vmax = 2.0;
                    vmin = 0.0;
                    vn = 9;
                    res_last = res_first + vn;
                    vec_t res_subvector(res_first, res_last);
                    kvadrvnesh = vec_t2value(res_subvector, vmin, vmax);
                }
                else if (index == 8) //kvadrvnutr
                {
                    vmax = 2.0;
                    vmin = 0.0;
                    vn = 9;
                    res_last = res_first + vn;
                    vec_t res_subvector(res_first, res_last);
                    kvadrvnutr = vec_t2value(res_subvector, vmin, vmax);
                }
                else if (index == 9) //cvetverh
                {
                    vmin = 0.0;
                    vmax = 0.8;
                    vn = 9;
                    res_last = res_first + vn;
                    vec_t res_subvector(res_first, res_last);
                    cvetverh = vec_t2value(res_subvector, vmin, vmax);
                }
                else if (index == 10) //cvetnijn
                {
                    vmin = 0.0;
                    vmax = 0.8;
                    vn = 9;
                    res_last = res_first + vn;
                    vec_t res_subvector(res_first, res_last);
                    cvetnijn = vec_t2value(res_subvector, vmin, vmax);
                }
                else
                {
                    LOG4CPLUS_ERROR(lg_, "Unknown NN result");
                }

                res_first = res_last;
            }

            itogo = razmer + bort + cheezlok + cheezgot
                + cheezvnesh + cheezvnutr + kvadrvnesh
                + kvadrvnutr + cvetverh + cvetnijn;
            itogo = itogo / 10;

            // { product_id,   razmer,   bort,   cheezlok,   cheezgot, cheezvnesh, cheezvnutr, kvadrvnesh, kvadrvnutr, cvetverh, cvetnijn };
            sql_ << "insert into pizzas1(PRODUCT_ID, RESTAURANT_ID, CAPTURE_DT, RAZMER, BORT, CHEEZLOK, CHEEZGOT, CHEEZVNESH, CHEEZVNUTR, KVADRVNESH, KVADRVNUTR, CVETVERH, CVETNIJN, ITOGO, VER, PARAMS, FILENAME)"
                    "values(    :id,          1,           :cdt,         :razmer, :bort, :cheezlok,  :cheezegot,  :cheezvnesh,    :cheezvnutr, :kvadrvnesh, :kvadrvnutr, :cvetverh, :cvetnijn, :itogo, :ver, :params, :fname)",
                use(product_id), use(cdt), use(razmer), use(bort), use(cheezlok), use(cheezgot), use(cheezvnesh), use(cheezvnutr), use(kvadrvnesh), use(kvadrvnutr), use(cvetverh), use(cvetnijn), use(itogo), use(ver), use(params), use(fname);
            if (sql_.get_last_insert_id("pizzas1", id))
            {
                sql_ << "insert into photos(id, capture_dt, filename, photo_src)"
                        "values( :id, :cdt, :fname, :jpeg_data)",
                    use(id), use(cdt), use(fname), use(jpeg_data);
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

    std::string get_cv_square(const cv::Rect& r)
    {
        std::string ret = "{ ";
        ret += (std::string("\"x\": ") + std::to_string(static_cast<int>(r.x)) + std::string(","));
        ret += (std::string("\"y\": ") + std::to_string(static_cast<int>(r.y)) + std::string(","));
        ret += (std::string("\"w\": ") + std::to_string(static_cast<int>(r.width)) + std::string(","));
        ret += (std::string("\"h\": ") + std::to_string(static_cast<int>(r.height)) + std::string(" }"));
        return ret;
    }

    std::string get_json_params(const opencv_frame_t& d)
    {
        std::string ret_json;
        if (d.squares_ && !(d.squares_->empty()))
        {
            int cnt = 0;
            ret_json = "{ \"squares\": [ ";
            const auto& sq = *(d.squares_);
            for (const auto& elm : sq)
            {
                if (cnt)
                {
                    ret_json += ", ";
                }
                ret_json += get_cv_square(elm);
                cnt++;
            }
            ret_json += " ] }";
        }
        return ret_json;
    }

    std::string get_fname(const opencv_frame_t& d)
    {
        auto ret = get_time_captured(d);

        auto pred = [](char& ch) { return (ch == ' ' || ch == ':' || ch == '.' || ch == ','); };
        std::replace_if(ret.begin(), ret.end(), pred, '-');
        ret += ".jpg";
        return ret;
    }

    int get_random_int_val()
    {
        return di_(dre_);
    }

    // Convert OpenCV image/Mat into tiny-dnn vec_t with 3 channels. Supports color image only
    vec_t Mat2vec_t(cv::Mat image, int padding = 0, int H = 1024, int W = 768)
    {
        if (image.channels() != 3)
        {
            LOG4CPLUS_ERROR(lg_, "Cannot prepare image for DNN, it has to have 3 channels");
        }

        cv::resize(image, image, cvSize(W - 2 * padding, H - 2 * padding));

        float denominator = 255;
        float min = -1.0;
        float max = 1.0;
        // As you can see, padding is applied (I am using a kernel of size 5 x 5 in the first convolutional layer). If your kernel is of size 2*k + 1 x 2*k + 1, then you should padd the image by k on each side. Further, the image is transformed to lie in [-1,1].

        vec_t sample;
        try
        {
            sample = vec_t((3 * (H + 2 * padding)) * (W + 2 * padding), min);
            for (int i = 0; i < H; i++)
            { // Go over all rows
                for (int j = 0; j < W; j++)
                { // Go over all columns
                    for (int c = 0; c < 3; c++)
                    { // Go through all channels
                        sample[W * H * c + W * (i + padding) + (j + padding)] = image.at<cv::Vec3b>(i, j)[c] / denominator * (max - min) + min;
                    }
                }
            }
        }
        catch (...)
        {
            throw std::current_exception();
        }
        return sample;
    }

    // function to build traning vector for single sample from double value
    double vec_t2value(vec_t vec, double min = 0.0, double max = 0.8, double* prob = NULL)
    {
        auto idx = max_index(vec);
        if (prob != NULL)
            *prob = vec[idx];
        return min + ((max - min) / (vec.size() - 1)) * idx;
    }

    log4cplus::Logger lg_;
    std::string dbip_;
    soci::session sql_;
    std::uniform_int_distribution<int> di_;
    std::default_random_engine dre_;
    tiny_dnn::network<tiny_dnn::sequential> nn;
};

#endif // JPEG_WRITTER_SQL_HPP__
