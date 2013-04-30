
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

/*----------------------------------------------------------------- 
	filename structure.c

	list all modifications below:
	original:  10/5/1992	Hyung K. Lee   compiled and checked
-------------------------------------------------------------------*/
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "hash.h"
#include "structure.h"

#include "parameter.h" 
#include "define.h"    
#include "macro.h"

extern int g_iNoGate, g_iNoPI, g_iNoPO, g_iNoFF, g_iMaxLevel, g_iPOLevel, g_iPPOLevel, g_iMaxFanOut;
extern int *g_PrimaryIn, *g_PrimaryOut, *g_FlipFlop;
extern GATEPTR *g_net;
extern STACKTYPE g_stack1, g_stack2, *g_pEventListStack;
extern HASHPTR pArrSymbolTable[];
/*extern char *strcat(),*strcpy();*/
// extern HASHPTR FindHash(), InsertHash();
// extern void fatalerror();

#ifdef INCLUDE_HOPE
void allocateStack1And2()
{
	ALLOCATE(g_stack1.list, GATEPTR, g_iNoGate + SPAREGATES + g_iNoPO);
	ALLOCATE(g_stack2.list, GATEPTR, g_iNoGate + SPAREGATES + g_iNoPO);
	clear(g_stack1);
	clear(g_stack2);
}

/*------compute_level----------------------------------------------
	computes psuedo-levels of each gate in net structure
	inputs : none
	outputs: returns real depth of the circuit, maxlevel+1

	Remark: PI, PPI --- level 0
		PO --- maxlevel+1 (if a PO gate is added)
		PPO --- maxlevel+2 (actual flip-flops)
-------------------------------------------------------------------*/
int computeLevels() //compute_level
{
	register int i, j, iFlag = 1;
	GATEPTR pGate, pOutGate;

	for (i = 0; i < g_iNoGate; i++)
	{
		pGate = g_net[i];
		if (pGate->type == PI || pGate->type == DFF)
		{
			pGate->dpi = 0;
			push(g_stack1, g_net[i]);
			pGate->changed = pGate->inCount; //pGate->inCount == 0
		}
		else
		{
			pGate->dpi = (-1);
			pGate->changed = 0;
		}
	}

	while (TRUE)
	{
		if (iFlag == 1)
		{
			while (!is_empty(g_stack1))
			{
				pGate = pop(g_stack1);
				for (i = 0; i< pGate->outCount; i++)
				{
					pOutGate = pGate->outList[i];
					if (++pOutGate->changed == pOutGate->inCount)
					{
						pOutGate->dpi = pGate->dpi + 1;
						push(g_stack2, pOutGate);
					}
				}
			}
		}
		else if (iFlag == 2)
		{
			while (!is_empty(g_stack2))
			{
				pGate = pop(g_stack2);
				for (i = 0; i< pGate->outCount; i++)
				{
					pOutGate = pGate->outList[i];
					if (++pOutGate->changed == pOutGate->inCount)
					{
						pOutGate->dpi = pGate->dpi + 1;
						push(g_stack1, pOutGate);
					}
				}
			}
		}
		iFlag = (iFlag == 1) ? 2 : 1;
		if (is_empty(g_stack1) && is_empty(g_stack2))
		{
			break;
		} /* exit */
	}

	/* Compute maxlevel */
	g_iMaxLevel = (-1);
	for (i = 0; i < g_iNoPO; i++)
	{
		pGate = g_net[g_PrimaryOut[i]];
		if (pGate->type == PO) //New PO
		{
			if (pGate->dpi > g_iMaxLevel)
			{
				g_iMaxLevel = pGate->dpi;
				iFlag = 1;
			}
		}
		else //Old PO
		{
			if (pGate->dpi >= g_iMaxLevel)
			{
				g_iMaxLevel = pGate->dpi;
				iFlag = 2;
			}
		}
	}
	for (i = 0; i < g_iNoFF; i++) //No FF
	{
		//STOP*************************************STOP
		pGate = g_net[g_FlipFlop[i]];
		for (j = 0; j< pGate->inCount; j++)
		{
			if (pGate->inList[j]->dpi >= g_iMaxLevel)
			{
				g_iMaxLevel = pGate->inList[j]->dpi;
				iFlag = 2;
			}
		}
	}

	/* Renumber levels of POs and PPO(DFF)s */
	if (iFlag == 1)
	{
		g_iMaxLevel--; //Old PO Level !!
	}
	g_iPOLevel = g_iMaxLevel + 1; //New PO Level !!
	g_iPPOLevel = g_iMaxLevel + 2; //PPO Level !!
	for (i = 0; i < g_iNoPO; i++)
	{
		if (g_net[g_PrimaryOut[i]]->type == PO) //PO to 
		{
			g_net[g_PrimaryOut[i]]->dpi = g_iPOLevel;
		}
	}
	for (i = 0; i < g_iNoFF; i++) //NO FF
	{
		//STOP*************************************STOP
		g_net[g_FlipFlop[i]]->dpi = g_iPPOLevel;
	}

	return(g_iMaxLevel + 1); //New PO Level !!
}

