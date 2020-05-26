#include <iostream>
#include <vector>
//Thread building blocks library
#include <tbb/task_scheduler_init.h>
#include "tbb/parallel_reduce.h"
#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>
#include <tbb/blocked_range.h>
//Free Image library
#include <FreeImagePlus.h>
//C++11 Threading
#include <thread>
//Timer
#include <chrono>


using namespace std;
using namespace std::chrono;
using namespace tbb;

// Global constants
const float PI = 3.142f;

//Prototypes
void PartOne(bool);
void PartTwo(bool);
void PartThree(bool);
void CreateBlurKernel(float***, float, unsigned int);

// Return the square of a
template <typename T>
T sqr(const T& a) {

	return a * a;
}

int main()
{
	int nt = task_scheduler_init::default_num_threads();
	task_scheduler_init T(nt);

	bool runSequential = false;
	char input;
	do {
		cout << "Run sequentially (y/n)? : ";
		cin >> input;
	} while (input != 'y' && input != 'n');
	runSequential = (input == 'y') ? true : false;
	system("CLS"); //Clear console
	

	//Part 1 (Image Comparison): -----------------DO NOT REMOVE THIS COMMENT----------------------------//

	PartOne(runSequential);

	//Part 2 (Blur & post-processing): -----------DO NOT REMOVE THIS COMMENT----------------------------//

	PartTwo(runSequential);

	//Part 3 (Image Mask): -----------------------DO NOT REMOVE THIS COMMENT----------------------------//

	PartThree(runSequential);

	system("pause");
	return 0;
}

void CreateBlurKernel(float*** blurKernel, float sigma, unsigned int kernelSize)
{
	int halfKernelSize = kernelSize / 2;

	*blurKernel = new float*[kernelSize];

	for (int y = -halfKernelSize; y <= halfKernelSize; ++y)
	{
		(*blurKernel)[y + halfKernelSize] = new float[kernelSize];

		for (int x = -halfKernelSize; x <= halfKernelSize; ++x)
		{
			(*blurKernel)[y + halfKernelSize][x + halfKernelSize] = 1.0f / (2.0f*PI*sqr(sigma)) * exp(-((sqr(x) + sqr(y)) / (2.0f*sqr(sigma))));
		}
	}
}

