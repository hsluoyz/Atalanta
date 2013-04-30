
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
	error.c
	Error handling of atalanta, fsim, soprano and hope

	list all modifications below:
	original:  8/15/1991	Hyung K. Lee
----------------------------------------------------------------------*/
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>

#include "error.h"

#include "parameter.h"

//extern void exit();

/* Error messages */
char messages[NUMERRORS + 1][75] =
{
	"Good status", "Unexpected end-of-file on circuit file", "Error in circuit file", "Error in dynamic memory allocation", "Error in symbol table", "Error in fault file",
};

/*------fatalerror------------------------------------------------------
	Report an error and exit
----------------------------------------------------------------------*/
void printFatalError(int errorcode)
{
	fprintf(stderr, "Fatal error:  %s\n", messages[errorcode]);
	exit(0);
}
