#ifndef MATLAB_ENG_WRAPPER_H__
#define MATLAB_ENG_WRAPPER_H__

/*

Copyright (C) 2017 Kyaw Kyaw Htike @ Ali Abdul Ghafur. All rights reserved.

Adapted from:
MatlabWrapper of
Andrej Karpathy
August 27, 2012
BSD Licence

KKH: 

The difference is that this one just keeps it at mxArray so that I can use any of my existing type converters that I've written for mex files for this purpose also.

Lets you use Matlab from C++ effortlessly
Example usage:
mxArray* X = created with something;
// create wrapper and visualize tiny 3x4 random matrix
MatlabWrapper mw;
mw.send("X", X);
mw.exec("imshow(X, [])", true); //true indicates that we want matlab to pause for keypress
// enter interactive session! Enter 'qq' to return 
mw.interact();
// receive matrix X back and print it to make sure it's the same
mxArray *output = created with something;
mw.receive("X", output);
*/


#include <iostream>
#include <string>
#include <map>
#include <stdio.h>
#include "engine.h"

#define MATLAB_BUF_SIZE 1024


class MatlabEngWrapper {
  
public:
  
    MatlabEngWrapper()
	{
		initted_ = false; 
		verbose_ = true;
	}
	
	
	~MatlabEngWrapper() 
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
	

	void send(const std::string name, mxArray* &X) 
	{  
		engPutVariable(ep_, name.c_str(), X);
		mxDestroyArray(X); // the engine takes ownership of a copy
	}
	

	void receive(const std::string name, mxArray* &result) const 
	{  
		result = NULL;
		if ((result = engGetVariable(ep_, name.c_str())) == NULL) 
		{
			printf("Variable %s is not defined! Ignoring request.\n", name.c_str());
			return;
		}  
		// Note: I need to call mxDestroyArray(result) after I use result!
	}
	

	mxArray* receive(const std::string name) const 
	{
		mxArray* result = NULL;
		if ((result = engGetVariable(ep_, name.c_str())) == NULL) 
		{
			printf("Variable %s is not defined! Ignoring request.\n", name.c_str());		
		}
		return result;
		// Note: I need to call mxDestroyArray(result) after I use result!
	}
	

	void exec(const std::string cmd, const bool waitForKey = false) const 
	{
		engEvalString(ep_, cmd.c_str());
		if(waitForKey) 
		{
			printf("Hit return to continue\n"); 
			fgetc(stdin);
		}
	}
	

	void interact() 
	{
	  
		printf("Started interactive MATLAB session! Enter 'qq' to exit.\n");
		char str[MATLAB_BUF_SIZE + 1];
		while(true) 
		{
			printf(">> ");
			fgets(str, MATLAB_BUF_SIZE, stdin);
			if(strcmp(str, "qq\n") == 0) break;

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