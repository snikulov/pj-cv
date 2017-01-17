/*
    Copyright (c) 2013, Taiga Nomi
    Copyright (c) 2016, Taiga Nomi, Edgar Riba
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

#define UI

#include <iostream>

#include "tiny_dnn/tiny_dnn.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include <csv.h> // CVS reader, TODO replace with BOOST class?

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
#if 1
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
	optimizer.reset();
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



	// TRAINING
#if 1

	std::vector<vec_t> images;
	std::vector<vec_t> targets;
	std::string dir_name = "D:/Projects/tiny-dnn/x64/Debug/images2/";
	
	//read CVS
	io::CSVReader<14> in(dir_name + "pizzas2.csv");
	in.read_header(io::ignore_extra_column, "id", "product_id", "razmer", "bort", "cheezlok", "cheezgot", "cheezvnesh", "cheezvnutr", "kvadrvnesh", "kvadrvnutr", "cvetverh", "cvetnijn", "itogo", "params");
	// pizzas parameters
	string id, params;  double product_id, razmer, bort, cheezlok, cheezgot, cheezvnesh, cheezvnutr, kvadrvnesh, kvadrvnutr, cvetverh, cvetnijn, itogo;
	
	

	while (in.read_row(id, product_id, razmer, bort, cheezlok, cheezgot, cheezvnesh, cheezvnutr, kvadrvnesh, kvadrvnutr, cvetverh, cvetnijn, itogo, params)) {
		double value = cheezgot;
		cv::String file_name = dir_name + id + ".jpg";

		Mat image = imread(file_name, IMREAD_COLOR); // Read the file

		if (!image.empty())
		{
			targets.push_back(value2vec_t(value));
			images.push_back(Mat2vec_t(image));
		}
	}

	network<sequential> nn;

	/* works ok for clearness*/
	// tan_h
	// relu
		nn << conv<tan_h>(1024, 768, 3, 3, 3, 9, padding::same, true, 1, 1);
	nn << average_pooling_layer<tan_h>(1024, 768, 9, 2);
	nn << conv<tan_h>(1024 / 2, 768 / 2, 3, 3, 9, 12, padding::same, true, 1, 1);
	nn << average_pooling_layer<tan_h>(1024 / 2, 768 / 2, 12, 2);
	nn << conv<tan_h>(1024 / 4, 768 / 4, 3, 3, 12, 12, padding::same, true, 1, 1);
	nn << conv<tan_h>(1024 / 4, 768 / 4, 3, 3, 12, 6, padding::same, true, 1, 1);
	nn << conv<tan_h>(1024 / 4, 768 / 4, 3, 3, 6, 6, padding::same, true, 1, 1);
	nn << average_pooling_layer<tan_h>(1024 / 4, 768 / 4, 6, 4);

	nn << fully_connected_layer<tan_h>(64 * 48 * 6, 200);
	nn << fully_connected_layer<tan_h>(200, targets[0].size());


	nn = train_dnn(nn, images, targets, 1, 3);


	for (int i = 0; i < targets.size(); i++)
	{
		vec_t res = nn.predict(images[i]);
		std::copy(targets[i].begin(), targets[i].end(), std::ostream_iterator<float>(std::cout, " "));
		std::cout << std::endl;
		std::copy(res.begin(), res.end(), std::ostream_iterator<float>(std::cout, " "));
		std::cout << std::endl;
		std::cout << vec_t2value(res) << " vs " << vec_t2value(targets[i]) << std::endl;
	}

	// save network model & trained weights
	std::string model_name = "PJ-model4";
	remove(model_name.c_str());
	nn.save(model_name);

#endif

#if 0

	//	Mat test_image[20];
	std::vector<vec_t> images;
	std::vector<vec_t> target;

	//read CVS
	io::CSVReader<9> in("D:/Projects/tiny-dnn/x64/Debug/images/images_test.csv");
	in.read_header(io::ignore_extra_column, "file name", "Size", "SizeEstimate", "Clearness", "CheesLock", "Quality of Cheese", "External Q", "Internal Q", "Total");
	std::string file; int size; float sizeEstimate, clearness, cheeselock, cheeseQ, externalQ, internalQ, totalQ;

	double min = 0.0, max = 0.8, k = 0;
	int n = 9, i = 0;
	vec_t datan = vec_t(n, 0);

	while (in.read_row(file, size, sizeEstimate, clearness, cheeselock, cheeseQ, externalQ, internalQ, totalQ)) {
		//data.push_back(sizeEstimate);
		//data.push_back(cheeselock);
		//data.push_back(cheeseQ);

		vec_t data = vec_t(n, 0);

		for (k = max, i = n - 1; clearness < k && i > 0; i--, k -= (max - min) / (n - 1));
		data[i] = 1;
		if (i > 0) data[i - 1] = 0.5f;
		if (i < n - 1) data[i + 1] = 0.5f;

		target.push_back(data);

		cv::String file_name = "D:/Projects/tiny-dnn/x64/Debug/images/" + file;
		Mat image = imread(file_name, IMREAD_COLOR); // Read the file

		int padding = 0;
		int H = 1024, W = 768; // TODO: chage to read from image
		cv::resize(image, image, cvSize(W - 2 * padding, H - 2 * padding));

		// convert to vec for tony-cnn
		float denominator = 255;
		float min = -1.0;
		float max = 1.0;
		// As you can see, padding is applied (I am using a kernel of size 5 x 5 in the first convolutional layer). If your kernel is of size 2*k + 1 x 2*k + 1, then you should padd the image by k on each side. Further, the image is transformed to lie in [-1,1].
		vec_t sample((3 * (H + 2 * padding))*(W + 2 * padding), min);
		for (int i = 0; i < H; i++) { // Go over all rows
			for (int j = 0; j < W; j++) { // Go over all columns
				for (int c = 0; c < 3; c++) { // Go through all channels
					sample[W*H*c + W*(i + padding) + (j + padding)] = image.at<cv::Vec3b>(i, j)[c] / denominator*(max - min) + min;
				}
			}
		}
		images.push_back(sample);
	}

	network<sequential> nn;
	adagrad optimizer;

	std::string model_name = "PJ-model2";
	nn.load(model_name);

	auto loss = nn.get_loss<mse>(images, target);
	std::cout << "loss:" << loss << std::endl;

	//nn.train<mse>(optimizer, images, target, 1, 5);
	// mse, absolute, absolute_eps
	// cross_entropy = cross-entropy loss function for (multiple independent) binary classifications,
	// cross_entropy_multiclass = cross-entropy loss function for multi-class classification

	for (int i = 0; i < target.size(); i++)
	{
		vec_t res = nn.predict(images[i]);
		std::copy(target[i].begin(), target[i].end(), std::ostream_iterator<float>(std::cout, " "));
		std::cout << std::endl;
		std::copy(res.begin(), res.end(), std::ostream_iterator<float>(std::cout, " "));
		std::cout << std::endl;
	}

#endif


	cout << cin.get();
}


