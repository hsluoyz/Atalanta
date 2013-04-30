
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
 
		atalanta: version 2.0   	 H. K. Lee, 6/30/1997

		   Linked HOPE with ATALANTA, H. K. Lee, 6/30/1997
 
***********************************************************************/

/*-----------------------------------------------------------------
	filename truth.c

	defines truth table of each gates
	four level logic values {0, 1, X, Z}

-----------------------------------------------------------------*/
#include "stdafx.h"
#include "define.h"

level g_truthTable1[MAXGTYPE][MAXLEVEL] =
{
	/*		0	1	X	Z */
	{	ZERO,	ONE,	X,	X	},	/* and */
	{	ONE,	ZERO,	X,	X	},	/* nand */
	{	ZERO,	ONE,	X,	X	},	/* or */
	{	ONE,	ZERO,	X,	X	},	/* nor */
	{	ZERO,	ONE,	X,	Z	},	/* pi */
	{	ZERO,	ONE,	X,	X	},	/* xor */
	{	ZERO,	ONE,	X,	X	},	/* xnor */
	{	ZERO,	ONE,	X,	X	},	/* dff */
	{	ZERO,	ONE,	X,	Z	},	/* dummy */
	{	ZERO,	ONE,	X,	X	},	/* buf */
	{	ONE,	ZERO,	X,	X	},	/* not */
	{	X,	X,	X,	X	},	/* 11 */
	{	X,	X,	X,	X	},	/* 12 */
	{	X,	X,	X,	X	},	/* 13 */
	{	X,	X,	X,	X	},	/* 14 */
	{	X,	X,	X,	X	},	/* 15 */
	{	X,	X,	X,	X	},	/* 16 */
	{	X,	X,	X,	X	},	/* 17 */
	{	X,	X,	X,	X	},	/* 18 */
	{	X,	X,	X,	X	},	/* 19 */
	{	ZERO,	ONE,	X,	Z	},	/* po */
};

/* 2 input gates
   usage:
   For NAND, NOR ---> truthtbl2[fn][truthtbl1[fn][in1]][in2].
   Else either above or
		 ---> truthtbl2[fn][in1][in2].
   For tristate gates, in1=D and in2=Enable Signal.
   logic Z is considered as X for all gates except
   tristate elements and bus, PO.   */

level g_truthTable2[MAXGTYPE][MAXLEVEL][MAXLEVEL] =
{
	/*	{	0	1	x	z	}	*/
	/* and */
	{{	ZERO,	ZERO,	ZERO,	ZERO	},	/* 0 */
	{	ZERO,	ONE,	X,	X	},	/* 1 */
	{	ZERO,	X,	X,	X	},	/* x */
	{	ZERO,	X,	X,	X	}},	/* z */
	/* nand */
	{{	ONE,	ZERO,	X,	X	},	/* 0 */
	{	ONE,	ONE,	ONE,	ONE	},	/* 1 */
	{	ONE,	X,	X,	X	},	/* x */
	{	ONE,	X,	X,	X	}},	/* z */
	/* or */
	{{	ZERO,	ONE,	X,	X	},	/* 0 */
	{	ONE,	ONE,	ONE,	ONE	},	/* 1 */
	{	X,	ONE,	X,	X	},	/* x */
	{	X,	ONE,	X,	X	}},	/* z */
	/* nor */
	{{	ZERO,	ZERO,	ZERO,	ZERO	},	/* 0 */
	{	ONE,	ZERO,	X,	X	},	/* 1 */
	{	X,	ZERO,	X,	X	},	/* x */
	{	X,	ZERO,	X,	X	}},	/* z */

	{{0, }, {0, }, {0, }, {0, }},			/* pi */
	/* xor */
	{{	ZERO,	ONE,	X,	X	},	/* 0 */
	{	ONE,	ZERO,	X,	X	},	/* 1 */
	{	X,	X,	X,	X	},	/* x */
	{	X,	X,	X,	X	}},	/* z */
	/* xnor */
	{{	ONE,	ZERO,	X,	X	},	/* 0 */
	{	ZERO,	ONE,	X,	X	},	/* 1 */
	{	X,	X,	X,	X	},	/* x */
	{	X,	X,	X,	X	}},	/* z */

	{{0, }, {0, }, {0, }, {0, }},			/* dff */
	{{0, }, {0, }, {0, }, {0, }},			/* dummy */
	{{0, }, {0, }, {0, }, {0, }},			/* buffer */
	{{0, }, {0, }, {0, }, {0, }},			/* not */
	{{0, }, {0, }, {0, }, {0, }},			/* 11 */
	{{0, }, {0, }, {0, }, {0, }},			/* 12 */
	{{0, }, {0, }, {0, }, {0, }},			/* 13 */
	{{0, }, {0, }, {0, }, {0, }},			/* 14 */
	{{0, }, {0, }, {0, }, {0, }},			/* 15 */
	{{0, }, {0, }, {0, }, {0, }},			/* 16 */
	{{0, }, {0, }, {0, }, {0, }},			/* 17 */
	{{0, }, {0, }, {0, }, {0, }},			/* 18 */
	{{0, }, {0, }, {0, }, {0, }},			/* 19 */
	/* po */
	{{	ZERO,	X,	X,	ZERO	},	/* 0 */
	{	X,	ONE,	X,	ONE	},	/* 1 */
	{	X,	X,	X,	X	},	/* x */
	{	ZERO,	ONE,	X,	Z	}},	/* z */
};
