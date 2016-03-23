// plugin.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "IPlugin.h"

#include <math.h>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>//to use cout
//#include <algorithm>
//#include <exception>


using namespace std;


////////////////////////////////////////////////////////////////////////
// A concrete plugin implementation
////////////////////////////////////////////////////////////////////////

// Photo-Reactor Plugin class

//****************************************************************************
//This code has been generated by the Mediachance photo reactor Code generator.


#define AddParameter(N,S,V,M1,M2,T,D) {strcpy (pParameters[N].m_sLabel,S);pParameters[N].m_dValue = V;pParameters[N].m_dMin = M1;pParameters[N].m_dMax = M2;pParameters[N].m_nType = T;pParameters[N].m_dSpecialValue = D;}

#define GetValue(N) (pParameters[N].m_dValue)
#define GetValueY(N) (pParameters[N].m_dSpecialValue)

#define SetValue(N,V) {pParameters[N].m_dValue = V;}

#define GetBOOLValue(N) ((BOOL)(pParameters[N].m_dValue==pParameters[N].m_dMax))

// if it is not defined, then here it is
//#define RGB(r,g,b) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

#define PARAM_ITERATIONS 0
#define PARAM_LAMBDA 1

#define NUMBER_OF_USER_PARAMS 2

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


class Plugin1 : public IPlugin	
{
public:

		//Plugin Icon:
	//you can add your own icon by creating 160x100 png file, naming it the same as plugin dll and then placing it in the plugins folder
	//otherwise a generic icon will be use


	//this is the title of the box in workspace. it should be short
	const char* GetTitle () const
	{
		return "Total Variation";
	}
	
	// this will appear in the help pane, you can put your credits and short info
	const char* GetDescription () const
	{
		return "Total Variation is a PDE (Partial Differential Equation) for noise removal and edge preserving smoothing.  It works as an iterative filter.  It is based on the principle that signals with excessive and possibly spurious detail have high total variation, the integral of the absolute gradient of the signal is high.";
	}

	// BASIC PARAMETERS
	// number of inputs 0,1 or 2
	int GetInputNumber ()
	{
		return 1;
	}

	// number of outputs 0 or 1
	int GetOutputNumber ()
	{
		return 1;
	}

	int GetBoxColor ()
	{
		return RGB(44,78,119);
	}

	int GetTextColor ()
	{
		return RGB(165,236,255);
	}

	// width of the box in the workspace
	// valid are between 50 and 100
	int GetBoxWidth ()
	{
		return 100;
	}

	// set the flags
	// see the interface builder
	// ex: nFlag = FLAG_FAST_PROCESS | FLAG_HELPER;

	//FLAG_NONE same as zero	Default, no other flags set
	//FLAG_UPDATE_IMMEDIATELY	It is very fast process that can update immediately. When user turns the sliders on UI the left display will update
	//							Use Update Immediately only for fast and single loop processes, for example Desaturate, Levels.
	//FLAG_HELPER				It is an helper object. Helper objects will remain visible in Devices and they can react to mouse messages. Example: Knob, Monitor, Bridge Pin
	//FLAG_BINDING				Binding object, attach to other objects and can change its binding value. It never goes to Process_Data functions.  Example: Knob, Switch, Slider
	//FLAG_DUMMY				It is only for interface but never process any data. Never goes to Process_Data functions. Example: Text note
	//FLAG_SKIPFINAL			Process data only during designing, doesn't process during final export. Example: Monitor, Vectorscope 
	//FLAG_LONGPROCESS			Process that takes > 1s to finish. Long Process will display the Progress dialog and will prevent user from changing values during the process.
	//FLAG_NEEDSIZEDATA		    Process need to know size of original image, the zoom and what part of image is visible in the preview. When set the plugin will receive SetSizeData
	//FLAG_NEEDMOUSE			Process will receive Mouse respond data from the workplace. This is only if your object is interactive, for example Knob, Slider

