#ifndef MATLAB_ENG_WRAPPER_OPENCV_H__
#define MATLAB_ENG_WRAPPER_OPENCV_H__

/*

Copyright (C) 2017 Kyaw Kyaw Htike @ Ali Abdul Ghafur. All rights reserved.

Adapted from:
MatlabWrapper of
Andrej Karpathy
August 27, 2012
BSD Licence

KKH:

Lets you use connect with Matlab engine from C++ with opencv effortlessly

==========================
Example usage 1:
==========================

mew.exec("img = imread('D:/Research/Datasets/INRIAPerson_Piotr/Test/images/set01/V000/I00000.png');");
cv::Mat m = mew.receive<unsigned char, 3>("img");
mew.send<unsigned char, 3>("img2", m);
mew.exec("img3 = imgaussfilt(img2,8);");
cv::Mat m3 = mew.receive<unsigned char, 3>("img3");	
cv::cvtColor(m, m, CV_RGB2BGR);
cv::cvtColor(m3, m3, CV_RGB2BGR);
imshow_qt(m, ui.label);
imshow_qt(m3, ui.label_4);

==========================
Example usage 2:
==========================

mew.exec("clear all; X = [1,2;3,4]; X = uint16(X);");
cv::Mat X_cv = mew.receive<unsigned short, 1>("X");
cv::Mat Y_cv = X_cv + 2;	
mew.send<unsigned short, 1>("Y", Y_cv);

Output:

» whos, X, Y
  Name      Size            Bytes  Class     Attributes

  X         2x2                 8  uint16              
  Y         2x2                 8  uint16              


X =

      1      2
      3      4


Y =

      3      4
      5      6
	  

==========================
Example usage 3:
==========================

mew.exec("clear all; X(:,:,1) = [1,2;3,4]; X(:,:,2) = [1,2;3,4]; X = single(X);");
cv::Mat X_cv = mew.receive<float, 2>("X");
cv::Mat Y_cv = X_cv + 2;	
mew.send<float, 2>("Y", Y_cv);

Output:

» whos, X, Y
  Name      Size             Bytes  Class     Attributes

  X         2x2x2               32  single              
  Y         2x2x2               32  single              


X(:,:,1) =

     1     2
     3     4


X(:,:,2) =

     1     2
     3     4


Y(:,:,1) =

     3     4
     5     6


Y(:,:,2) =

     1     2
     3     4

*/

#include <iostream>
#include <string>
#include <map>
#include <stdio.h>
#include "engine.h"
#include "typeExg_matlab_opencv.h"

#define MATLAB_BUF_SIZE 1024


class MatlabEngWrapperOpencv {

public:


	MatlabEngWrapperOpencv()
	{
		initted_ = false;
		verbose_ = true;
	}


	~MatlabEngWrapperOpencv() 
	{
		engClose(ep_); // close down the engine
	}


	bool init()
	{
		if (!(ep_ = engOpen("")))
		{
			fprintf(stderr, "\nCan't start MATLAB engine.\n");
			return false;
		}

		// connect Matlab engine to a char buffer for output
		buffer_[MATLAB_BUF_SIZE - 1] = '\0';
		engOutputBuffer(ep_, buffer_, MATLAB_BUF_SIZE - 1);

		initted_ = true;
		return true;
	}


	template <typename T, int nchannels>
	void send(const std::string name, const cv::Mat mcv)
	{
		mxArray* mm;
		opencv2matlab<T, nchannels>(mcv, mm);
		engPutVariable(ep_, name.c_str(), mm);
		mxDestroyArray(mm); // the engine takes ownership of a copy
	}


	template <typename T, int nchannels>
	void receive(const std::string name, cv::Mat &mcv) const
	{
		result = NULL;
		if ((result = enggetvariable(ep_, name.c_str())) == NULL)
		{
			printf("variable %s is not defined! ignoring request.\n", name.c_str());
			return;
		}

		matlab2opencv<T, nchannels>(result, mcv);
		mxDestroyArray(result);
	}


	template <typename T, int nchannels>
	cv::Mat receive(const std::string name) const
	{
		mxArray* result = NULL;
		if ((result = engGetVariable(ep_, name.c_str())) == NULL)
		{
			printf("Variable %s is not defined! Ignoring request.\n", name.c_str());
			return cv::Mat();
		}
		cv::Mat mcv;
		matlab2opencv<T, nchannels>(result, mcv);
		mxDestroyArray(result);
		return mcv;
	}


	void exec(const std::string cmd, const bool waitForKey = false) const
	{
		engEvalString(ep_, cmd.c_str());
		if (waitForKey)
		{
			printf("Hit return to continue\n");
			fgetc(stdin);
		}
	}


	void interact()
	{
		printf("Started interactive MATLAB session! Enter 'qq' to exit.\n");
		char str[MATLAB_BUF_SIZE + 1];
		while (true)
		{
			printf(">> ");
			fgets(str, MATLAB_BUF_SIZE, stdin);
			if (strcmp(str, "qq\n") == 0) break;

			engEvalString(ep_, str);
			printf("%s\n", buffer_);
		}
	}
	

	bool verbose_;
	

protected:

	Engine *ep_;
	bool initted_;
	char buffer_[MATLAB_BUF_SIZE]; // output buffer for matlab to talk back to us
};

#undef MATLAB_BUF_SIZE

#endif