void PartOne(bool runSequential)
{
	cout << "-----------------------------" << endl;
	cout << "Part 1" << endl;
	cout << "-----------------------------" << endl;

	if (runSequential)
	{
		auto timer1 = high_resolution_clock::now();

		fipImage inputImages[4];
		float* buffers[4];

		//compare images
		for (unsigned int t = 0; t < 4; t++)
		{
			switch (t)
			{
			case 0:
				inputImages[t].load("../Images/render_top_1.png");
				break;
			case 1:
				inputImages[t].load("../Images/render_top_2.png");
				break;
			case 2:
				inputImages[t].load("../Images/render_bottom_1.png");
				break;
			case 3:
				inputImages[t].load("../Images/render_bottom_2.png");
				break;
			}
			inputImages[t].convertToFloat();
			buffers[t] = (float*)inputImages[t].accessPixels();
		}

		const float* const inputTopImageOneBuffer = buffers[0];
		const float* const inputTopImageTwoBuffer = buffers[1];
		const float* const inputBottomImageOneBuffer = buffers[2];
		const float* const inputBottomImageTwoBuffer = buffers[3];

		auto timer2 = high_resolution_clock::now();
		auto timerResult = duration_cast<microseconds>(timer2 - timer1);
		std::cout << "Sequential loading image operation took = " << timerResult.count() << "\n";

		auto width = inputImages[0].getWidth();
		auto height = inputImages[1].getHeight();
		uint64_t numElements = width * height;

		fipImage outputImages[3];
		float* outputBuffers[3];

		timer1 = high_resolution_clock::now();
		
		for (int i = 0; i < 3; i++)
		{
			outputImages[i] = fipImage(FIT_FLOAT, width, height, 32);
			outputBuffers[i] = (float*)outputImages[i].accessPixels();
		}

		timer2 = high_resolution_clock::now();
		timerResult = duration_cast<microseconds>(timer2 - timer1);
		std::cout << "Sequential output image container creation operation took = " << timerResult.count() << "\n";

		timer1 = high_resolution_clock::now();

		//Compare both pairs
		for (uint64_t i = 0; i < numElements; ++i)
		{
			outputBuffers[0][i] = (inputTopImageOneBuffer[i] != inputTopImageTwoBuffer[i]);
			outputBuffers[1][i] = (inputBottomImageOneBuffer[i] != inputBottomImageTwoBuffer[i]);
		}

		timer2 = high_resolution_clock::now();
		timerResult = duration_cast<microseconds>(timer2 - timer1);
		std::cout << "Sequential comparison operation took = " << timerResult.count() << "\n";

		timer1 = high_resolution_clock::now();

		//Combine images
		for (uint64_t i = 0; i < numElements; ++i)
		{
			outputBuffers[2][i] = outputBuffers[0][i] + outputBuffers[1][i];
		}

		timer2 = high_resolution_clock::now();
		timerResult = duration_cast<microseconds>(timer2 - timer1);
		std::cout << "Sequential combination operation took = " << timerResult.count() << "\n";

		timer1 = high_resolution_clock::now();

		for (int i = 0; i < 3; i++)
		{
			outputImages[i].convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
			outputImages[i].convertTo24Bits();
			switch (i)
			{
				case 0:
					outputImages[i].save("../Images/stage1_top.png");
					break;
				case 1:
					outputImages[1].save("../Images/stage1_bottom.png");
					break;
				case 2:
					outputImages[2].save("../Images/stage1_combined.png");
					break;
			}
		}

		timer2 = high_resolution_clock::now();
		timerResult = duration_cast<microseconds>(timer2 - timer1);
		std::cout << "Sequential saving operation took = " << timerResult.count() << "\n";
	}
	else
	{
		auto timer1 = high_resolution_clock::now();

		fipImage inputImages[4];
		float* buffers[4];
		vector<thread> threads;
		
		//compare images
		for (unsigned int t = 0; t < 4; t++)
		{
			threads.push_back(thread([&inputImages, &buffers, t]() {
			switch (t)
			{
				case 0:
					inputImages[t].load("../Images/render_top_1.png");
					break;
				case 1:
					inputImages[t].load("../Images/render_top_2.png");
					break;
				case 2:
					inputImages[t].load("../Images/render_bottom_1.png");
					break;
				case 3:
					inputImages[t].load("../Images/render_bottom_2.png");
					break;
			}
			inputImages[t].convertToFloat();
			buffers[t] = (float*)inputImages[t].accessPixels();}));
		}
		
		for (auto &thread : threads) { thread.join(); }
		const float* const inputTopImageOneBuffer = buffers[0];
		const float* const inputTopImageTwoBuffer = buffers[1];
		const float* const inputBottomImageOneBuffer = buffers[2];
		const float* const inputBottomImageTwoBuffer = buffers[3];

		auto timer2 = high_resolution_clock::now();
		auto timerResult = duration_cast<microseconds>(timer2 - timer1);
		std::cout << "Parallel loading image operation took = " << timerResult.count() << "\n";

		//All images have the same width and height
		auto width = inputImages[0].getWidth();
		auto height = inputImages[0].getHeight();
		uint64_t numElements = width * height;

		threads.clear();

		fipImage outputImages[3];
		float* outputBuffers[3];

		timer1 = high_resolution_clock::now();

		for (int i = 0; i < 3; i++)
		{
			threads.push_back(thread([&outputImages, &outputBuffers, i, width, height]()
			{
				outputImages[i] = fipImage(FIT_FLOAT, width, height, 32);
				outputBuffers[i] = (float*)outputImages[i].accessPixels();
			}));
		}

		for (auto &thread : threads) { thread.join(); }

		timer2 = high_resolution_clock::now();
		timerResult = duration_cast<microseconds>(timer2 - timer1);
		std::cout << "Parallel output image container creation operation took = " << timerResult.count() << "\n";

		unsigned int availableCores = std::thread::hardware_concurrency()-1;
		uint64_t sizePerThread = numElements / availableCores;
		int remainingElements = numElements % availableCores; //If the above division wasn't equal this gets count of the remaining elements
		
		threads.clear();

		timer1 = high_resolution_clock::now();

		//compare images
		for (unsigned int t = 0; t < availableCores; t++)
		{ 
			const uint64_t begin = t * sizePerThread;
			const uint64_t end = (t < availableCores - 1) ? (t + 1) * sizePerThread : (t + 1) * sizePerThread + remainingElements;
			threads.push_back(thread([&, begin, end]()
			{
				for (uint64_t i = begin; i < end; ++i)
				{
					//compare top pair
					outputBuffers[0][i] = (inputTopImageOneBuffer[i] != inputTopImageTwoBuffer[i]);
					
					//compare bottom pair
					outputBuffers[1][i] = (inputBottomImageOneBuffer[i] != inputBottomImageTwoBuffer[i]);
				}
			}));
		}

		for (auto &thread : threads) { thread.join(); }

		timer2 = high_resolution_clock::now();
		timerResult = duration_cast<microseconds>(timer2 - timer1);
		std::cout << "Parallel comparison operation took = " << timerResult.count() << "\n";
		
		timer1 = high_resolution_clock::now();

		//combine images
		for (unsigned int t = 0; t < availableCores; t++)
		{
			const uint64_t begin = t * sizePerThread;
			const uint64_t end = (t < availableCores - 1) ? (t + 1) * sizePerThread : (t + 1) * sizePerThread + remainingElements;
			threads[t] = (thread([&, begin, end]()
			{
				for (uint64_t i = begin; i < end; i++)
				{
					outputBuffers[2][i] = outputBuffers[0][i] + outputBuffers[1][i];
				}
			}));
		}
		
		for (auto &thread : threads) { thread.join(); }

		timer2 = high_resolution_clock::now();
		timerResult = duration_cast<microseconds>(timer2 - timer1);
		std::cout << "Parallel combination operation took = " << timerResult.count() << "\n";

		threads.clear();

		timer1 = high_resolution_clock::now();

		//combine images
		for (unsigned int t = 0; t < 3; t++)
		{
			threads.push_back(thread([&outputImages, t]()
			{
				outputImages[t].convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
				outputImages[t].convertTo24Bits();
				switch (t)
				{
					case 0:
						outputImages[t].save("../Images/stage1_top.png");
						break;
					case 1:
						outputImages[1].save("../Images/stage1_bottom.png");
						break;
					case 2:
						outputImages[2].save("../Images/stage1_combined.png");
						break;
				}
			}));
		}

		for (auto &thread : threads) { thread.join(); }

		timer2 = high_resolution_clock::now();
		timerResult = duration_cast<microseconds>(timer2 - timer1);
		std::cout << "Parallel saving operation took = " << timerResult.count() << "\n";
	}


	cout << "-----------------------------" << endl;
	system("pause");
	cout << "-----------------------------" << endl;
}