	int GetFlags ()
	{
		// it is fast process
		//int nFlag = FLAG_UPDATE_IMMEDIATELY;
		
		int nFlag = FLAG_LONGPROCESS;

		return nFlag;
	}


	// User Interface Build
	// there is maximum 29 Parameters

	int GetUIParameters (UIParameters* pParameters)
	{

		// label, value, min, max, type_of_control, special_value
		// use the UI builder in the software to generate this

		
		AddParameter( PARAM_ITERATIONS ,"Iterations", 30.0, 1.0, 100.0, TYPE_SLIDER, 0.0);
		AddParameter( PARAM_LAMBDA ,"Lambda", 1.0, 0.01, 1.0, TYPE_SLIDER, 0.0);//default  min   max

		return NUMBER_OF_USER_PARAMS;
	}
	


	// Actual processing function for 1 veloc
	//***************************************************************************************************
	// Both buffers are the same size
	// don't change the IN buffer or things will go bad for other objects in random fashion
	// the pBGRA_out comes already with pre-copied data from pBGRA_in
	// Note: Don't assume the nWidth and nHeight will be every run the same or that it contains the whole image!!!! 
	// This function receives buffer of the actual preview (it can be just a crop of image when zoomed in) and during the final calculation of the full buffer
	// this is where the image processing happens
	virtual void Process_Data (BYTE* pBGRA_out,BYTE* pBGRA_in, int nWidth, int nHeight, UIParameters* pParameters)
	{
		//List of Parameters
		int iterations = GetValue(PARAM_ITERATIONS);
		double nlambda = GetValue(PARAM_LAMBDA);
		
		int imagesize = nWidth * nHeight * 4;
		float colorspace = 255.0;

		int heightminus = nHeight - 1;
		int widthminus = nWidth - 1;

		double lambda = (1.0f + .01f) - nlambda;//invert the Lambda function where 1 is maximal strength and .01 is minimal

		float* iterativeArray=new float[nWidth * nHeight * 4];
		memset( iterativeArray, 0, sizeof(imagesize) );

		//place our image in a seperate array in the 0-1 range, where 0 is darkest and 1 is lightest
		for (int x = 0; x < nWidth; x++)
		{
			for (int y = 0; y < nHeight; y++)
			{
				int nIdx = x * 4 + y * 4 * nWidth;

				//Image0[y][x] = Image [y][x] = my_image -> GetPixelIndex (x, height - y - 1);

				float red = (float)pBGRA_in[nIdx + CHANNEL_R] / colorspace;
				float green = (float)pBGRA_in[nIdx + CHANNEL_G] / colorspace;
				float blue = (float)pBGRA_in[nIdx + CHANNEL_B] / colorspace;

				iterativeArray[nIdx + CHANNEL_R] = red;
				iterativeArray[nIdx + CHANNEL_G] = green;
				iterativeArray[nIdx + CHANNEL_B] = blue;	
			}//end x
		}//end y



		//This is our Total Variation routine
		for(int t=0; t<iterations; t++) 
		{			
			float betamaxR = 0;
			float betamaxG = 0;
			float betamaxB = 0;

			//horizontalGradient = ((x + 1 , y) - (x - 1 , y)) / 2;
			float* horizontalGradient=new float[nWidth * nHeight * 4];
			memset( horizontalGradient, 0, sizeof(imagesize) );

			//verticalGradient = ((x , y + 1) - (x , y - 1)) / 2;
			float* verticalGradient=new float[nWidth * nHeight * 4];
			memset( verticalGradient, 0, sizeof(imagesize) );

			//ng = horizontalGradient*horizontalGradient + verticalGradient*verticalGradient;
			float* ng=new float[nWidth * nHeight * 4];
			memset( ng, 0, sizeof(imagesize) );

			//horizontalGradientOffset = (x + 1 , y) + (x - 1 , y) - 2 * (x , y);
			float* horizontalGradientOffset=new float[nWidth * nHeight * 4];
			memset( horizontalGradientOffset, 0, sizeof(imagesize) );

			//verticalGradientOffset = (x , y + 1) + (x , y - 1) - 2 * (x , y);
			float* verticalGradientOffset=new float[nWidth * nHeight * 4];
			memset( verticalGradientOffset, 0, sizeof(imagesize) );

			//combineGradient = 0.25f*(Inn + Ipp - Ipn - Inp);
			float* combineGradient=new float[nWidth * nHeight * 4];
			memset( combineGradient, 0, sizeof(imagesize) );

			//const float iee = (horizontalGradient * horizontalGradient * verticalGradientOffset + verticalGradient * verticalGradient * horizontalGradientOffset - 2 * horizontalGradient * verticalGradient * combineGradient) / (ng * ng);
			float* iee=new float[nWidth * nHeight * 4];
			memset( iee, 0, sizeof(imagesize) );

			//const float beta = iee / (0.1f + ng);
			float* beta=new float[nWidth * nHeight * 4];
			memset( beta, 0, sizeof(imagesize) );

			//horizontalGradient = ((x + 1 , y) - (x - 1 , y)) / 2;
			for (int y = 0; y < nHeight; y++)
			{
				for(int x = 0; x < nWidth; x++)//for (int x = 1; x < widthminus; x++)
				{
					int xplus  = x + 1;
					int xminus = x - 1;
					int yplus  = y + 1;
					int yminus = y - 1;

					//prevent filter from exceeding the edges
					if (xminus < 1) xminus = 1;
					if (xplus > widthminus) xplus = widthminus;
					if (yplus > heightminus) yplus = heightminus;
					if (yminus < 1) yminus = 1;
					//prevent filter from exceeding the edges

					int nIdx1 = x        * 4 + y * 4 * nWidth;
					int nIdx2 = (xplus)  * 4 + y * 4 * nWidth;
					int nIdx3 = (xminus) * 4 + y * 4 * nWidth;

					horizontalGradient[nIdx1 + CHANNEL_R] = (iterativeArray[nIdx2 + CHANNEL_R] - iterativeArray[nIdx3 + CHANNEL_R]) / 2.0f;
					horizontalGradient[nIdx1 + CHANNEL_G] = (iterativeArray[nIdx2 + CHANNEL_G] - iterativeArray[nIdx3 + CHANNEL_G]) / 2.0f;
					horizontalGradient[nIdx1 + CHANNEL_B] = (iterativeArray[nIdx2 + CHANNEL_B] - iterativeArray[nIdx3 + CHANNEL_B]) / 2.0f;
				}
			}
		
			//verticalGradient = ((x , y + 1) - (x , y - 1)) / 2;
			for (int y = 0; y < nHeight; y++)//for(int y = 1; y < heightminus; y++)
			{
				for(int x = 0; x < nWidth; x++)
				{
					int xplus  = x + 1;
					int xminus = x - 1;
					int yplus  = y + 1;
					int yminus = y - 1;

					//prevent filter from exceeding the edges
					if (xminus < 1) xminus = 1;
					if (xplus > widthminus) xplus = widthminus;
					if (yplus > heightminus) yplus = heightminus;
					if (yminus < 1) yminus = 1;
					//prevent filter from exceeding the edges

					int nIdx1 = x * 4 + y        * 4 * nWidth;
					int nIdx2 = x * 4 + (yplus)  * 4 * nWidth;// int nIdx2 = x * 4 + (yplus) * 4 * nWidth;//
					int nIdx3 = x * 4 + (yminus) * 4 * nWidth;// int nIdx3 = x * 4 + (yminus) * 4 * nWidth;//

					verticalGradient[nIdx1 + CHANNEL_R] = (iterativeArray[nIdx2 + CHANNEL_R] - iterativeArray[nIdx3 + CHANNEL_R]) / 2.0f;
					verticalGradient[nIdx1 + CHANNEL_G] = (iterativeArray[nIdx2 + CHANNEL_G] - iterativeArray[nIdx3 + CHANNEL_G]) / 2.0f;
					verticalGradient[nIdx1 + CHANNEL_B] = (iterativeArray[nIdx2 + CHANNEL_B] - iterativeArray[nIdx3 + CHANNEL_B]) / 2.0f;
				}
			}
			
			//ng = horizontalGradient*horizontalGradient + verticalGradient*verticalGradient;
			for (int x = 0; x < nWidth; x++)
			{
				for (int y = 0; y < nHeight; y++)
				{
					int nIdx1 = x * 4 + y * 4 * nWidth;

					ng[nIdx1 + CHANNEL_R] = (sqrtf(1e-10f + horizontalGradient[nIdx1 + CHANNEL_R] * horizontalGradient[nIdx1 + CHANNEL_R] + verticalGradient[nIdx1 + CHANNEL_R] * verticalGradient[nIdx1 + CHANNEL_R]));
					ng[nIdx1 + CHANNEL_G] = (sqrtf(1e-10f + horizontalGradient[nIdx1 + CHANNEL_G] * horizontalGradient[nIdx1 + CHANNEL_G] + verticalGradient[nIdx1 + CHANNEL_G] * verticalGradient[nIdx1 + CHANNEL_G]));
					ng[nIdx1 + CHANNEL_B] = (sqrtf(1e-10f + horizontalGradient[nIdx1 + CHANNEL_B] * horizontalGradient[nIdx1 + CHANNEL_B] + verticalGradient[nIdx1 + CHANNEL_B] * verticalGradient[nIdx1 + CHANNEL_B]));
				}
			}

			//horizontalGradientOffset = (x + 1 , y) + (x - 1 , y) - 2 * (x , y);
			for (int y = 0; y < nHeight; y++)
			{
				for(int x = 0; x < nWidth; x++)//for (int x = 1; x < widthminus; x++)
				{
					int xplus  = x + 1;
					int xminus = x - 1;
					int yplus  = y + 1;
					int yminus = y - 1;

					//prevent filter from exceeding the edges
					if (xminus < 1) xminus = 1;
					if (xplus > widthminus) xplus = widthminus;
					if (yplus > heightminus) yplus = heightminus;
					if (yminus < 1) yminus = 1;
					//prevent filter from exceeding the edges

					int nIdx1 = x        * 4 + y * 4 * nWidth;
					int nIdx2 = (xplus)  * 4 + y * 4 * nWidth;
					int nIdx3 = (xminus) * 4 + y * 4 * nWidth;

					horizontalGradientOffset[nIdx1 + CHANNEL_R] = iterativeArray[nIdx2 + CHANNEL_R] + iterativeArray[nIdx3 + CHANNEL_R] - (2.0f * iterativeArray[nIdx1 + CHANNEL_R]);
					horizontalGradientOffset[nIdx1 + CHANNEL_G] = iterativeArray[nIdx2 + CHANNEL_G] + iterativeArray[nIdx3 + CHANNEL_G] - (2.0f * iterativeArray[nIdx1 + CHANNEL_G]);
					horizontalGradientOffset[nIdx1 + CHANNEL_B] = iterativeArray[nIdx2 + CHANNEL_B] + iterativeArray[nIdx3 + CHANNEL_B] - (2.0f * iterativeArray[nIdx1 + CHANNEL_B]);
				}
			}

			//verticalGradientOffset = (x , y + 1) + (x , y - 1) - 2 * (x , y);
			for(int y = 0; y < nHeight; y++)//for (int y = 1; y < heightminus; y++)
			{
				for (int x = 0; x < nWidth; x++)
				{
					int xplus = x + 1;
					int xminus = x - 1;
					int yplus = y + 1;
					int yminus = y - 1;

					//prevent filter from exceeding the edges
					if (xminus < 1) xminus = 1;
					if (xplus > widthminus) xplus = widthminus;
					if (yplus > heightminus) yplus = heightminus;
					if (yminus < 1) yminus = 1;
					//prevent filter from exceeding the edges

					int nIdx1 = x * 4 + y        * 4 * nWidth;
					int nIdx2 = x * 4 + (yplus)  * 4 * nWidth;
					int nIdx3 = x * 4 + (yminus) * 4 * nWidth;

					verticalGradientOffset[nIdx1 + CHANNEL_R] = (iterativeArray[nIdx2 + CHANNEL_R] + iterativeArray[nIdx3 + CHANNEL_R]) - (2.0f * iterativeArray[nIdx1 + CHANNEL_R]);
					verticalGradientOffset[nIdx1 + CHANNEL_G] = (iterativeArray[nIdx2 + CHANNEL_G] + iterativeArray[nIdx3 + CHANNEL_G]) - (2.0f * iterativeArray[nIdx1 + CHANNEL_G]);
					verticalGradientOffset[nIdx1 + CHANNEL_B] = (iterativeArray[nIdx2 + CHANNEL_B] + iterativeArray[nIdx3 + CHANNEL_B]) - (2.0f * iterativeArray[nIdx1 + CHANNEL_B]);
				}
			}

			//combineGradient = 0.25f*((x + 1 , y + 1) + (x - 1 , y - 1) - (x - 1 , y + 1) - (x + 1 , y - 1));
			for(int y = 0; y < nHeight; y++)//for (int y = 1; y < heightminus; y++)
			{
				for(int x = 0; x < nWidth; x++)//for (int x = 1; x < widthminus; x++)
				{
					int xplus  = x + 1;
					int xminus = x - 1;
					int yplus  = y + 1;
					int yminus = y - 1;

					//prevent filter from exceeding the edges
					if (xminus < 1) xminus = 1;
					if (xplus > widthminus) xplus = widthminus;
					if (yplus > heightminus) yplus = heightminus;
					if (yminus < 1) yminus = 1;
					//prevent filter from exceeding the edges

					int nIdx1 = x * 4 + y * 4 * nWidth;
					int nIdx2 = (xplus)  * 4 + (yplus)  * 4 * nWidth;
					int nIdx3 = (xminus) * 4 + (yminus) * 4 * nWidth;
					int nIdx4 = (xminus) * 4 + (yplus)  * 4 * nWidth;
					int nIdx5 = (xplus)  * 4 + (yminus) * 4 * nWidth;

					combineGradient[nIdx1 + CHANNEL_R] = 0.25f * (iterativeArray[nIdx2 + CHANNEL_R] + iterativeArray[nIdx3 + CHANNEL_R] - iterativeArray[nIdx4 + CHANNEL_R] - iterativeArray[nIdx5 + CHANNEL_R] );
					combineGradient[nIdx1 + CHANNEL_G] = 0.25f * (iterativeArray[nIdx2 + CHANNEL_G] + iterativeArray[nIdx3 + CHANNEL_G] - iterativeArray[nIdx4 + CHANNEL_G] - iterativeArray[nIdx5 + CHANNEL_G] );
					combineGradient[nIdx1 + CHANNEL_B] = 0.25f * (iterativeArray[nIdx2 + CHANNEL_B] + iterativeArray[nIdx3 + CHANNEL_B] - iterativeArray[nIdx4 + CHANNEL_B] - iterativeArray[nIdx5 + CHANNEL_B] );
				}
			}

			//iee = (horizontalGradient*horizontalGradient*verticalGradientOffset + verticalGradient*verticalGradient*horizontalGradientOffset - 2*horizontalGradient*verticalGradient*combineGradient)/(ng*ng)
			for(int x = 0; x < nWidth; x++)//for(int x = 1; x < widthminus; x++)
			{
				for(int y = 0; y < nHeight; y++)//for(int y = 1; y < heightminus; y++)
				{
					int xplus  = x + 1;
					int xminus = x - 1;
					int yplus  = y + 1;
					int yminus = y - 1;

					//prevent filter from exceeding the edges
					if (xminus < 1) xminus = 1;
					if (yminus < 1) yminus = 1;
					if (xplus > widthminus) xplus = widthminus;
					if (yplus > heightminus) yplus = heightminus;
					//prevent filter from exceeding the edges

					int nIdx1 = x * 4 + y * 4 * nWidth;

					int nIdx2 = (xplus)  * 4 + (yplus)  * 4 * nWidth;
					int nIdx3 = (xminus) * 4 + (yminus) * 4 * nWidth;
					int nIdx4 = (xminus) * 4 + (yplus)  * 4 * nWidth;
					int nIdx5 = (xplus)  * 4 + (yminus) * 4 * nWidth;



					//verticalGradientOffset = (x , y + 1) + (x , y - 1) - 2 * (x , y);
					float IyyR = verticalGradientOffset[nIdx1 + CHANNEL_R]; // verticalGradientOffset
					float IyyG = verticalGradientOffset[nIdx1 + CHANNEL_G]; // verticalGradientOffset
					float IyyB = verticalGradientOffset[nIdx1 + CHANNEL_B]; // verticalGradientOffset

					//verticalGradient = ((x , y + 1) - (x , y - 1)) / 2;
					float IyR = verticalGradient[nIdx1 + CHANNEL_R]; // verticalGradient
					float IyG = verticalGradient[nIdx1 + CHANNEL_G]; // verticalGradient
					float IyB = verticalGradient[nIdx1 + CHANNEL_B]; // verticalGradient					

					//horizontalGradientOffset = (x + 1 , y) + (x - 1 , y) - 2 * (x , y);
					float IxxR = horizontalGradientOffset[nIdx1 + CHANNEL_R]; // horizontalGradientOffset
					float IxxG = horizontalGradientOffset[nIdx1 + CHANNEL_G]; // horizontalGradientOffset
					float IxxB = horizontalGradientOffset[nIdx1 + CHANNEL_B]; // horizontalGradientOffset

					//horizontalGradient = ((x + 1 , y) - (x - 1 , y)) / 2;
					float IxR = horizontalGradient[nIdx1 + CHANNEL_R]; // horizontalGradient
					float IxG = horizontalGradient[nIdx1 + CHANNEL_G]; // horizontalGradient
					float IxB = horizontalGradient[nIdx1 + CHANNEL_B]; // horizontalGradient





					float IxyR = combineGradient[nIdx1 + CHANNEL_R]; // combineGradient
					float IxyG = combineGradient[nIdx1 + CHANNEL_G]; // combineGradient
					float IxyB = combineGradient[nIdx1 + CHANNEL_B]; // combineGradient

					float ngR = ng[nIdx1 + CHANNEL_R]; // ng
					float ngG = ng[nIdx1 + CHANNEL_G]; // ng
					float ngB = ng[nIdx1 + CHANNEL_B]; // ng

					iee[nIdx1 + CHANNEL_R] = ((IxR * IxR * IyyR) + (IyR * IyR * IxxR) - (2.0f * IxR * IyR * IxyR)) / (ngR * ngR);
					iee[nIdx1 + CHANNEL_G] = ((IxG * IxG * IyyG) + (IyG * IyG * IxxG) - (2.0f * IxG * IyG * IxyG)) / (ngG * ngG);
					iee[nIdx1 + CHANNEL_B] = ((IxB * IxB * IyyB) + (IyB * IyB * IxxB) - (2.0f * IxB * IyB * IxyB)) / (ngB * ngB);
				}
			}

			delete [] horizontalGradient;
			delete [] verticalGradient;
			delete [] combineGradient;
			delete [] verticalGradientOffset;
			delete [] horizontalGradientOffset;


			//beta = iee / (0.1f + ng);
			for (int x = 0; x < nWidth; x++)
			{
				for (int y = 0; y < nHeight; y++)
				{
					int nIdx1 = x * 4 + y * 4 * nWidth;

					beta[nIdx1 + CHANNEL_R] = iee[nIdx1 + CHANNEL_R] / (lambda + ng[nIdx1 + CHANNEL_R]);
					beta[nIdx1 + CHANNEL_G] = iee[nIdx1 + CHANNEL_G] / (lambda + ng[nIdx1 + CHANNEL_G]);
					beta[nIdx1 + CHANNEL_B] = iee[nIdx1 + CHANNEL_B] / (lambda + ng[nIdx1 + CHANNEL_B]);

					iterativeArray[nIdx1 + CHANNEL_R] += beta[nIdx1 + CHANNEL_R] * 0.01f;
					iterativeArray[nIdx1 + CHANNEL_G] += beta[nIdx1 + CHANNEL_G] * 0.01f;
					iterativeArray[nIdx1 + CHANNEL_B] += beta[nIdx1 + CHANNEL_B] * 0.01f;
				}
			}

			delete [] ng;
			delete [] iee;
			delete [] beta;

		}//end iterations


#pragma region



		
		

		for (int x = 0; x < nWidth; x++)
		{
			for (int y = 0; y < nHeight; y++)
			{
					int nIdx = x * 4 + y * 4 * nWidth;

					float nR = iterativeArray [nIdx + CHANNEL_R] * colorspace;
					float nG = iterativeArray [nIdx + CHANNEL_G] * colorspace;
					float nB = iterativeArray [nIdx + CHANNEL_B] * colorspace;

					pBGRA_out[nIdx + CHANNEL_R] = CLAMP255(nR);
					pBGRA_out[nIdx + CHANNEL_G] = CLAMP255(nG);
					pBGRA_out[nIdx + CHANNEL_B] = CLAMP255(nB);
			}
		}

#pragma endregion
		delete [] iterativeArray;


	}//end routine




