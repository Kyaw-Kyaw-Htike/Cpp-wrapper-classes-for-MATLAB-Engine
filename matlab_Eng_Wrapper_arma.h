#ifndef MATLAB_ENG_WRAPPER_ARMA_H__
#define MATLAB_ENG_WRAPPER_ARMA_H__

/*

Copyright (C) 2017 Kyaw Kyaw Htike @ Ali Abdul Ghafur. All rights reserved.

Adapted from:
MatlabWrapper of
Andrej Karpathy
August 27, 2012
BSD Licence

KKH:

Lets you use connect with Matlab engine from C++ with Armadillo library effortlessly

==========================
Example usage 1:
==========================

mew.exec("clear all; X = [1,2;3,4]; X = int32(X);");
arma::Mat<int> X_cv; mew.receive<int>("X", X_cv);
arma::Mat<int> Y_cv = X_cv + 2;	
mew.send<int>("Y", Y_cv); 

Output:

» whos, X, Y
  Name      Size            Bytes  Class    Attributes

  X         2x2                16  int32              
  Y         2x2                16  int32              


X =

           1           2
           3           4


Y =

           3           4
           5           6


==========================
Example usage 2:
==========================
mew.exec("clear all; X(:,:,1) = [1,2;3,4]; X(:,:,2) = [1,2;3,4]; X = single(X);");
arma::Cube<float> X_cv; mew.receive<float>("X", X_cv);
arma::Cube<float> Y_cv = X_cv + 2;
mew.send<float>("Y", Y_cv);

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

     3     4
     5     6


*/

#include <iostream>
#include <string>
#include <map>
#include <stdio.h>
#include "engine.h"
#include "typeExg_matlab_arma.h"

#define MATLAB_BUF_SIZE 1024


class MatlabEngWrapperArma {

public:

	MatlabEngWrapperArma()
	{
		initted_ = false;
		verbose_ = true;
	}


	~MatlabEngWrapperArma()
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


	template <typename T>
	void send(const std::string name, const arma::Mat<T> &matIn)
	{
		mxArray* matOut;
		arma2matlab<T>(matIn, matOut);
		engPutVariable(ep_, name.c_str(), matOut);
		mxDestroyArray(matOut); // the engine takes ownership of a copy
	}


	template <typename T>
	void send(const std::string name, const arma::Cube<T>& matIn)
	{
		mxArray* matOut;
		arma2matlab<T>(matIn, matOut);
		engPutVariable(ep_, name.c_str(), matOut);
		mxDestroyArray(matOut); // the engine takes ownership of a copy
	}


	template <typename T>
	void receive(const std::string name, arma::Mat<T> &matOut) const
	{
		mxArray* result = NULL;
		if ((result = engGetVariable(ep_, name.c_str())) == NULL) 
		{
			printf("variable %s is not defined! ignoring request.\n", name.c_str());
			return;
		}
		matlab2arma<T>(result, matOut, true);
		mxDestroyArray(result);
	}

	
	template <typename T>
	void receive(const std::string name, arma::Cube<T> &matOut) const 
	{
		mxArray* result = NULL;
		if ((result = engGetVariable(ep_, name.c_str())) == NULL) 
		{
			printf("variable %s is not defined! ignoring request.\n", name.c_str());
			return;
		}
		matlab2arma<T>(result, matOut, true);
		mxDestroyArray(result);
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