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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "typedef.h"
#include "basic_op.h"
#include "ld8k.h"

#define WAV_HEAD 44

int enc(int argc, char *argv[] )
{
	
	char head[54];
	int ret;
	
	FILE *f_speech;               /* File of speech data                   */
	FILE *f_serial;               /* File of serial bits for transmission  */

	extern Word16 *new_speech;     /* Pointer to new speech data            */

	Word16 prm[PRM_SIZE];          /* Analysis parameters.                  */
	Word16 serial[SERIAL_SIZE];    /* Output bitstream buffer               */
	Word16 syn[L_FRAME];           /* Buffer for synthesis speech           */

	Word16 i, frame;               /* frame counter */

	printf("\n");
	printf("***********     ITU G.729 8 KBIT/S SPEECH CODER    ***********\n");
	printf("\n");
	printf("------------------- Fixed point C simulation -----------------\n");
	printf("\n");
	printf("------------ Version 3.3 (Release 2, November 2006) --------\n");
	printf("\n");


	/*--------------------------------------------------------------------------*
	* Open speech file and result file (output serial bit stream)              *
	*--------------------------------------------------------------------------*/

	if ( argc != 2 )
	{
		printf("Usage : coder speech_file  bitstream_file\n");
		printf("\n");
		printf("Format for speech_file:\n");
		printf("  Speech is read from a binary file of 16 bits PCM data.\n");
		printf("\n");
		printf("Format for bitstream_file:\n");
		printf("  One (2-byte) synchronization word \n");
		printf("  One (2-byte) size word,\n");
		printf("  80 words (2-byte) containing 80 bits.\n");
		printf("\n");
		return -1;
	}

	if ( (f_speech = fopen(argv[0], "rb")) == NULL)
	{
		printf("coder - Error opening file  %s !!\n", argv[0]);
		return -1;
	}
	printf(" Input speech file    :  %s\n", argv[0]);

	if ( (f_serial = fopen(argv[1], "wb")) == NULL)
	{
		printf("coder - Error opening file  %s !!\n", argv[1]);
		return -1;
	}
	printf(" Output bitstream file:  %s\n", argv[1]);

	if( strstr(argv[0], ".wav")!=NULL)
	{
		ret = fread( head, 1, WAV_HEAD, f_speech );
	}

	/*--------------------------------------------------------------------------*
	* Initialization of the coder.                                             *
	*--------------------------------------------------------------------------*/

	Init_Pre_Process();
	Init_Coder_ld8k();
	for(i=0; i<PRM_SIZE; i++)
	{
		prm[i] = (Word16)0;
	}

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
	{
		new_speech[-L_NEXT+i] = new_speech[-L_NEXT+i] & 0xFFF8;
	}
#endif
	Pre_Process(&new_speech[-L_NEXT], L_NEXT);
#endif

	/* Loop for each "L_FRAME" speech data. */

	frame =0;

	while( fread(new_speech, sizeof(Word16), L_FRAME, f_speech) == L_FRAME)
	{

#ifdef HARDW
/* set 3 LSB's to zero */
		for(i=0; i < L_FRAME; i++)
		{
			new_speech[i] = new_speech[i] & 0xFFF8;
		}
#endif

		Pre_Process(new_speech, L_FRAME);

		Coder_ld8k(prm, syn);

		prm2bits_ld8k( prm, serial);

		if (fwrite(serial, sizeof(Word16), SERIAL_SIZE, f_serial) != SERIAL_SIZE)
		{
			printf("Write Error for frame %d\n", frame);
		}
		frame++;
		printf("Frame =%d\r", frame);
	}
	printf("Frame =%d\n", frame);
	fclose(f_speech);
	fclose(f_serial);
	return (0);
}