void PartTwo(bool runSequential)
{
	cout << "-----------------------------" << endl;
	cout << "Part 2" << endl;
	cout << "-----------------------------" << endl;

	fipImage inputStageOne;
	inputStageOne.load("../Images/stage1_combined.png");
	inputStageOne.convertToFloat();
	const float* const inputStageOneBuffer = (float*)inputStageOne.accessPixels();

	auto width = inputStageOne.getWidth();
	auto height = inputStageOne.getHeight();
	uint64_t numElements = width * height;

	fipImage outputBlurredImage = fipImage(FIT_FLOAT, width, height, 32);
	float *outputBlurredBuffer = (float*)outputBlurredImage.accessPixels();

	fipImage outputThresholdImage = fipImage(FIT_FLOAT, width, height, 32);
	float *outputThresholdBuffer = (float*)outputThresholdImage.accessPixels();

	float sigma;
	unsigned int kernelSize;

	do {
		cout << "Input sigma (positive number only): ";
		cin >> sigma;
	} while (sigma < 0);
	cout << "Sigma : " << sigma << endl;

	do {
		cout << "Input kernel size (positive odd number only): ";
		cin >> kernelSize;
	} while (kernelSize < 0 || kernelSize % 2 == 0);
	cout << "Kernel : " << kernelSize << "x" << kernelSize << endl;

	int halfKernelSize = kernelSize / 2;
	float **blurKernel;
	CreateBlurKernel(&blurKernel, sigma, kernelSize);

	auto lambdaBlur = [=](uint64_t y, uint64_t x)->void
	{
		float result = 0;
		for (int i = -halfKernelSize; i <= halfKernelSize; ++i)
			for (int a = -halfKernelSize; a <= halfKernelSize; ++a)
				result += inputStageOneBuffer[((y + i) * width) + (x + a)] * blurKernel[i + halfKernelSize][a + halfKernelSize];

		outputBlurredBuffer[(y * width) + x] = result;
	};

	if (runSequential)
	{	
		auto timer1 = high_resolution_clock::now();

		for (uint64_t y = halfKernelSize; y < height - halfKernelSize; y++)
		{
			for (uint64_t x = halfKernelSize; x < width - halfKernelSize; x++)
			{
				lambdaBlur(y, x);
			}
		}

		auto timer2 = high_resolution_clock::now();
		auto timerResult = duration_cast<microseconds>(timer2 - timer1);
		std::cout << "Sequential blur operation took = " << timerResult.count() << "\n";

		for (uint64_t i = 0; i < numElements; ++i)
		{
			outputThresholdBuffer[i] = (outputBlurredBuffer[i] > 0.0f) ? 1.0f : 0.0f;
		}

		timer2 = high_resolution_clock::now();
		timerResult = duration_cast<microseconds>(timer2 - timer1);
		std::cout << "Sequential threshold operation took = " << timerResult.count() << "\n";
	}
	else
	{
		auto timer1 = high_resolution_clock::now();

		tbb::parallel_for(blocked_range2d<uint64_t, uint64_t>
			(halfKernelSize, height- halfKernelSize, 8, halfKernelSize, width- halfKernelSize, width>>2), 
			[=](const blocked_range2d<uint64_t, uint64_t>& r) 
		{

			auto y1 = r.rows().begin();
			auto y2 = r.rows().end();
			auto x1 = r.cols().begin();
			auto x2 = r.cols().end();

			for (uint64_t y = y1; y < y2; ++y)
				for (uint64_t x = x1; x < x2; ++x)
					lambdaBlur(y, x);
		});

		auto timer2 = high_resolution_clock::now();
		auto timerResult = duration_cast<microseconds>(timer2 - timer1);
		std::cout << "Parallel blur operation took = " << timerResult.count() << "\n";

		timer1 = high_resolution_clock::now();

		//parallel threshold
		tbb::parallel_for(blocked_range<uint64_t>(0, numElements), [&](const blocked_range<uint64_t>& range) {

			for (uint64_t i = range.begin(); i < range.end(); ++i)
				outputThresholdBuffer[i] = (outputBlurredBuffer[i] > 0.0f) ? 1.0f : 0.0f;
		});

		timer2 = high_resolution_clock::now();
		timerResult = duration_cast<microseconds>(timer2 - timer1);
		std::cout << "Parallel threshold operation took = " << timerResult.count() << "\n";
	}

	outputBlurredImage.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
	outputBlurredImage.convertTo24Bits();
	outputBlurredImage.save("../Images/stage2_blurred.png");

	outputThresholdImage.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
	outputThresholdImage.convertTo24Bits();
	outputThresholdImage.save("../Images/stage2_threshold.png");


	cout << "-----------------------------" << endl;
	system("pause");
	cout << "-----------------------------" << endl;
}

