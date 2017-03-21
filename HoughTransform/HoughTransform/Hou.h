#ifndef HOU_H_
#define HOU_H_

#include <vector>

//opencv
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"

namespace keymolen {

	class Hou {
	public:
		Hou();
		virtual ~Hou();
	public:
		int Transform(cv::Mat& img_data, int w, int h);
		std::vector< std::pair< std::pair<int, int>, std::pair<int, int> > > GetLines(int threshold, cv::Mat& img_data);
		const unsigned int* GetAccu(int *w, int *h);
		void nuluj(int* x, int* y, int* poci);
		bool okoliex(int* x, int* y, int* poci, cv::Mat& img_data);
		bool okoliey(int* x, int* y, int* poci, cv::Mat& img_data);

	private:
		unsigned int* _accu;
		int _accu_w;
		int _accu_h;
		int _img_w;
		int _img_h;
	};

}

#endif /* Hou_H_ */