	// actual processing function for 2 inputs
	//********************************************************************************
	// all buffers are the same size
	// don't change the IN buffers or things will go bad
	// the pBGRA_out comes already with copied data from pBGRA_in1
	virtual void Process_Data2 (BYTE* pBGRA_out, BYTE* pBGRA_in1, BYTE* pBGRA_in2, int nWidth, int nHeight, UIParameters* pParameters)
	{

	}


	//*****************Drawing functions for the BOX *********************************
	//how is the drawing handled
	//DRAW_AUTOMATICALLY	the main program will fully take care of this and draw a box, title, socket and thumbnail
	//DRAW_SIMPLE_A			will draw a box, title and sockets and call CustomDraw
	//DRAW_SIMPLE_B			will draw a box and sockets and call CustomDraw
	//DRAW_SOCKETSONLY      will call CustomDraw and then draw sockets on top of it
	
	// highlighting rectangle around is always drawn except for DRAW_SOCKETSONLY

	virtual int GetDrawingType ()
	{

		int nType = DRAW_AUTOMATICALLY;

		return nType;

	}


	// Custom Drawing
	// custom drawing function called when drawing type is different than DRAW_AUTOMATICALLY
	// it is not always in real pixels but scaled depending on where it is drawn
	// the scale could be from 1.0 to > 1.0
	// so you always multiply the position, sizes, font size, line width with the scale
	