void PartThree(bool runSequential)
{
	cout << "-----------------------------" << endl;
	cout << "Part 3" << endl;
	cout << "-----------------------------" << endl;
	fipImage renderTopImage;
	renderTopImage.load("../Images/render_top_1.png");
	renderTopImage.convertTo32Bits();
	const RGBQUAD* const renderTopBuffer = (RGBQUAD*)renderTopImage.accessPixels();

	fipImage stageTwoThresholdImage;
	stageTwoThresholdImage.load("../Images/stage2_threshold.png");
	stageTwoThresholdImage.convertToFloat();
	const float* const stageTwoThresholdBuffer = (float*)stageTwoThresholdImage.accessPixels();

	//All images have the same width and height
	auto width = renderTopImage.getWidth();
	auto height = renderTopImage.getHeight();
	uint64_t numElements = width * height;

	fipImage outputStageThreeImage = fipImage(FIT_BITMAP, width, height, 32);
	RGBQUAD* outputStageThreeBuffer = (RGBQUAD*)outputStageThreeImage.accessPixels();

	float whitePixelCount = 0;
	float whitePixelPercentage = 0;

	auto lambdaInvert = [=](RGBQUAD p)->RGBQUAD
	{
		p.rgbBlue = ~p.rgbBlue;
		p.rgbGreen = ~p.rgbGreen;
		p.rgbRed = ~p.rgbRed;

		return p;
	};

	if (runSequential)
	{
		auto timer1 = high_resolution_clock::now();

		for (uint64_t i = 0; i < numElements; ++i)
		{
			whitePixelCount += stageTwoThresholdBuffer[i];
		}

		whitePixelPercentage = (whitePixelCount / numElements) * 100;

		auto timer2 = high_resolution_clock::now();
		auto timerResult = duration_cast<microseconds>(timer2 - timer1);
		std::cout << "Sequential white percentage calculation operation took = " << timerResult.count() << "\n";

		cout << "White Percentage :" << whitePixelPercentage << "%" << endl;

		timer1 = high_resolution_clock::now();

		for (uint64_t i = 0; i < numElements; ++i)
		{
			outputStageThreeBuffer[i] = (stageTwoThresholdBuffer[i] > 0) ? lambdaInvert(renderTopBuffer[i]) : renderTopBuffer[i];	
		}


		timer2 = high_resolution_clock::now();
		timerResult = duration_cast<microseconds>(timer2 - timer1);
		std::cout << "Sequential invert operation took = " << timerResult.count() << "\n";

	}
	else
	{
		auto timer1 = high_resolution_clock::now();

		//parallel
		whitePixelCount = tbb::parallel_reduce(blocked_range<uint64_t>(0, numElements), 0.0f, [=](const blocked_range<uint64_t>& range, float whitePixels) {

			for (uint64_t i = range.begin(); i < range.end(); ++i)
				whitePixels += stageTwoThresholdBuffer[i];

			return whitePixels;
		}, std::plus<float>());

		whitePixelPercentage = (whitePixelCount / numElements) * 100;

		auto timer2 = high_resolution_clock::now();
		auto timerResult = duration_cast<microseconds>(timer2 - timer1);
		std::cout << "Parallel white percentage calculation operation took = " << timerResult.count() << "\n";

		cout << "White Percentage :" << whitePixelPercentage << "%" << endl;

		timer1 = high_resolution_clock::now();

		tbb::parallel_for(blocked_range2d<uint64_t, uint64_t>(0, height, 8, 0, width, width >> 2), [=](const blocked_range2d<uint64_t, uint64_t>& r) {

			auto y1 = r.rows().begin();
			auto y2 = r.rows().end();
			auto x1 = r.cols().begin();
			auto x2 = r.cols().end();

			for (uint64_t y = y1; y < y2; ++y) {

				for (uint64_t x = x1; x < x2; ++x)
				{
					uint64_t currentIndex = x + (y * width);
					outputStageThreeBuffer[currentIndex] = (stageTwoThresholdBuffer[currentIndex] > 0) ? lambdaInvert(renderTopBuffer[currentIndex]) : renderTopBuffer[currentIndex];
				}
			}
		});

		timer2 = high_resolution_clock::now();
		timerResult = duration_cast<microseconds>(timer2 - timer1);
		std::cout << "Parallel invert operation took = " << timerResult.count() << "\n";
	}

	outputStageThreeImage.save("../Images/stage3.png");

	cout << "-----------------------------" << endl;
	cout << "End..." << endl;
	cout << "-----------------------------" << endl;
}
