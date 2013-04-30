
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
	testability.c
	Assign controllibility and observability.

	Uses depth of the gate as parameter.
-----------------------------------------------------------------------*/
#include "stdafx.h"
#include <stdio.h>

#include "testability.h"

#include "parameter.h" 
#include "define.h"    
#include "macro.h"

extern GATEPTR *g_net;

#define max(a, b) ((a) > (b) ? (a) : (b))

void setGateTestability(int iNoGate) //set_testability
{
	register int i, j, iDepth;

	/* cont0 and cont1 */
	//cont0 & cont1 small is better!!
	for (i = 0; i < iNoGate; i++)
	{
		if (is_free(g_net[i]) || is_head(g_net[i]))
		{
			g_net[i]->cont0 = 0;
		}
		else
		{
			iDepth = (-1);
			for (j = 0; j< g_net[i]->inCount; j++)
			{
				iDepth = max(iDepth, g_net[i]->inList[j]->cont0);
			}
			g_net[i]->cont0 = iDepth + 1; //cont0 = max{pGate->inList[j]->cont0} + 1
		}
		g_net[i]->cont1 = g_net[i]->cont0;
	}

	/* iDepth from output */
	//dpo small is better!!
	for (i = iNoGate - 1; i >= 0; i--)
	{
		if (g_net[i]->type == PO)
		{
			g_net[i]->dpo = 0;
		}
		else
		{
			iDepth = (-1);
			for (j = 0; j< g_net[i]->outCount; j++)
			{
				iDepth = max(iDepth, g_net[i]->outList[j]->dpo);
			}
			g_net[i]->dpo = iDepth + 1;  //dpo = max{pGate->outList[j]->dpo} + 1
		}
	}
}
