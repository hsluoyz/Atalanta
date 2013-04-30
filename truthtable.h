
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

/*----------------------------------------------------------------------
	truthtable.h
	defines the truthtable of 1 and 2 input primitive gates
	should include atpg.h prior to include truthtable.h
----------------------------------------------------------------------*/

#ifndef __ATALANTA_TRUTH_H__
#define __ATALANTA_TRUTH_H__

/* 1 input gates: */
level g_iTruthTable1[MAXGTYPE][ATALEVEL] =
{
	/*  0		1		x		d		dbar		*/
	{	ZERO,	ONE,	X,		D,		DBAR	},	/* and */
	{	ONE,	ZERO,	X,		DBAR,	D	},		/* nand */
	{	ZERO,	ONE,	X,		D,		DBAR	},	/* or */
	{	ONE,	ZERO,	X,		DBAR,	D	},		/* nor */
	{	ZERO,	ONE,	X,		D,		DBAR	},	/* pi */
	{	ZERO,	ONE,	X,		D,		DBAR	},	/* xor */
	{	ONE,	ZERO,	X,		DBAR,	D	},		/* xnor */
	{	X,  	X,  	X,  	X,  	X   	},	/* dff */
	{	X,  	X,  	X,  	X,  	X,  	},	/* dummy */
	{	ZERO,   ONE,	X,  	D,  	DBAR	},	/* buffer */
	{	ONE,	ZERO,	X,		DBAR,	D	},		/* inverter */
	{	X,  	X,  	X,  	X,  	X,  	},	/* 11 */
	{	X,  	X,  	X,  	X,  	X,  	},	/* 12 */
	{	X,  	X,  	X,  	X,  	X,  	},	/* 13 */
	{	X,  	X,  	X,  	X,  	X,  	},	/* 14 */
	{	X,  	X,  	X,  	X,  	X,  	},	/* 15 */
	{	X,  	X,  	X,  	X,  	X,  	},	/* 16 */
	{	X,  	X,  	X,  	X,  	X,  	},	/* 17 */
	{	X,  	X,  	X,  	X,  	X,  	},	/* 18 */
	{	X,  	X,  	X,  	X,  	X,  	},	/* 19 */
	{	ZERO,   ONE,	X,  	D,  	DBAR	},	/* po */
};

// #define 			ZERO				0
// #define 			ONE 				1
// #define 			X   				2
// #define			D					3
// #define			DBAR				4

/* 2 input gates */
level g_iTruthTable2[MAXGTYPE][ATALEVEL][ATALEVEL] =
{
	/*   0		1		x		d		dbar	*/
	/* and */
	{{	ZERO,	ZERO,	ZERO,	ZERO,	ZERO	},		/* 0 */
	{	ZERO,	ONE,	X,		D,		DBAR	},		/* 1 */
	{	ZERO,	X,		X,		X,		X	},			/* x */
	{	ZERO,	D,		X,		D,		ZERO	},		/* d */
	{	ZERO,	DBAR,	X,		ZERO,	DBAR	}},		/* dbar */
	/*nand */
	{{	ONE,	ONE,	ONE,	ONE,	ONE	},			/* 0 */
	{	ONE,	ZERO,	X,	DBAR,	D	},				/* 1 */
	{	ONE,	X,	X,	X,	X	},						/* x */
	{	ONE,	DBAR,	X,	DBAR,	ONE	},				/* d */
	{	ONE,	D,	X,	ONE,	D	}},					/* dbar */
	/*or   */
	{{	ZERO,	ONE,	X,	D,	DBAR	},				/* 0 */
	{	ONE,	ONE,	ONE,	ONE,	ONE	},			/* 1 */
	{	X,	ONE,	X,	X,	X	},						/* x */
	{	D,	ONE,	X,	D,	ONE	},						/* d */
	{	DBAR,	ONE,	X,	ONE,	DBAR	}},			/* dbar */
	/*nor  */
	{{	ONE,	ZERO,	X,	DBAR,	D	},				/* 0 */
	{	ZERO,	ZERO,	ZERO,	ZERO,	ZERO	},		/* 1 */
	{	X,	ZERO,	X,	X,	X	},						/* x */
	{	DBAR,	ZERO,	X,	DBAR,	ZERO	},			/* d */
	{	D,	ZERO,	X,	ZERO,	D	}},					/* dbar */
	/*pi   */
	{{	X,	X,	X,	X,	X	},							/* 0 */
	{	X,	X,	X,	X,	X	},							/* 1 */
	{	X,	X,	X,	X,	X	},							/* x */
	{	X,	X,	X,	X,	X	},							/* d */
	{	X,	X,	X,	X,	X	}},							/* dbar */
	/*xor  */
	{{	ZERO,	ONE,	X,	D,	DBAR	},	/* 0 */
	{	ONE,	ZERO,	X,	DBAR,	D	},	/* 1 */
	{	X,	X,	X,	X,	X	},	/* x */
	{	D,	DBAR,	X,	ZERO,	ONE	},	/* d */
	{	DBAR,	D,	X,	ONE,	ZERO	}},	/* dbar */
	/*xnor */
	{{	ONE,	ZERO,	X,	DBAR,	D	},	/* 0 */
	{	ZERO,	ONE,	X,	D,	DBAR	},	/* 1 */
	{	X,	X,	X,	X,	X	},	/* x */
	{	DBAR,	D,	X,	ONE,	ZERO	},	/* d */
	{	D,	DBAR,	X,	ZERO,	ONE	}},	/* dbar */

	{{0, }, {0, }, {0, }, {0, }, {0,}}, 				/* dff */
	{{0, }, {0, }, {0, }, {0, }, {0,}}, 				/* dummy */
	{{0, }, {0, }, {0, }, {0, }, {0,}}, 				/* buffer */
	{{0, }, {0, }, {0, }, {0, }, {0,}}, 				/* not */
	{{0, }, {0, }, {0, }, {0, }, {0,}}, 				/* 11 */
	{{0, }, {0, }, {0, }, {0, }, {0,}}, 				/* 12 */
	{{0, }, {0, }, {0, }, {0, }, {0,}}, 				/* 13 */
	{{0, }, {0, }, {0, }, {0, }, {0,}}, 				/* 14 */
	{{0, }, {0, }, {0, }, {0, }, {0,}}, 				/* 15 */
	{{0, }, {0, }, {0, }, {0, }, {0,}}, 				/* 16 */
	{{0, }, {0, }, {0, }, {0, }, {0,}}, 				/* 17 */
	{{0, }, {0, }, {0, }, {0, }, {0,}}, 				/* 18 */
	{{0, }, {0, }, {0, }, {0, }, {0,}}, 				/* 19 */
	{{0, }, {0, }, {0, }, {0, }, {0,}}, 				/* po */
};

#endif /* __ATALANTA_TRUTH_H__ */
