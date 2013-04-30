
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
 
		   Added functional fault injection: H. K. Lee, 3/15/1993
		   Added static & dynamic fault grouping: H. K. Lee, 3/15/1993
		   Changed parser: H. K. Lee, 7/31/1993
 
		atalanta: version 2.0   	 H. K. Lee, 6/30/1997
 
***********************************************************************/

/*----------------------------------------------------------------- 
	filename stemIndex.c
	This file contains all subroutines necessary for
	fanout free region and dominator analysis.
-------------------------------------------------------------------*/
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>

#include "error.h"

#include "stem.h"
#include "parameter.h"
#include "define.h"
#include "macro.h"


extern int g_iNoGate, g_iNoPI, g_iNoPO, g_iNoFF, g_iMaxLevel, g_iPOLevel, g_iPPOLevel, g_iMaxFanOut;
extern int *g_PrimaryIn, *g_PrimaryOut, *g_FlipFlop;
extern GATEPTR *g_net;
extern STACKTYPE g_stack1, g_stack2, *g_pEventListStack;
extern FAULTPTR g_pHeadFault;
// extern int strcopy();
// extern char *strcat();
// extern void fatalerror();

STEMTYPE *g_pStems;
int g_iNoRStem = 0,g_iNoStem = 0;

/*------SetFFR----------------------------------------------------
	Identifies Fanout Free Region (FFR) and g_pStems.
	Input:	none
	Output: none
-----------------------------------------------------------------*/
int initStemAndFFR() //SetFFR
{
	register int i, j;
	register GATEPTR pGate;

	//Just calculate buffer size !!
	for (i = 0; i < g_iNoGate; i++) //-->g_iNoStem
	{
		pGate = g_net[i];
		reset(pGate->changed);
		if (pGate->outCount != 1)
		{
			g_iNoStem++;
		}
		else if (pGate->outList[0]->type == DFF) //Not possible
		{
			//STOP*********************STOP
			g_iNoStem++;
		}
		pGate->stemIndex = 0;
	}
	ALLOCATE(g_pStems, STEMTYPE, g_iNoStem + 1);
	g_iNoStem = 0; //Start from 1
	
	for (i = 0; i < g_iNoFF; i++) //NO FF Here
	{
		//STOP*********************STOP
		/* PPI */
		pGate = g_net[g_FlipFlop[i]];
		if (pGate->outCount != 1)
		{
			pGate->stemIndex = ++g_iNoStem; //[stemIndex] start from 1
			g_pStems[g_iNoStem].gate = pGate->index;
			g_pStems[g_iNoStem].dominatorIndex = (-1);
		}
	}
	
	for (i = 0; i < g_iNoGate; i++) //connect [g_net] with [g_pStems]
	{
		pGate = g_net[i];
		if (pGate->type == DFF || pGate->type == PO)
		{
			continue;
		}
		if (pGate->outCount != 1) //FANOUT
		{
			pGate->stemIndex = ++g_iNoStem;
			g_pStems[g_iNoStem].gate = pGate->index;
			g_pStems[g_iNoStem].dominatorIndex = (-1);
		}
	}
	g_iNoRStem = g_iNoStem;
	//g_iNoRStem = FANOUT			g_iNoStem = FANOUT + PO
	for (i = 0; i < g_iNoPO; i++) //connect [g_PrimaryOut] with [g_pStems]
	{
		/* PO */
		pGate = g_net[g_PrimaryOut[i]];
		pGate->stemIndex = ++g_iNoStem;
		g_pStems[g_iNoStem].gate = pGate->index;
		g_pStems[g_iNoStem].dominatorIndex = (-1);
	}
	for (i = 0; i < g_iNoFF; i++) //NO FF Here
	{
		////STOP*********************STOP
		/* PPO */
		pGate = g_net[g_FlipFlop[i]]->inList[0];
		if (pGate->outCount != 1)
		{
			continue;
		}
		pGate->stemIndex = ++g_iNoStem;
		g_pStems[g_iNoStem].gate = pGate->index;
		g_pStems[g_iNoStem].dominatorIndex = (-1);
	}

	clear(g_stack1);

	/* FFR region analysis */
	//From FANOUT gates to all inputs except FANOUT  !!1
	for (i = g_iNoStem; i > 0; i--)
	{
		pGate = g_net[g_pStems[i].gate];
		for (j = 0; j< pGate->inCount; j++)
		{
			if (pGate->inList[j]->stemIndex <= 0) //stemIndex Start from 1 !!
			{
				push(g_stack1, pGate->inList[j]);
			}
		}
		while (!is_empty(g_stack1))
		{
			pGate = pop(g_stack1);
			pGate->stemIndex = (-1)*i; ///2 stems CANNOT conflict !!! 
			for (j = 0; j< pGate->inCount; j++)
			{
				if (pGate->inList[j]->stemIndex <= 0) //stemIndex Start from 1 !!
				{
					push(g_stack1, pGate->inList[j]);
				}
			}
		}
	}

	return(g_iNoStem);
}

