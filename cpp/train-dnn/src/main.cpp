/*
    Copyright (c) 2017, Evgeny Semenov
    All rights reserved.
    
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY 
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// cmake .. -DCMAKE_INSTALL_PREFIX=D:\projects\OpenCV\build -G"Visual Studio 14 2015 Win64"

#define UI

#include <iostream>

#include "tiny_dnn/tiny_dnn.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include <csv.h> // CVS reader, TODO replace with BOOST class?
#include "cxxopts.hpp" // https://github.com/jarro2783/cxxopts/blob/master/src/example.cpp - TODO: replace with boost programm options?

using namespace cv;
using namespace tiny_dnn;
using namespace std;

// Convert OpenCV image/Mat into tiny-dnn vec_t with 3 channels. Supports color image only
vec_t Mat2vec_t(Mat image, int padding = 0, int H = 1024, int W = 768)
{
	if (image.channels() != 3) throw "image has to have 3 channels";

	cv::resize(image, image, cvSize(W - 2 * padding, H - 2 * padding));

	float denominator = 255;
	float min = -1.0;
	float max = 1.0;
	// As you can see, padding is applied (I am using a kernel of size 5 x 5 in the first convolutional layer). If your kernel is of size 2*k + 1 x 2*k + 1, then you should padd the image by k on each side. Further, the image is transformed to lie in [-1,1].

	vec_t sample;
	try
	{
		sample = vec_t((3 * (H + 2 * padding))*(W + 2 * padding), min);
		for (int i = 0; i < H; i++) { // Go over all rows
			for (int j = 0; j < W; j++) { // Go over all columns
				for (int c = 0; c < 3; c++) { // Go through all channels
					sample[W*H*c + W*(i + padding) + (j + padding)] = image.at<cv::Vec3b>(i, j)[c] / denominator*(max - min) + min;
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
vec_t value2vec_t(double value, double min = 0.0, double max = 0.8, int n = 9)
{
	double k = 0;
	int i = 0;

	double negative_value = -1.0;
	double positive_value = 1.0;
	vec_t data = vec_t(n, negative_value);

	// find which backed to use		
	for (k = max, i = n - 1; value < k && i > 0; i--, k -= (max - min) / (n - 1));
	data[i] = positive_value;

#if 1 // workaround to make false estiamte closer to real value, like .7 instead of 0.8
	if (i > 0)   data[i - 1] = (negative_value + positive_value) / 2;
	if (i < n - 1) data[i + 1] = (negative_value + positive_value) / 2;
#endif
	return data;
}


// function to build traning vector for single sample from double value
double vec_t2value(vec_t vec, double min = 0.0, double max = 0.8, double* prob = NULL)
{
	auto idx = max_index(vec);
	if (prob != NULL) *prob = vec[idx];
	return ((max - min) / (vec.size() - 1)) * idx;
}

network<sequential> train_dnn(network<sequential> nn, std::vector<vec_t> images, std::vector<vec_t> targets, int minibatch_size = 1, int num_epochs = 50)
{
	timer t; // start the timer

	std::cout << "Start training." << std::endl;

	adagrad optimizer;
#if 0
	nn.weight_init(weight_init::lecun());
	nn.bias_init(weight_init::lecun());
	nn.init_weight();
	//optimizer.reset();
#endif

	progress_display disp(static_cast<unsigned long>(images.size()));

	double learning_rate = 0.2;
	optimizer.alpha *= static_cast<tiny_dnn::float_t>(sqrt(minibatch_size) * learning_rate);

	// create callback
	auto on_enumerate_epoch = [&]() {
		std::cout << t.elapsed() << "s elapsed." << std::endl;

		auto loss = nn.get_loss<mse>(images, targets);
		//nn.test(images).print_detail(std::cout);

		std::cout << "loss:" << loss << std::endl;

		disp.restart(static_cast<unsigned long>(images.size()));
		t.restart();
	};

	auto on_enumerate_minibatch = [&]() {
		disp += minibatch_size;
	};

	// potential classes
	// mse, absolute, absolute_eps
	// cross_entropy = cross-entropy loss function for (multiple independent) binary classifications,
	// cross_entropy_multiclass = cross-entropy loss function for multi-class classification

	nn.fit<mse>(optimizer, images, targets, minibatch_size, num_epochs,
		on_enumerate_minibatch, on_enumerate_epoch);

	std::cout << "end training." << std::endl;
	
	double elapsed_ms = t.elapsed();
	t.stop();

	cout << "Elapsed time(ms): " << elapsed_ms << endl;

	return nn;
}

int  main(int argc, char** argv) {

	cxxopts::Options options(argv[0], " - example command line options");
	// use this as example https://github.com/jarro2783/cxxopts/blob/master/src/example.cpp

	options.add_options()
		("t,train", "CSV File to train model", cxxopts::value<std::string>(), "CSV file used to train model")
		("d,dir", "Input directory", cxxopts::value<std::string>()->default_value("./"), "Path to dirrectory where to find images wtih IDs from CSV file")
		("m,model", "Model name", cxxopts::value<std::string>()->default_value("test.model"), "file name for model used for training or classification")
		("i,image", "Image to classify", cxxopts::value<std::string>(), "Image file to classify using defined model")
		("p,predict", "Use all images from CSV file to test model", cxxopts::value<std::string>(), "CSV file to test model, same format as training")
		("n,name", "Parameter name from CSV file to train or test against", cxxopts::value<std::string>()->default_value("bort"), "One of the parameter: product_id,razmer,bort,cheezlok,cheezgot,cheezvnesh,cheezvnutr,kvadrvnesh,kvadrvnutr,cvetverh,cvetnijn")
		("help", "Help")
		("e,epochs", "Number of training cycler - more is better", cxxopts::value<int>()->default_value("5"), "Number of Training cycles for DNN.")
		("c,continue", "Continue of model training: Y/N", cxxopts::value<std::string>()->default_value("N"), "DNN will preload model to continue training = Y/N")
		;

	options.parse(argc, argv);

	if (options.count("help"))
	{
		std::cout << options.help({ "", "Group" }) << std::endl;
		exit(0);
	}
	
	std::vector<vec_t> images2train, images2test;
	std::vector<vec_t> targets2train, targets2test;

	// pizzas parameters
	vector<std::string> params = { "product_id", "razmer", "bort", "cheezlok", "cheezgot", "cheezvnesh", "cheezvnutr", "kvadrvnesh", "kvadrvnutr", "cvetverh", "cvetnijn", "itogo" };
	string id, parameters;  double product_id, razmer, bort, cheezlok, cheezgot, cheezvnesh, cheezvnutr, kvadrvnesh, kvadrvnutr, cvetverh, cvetnijn, itogo;

	// GET name of the parameter to train or test agains
	auto it = std::find(params.begin(), params.end(), options["n"].as<std::string>());
	if (it == params.end())
		throw options["n"].as<std::string>() + " is wrong param";
	auto index = std::distance(params.begin(), it);

	// init vector based on type of parameter, default is 0.0-0.8 9 steps: 0.0 0.1 0.2 etc.
	double vmin = 0.0, vmax = 0.8; int vn = 9;

	if (index == 0) // product_id
	{
		//  1	Super Papa
		//  2	Pepperoni
		//	3	Little Italy
		//	4	Vegetarian
		//	5	Meat
		//	6	Hawaiian
		//	7	Margherita
		//	8	Chicken Ranch
		vmax = 8.0;
		vmin = 1.0;
		vn = 8;
	}
	if (index == 8 || index == 9) // "kvadrvnesh", "kvadrvnutr"
	{
		vmax = 2.0;
		vmin = 0.0;
		vn = 9;
	}
	if (index == 11) // itogo
	{
		vmax = 10.0;
		vmin = 0.0;
		vn = 101;
	}
	if (index == 8 || index == 9) // "kvadrvnesh", "kvadrvnutr"
	{
		vmax = 2.0;
		vmin = 0.0;
		vn = 9;
	}

	if (options.count("t"))
	{
		std::string csv_file = options["t"].as<std::string>();
		cout << "Loading " << csv_file << " to TRAIN model. Using :" << options["d"].as<std::string>() << " dirrectory as source for images" << std::endl;
		//read CVS
		io::CSVReader<14> csv2train(csv_file);
		csv2train.read_header(io::ignore_extra_column, "id", "product_id", "razmer", "bort", "cheezlok", "cheezgot", "cheezvnesh", "cheezvnutr", "kvadrvnesh", "kvadrvnutr", "cvetverh", "cvetnijn", "itogo", "params");
		
		while (csv2train.read_row(id, product_id, razmer, bort, cheezlok, cheezgot, cheezvnesh, cheezvnutr, kvadrvnesh, kvadrvnutr, cvetverh, cvetnijn, itogo, parameters)) {

			vector<double> params_values = { product_id,   razmer,   bort,   cheezlok,   cheezgot, cheezvnesh, cheezvnutr, kvadrvnesh, kvadrvnutr, cvetverh, cvetnijn, itogo};

			double value = params_values[index];

			cv::String file_name = options["d"].as<std::string>() + id + ".jpg";
			Mat image = imread(file_name, IMREAD_COLOR); // Read the file

			if (!image.empty())
			{
				targets2train.push_back(value2vec_t(value, vmin, vmax, vn));
				images2train.push_back(Mat2vec_t(image));
			}
			else throw "Cannot load image: " + file_name + " ABORTING";
		}
	}
	
	if (options.count("p"))
	{
		std::string csv_file = options["p"].as<std::string>();
		cout << "Loading " << csv_file << " to TEST model. Using :" << options["d"].as<std::string>() << " dirrectory as source for images" << std::endl;

		//read CVS
		io::CSVReader<14> csv2train(csv_file);
		csv2train.read_header(io::ignore_extra_column, "id", "product_id", "razmer", "bort", "cheezlok", "cheezgot", "cheezvnesh", "cheezvnutr", "kvadrvnesh", "kvadrvnutr", "cvetverh", "cvetnijn", "itogo", "params");

		while (csv2train.read_row(id, product_id, razmer, bort, cheezlok, cheezgot, cheezvnesh, cheezvnutr, kvadrvnesh, kvadrvnutr, cvetverh, cvetnijn, itogo, parameters)) {

			vector<double> params_values = { product_id,   razmer,   bort,   cheezlok,   cheezgot, cheezvnesh, cheezvnutr, kvadrvnesh, kvadrvnutr, cvetverh, cvetnijn, itogo };

			double value = params_values[index];

			cv::String file_name = options["d"].as<std::string>() + id + ".jpg";
			Mat image = imread(file_name, IMREAD_COLOR); // Read the file

			if (!image.empty())
			{
				targets2test.push_back(value2vec_t(value, vmin, vmax, vn));
				images2test.push_back(Mat2vec_t(image));
			}
			else throw "Cannot load image: " + file_name + " ABORTING";
		}
	}

	network<sequential> nn;
	// DO TRAINING
	if (targets2train.size())
	{
		std::string model_name = options["m"].as<std::string>();
		if (!options.count("c"))
		{
			std::cout << "Creating new model " << model_name << std::endl;
		// Pther options: tan_h, relu
		nn << conv<tan_h>(1024, 768, 3, 3, 3, 9, padding::same, true, 1, 1);
		nn << average_pooling_layer<tan_h>(1024, 768, 9, 2);
		nn << conv<tan_h>(1024 / 2, 768 / 2, 3, 3, 9, 12, padding::same, true, 1, 1);
		nn << average_pooling_layer<tan_h>(1024 / 2, 768 / 2, 12, 2);
		nn << conv<tan_h>(1024 / 4, 768 / 4, 3, 3, 12, 12, padding::same, true, 1, 1);
		nn << conv<tan_h>(1024 / 4, 768 / 4, 3, 3, 12, 6, padding::same, true, 1, 1);
		nn << conv<tan_h>(1024 / 4, 768 / 4, 3, 3, 6, 6, padding::same, true, 1, 1);
		nn << average_pooling_layer<tan_h>(1024 / 4, 768 / 4, 6, 4);
		nn << fully_connected_layer<tan_h>(64 * 48 * 6, 200);
		nn << fully_connected_layer<tan_h>(200, targets2train[0].size());
		}
		else
		{
			std::cout << "Loading model to continue training " << model_name << std::endl;
			nn.load(model_name);
		}

		nn = train_dnn(nn, images2train, targets2train, 1, options["e"].as<int>());

		// save network model & trained weights
		remove(model_name.c_str());
		nn.save(model_name);
	}
	else
	{
		std::string model_name = options["m"].as<std::string>();
		
		std::cout << "Loading model " << model_name << std::endl;
		nn.load(model_name);
	}
	

	if (options.count("i"))
	{
		std::cout << "Testing:" << options["i"].as<std::string>() << std::endl;

		Mat image = imread(options["i"].as<std::string>(), IMREAD_COLOR); // Read the file

		if (!image.empty())
		{
			vec_t res = nn.predict(Mat2vec_t(image));
			std::cout << "Predicted:" << vec_t2value(res, vmin, vmax) << std::endl;
			std::copy(res.begin(), res.end(), std::ostream_iterator<float>(std::cout, " "));
			std::cout << std::endl;
		}
		else throw "Cannot load image: " + options["i"].as<std::string>() + " ABORTING";
	}

	if (options.count("p"))
	{
		std::cout << "Calculating loss from TEST ..." << std::endl;
		auto loss = nn.get_loss<mse>(images2test, targets2test);
		std::cout << "loss:" << loss << std::endl;

		for (int i = 0; i < targets2test.size(); i++)
		{
			vec_t res = nn.predict(images2test[i]);
			std::copy(targets2test[i].begin(), targets2test[i].end(), std::ostream_iterator<float>(std::cout, " "));
			std::cout << std::endl;
			std::copy(res.begin(), res.end(), std::ostream_iterator<float>(std::cout, " "));
			std::cout << std::endl;
			std::cout << "Predicted: " << vec_t2value(res, vmin, vmax) << " vs real " << vec_t2value(targets2test[i], vmin, vmax) << std::endl;
		}
	}
	else
	{
		std::cout << "Calculating loss from TRAIN ..." << std::endl;
		auto loss = nn.get_loss<mse>(images2train, targets2train);
		std::cout << "loss:" << loss << std::endl;

		for (int i = 0; i < targets2train.size(); i++)
		{
			vec_t res = nn.predict(images2train[i]);
			std::copy(targets2train[i].begin(), targets2train[i].end(), std::ostream_iterator<float>(std::cout, " "));
			std::cout << std::endl;
			std::copy(res.begin(), res.end(), std::ostream_iterator<float>(std::cout, " "));
			std::cout << std::endl;
			std::cout << "Predicted: " << vec_t2value(res, vmin, vmax) << " vs real " << vec_t2value(targets2train[i], vmin, vmax) << std::endl;
		}
	}

	cout << "Press any key";
	cin.get();
}


