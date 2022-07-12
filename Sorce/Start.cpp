#define _CRT_SECURE_NO_WARNINGS

#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"

#include<iostream>
#include<string>
#include<fstream>


int main(int argc, char* argv[])
{
#define __GAUS(a) cv::GaussianBlur(a, a, { 5,5 },5);

	std::string
		video_path = "TestCard.mp4",
		mask_path = "templates.bmp",
		log_path = "log.log";

	for(int i = 1; i < argc; i++)
	{
		std::string str = argv[i];

		if(str == "-h" || str == "--help")
		{
			std::string str = "";
			str += "\n";
			str += "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n";
			str += "-=-=-=-=-=-=-=- HELP =-=-=-=-=-=-=-=-\n";
			str += "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n";
			str += "\nBase settings:\n";
			str += "-v (--video) \"path\" : path to video file. By default \"TestCard.mp4\".\n";
			str += "-m (--mask) \"path\" : path to mask file. By default \"templates.bmp\".\n";
			str += "-l (--log) \"path\" : path to out log file. By default \"log.log\".\n";

			std::cout << str;

			return 1;
		}

		if(str == "-v" || str == "--video")
		{
			if(i + 1 >= argc) {
				std::cout << "Err --video\n";
				return 100;
			}
			video_path = argv[i + 1];
			i++;
		}

		if(str == "-m" || str == "--mask")
		{
			if(i + 1 >= argc) {
				std::cout << "Err --mask\n";
				return 100;
			}
			mask_path = argv[i + 1];
			i++;
		}

		if(str == "-l" || str == "--log")
		{
			if(i + 1 >= argc) {
				std::cout << "Err --log\n";
				return 100;
			}
			log_path = argv[i + 1];
			i++;
		}

	}

	cv::VideoCapture cap(video_path);

	auto base_mask = cv::imread(mask_path);

	base_mask.convertTo(base_mask, CV_8UC3);

	std::ofstream log_out = std::ofstream(log_path);

	cv::Mat 
		frame,
		resized_frame,
		resized_mask;

	cv::resize(base_mask,resized_mask,cv::Size(base_mask.cols / 4, base_mask. rows / 4));

	__GAUS(resized_mask)

	cv::imshow("B", resized_mask);

	int frame_num = 1;

	while(cap.isOpened())
	{
		if(!cap.read(frame))
			break;

		frame.convertTo(frame, CV_8UC3);

		cv::resize(frame,resized_frame,cv::Size(resized_mask.cols, resized_mask.rows));

		double med_err[3]{ 0,0,0 };

		cv::imshow("t_1", resized_frame);

		__GAUS(resized_frame)

		for(int r = 0; r < resized_mask.rows; r++)
		{
			for(int c = 0; c < resized_mask.cols; c++)
			{
				for(int d = 0; d < 3; d++)
					med_err[d] += resized_mask.at<cv::Vec3b>(r, c)[d] - resized_frame.at<cv::Vec3b>(r, c)[d];
			}
		}

		for(int d = 0; d < 3; d++)
		{
			med_err[d] = med_err[d] / double(resized_mask.rows * resized_mask.cols);
			std::cout << round(med_err[d]) << " ";
		}

		auto temp = resized_frame.clone();

		for(int r = 0; r < resized_mask.rows; r++)
		{
			for(int c = 0; c < resized_mask.cols; c++)
			{
				for(int d = 0; d < 3; d++)
				{
					unsigned char& v = resized_frame.at<cv::Vec3b>(r, c)[d];
					int t = v + round(med_err[d]);
					if(t > 255)
						t = 255;
					if(t < 0)
						t = 0;
					temp.at<cv::Vec3b>(r, c)[d] = t;
				}
			}
		}

		for(int d = 0; d < 3; d++)
		{
			med_err[d] = 0;		
		}

		double med_disp = 0;

		for(int r = 0, err; r < resized_mask.rows; r++)
		{
			for(int c = 0; c < resized_mask.cols; c++)
			{
				for(int d = 0; d < 3; d++)
				{
					med_err[d] += err = resized_mask.at<cv::Vec3b>(r, c)[d] - temp.at<cv::Vec3b>(r, c)[d];
				}
			}
		}

		for(int d = 0; d < 3; d++)
		{
			med_err[d] = med_err[d] / double(resized_mask.rows * resized_mask.cols);	
			std::cout << round(med_err[d]) << " ";
		}


		for(int r = 0, err; r < resized_mask.rows; r++)
		{
			for(int c = 0; c < resized_mask.cols; c++)
			{
				for(int d = 0; d < 3; d++)
				{
					err = (resized_mask.at<cv::Vec3b>(r, c)[d] - temp.at<cv::Vec3b>(r, c)[d]) - med_err[d];
					med_disp += err * err;
				}
			}
		}

		med_disp = sqrt(med_disp / double(resized_mask.rows * resized_mask.cols * 3));
		std::cout << round(med_disp) << " ";

		if(med_disp < 30)
		{
			log_out << frame_num << " : Find\n";
			std::cout << "Find";
		}
		else if(med_disp < 80)
		{
			log_out << frame_num << " : Prob\n";
			std::cout << "Prob";
		}
		else
		{
			log_out << frame_num << " : No\n";
			std::cout << "No";
		}

		cv::imshow("t", temp);

		cv::waitKey(1);

		std::cout << "         \r";
		frame_num++;
	}

	return 0;
};