	virtual void CustomDraw (HDC hDC, int nX,int nY, int nWidth, int nHeight, float scale, BOOL bIsHighlighted, UIParameters* pParameters)
	{
	}


	//************ Optional Functions *****************************************************************************************
	// those functions are not necessary for normal effect, they are mostly for special effects and objects


	// Called when FLAG_HELPER set. 
	// When UI data changed (user turned knob) this function will be called as soon as user finish channging the data
	// You will get the latest parameters and also which parameter changed
	// Normally for effects you don't have to do anything here because you will get the same parameters in the process function
	// It is only for helper objects that may not go to Process Data 
	BOOL UIParametersChanged (UIParameters* pParameters, int nParameter)
	{
		
		return FALSE;
	}

	// when button is pressed on UI, this function will be called with the parameter and sub button (for multi button line)
	BOOL UIButtonPushed (int nParam, int nSubButton, UIParameters* pParameters)
	{

		return TRUE;
	}


	// Called when FLAG_NEEDSIZEDATA set
	// Called before each calculation (Process_Data)
	// If your process depends on a position on a frame you may need the data to correctly display it because Process_Data receives only a preview crop
	// Most normal effects don't depend on the position in frame so you don't need the data
	// Example: drawing a circle at a certain position requires to know what is displayed in preview or the circle will be at the same size and position regardless of zoom
	
