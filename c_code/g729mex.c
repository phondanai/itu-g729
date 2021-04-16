/*
G729(ACELP) 

inputs
IN1=argv[0]
'e'=encode
'd'=decode
'ed'=encode&decode

IN2=argv[1]
'inputfilename'

IN3=argv[2]
'outputfilename'



*/


#include "mex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"

extern int enc(int argc, char *argv[] );
extern int dec(int argc, char *argv[] );


#define WAV_HEAD 44
void mexFunction(int nlhs,
		 mxArray *plhs[],
		 int nrhs,
		 const mxArray *prhs[])
{
	int argc;			/* input num 	*/
	char *argv[5];		/* inputs		*/
	char *argv2[5];		/* inputs		*/
	int i,j;			/* counter		*/
	char head[54];		/* wav_head		*/
	int buflen;			/* buff length */
	int ret;
	double *retd;		/* return value	*/

	FILE *infile;		/* input 		*/
	FILE *outfile;		/* output		*/
	FILE *temp;			/* temp 		*/
	char buff[5120];	/* buff 		*/

	
	int flag = 0;
	int len = 0;
	
	argc = nrhs;
	
	for(i=0;i<argc;i++)
	{
		buflen = (mxGetN(prhs[i])+1);
		argv[i] = mxCalloc(buflen, sizeof(char));
		ret = mxGetString(prhs[i],argv[i],buflen);
		j++;
		/* debug--
		mexPrintf("buflen=%d,ret=%d,argv[i]=%s\n",buflen,ret,argv[i]);
		*/
		
	}

	/* return value (error code)*/
	plhs[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
	retd = mxGetPr(plhs[0]);
	*retd = 0;
   /* Passed arguments */

	if ( argc != 3 )
	{
		mexPrintf("Usage : mexg729('ed','input','output')\n");
		mexPrintf("\n");
		mexPrintf("Format for bitstream_file:\n");
		mexPrintf("  One (2-byte) synchronization word,\n");
		mexPrintf("  One (2-byte) size word,\n");
		mexPrintf("  80 words (2-byte) containing 80 bits.\n");
		mexPrintf("\n");
		mexPrintf("Format for outputspeech_file:\n");
		mexPrintf("  Output is written to a binary file of 16 bits data.\n");
		*retd = 1;
		return;
	}
	
	if( strcmp(argv[0], "ed") == 0)
	{
		/* enc & dec */
			
			argv2[0] = argv[1];	/*input file*/
			argv2[1] = "tempout.dat";
			
			(*retd) = enc( 2, argv2 );
			
			argv2[0] = "tempout.dat";
			argv2[1] = "tempout2.dat";
			*retd = (double) dec( 2, argv2 );
			
		if( strstr(argv[1],".wav") &&  strstr(argv[2],".wav"))
		{
			infile = fopen( argv[1], "rb" );
			fread( head, 1, WAV_HEAD, infile );
			fclose(infile);
			
			temp = fopen( "tempout2.dat", "rb" );
			outfile = fopen( argv[2], "wb");
			fwrite( head, 1, WAV_HEAD, outfile );
			i = 0;
			while( feof( temp ) == 0)
			{
				buflen = fread( buff ,1, 5120, temp);
				fwrite( buff,1,buflen,outfile);
			}
			fclose(temp);
			fclose(outfile);
			
			mexPrintf("\n----------------------------------------\n");
			mexPrintf("Input wav file    :  %s\n", argv[1]);
			mexPrintf("Output wav file    :  %s\n", argv[2]);
			mexPrintf("----------------------------------------\n");
			
			ret = remove("tempout.dat");
			ret = remove("tempout2.dat");

			return;
		}
		else
		{
			mexPrintf("\n----------------------------------------\n");
			mexPrintf("Input file    :  %s\n", argv[1]);
			mexPrintf("Output file    :  %s\n", argv[2]);
			mexPrintf("----------------------------------------\n");

			ret = remove("tempout.dat");
			ret = rename("tempout2.dat",argv[2]);
			return;
		}
			
		
	}
	else if( strcmp(argv[0], "e") == 0)
	{
		/* enc */
		*retd = (double) enc( argc-1, &(argv[1]) );
	}
	else if( strcmp(argv[0], "d") == 0)
	{
		/* dec */
		*retd = (double) dec( argc-1, &(argv[1]) );
	}
	else
	{
		mexPrintf(" 1st arg should be selected { 'e','d','ed' } \n");
		*retd = -1;
		flag = 0;
	}

	
	return ;
}
