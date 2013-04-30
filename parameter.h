
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
		parameter.h
		Defines parameters
----------------------------------------------------------------------*/

#ifndef __ATALANTA_PARAM_H__
#define __ATALANTA_PARAM_H__

#define MAXGATE 		2000000   /* number of gate */
#define MAXPI   		100000    /* numebr of primary inputs */
#define MAXPO   		200000    /* number of primary outputs */
#define MAXFAULT		4000000   /* number of faults */
#define MAXFIN  		150 	 /* number of fanin lines */
#define	MAXFOUT		500	/* maximum number of fanout lines */

#define MAXSTRING   	2048	 /* maximum size of a string */
#define HASHSIZE		299999   /* symbol table size, prime */
#define SPAREGATES  	100 	/* should be larger than SIZE_OF_FUC*2 */
#define MAXINTEGER  	999999
#define	INFINITY	999999

/* For ISCAS85 circuits */
#define	MAXLINE		12000	/* size of a line */

/* For ATALANTA only */
#define	MAXBUF		5000	/* number of buffers for backtrace */
#define	MAXOBJ		10000
#define	MAXTREE		4000
#define	MAXTEST		10000
#define SHUFFLES	30
#define TWO		2
#define STOP		-1	

#define LEARNFLG	1

/* For HOPE only */
#define SIZE_OF_FUT 	32  	/* number of faults under test in one pass */
#define WORDSIZE		32  	/* number of bits in a word */

/* error messages */
#define NUMERRORS   	10  	/* number of error message */
#define NOERROR 		0
#define EOFERROR		1
#define CIRCUITERROR	2
#define MEMORYERROR 	3
#define HASHERROR   	4
#define FAULTERROR  	5

#endif /* __ATALANTA_PARAM_H__ */
