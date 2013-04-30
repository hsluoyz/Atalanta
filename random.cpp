
/***********************************************************************

		Copyright (C) 1991,
		Virginia Polytechnic Institute & State University

		This program was originally written by Mr. Hyung K. Lee
		under the supervision of Dr. Dong S. Ha, in the Bradley
		Department of Electrical Engineering, VPI&SU, in 1991.

		This program is released for research use only. This program,
		or any derivative thereof, may not be reproduced nor used
		for any commercial product without the written permission
		of the authors.

		For detailed information, please contact to

		Dr. Dong S. Ha
		Bradley Department of Electrical Engineering
		Virginia Polytechnic Institute & State University
		Blacksburg, VA 24061

		Ph.: (540) 231-4942
		Fax: (540) 231-3362
		E-Mail: ha@vt.edu
		Web: http://www.ee.vt.edu/ha

		REFERENCE:
		   H. K. Lee and D. S. Ha, "On the Generation of Test Patterns
		   for Combinational Circuits," Technical Report No. 12_93,
		   Dep't of Electrical Eng., Virginia Polytechnic Institute
		   and State University.

***********************************************************************/

/**************************** HISTORY **********************************
 
		atalanta: version 1.0   	 H. K. Lee, 8/15/1991
		atalanta: version 1.1   	 H. K. Lee, 10/5/1992
		atalanta: version 2.0   	 H. K. Lee, 6/30/1997
 
***********************************************************************/

/*------------------------------------------------------------------
	file name: random.c
	This file generates pseudo-random patterns for the test
	pattern generator and the fault simulator.

	Uses random() functions for random pattern generation.
-------------------------------------------------------------------*/
#include "stdafx.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "random.h"

#include "define.h"

//#define NULL 0
#define WORDSIZE 32
#define ALL0 0
#define ALL1 (~0)

//typedef unsigned level;

// extern long random();
// extern long time();

/*	Seed
	Initializes random() with the given seed.
	If the given seed is 0, uses current time as seed.
*/ 
int Seed(int startvalue)
{
	long now;

	if (startvalue == 0)
	{
		/* Use current time as seed */
#if (_MSC_VER > 1200)
		now = (long) time((time_t*)NULL);
#else
		now = time((long*)NULL);
#endif
		srandom((int)now);
		return((int)now);
	}
	else
	{
		/* Use input as seed */
		srandom(startvalue);
		return(startvalue);
	}
}

/*	GetRandompattern:
	Generates a given # of parallel random patterns.
	Only the given number of bits are valid.
*/
void GetRandompattern(register int number, level array[], int nbit)
{
	register int i, mask;

	if (nbit == WORDSIZE)
	{
		for (i = 0; i < number; i++)
			array[i] = (level)random();
	}
	else
	{
		mask = ~(ALL1 << nbit);
		for (i = 0; i < number; i++)
			array[i] = (level)random() & mask;
	}
}

/*	GetPRandompattern
	Generates 32 parallel random patterns.
*/
void GetPRandompattern(register int number, level array[])
{
	register int i;

	for (i = 0; i < number; i++)
		array[i] = (level)random();
}