/*------SetDominator--------------------------------------------------
	Identifies dominators of each fanout stemIndex
---------------------------------------------------------------------*/
int initStemDominators() //SetDominator
{
	int i, j, iNoEventStack, iNoDominator = 0;
	GATEPTR pGate, pDomiGate;

	for (i = g_iNoStem; i > 0; i--)
	{
		pGate = g_net[g_pStems[i].gate];
		g_pStems[i].fault[0] = g_pStems[i].fault[1] = g_pStems[i].fault[2] = NULL;
		if (i > g_iNoRStem) //FANOUT !!!
		{
			continue;
		}

		iNoEventStack = 0;
		for (j = 0; j< pGate->outCount; j++)
		{
			pushGate(pGate->outList[j]); //---------------------> g_pEventListStack
			iNoEventStack++; //iNoEventStack = pGate->outCount
			set(pGate->outList[j]->changed);
		}
		
		for (j = 0; j <= g_iPPOLevel; j++)
		{
			while (!is_empty(g_pEventListStack[j])) //g_pEventListStack[j] is empty, it's impossible to execute the code below
			{
				pGate = pop(g_pEventListStack[j]);
				reset(pGate->changed);

				if (iNoEventStack <= 0) //EXIT2-2 & EXIT3-2 !!!
				{
					continue;
				}

				if (--iNoEventStack == 0)
				{
					g_pStems[i].dominatorIndex = pGate->index;
					break; //EXIT1 !!!
				}

				//PO
				if (pGate->outCount == 0 || pGate->type == DFF)
				{
					iNoEventStack = 0; //EXIT2-1 !!!		dominatorIndex == -1
				}
				//FANOUT
				else if (pGate->outCount > 1)
				{
					if (g_pStems[pGate->stemIndex].dominatorIndex == (-1))
					{
						iNoEventStack = 0; //EXIT3-1 !!!		dominatorIndex == -1
					}
					else
					{
						pDomiGate = g_net[g_pStems[pGate->stemIndex].dominatorIndex];
						if (!pDomiGate->changed)
						{
							pushGate(pDomiGate);
							set(pDomiGate->changed);
							iNoEventStack++;
						}
					}
				}
				//NORMAL GATE
				else
				{
					if (!pGate->outList[0]->changed)
					{
						pushGate(pGate->outList[0]);
						set(pGate->outList[0]->changed);
						iNoEventStack++;
					}
				}
			}
		}
	}

	for (i = g_iNoStem; i > 0; i--)
	{
		if (g_pStems[i].dominatorIndex >= 0)
		{
			iNoDominator++;
		}
		g_pStems[i].checkup = 0;
	}

	/* Check-up criteria */
	for (i = g_iNoStem; i > 0; i--)
	{
		pGate = g_net[g_pStems[i].gate];
		if (pGate->outCount <= 1)
		{
			continue;
		}
		if (g_pStems[i].dominatorIndex > 0)
		{
			pGate = g_net[g_pStems[i].gate];
			g_pStems[i].dominatorIndex = g_pStems[ABS(g_net[g_pStems[i].dominatorIndex]->stemIndex)].gate;

			/*	 if((j=sizeSR(pGate))<100)  */
			g_pStems[i].checkup = 1;
			continue;
		}


		iNoEventStack = 0;

		for (j = 0; j< pGate->outCount; j++)
		{
			if (pGate->outList[j]->inCount == 1)
			{
				iNoEventStack = 1;
			}
		}
		if (iNoEventStack == 0)
		{
			g_pStems[i].checkup = 1;
		}
	}

	return(iNoDominator);
}

//NO USE !!!
//STOP****************************************STOP
int sizeSR(GATEPTR pStemGate)
{
	int gcount = 0,i,j;
	GATEPTR pDomGate;

	if (pStemGate->stemIndex <= 0)
	{
		return(gcount);
	}
	if (g_pStems[pStemGate->stemIndex].dominatorIndex >= 0)
	{
		gcount = 1;
		pDomGate = g_net[g_pStems[pStemGate->stemIndex].dominatorIndex];
		for (i = 0; i< pStemGate->outCount; i++)
			if (!pStemGate->outList[i]->changed)
			{
				pushGate(pStemGate->outList[i]);
				set(pStemGate->outList[i]->changed);
			}
		for (i = 0; i <= g_iPPOLevel; i++)
		{
			while (!is_empty(g_pEventListStack[i]))
			{
				pStemGate = pop(g_pEventListStack[i]);
				reset(pStemGate->changed);
				gcount++;
				if (pStemGate == pDomGate)
				{
					break;
				}
				for (j = 0; j< pStemGate->outCount; j++)
				{
					if (!pStemGate->outList[j]->changed)
					{
						pushGate(pStemGate->outList[j]);
						set(pStemGate->outList[j]->changed);
					}
				}
			}
		}
	}
	return(gcount);
}
