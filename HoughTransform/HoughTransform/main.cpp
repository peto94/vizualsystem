// Haf.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <string>
#include <map>
#include <iostream>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"


#include "Hou.h"

std::string img_path;
int threshold = 0;

const char* CW_IMG_ORIGINAL = "Result";
const char* CW_IMG_EDGE 	= "Canny Edge Detection";
const char* CW_ACCUMULATOR  = "Accumulator";

void doTransform(std::string, int threshold);


void usage(char * s)
{

	fprintf( stderr, "\n");
    fprintf( stderr, "%s -s <source file> [-t <threshold>] - hough transform. build: %s-%s \n", s, __DATE__, __TIME__);
	fprintf( stderr, "   s: path image file\n");
	fprintf( stderr, "   t: hough threshold\n");
	fprintf( stderr, "\nexample: ./hough -s ./img/hangover-0232.jpg -t 80\n");
	fprintf( stderr, "\n");
}

int main(int argc, char** argv) {

	img_path = "Cary.png";
	
	if(img_path.empty())
	{
		usage(argv[0]);
		return -1;
	}

    cv::namedWindow(CW_IMG_ORIGINAL, cv::WINDOW_AUTOSIZE);
    cv::namedWindow(CW_IMG_EDGE, 	 cv::WINDOW_AUTOSIZE);  

    cvMoveWindow(CW_IMG_ORIGINAL, 10, 10);
    cvMoveWindow(CW_IMG_EDGE, 680, 10);

    doTransform(img_path, threshold);

	return 0;
}



void doTransform(std::string file_path, int threshold)
{
	cv::Mat img_edge;
	cv::Mat img_blur;

	cv::Mat img_ori = cv::imread( file_path, 1 );
	cv::blur( img_ori, img_blur, cv::Size(5,5) );
	cv::Canny(img_blur, img_edge, 100, 150, 3);

	int w = img_edge.cols;
	int h = img_edge.rows;


	keymolen::Hou Hou;
	Hou.Transform(img_edge, w, h);

	threshold = 65;

	while(1)
	{	
		cv::Mat img_res = img_ori.clone();

		std::vector< std::pair< std::pair<int, int>, std::pair<int, int> > > lines = Hou.GetLines(threshold, img_edge);

		std::vector< std::pair< std::pair<int, int>, std::pair<int, int> > >::iterator it;
		for(it=lines.begin();it!=lines.end();it++)
		{
			cv::line(img_res, cv::Point(it->first.first, it->first.second), cv::Point(it->second.first, it->second.second), cv::Scalar( 0, 0, 255), 2, 8);
			cv::circle(img_res, cv::Point(it->first.first, it->first.second), 2, cv::Scalar( 0, 0, 255), 2);
			cv::circle(img_res, cv::Point(it->second.first, it->second.second), 2, cv::Scalar( 0, 0, 255), 2);
		}


		cv::imshow(CW_IMG_ORIGINAL, img_res);
		cv::imshow(CW_IMG_EDGE, img_edge);


		char c = cv::waitKey(360000);
		if(c == '+')
			threshold += 5;
		if(c == '-')
			threshold -= 5;
		if(c == 27)
			break;
		if ( threshold <= 40)
			threshold = 40;
	}
}