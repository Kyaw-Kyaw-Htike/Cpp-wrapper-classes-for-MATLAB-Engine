#ifndef MATLAB_ENG_WRAPPER_EIG_H__
#define MATLAB_ENG_WRAPPER_EIG_H__

/*

Copyright (C) 2017 Kyaw Kyaw Htike @ Ali Abdul Ghafur. All rights reserved.

Adapted from:
MatlabWrapper of
Andrej Karpathy
August 27, 2012
BSD Licence

KKH:

Lets you use connect with Matlab engine from C++ with Eigen library effortlessly

==========================
Example usage 1:
==========================

# define EGM Eigen::Matrix<unsigned int, Eigen::Dynamic, Eigen::Dynamic>
mew.exec("clear all; X = [1,2;3,4]; X = uint32(X);");
EGM X_cv; mew.receive<unsigned int>("X", X_cv);
EGM Y_cv = X_cv + X_cv;
mew.send<unsigned int>("Y", Y_cv);


Output:

» whos, X, Y
  Name      Size            Bytes  Class     Attributes

  X         2x2                16  uint32
  Y         2x2                16  uint32


X =

		   1           2
		   3           4


Y =

		   2           4
		   6           8



==========================
Example usage 2:
==========================

# define EGM Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>
mew.exec("clear all; X(:,:,1) = [1,2;3,4]; X(:,:,2) = [5,6;7,8]; X = single(X);");
vector<EGM> X_cv; mew.receive<float>("X", X_cv);
vector<EGM> Y_cv = X_cv; Y_cv[0] = X_cv[0] + X_cv[0]; Y_cv[1] = X_cv[1] + X_cv[1];
mew.send<float>("Y", Y_cv);

Output:

» whos, X,Y
  Name      Size             Bytes  Class     Attributes

  X         2x2x2               32  single
  Y         2x2x2               32  single


X(:,:,1) =

	 1     2
	 3     4


X(:,:,2) =

	 5     6
	 7     8


Y(:,:,1) =

	 2     4
	 6     8


Y(:,:,2) =

	10    12
	14    16


*/

#include <iostream>
#include <string>
#include <map>
#include <stdio.h>
#include "engine.h"
#include "typeExg_matlab_eig.h"

#define MATLAB_BUF_SIZE 1024
#define EigenMatrix Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>

class MatlabEngWrapperEig {

public:

	MatlabEngWrapperEig()
	{
		initted_ = false;
		verbose_ = true;
	}
	
	
	~MatlabEngWrapperEig() 
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
	void send(const std::string name, const EigenMatrix& matIn) 
	{
		mxArray* matOut;
		eigen2matlab(matIn, matOut);
		engPutVariable(ep_, name.c_str(), matOut);
		mxDestroyArray(matOut); // the engine takes ownership of a copy
	}

	
	template <typename T>
	void send(const std::string name, const std::vector<EigenMatrix> &matIn) 
	{
		mxArray* matOut;
		eigen2matlab(matIn, matOut);
		engPutVariable(ep_, name.c_str(), matOut);
		mxDestroyArray(matOut); // the engine takes ownership of a copy
	}

	
	template <typename T>
	void receive(const std::string name, EigenMatrix &matOut) const 
	{
		mxArray* result = NULL;
		if ((result = engGetVariable(ep_, name.c_str())) == NULL) {
			printf("variable %s is not defined! ignoring request.\n", name.c_str());
			return;
		}
		matlab2eigen(result, matOut, true);
		mxDestroyArray(result);
	}

	
	template <typename T>
	void receive(const std::string name, vector<EigenMatrix> &matOut) const 
	{
		mxArray* result = NULL;
		if ((result = engGetVariable(ep_, name.c_str())) == NULL) 
		{
			printf("variable %s is not defined! ignoring request.\n", name.c_str());
			return;
		}
		matlab2eigen(result, matOut, true);
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


#undef EigenMatrix
#undef MATLAB_BUF_SIZE


#endif