/*------allocate_event_list-------------------------------------------
	allocates levelized queue for event_list
	inputs : none
	outputs: event_list
		dpi of each gate and maxlevel should be set 
--------------------------------------------------------------------*/
void allocateEventListStacks()
{
	register int i;

	ALLOCATE(g_pEventListStack, STACKTYPE, g_iMaxLevel + 2);
	for (i = 0; i < g_iMaxLevel + 2; i++)
	{
		clear(g_pEventListStack[i]); //last = 0
	}
	
	for (i = 0; i < g_iNoGate; i++)
	{
		g_pEventListStack[g_net[i]->dpi].last++;
	}
	
	for (i = 0; i < g_iMaxLevel + 2; i++)
	{
		ALLOCATE(g_pEventListStack[i].list, GATEPTR, g_pEventListStack[i].last + 1 + SIZE_OF_FUT);
		clear(g_pEventListStack[i]);
	}
}

/*------levelize------------------------------------------------------
	gates are re-numbered and sorted in the levelized order
	affected data structure: 
		net,primaryin,flip_flops,event_list,symbol_tbl
--------------------------------------------------------------------*/
void updateGateIndexByLevel() //levelize
{
	register int i,j,iNewIndex = 0;
	GATEPTR pGate;

	/* re-number gates */
	for (i = 0; i < g_iNoGate; i++)
	{
		pushGate(g_net[i]);
	}
	for (i = 0; i < g_iMaxLevel + 2; i++)
	{
		for (j = 0; j <= g_pEventListStack[i].last; j++)
		{
			pGate = g_pEventListStack[i].list[j];
			pGate->index = iNewIndex++;
		}
		clear(g_pEventListStack[i]);
	}

	/* update gate numbers */
	for (i = 0; i < g_iNoPI; i++)				/* primaryin */
	{
		g_PrimaryIn[i] = g_net[g_PrimaryIn[i]]->index;
	}
	for (i = 0; i < g_iNoPO; i++)				/* primaryout */
	{
		g_PrimaryOut[i] = g_net[g_PrimaryOut[i]]->index;
	}
	for (i = 0; i < g_iNoFF; i++)				/* flip_flops */
	{
		g_FlipFlop[i] = g_net[g_FlipFlop[i]]->index;
	}

	/*
	   for(i=0;i<nog;i++) {
		  cg=net[i];
		  cg->symbol->index=cg->index;
	   }
	*/

	/* sort gates by index */
	i = 0;
	while (i < g_iNoGate)
	{
		if (i == g_net[i]->index)
		{
			i++;
		}
		else
		{
			/* swap net[i] and net[net[i]->gid] */
			pGate = g_net[i];
			g_net[i] = g_net[pGate->index];
			g_net[pGate->index] = pGate;
		}
	}
}

#else
/*------levelize: 
	topologically sorts the circuit in the levelized order------*/
int updateGateIndexByLevel(GATEPTR *net, int n, int nopi, int nopo, int noff, GATEPTR *stack)
{
	register int i, int_nog;
	register GATEPTR *first, *last;
	register GATEPTR ele, next;
	int G_index = 0;
	int maxdpi = (-1);

	/* initialize */
	for (i = 0; i < n; i++)
		if ((ele = net[i]) != NULL)
		{
			ele->changed = ele->inCount;
			ele->dpi = (-1);
		}

	first = last = stack;	/* empty stack */

	/* Find gates with indegree=0 */
	for (i = 0; i < nopi; i++)
	{
		ele = net[g_PrimaryIn[i]];
		ele->changed = 0;
		*last++ = ele;
		g_PrimaryIn[i] = G_index++;
	}
	for (i = 0; i < noff; i++)
	{
		ele = net[g_FlipFlop[i]];
		ele->changed = 0;
		*last++ = ele;
		g_FlipFlop[i] = G_index++;
	}

	/* levelize */
	G_index = 0;
	int_nog = 0;
	while (first != last)
	{
		int_nog++;
		ele = *first++;
		ele->index = G_index++;
		if (++(ele->dpi) > maxdpi)
		{
			maxdpi = ele->dpi;
		}
		for (i = 0; i< ele->outCount; i++)
			if ((next = ele->outList[i]) != NULL)
			{
				if (next->changed > 0)
				{
					if (--(next->changed) == 0)
					{
						*last++ = next;
					}
					next->dpi = ele->dpi;
				}
			}
	}

	/* check for levelization */
	if (int_nog != n)
	{
		fprintf(stderr, "Error in circuit file.\n");
		fprintf(stderr, "Some gates are not reachable from PIs.\n");
		for (i = 0; i < n; i++)
		{
			ele = net[i];
			if (ele->changed > 0 || ele->dpi < 0)
			{
				fprintf(stderr, "*** Unreachable gate from an input:");
				fprintf(stderr, "%d'th element %s\n", ele->index, ele->hash->symbol);
			}
		}
		return(-1);
	}

	for (i = 0; i < nopo; i++)	/* primaryout */
		g_PrimaryOut[i] = net[g_PrimaryOut[i]]->index;

	/* sort gates by index */
	for (i = 0; i < n;)
	{
		if (i == net[i]->index)
		{
			i++;
		}
		else
		{
			/* swap net[i] and net[net[i]->index] */
			ele = net[i];
			net[i] = net[ele->index];
			net[ele->index] = ele;
		}
	}

	return(maxdpi + 1);
}
#endif