	// Note: Even if you need position but you don't want to mess with the crop data, just ignore it and pretend the Process_Data are always of full image (they are not). 
	// In worst case this affects only preview when using zoom. The full process image always sends the whole data

	// nOriginalW, nOriginalH - the size of the original - full image. If user sets Resize on veloc - this will be the resized image
	// nPreviewW, nPreviewH   - this is the currently processed preview width/height - it is the same that Process_Data will receive
	//                        - in full process the nPreviewW, nPreviewH is equal nOriginalW, nOriginalH
	// Crop X1,Y1,X2,Y2       - relative coordinates of preview crop rectangle in <0...1>, for full process they are 0,0,1,1 (full rectangle)	
	// dZoom                  - Zoom of the Preview, for full process the dZoom = 1.0
	void SetSizeData(int nOriginalW, int nOriginalH, int nPreviewW, int nPreviewH, double dCropX1, double dCropY1, double dCropX2, double dCropY2, double dZoom)
	{

		// so if you need the position and zoom, this is the place to get it.
		// Note: because of IBM wisdom the internal bitmaps are on PC always upside down, but the coordinates are not


	}


	// ***** Mouse handling on workplace *************************** 
	// only if FLAG_NEEDMOUSE is set
	//****************************************************************
	//this is for special objects that need to receive mouse, like a knob or slider on workplace
	// normally you use this for FLAG_BINDING objects

