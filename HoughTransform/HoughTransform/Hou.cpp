#include "stdafx.h"

#include "hou.h"
#include <cmath>
#include <iostream>
#include <string.h>

#define DEG2RAD 0.017453293f

namespace keymolen {

	Hou::Hou():_accu(0), _accu_w(0), _accu_h(0), _img_w(0), _img_h(0)
	{

	}

	Hou::~Hou() {
		if(_accu)
			free(_accu);
	}


	int Hou::Transform(cv::Mat& img_data, int w, int h)
	{
		_img_w = w;
		_img_h = h;

		//Create the accu
		double Hou_h = ((sqrt(2.0) * (double)(h>w?h:w)) / 2.0);
		_accu_h = Hou_h * 2.0; // -r -> +r
		_accu_w = 180;

		_accu = (unsigned int*)calloc(_accu_h * _accu_w, sizeof(unsigned int));

		double center_x = w/2;
		double center_y = h/2;
		
		for(int y=0;y<h;y++)
		{
			for(int x=0;x<w;x++)
			{
				if( img_data.row(y).col(x).data[0] > 250 )
				{
					for(int t=0;t<180;t++)
					{
						double r = ( ((double)x - center_x) * cos((double)t * DEG2RAD)) + (((double)y - center_y) * sin((double)t * DEG2RAD));
						_accu[ (int)(((int)(r + Hou_h) * 180.0)) + t]++;
						//img_data[ (y*w) + x] > 250
					}
				}
			}
		}

		return 0;
	}

	std::vector< std::pair< std::pair<int, int>, std::pair<int, int> > > Hou::GetLines(int threshold, cv::Mat& img_data)
	{
		std::vector< std::pair< std::pair<int, int>, std::pair<int, int> > > lines;

		if(_accu == 0)
			return lines;

		for(int r=0; r<_accu_h; r++)
		{
			for(int t=0; t<_accu_w; t++)
			{
				if((int)_accu[(r*_accu_w) + t] >= threshold)
				{
					//lokalne maximum 
					int max = _accu[(r*_accu_w) + t];
					for(int ly=-4;ly<=4;ly++)
					{
						for(int lx=-4;lx<=4;lx++)
						{
							if( (ly+r>=0 && ly+r<_accu_h) && (lx+t>=0 && lx+t<_accu_w)  )
							{
								if( (int)_accu[( (r+ly)*_accu_w) + (t+lx)] > max )
								{
									max = _accu[( (r+ly)*_accu_w) + (t+lx)];
									ly = lx = 5;
								}
							}
						}
					}
					if(max > (int)_accu[(r*_accu_w) + t])
						continue;


					int x1[8] = {0, 0, 0, 0, 0, 0, 0, 0}, y1[8] = {0, 0, 0, 0, 0, 0, 0, 0}, 
						x2[8] = {0, 0, 0, 0, 0, 0, 0, 0}, y2[8] = {0, 0, 0, 0, 0, 0, 0, 0};


					if(t >= 45 && t <= 135)
					{
						//y = (r - x cos(t)) / sin(t)
						x1[0] = 0;
						x2[0] = _img_w - 1;

						int pocit = 0;
						while(1)
						{
							
							y1[pocit] = ((double)(r-(_accu_h/2)) - ((x1[pocit] - (_img_w/2) ) * cos(t * DEG2RAD))) / sin(t * DEG2RAD) + (_img_h / 2);
							if (y1[pocit] < 0)
							{
								x1[pocit]++;
							} else
							{
								if (okoliey(x1, y1, &pocit, img_data) )
								{
									pocit++;
									x1[pocit] = x1[pocit - 1];
									y1[pocit] = y1[pocit - 1];
									x1[pocit]++;
									if (pocit == 3)
									{
										if  (okoliey(x1, y1, &pocit, img_data) )
										{
											break;
										}
										else
										{		
											nuluj(x1, y1, &pocit);
											pocit = 0;
										}
									}
								} else if (pocit > 0)
								{
									nuluj(x1, y1, &pocit);
									pocit = 0;
								} else
								{
									x1[pocit]++;
								}
							}
						}

						pocit = 0;
						while(1)
						{
							y2[pocit] = ((double)(r-(_accu_h/2)) - ((x2[pocit] - (_img_w/2) ) * cos(t * DEG2RAD))) / sin(t * DEG2RAD) + (_img_h / 2);
							if (y2[pocit] > img_data.rows)
							{
								x2[pocit]--;
							} else
							{
								if (okoliey(x2, y2, &pocit, img_data) )
								{
									pocit++;
									x2[pocit] = x2[pocit - 1];
									y2[pocit] = y2[pocit - 1];
									x2[pocit]--;
									if (pocit == 3)
									{
										if  (okoliey(x2, y2, &pocit, img_data) )
										{
											break;
										}
										else
										{
											nuluj(x2, y2, &pocit);
											pocit = 0;
										}
									}
								} else if (pocit > 0)
								{
									nuluj(x2, y2, &pocit);
									pocit = 0;
								} else
								{
									x2[pocit]--;
								}
							}
						}

					}
					else
					{
						//x = (r - y sin(t)) / cos(t);
						y1[0] = 0;
						y2[0] = _img_h - 1;	

						int pocit = 0;
						while(1)
						{
							x1[pocit] = ((double)(r-(_accu_h/2)) - ((y1[pocit] - (_img_h/2) ) * sin(t * DEG2RAD))) / cos(t * DEG2RAD) + (_img_w / 2);
							if (x1[pocit] < 0)
							{
								y1[pocit]++;
							} else
							{
								if ((x1[pocit] > 40) || (y1[pocit] > 20))  //640 359
										{
											int l = 2;
										}
								if (okoliex(x1, y1, &pocit, img_data) )
								{
									pocit++;
									x1[pocit] = x1[pocit - 1];
									y1[pocit] = y1[pocit - 1];
									y1[pocit]++;
									if (pocit == 3)
									{						
										if  (okoliex(x1, y1, &pocit, img_data) )
										{
											break;
										}
										else
										{
											nuluj(x1, y1, &pocit);
											pocit = 0;
										}
									}
								} else if (pocit > 0)
								{
									nuluj(x1, y1, &pocit);
									pocit = 0;
								} else
								{
									y1[pocit]++;
								}
							}
						}

						pocit = 0;
						while(1)
						{
							x2[pocit] = ((double)(r-(_accu_h/2)) - ((y2[pocit] - (_img_h/2) ) * sin(t * DEG2RAD))) / cos(t * DEG2RAD) + (_img_w / 2);
							if (x2[pocit] > img_data.cols)
							{
								y2[pocit]--;
							} else
							{
								if (okoliex(x2, y2, &pocit, img_data) )
								{
									pocit++;
									x2[pocit] = x2[pocit - 1];
									y2[pocit] = y2[pocit - 1];
									y2[pocit]--;
									if (pocit == 3)
									{
										if  (okoliex(x2, y2, &pocit, img_data) )
										{
											break;
										}
										else
										{
											nuluj(x2, y2, &pocit);
											pocit = 0;
										}
									}
								} else if (pocit > 0)
								{
									nuluj(x2, y2, &pocit);
									pocit = 0;
								} else
								{
									y2[pocit]--;
								}
							}
						}

					}

					lines.push_back(std::pair< std::pair<int, int>, std::pair<int, int> >(std::pair<int, int>(x1[1],y1[1]), std::pair<int, int>(x2[1],y2[1])));

				}
			}
	 }

		std::cout << "lines: " << lines.size() << " " << threshold << std::endl;
		return lines;
	}