/*------add_PO-------------------------------------------------------
	adds a PO gate at each primary output
	should be called before levelize()

	modified by Hyung K Lee  2/15/1991
--------------------------------------------------------------------*/
int addPOGates() //add_PO
{
	//PO Gates only have 1 input from now on !!
	register int i, j;
	GATEPTR pGate, pNewPOGate, *pOutList;
	char strNewPOName[MAXSTRING];

	for (i = 0; i < g_iNoPO; i++)
	{
		pGate = g_net[g_PrimaryOut[i]];
		if ((pNewPOGate = g_net[g_iNoGate]) == NULL)
		{
			//ALWAYS Come Here !!
			ALLOCATE(pNewPOGate, GATETYPE, 1);
		}
		pNewPOGate->index = g_iNoGate;
		pNewPOGate->type = PO;
		pNewPOGate->inCount = 1;
		ALLOCATE(pNewPOGate->inList, GATEPTR, 1);
		pNewPOGate->inList[0] = pGate;
		pNewPOGate->outCount = 0;
		pNewPOGate->outList = NULL;
#ifdef LEARNFLG
		pNewPOGate->plearn = NULL;
#endif
		strcpy(strNewPOName, pGate->hash->symbol);
		strcat(strNewPOName, "_PO");
		while ((pNewPOGate->hash = FindHash(pArrSymbolTable, HASHSIZE, strNewPOName, 0)) != NULL)
		{
			//Avoid name conflict
			strcat(strNewPOName, "_PO");
		}
		if ((pNewPOGate->hash = InsertHash(pArrSymbolTable, HASHSIZE, strNewPOName, 0)) == NULL)
		{
			printFatalError(HASHERROR);
		}
		else
		{
			//ALWAYS Come Here !!
			pNewPOGate->hash->gate = pNewPOGate;
		}

		//////////////Memory Leak
		// 		pOutList = pGate->outList;
		// 		ALLOCATE(pGate->outList, GATEPTR, pGate->outCount + 1);
		// 		for (j = 0; j< pGate->outCount; j++)
		// 			pGate->outList[j] = pOutList[j];
		///////////////////////////
		GATEPTR *pTempOutList;
		pOutList = pGate->outList;
		ALLOCATE(pTempOutList, GATEPTR, pGate->outCount + 1);
		for (j = 0; j< pGate->outCount; j++)
		{
			pTempOutList[j] = pOutList[j];
		}
		free(pGate->outList); // If not, then memory leak !!
		pGate->outList = pTempOutList;
		//////////////////////////////////

		pGate->outList[pGate->outCount] = pNewPOGate;
		pGate->outCount += 1;

		g_PrimaryOut[i] = g_iNoGate;
		g_net[g_iNoGate++] = pNewPOGate;
		/*  	if(pOutList!=NULL) FREE(pOutList); */
	}

	return(g_iNoPO);
}

/*------add_sparegates-----------------------------------------------
	allocates memory spaces for the fault injection.
	Should be called after levelization.
	Do not assign symbolic names.
	Inputs: none
--------------------------------------------------------------------*/
void addSpareGates() //add_sparegates
{
	register int i;
	register GATEPTR pGate;

	/* add sparegates */
	for (i = 0; i < SIZE_OF_FUT; i++)
	{
		ALLOCATE(pGate, GATETYPE, 1);			/* CONSTANT Gate */
		pGate->index = g_iNoGate + 2 * i;
		pGate->type = DUMMY;
		pGate->inCount = 0;
		pGate->inList = NULL;
		pGate->outCount = 1;
		ALLOCATE(pGate->outList, GATEPTR, 1);
		pGate->dpi = 0;
		reset(pGate->changed);
		pGate->hash = NULL;
		pGate->Gid = 0;
		pGate->stemIndex = 0;
		g_net[pGate->index] = pGate;

		ALLOCATE(pGate, GATETYPE, 1);			/* 2-input AND, OR, XOR */
		pGate->index = g_iNoGate + 2 * i + 1;
		pGate->type = DUMMY;
		pGate->inCount = 2;
		ALLOCATE(pGate->inList, GATEPTR, 2);
		pGate->inList[0] = g_net[g_iNoGate + 2 * i];
		pGate->outCount = 0;
		ALLOCATE(pGate->outList, GATEPTR, g_iMaxFanOut);
		pGate->dpi = 0;
		reset(pGate->changed);
		pGate->Gid = 0;
		pGate->stemIndex = 0;
		pGate->hash = NULL;
		g_net[pGate->index] = pGate;
	}

	/* add one memory space for output list of POs */
	for (i = 0; i < g_iNoPO; i++)
	{
		pGate = g_net[g_PrimaryOut[i]];
		if (pGate->outCount == 0)
		{
			ALLOCATE(pGate->outList, GATEPTR, 1);
			pGate->outList[0] = (GATEPTR)NULL;
		}
	}
}