	// in coordinates relative to top, left corner of the object (0,0)
	virtual BOOL MouseButtonDown (int nX, int nY, int nWidth, int nHeight, UIParameters* pParameters)
	{
		
		// return FALSE if not handled
		// return TRUE if handled
		return FALSE;
	}

	// in coordinates relative to top, left corner of the object (0,0)
	virtual BOOL MouseMove (int nX, int nY, int nWidth, int nHeight, UIParameters* pParameters)
	{
	

		return FALSE;
	}
	
	// in coordinates relative to top, left corner of the object (0,0)
	virtual BOOL MouseButtonUp (int nX, int nY, int nWidth, int nHeight, UIParameters* pParameters)
	{
		
		// Note: if we changed data and need to recalculate the flow we need to return TRUE

		// return FALSE if not handled
		// return TRUE if handled
		
		return TRUE;
	}


};

extern "C"
{
	// Plugin factory function
	__declspec(dllexport) IPlugin* Create_Plugin ()
	{
		//allocate a new object and return it
		return new Plugin1 ();
	}
	
	// Plugin cleanup function
	__declspec(dllexport) void Release_Plugin (IPlugin* p_plugin)
	{
		//we allocated in the factory with new, delete the passed object
		delete p_plugin;
	}
	
}


// this is the name that will appear in the object library
extern "C" __declspec(dllexport) char* GetPluginName()
{
	return "Andy's Total Variation";	
}


// This MUST be unique string for each plugin so we can save the data

extern "C" __declspec(dllexport) char* GetPluginID()
{
	
	
	return "com.lumafilters.totalvariation";
	
}


// category of plugin, for now the EFFECT go to top library box, everything else goes to the middle library box
extern "C" __declspec(dllexport) int GetCategory()
{
		
	return CATEGORY_EFFECT;
	
}