	//========================================================================================================================================
	const unsigned int* Hou::GetAccu(int *w, int *h)
	{
		*w = _accu_w;
		*h = _accu_h;

		return _accu;
	}

	//========================================================================================================================================
	void Hou::nuluj(int *cix, int *ciy, int *poci)
	{
		cix[0] = cix[*poci];
		ciy[0] = ciy[*poci];
		for (int i = 1; i <= 7; i++)
		{
			cix[i] = 0;
			ciy[i] = 0;
		}
	}

	//========================================================================================================================================
	// OK ... Konecne -_-
	bool Hou::okoliex(int *cix, int *ciy, int *poci, cv::Mat& img_data)
	{
		int pocitadlo = 0;
		int	limit = 1;
		int zaciatok = -1;
		int stlpce = img_data.cols;
		for (int i = 0; i <= *poci; i++)
		{
			if (cix[i] >= (stlpce-1))
			{
				limit = -1;
			} else
			{
				limit = 1;
			}
			if (cix[i] <= 1)
			{
				zaciatok = 0;
			}else
			{
				zaciatok = -1;
			}
			for (int a = zaciatok; a <= limit; a++)
			{
				//std::cout << "M = " << std::endl << " " << img_data.row((cix[i])+a).col(ciy[i]).data[0] << std::endl << std::endl;
				if ((int)img_data.row(ciy[i]).col(cix[i]+a).data[0] >= 250)
				{
					pocitadlo++;
					i++;
					a=-1;
				}
				if (pocitadlo > *poci)
				{
					return true;
				}
			}
		}

		return false;
	}

	//========================================================================================================================================
	bool Hou::okoliey(int *cix, int *ciy, int *poci, cv::Mat& img_data)
	{
		int pocitadlo = 0;
		int	limit = 1;
		int zaciatok = -1;
		int riadky = img_data.rows;
		for (int i = 0; i <= *poci; i++)
		{
			if (ciy[i] >= (riadky-1))
			{
				limit = -1;
			} else
			{
				limit = 1;
			}
			if (ciy[i] <= 1)
			{
				zaciatok = 0;
			}else
			{
				zaciatok = -1;
			}
			for (int a = zaciatok; a <= limit; a++)
			{
				if ((int)img_data.row(ciy[i]+a).col(cix[i]).data[0] >= 250)
				{
					pocitadlo++;
					i++;
					a = -1;
				}
				if (pocitadlo > *poci)
				{
					return true;
				}
			}
		}

		return false;
	}
}

// X su stlpce
// Y su riadky