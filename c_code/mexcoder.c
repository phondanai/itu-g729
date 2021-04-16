/* ITU-T G.729 Software Package Release 2 (November 2006) */
/* ITU-T G.729 - Reference C code for fixed point implementation */
/* Version 3.3    Last modified: December 26, 1995 */

/*
   ITU-T G.729 Speech Coder     ANSI-C Source Code
   Copyright (c) 1995, AT&T, France Telecom, NTT, Universite de Sherbrooke.
   All rights reserved.
*/
/*-------------------------------------------------------------------*
 * Main program of the ITU-T G.729  8 kbit/s encoder.                *
 *                                                                   *
 *    Usage : coder speech_file  bitstream_file                      *
 *-------------------------------------------------------------------*/

#include "mex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"

void mexFunction(int nlhs,
		 mxArray *plhs[],
		 int nrhs,
		 const mxArray *prhs[])
{
	int argc;
	char *argv[5];
	int buflen;
	int j;
	int ret;
	double* retd;
	char head[54];

	FILE *f_speech;               /* File of speech data                   */
	FILE *f_serial;               /* File of serial bits for transmission  */

	extern Word16 *new_speech;     /* Pointer to new speech data            */

	Word16 prm[PRM_SIZE];          /* Analysis parameters.                  */
	Word16 serial[SERIAL_SIZE];    /* Output bitstream buffer               */
	Word16 syn[L_FRAME];           /* Buffer for synthesis speech           */

	Word16 i, frame;               /* frame counter */

	mexPrintf("\n");
	mexPrintf("***********     ITU G.729 8 KBIT/S SPEECH CODER    ***********\n");
	mexPrintf("\n");
	mexPrintf("------------------- Fixed point C simulation -----------------\n");
	mexPrintf("\n");
	mexPrintf("------------ Version 3.3 (Release 2, November 2006) --------\n");
	mexPrintf("\n");


	argc = nrhs;
	for(j=0;j<argc;j++)
	{
		buflen = (mxGetN(prhs[j])+1);
		argv[j] = mxCalloc(buflen, sizeof(char));
		ret = mxGetString(prhs[j],argv[j],buflen);
		
		/* debug--
		mexPrintf("buflen=%d,ret=%d,argv[j]=%s\n",buflen,ret,argv[j]);
		*/
		
	}

	/* return value (error code)*/
	plhs[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
	retd = mxGetPr(plhs[0]);
	*retd = 0;

	/*--------------------------------------------------------------------------*
	* Open speech file and result file (output serial bit stream)              *
	*--------------------------------------------------------------------------*/

	if ( argc != 2 )
	{
		mexPrintf("Usage : coder speech_file  bitstream_file\n");
		mexPrintf("\n");
		mexPrintf("Format for speech_file:\n");
		mexPrintf("  Speech is read from a binary file of 16 bits PCM data.\n");
		mexPrintf("\n");
		mexPrintf("Format for bitstream_file:\n");
		mexPrintf("  One (2-byte) synchronization word \n");
		mexPrintf("  One (2-byte) size word,\n");
		mexPrintf("  80 words (2-byte) containing 80 bits.\n");
		mexPrintf("\n");
		*retd = 1;
		return;
	}

	if ( (f_speech = fopen(argv[0], "rb")) == NULL) {
		mexPrintf("g729enc - Error opening file  %s !!\n", argv[0]);
		*retd = 1;
		return;
	}
	mexPrintf(" Input speech file    :  %s\n", argv[0]);

	/* Coyp Wav file Header */
	if(strstr(argv[0], ".wav"))
	{
		ret = fread( head, 1, 54, f_speech );
	}


	if ( (f_serial = fopen(argv[1], "wb")) == NULL) {
		mexPrintf("g729enc - Error opening file  %s !!\n", argv[1]);
		*retd = 1;
		return;
	}
	mexPrintf(" Output bitstream file:  %s\n", argv[1]);

	/* Write Wav FIle Header */
	if(strstr(argv[1], ".wav") )
	{
		ret = fwrite( head, sizeof(char), 54, f_serial);
	}
	
	/* mxCalloc Free*/
	for( j = 0; j<argc; j++)
	{
		mxFree(argv[j]);
	}
	j=0;


	/*--------------------------------------------------------------------------*
	* Initialization of the coder.                                             *
	*--------------------------------------------------------------------------*/

	Init_Pre_Process();
	Init_Coder_ld8k();
	for(i=0; i<PRM_SIZE; i++) prm[i] = (Word16)0;

	/* To force the input and output to be time-aligned the variable SYNC
	has to be defined. Note: the test vectors were generated with this option
	disabled
	*/

#ifdef SYNC
	/* Read L_NEXT first speech data */

	fread(&new_speech[-L_NEXT], sizeof(Word16), L_NEXT, f_speech);
#ifdef HARDW
	/* set 3 LSB's to zero */
	for(i=0; i < L_NEXT; i++)
		new_speech[-L_NEXT+i] = new_speech[-L_NEXT+i] & 0xFFF8;
#endif
	Pre_Process(&new_speech[-L_NEXT], L_NEXT);
#endif

	/* Loop for each "L_FRAME" speech data. */

	frame =0;
	while( fread(new_speech, sizeof(Word16), L_FRAME, f_speech) == L_FRAME)
	{
#ifdef HARDW
		/* set 3 LSB's to zero */
		for(i=0; i < L_FRAME; i++) new_speech[i] = new_speech[i] & 0xFFF8;
#endif

		Pre_Process(new_speech, L_FRAME);

		Coder_ld8k(prm, syn);

		prm2bits_ld8k( prm, serial);

		if (fwrite(serial, sizeof(Word16), SERIAL_SIZE, f_serial) != SERIAL_SIZE)
		{
			mexPrintf("Write Error for frame %d\n", frame);
		}
		frame++;
		mexPrintf("Frame =%d\r", frame);
	}
	mexPrintf("Frame =%d\n", frame);
	fclose(f_serial);
	return ;
}
