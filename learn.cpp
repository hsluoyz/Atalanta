
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
 
		atalanta: version 1.1   	 H. K. Lee, 10/5/1992
		atalanta: version 2.0   	 H. K. Lee, 6/30/1997
 
***********************************************************************/

/*-----------------------------------------------------------------
	filename learn.c

	Static Learning procedure
-----------------------------------------------------------------*/
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>

#include "learn.h"
#include "error.h"

#include "parameter.h" 
#include "define.h"    
#include "macro.h"

/* #define	DEBUGLEARN	1 */

#define	FAIL	0
#define	PASS	1
#define	PASS1	2

extern GATEPTR *g_net;
// extern void learn_node(), store_learn();
// extern void initgood(), fatalerror();
extern level g_iTruthTable1[MAXGTYPE][ATALEVEL];
extern level g_iTruthTable2[MAXGTYPE][ATALEVEL][ATALEVEL];
extern STACKTYPE g_stack, *g_pEventListStack;
extern int mac_i;

int SNODE;
level SVAL;
struct EDEN *impo;
int g_iLid = 0;

#ifdef DEBUGLEARN
int norecord = 0, noimpo = 0, learnmemory = 0;
double slearntime_beg, ulearntime_beg, learntime_beg;
double slearntime_end, ulearntime_end, learntime_end;

char *level2str[4] =
{
	"0", "1", "x", "z"
};

#endif

/* macros for the gate evaluation using the truthtable */
#define gate_eval1(pGate, iValue, iGateType, i) \
	if (pGate->inCount == 1) \
		iValue = g_iTruthTable1[pGate->type][pGate->inList[0]->output]; \
	else if (pGate->inCount == 2) \
	   iValue = g_iTruthTable2[pGate->type][pGate->inList[0]->output][pGate->inList[1]->output];\
	else if(pGate->inCount == 0) \
		iValue = pGate->output; \
	else \
	{ \
	   iGateType = (pGate->type == NAND) ? AND : \
		   (pGate->type == NOR) ? OR : pGate->type; \
	   iValue = g_iTruthTable2[iGateType][pGate->inList[0]->output][pGate->inList[1]->output]; \
	   for (i = 2; i < pGate->inCount; i++) \
		  iValue = g_iTruthTable2[iGateType][iValue][pGate->inList[i]->output]; \
	   iValue = (pGate->type == NAND||pGate->type == NOR)? A_NOT(iValue) : iValue; \
	}

/*------leval---------------------------------------------------
	Evaluate good circuit in forward and backward completely.
	Set changed flag if the gate is evaluated permanently.
	Push the evaluated gate to stack.
	Schedule next events.
---------------------------------------------------------------*/
status leval(register GATEPTR pGate)
{
	register int i, j;
	register level iValue, v1;
	int num_x;
	logic iGateType;
	GATEPTR *pInListGates;

	/* forward pGate evaluation */
	reset(pGate->changed);
	pInListGates = pGate->inList;

	/* if a line is a head line, stop */
	if (is_free(pGate))
	{
		return(FORWARD);
	}

	/* fault free pGate evaluation */
	for (i = 0; i< pGate->inCount; i++)
		if (pGate->inList[i]->numzero == g_iLid)
		{
			pGate->numzero = g_iLid; break;
		}

	gate_eval1(pGate, iValue, iGateType, i);

	if (iValue == pGate->output)
	{
		/* no event */
		if (iValue != X)
		{
			set(pGate->changed);
		}
		return(FORWARD);
	}
	if (pGate->output == X)
	{
		/* forward evaluation */
		pGate->output = iValue;				/* update pGate output */
		push(g_stack, pGate);
		set(pGate->changed);
		schedule_output(pGate);
		store_learn(pGate, iValue);
		return(FORWARD);
	}

	if (iValue != X)
	{
		return(CONFLICT);
	}			/* report conflict */

	/* backward implication */

	switch (pGate->type)
	{
	case AND:
	case NAND:
	case OR:
	case NOR:
		v1 = (pGate->type == AND || pGate->type == NOR) ? ONE : ZERO;
		if (pGate->output == v1)
		{
			set(pGate->changed);
			for (i = 0; i< pGate->inCount; i++)
				if (pInListGates[i]->output == X)
				{
					pInListGates[i]->output = g_iTruthTable1[pGate->type][v1];
					push(g_stack, pInListGates[i]);
					schedule_input(pGate, i);
				}
			return(BACKWARD);
		}
		else
		{
			for (i = num_x = 0; i< pGate->inCount; i++)
				if (pInListGates[i]->output == X)
				{
					num_x++; j = i;
				}
			if (num_x == 1)
			{
				pInListGates[j]->output = g_iTruthTable1[pGate->type][pGate->output];
				set(pGate->changed);
				push(g_stack, pInListGates[j]);
				schedule_input(pGate, j);
				return(BACKWARD);
			}
		}
		break;

	case BUFF:
	case NOT:
	case PO:
		pInListGates[0]->output = g_iTruthTable1[pGate->type][pGate->output];
		set(pGate->changed);
		push(g_stack, pInListGates[0]);
		schedule_input(pGate, 0);
		return(BACKWARD);
		break;

	case XOR:
	case XNOR:
		for (i = num_x = 0; i< pGate->inCount; i++)
			if (pInListGates[i]->output == X)
			{
				num_x++; j = i;
			}
		if (num_x == 1)
		{
			v1 = (j == 0) ? pInListGates[1]->output : pInListGates[0]->output;
			iValue = g_iTruthTable1[pGate->type][pGate->output];
			if (v1 == ONE)
			{
				iValue = g_iTruthTable1[NOT][iValue];
			}
			pInListGates[j]->output = iValue;
			set(pGate->changed);
			push(g_stack, pInListGates[j]);
			schedule_input(pGate, j);
			return(BACKWARD);
		}
		break;
	}

	/*
	   v1=(pGate->type==AND || pGate->type==NOR) ? ONE :
		  (pGate->type==NAND || pGate->type==OR) ? ZERO : X;
	   if(pGate->output==v1) {
		  set(pGate->changed);
		  if(pGate->type==NAND || pGate->type==NOR) v1=A_NOT(v1);
		  for(i=0;i<pGate->inCount;i++)
		 if(pInListGates[i]->output==X) {
			pInListGates[i]->output=v1;
			push(stack,pInListGates[i]);
			schedule_input(pGate,i);
		 }
		  return(BACKWARD);
	   }
	   else {
		  num_x=0;
		  for(i=0;i<pGate->inCount;i++)
			 if(pInListGates[i]->output==X) {
		  num_x++;
		  j=i;
			 }
		  if(num_x==1) {
		 if(v1!=X)
			pInListGates[j]->output = (pGate->type==NAND||pGate->type==NOR) ?
				   A_NOT(pGate->output) : pGate->output;
			 else {
			v1=(j==0) ? pInListGates[1]->output : pInListGates[0]->output;
			pInListGates[j]->output=(v1==pGate->output)?ZERO:ONE;
		 }
		 set(pGate->changed);
		 push(stack,pInListGates[j]);
		 schedule_input(pGate,j);
			 return(BACKWARD);
		  }
	   }
	*/

	return(FORWARD);
}

/*	impval
	Forward and backward implication.
*/
bool impval(int maxdpi, bool backward, int last)
{
	register int i, start;
	status st;
	GATEPTR g;

	if (backward)
	{
		start = last;
	}
	else
	{
		start = 0;
	}
	while (TRUE)
	{
		/* backward implication */
		if (backward)
		{
			for (i = start; i >= 0; i--)
				while (!is_empty(g_pEventListStack[i]))
				{
					g = pop(g_pEventListStack[i]);
					if ((st = leval(g)) == CONFLICT)
					{
						return(FALSE);
					}
				}
		}
		/* forward implication */
		reset(backward);
		for (i = 0; i < maxdpi; i++)
		{
			while (!is_empty(g_pEventListStack[i]))
			{
				if ((st = leval(pop(g_pEventListStack[i]))) == CONFLICT)
				{
					return(FALSE);
				}
				else if (st == BACKWARD)
				{
					start = i - 1;
					set(backward);
					break;
				}
			}
			if (backward)
			{
				break;
			}
		}
		if (!backward)
		{
			break;
		}
	}
	return(TRUE);
}

/*------learn---------------------------------------------
	preprocessing for static learning
	Good cct only
----------------------------------------------------------*/
void learn(int iNoGate, int iMaxLevelAdd2)
{
	register int ix;
	GATEPTR pGate;
	struct EDEN *temp;
	struct LEARN *record;

#ifdef DEBUGLEARN
	getTime(&slearntime_beg, &ulearntime_beg, &learntime_beg);
	printf("DEBUG: Start learn(iNoGate,iMaxLevelAdd2), iNoGate=%d, iMaxLevelAdd2=%d\n", iNoGate, iMaxLevelAdd2);
#endif

	for (temp = impo; temp != NULL;)
	{
		impo = temp->next;
		MFREE(temp);
		temp = impo->next;
	}
	for (ix = 0; ix < iNoGate; ix++)
	{
		pGate = g_net[ix];
		reset(pGate->changed);
		pGate->numzero = (-1);
		pGate->output = X;
		for (record = pGate->plearn; record != NULL;)
		{
			pGate->plearn = record->next;
			MFREE(record);
			record = pGate->plearn;
		}
	}

	for (ix = 0; ix < iNoGate; ix++)
	{
		pGate = g_net[ix];
		if (is_free(pGate))
		{
			continue;
		}
		if (pGate->inCount == 1)
		{
			continue;
		}

#ifdef DEBUGLEARN
		printf("** Learn node %d: symbol=%s fi=%d fo=%d\n", pGate->index, pGate->hash->symbol, pGate->inCount, pGate->outCount);
#endif
		switch (pGate->type)
		{
		case AND:
		case NOR:
			learn_node(iMaxLevelAdd2, ix, ONE);
			if (pGate->outCount > 1)
			{
				learn_node(iMaxLevelAdd2, ix, ZERO);
			}
			break;
		case OR:
		case NAND:
			learn_node(iMaxLevelAdd2, ix, ZERO);
			if (pGate->outCount > 1)
			{
				learn_node(iMaxLevelAdd2, ix, ONE);
			}
			break;
		default:
			if (pGate->outCount > 1)
			{
				learn_node(iMaxLevelAdd2, ix, ZERO);
				learn_node(iMaxLevelAdd2, ix, ONE);
			}
		}
	}

	clear(g_stack);

#ifdef DEBUGLEARN
	getTime(&slearntime_end, &ulearntime_end, &learntime_end);
	printf("\n*** End of learning\n");
	printf("*** Number of redundant nodes = %d\n", noimpo);
	printf("*** Number of learning records = %d\n", norecord);
	printf("*** Memory required = %d bytes\n", learnmemory);
	printf("*** CPU time spent = %.2f seconds\n", learntime_end - learntime_beg);
#endif
}


/*------learn_node: learning procedure for a node------*/
void learn_node(int maxdpi, int node, level val)
{
	register int ix;
	struct EDEN *tmp;
	GATEPTR gut = g_net[node], tg;

	SNODE = node;
	gut->output = val;
	push(g_stack, gut);
	SVAL = A_NOT(val);

	pushevent(gut);
	gut->numzero = ++g_iLid;
	schedule_output(gut);

	if (impval(maxdpi, FORWARD, 0) == FALSE)
	{
		ALLOCATE(tmp, struct EDEN, 1);
		tmp->node = node;
		tmp->val = A_NOT(val);
		tmp->next = impo;
		impo = tmp;
#ifdef DEBUGLEARN
		noimpo++;
		learnmemory += sizeof(struct EDEN);
		printf("learn: impo: node %d = %s\n", tmp->node, level2str[tmp->val]);
#endif
		for (ix = 0; ix < maxdpi; ix++)
			while (!is_empty(g_pEventListStack[ix]))
			{
				gut = pop(g_pEventListStack[ix]);
				reset(gut->changed);
			}
	}

	/* restore good values */
	for (ix = g_stack.last; ix >= 0; ix--)
	{
		tg = g_stack.list[ix];
		tg->output = X;
		reset(tg->changed);
	}
	clear(g_stack);
}

/*------store_learn: gather info from learning------*/
void store_learn(GATEPTR pGate, level iValue)
{
	struct LEARN *pLearn;

	if (pGate->inCount < 2)
	{
		return;
	}
	if (pGate->numzero != g_iLid)
	{
		return;
	}

	switch (pGate->type)
	{
	case AND:
	case NOR:
		if (iValue == ZERO)
		{
			return;
		}
		break;
		
	case OR:
	case NAND:
		if (iValue == ONE)
		{
			return;
		}
		break;
		
	case XOR:
	case XNOR:
		break;
		
	default:
		return;
	}

	ALLOCATE(pLearn, struct LEARN, 1);
	pLearn->node = SNODE;
	pLearn->tval = SVAL;
	pLearn->sval = A_NOT(iValue);
	pLearn->next = pGate->plearn;
	pGate->plearn = pLearn;

#ifdef DEBUGLEARN
	norecord++;
	learnmemory += sizeof(struct LEARN);
	printf("learn: (node %d = %s) from ", pLearn->node, level2str[pLearn->tval]);
	printf("(node %d = %s)\n", pGate->index, level2str[pLearn->sval]);
#endif
}

int conflict_tbl[3][5] =
/*		0		1		x		d		dbar		old/new	*/
{
	{	PASS,	FAIL,	PASS1,	FAIL,	PASS	},	/* 0 */
	{	FAIL,	PASS,	PASS1,	PASS,	FAIL	},	/* 1 */
	{	PASS,	PASS,	PASS,	PASS,	PASS	}	/* x */
};

/*------imply_learn--------------------------------------*/
status imply_learn(register GATEPTR pGate, register level iValue)
{
	register struct LEARN *pLearn;
	register GATEPTR pLearnGate;
	int iState;

	switch (pGate->type)
	{
	case AND:
	case NOR:
		if (iValue == ONE)
		{
			return(FORWARD);
		} break;
	case OR:
	case NAND:
		if (iValue == ZERO)
		{
			return(FORWARD);
		} break;
	}

	iState = FORWARD;
	for (pLearn = pGate->plearn; pLearn != NULL; pLearn = pLearn->next)
		if (pLearn->sval == iValue)
		{
			pLearnGate = g_net[pLearn->node];
			switch (conflict_tbl[pLearnGate->output][pLearn->tval])
			{
			case PASS:
				break;
			case FAIL:
#ifdef DEBUGLEARN
				printf("Learned: conflict at node=%d old=%s new=%s from node=%d iValue=%s\n", pLearnGate->index, level2str[pLearnGate->output], level2str[pLearn->tval], pGate->index, level2str[iValue]);
#endif
				return(CONFLICT);
			case PASS1:
				pLearnGate->output = pLearn->tval;
				push(g_stack, pLearnGate);
				pushevent(pLearnGate);
				schedule_output(pLearnGate);
#ifdef DEBUGLEARN
				printf("Learned: node=%d iValue=%s from node=%d iValue=%s\n", pLearnGate->index, level2str[pLearnGate->output], pGate->index, level2str[iValue]);
#endif
				iState = BACKWARD;
				break;
			}
		}

	return(iState);
}

/*------imply_learn1-------------------------------------*/
status imply_learn1(register GATEPTR gut, register level val)
{
	register struct LEARN *tmp;
	register GATEPTR tg;
	int state;

	if (val == D)
	{
		val = 1;
	}
	else if (val == DBAR)
	{
		val = 0;
	}

	switch (gut->type)
	{
	case AND:
	case NOR:
		if (val == ONE)
		{
			return(FORWARD);
		} break;
	case OR:
	case NAND:
		if (val == ZERO)
		{
			return(FORWARD);
		} break;
	}

	state = FORWARD;
	for (tmp = gut->plearn; tmp != NULL; tmp = tmp->next)
		if (tmp->sval == val)
		{
			tg = g_net[tmp->node];
			if (tg->freach)
			{
				continue;
			}
			switch (conflict_tbl[tg->output][tmp->tval])
			{
			case PASS:
				break;
			case FAIL:
#ifdef DEBUGLEARN
				printf("Learned: conflict at node=%d old=%s new=%s from node=%d val=%s\n", tg->index, level2str[tg->output], level2str[tmp->tval], gut->index, level2str[val]);
#endif
				return(CONFLICT);
			case PASS1:
				tg->output = tmp->tval;
				push(g_stack, tg);
				pushevent(tg);
				schedule_output(tg);
#ifdef DEBUGLEARN
				printf("Learned: node=%d val=%s from node=%d val=%s\n", tg->index, level2str[tg->output], gut->index, level2str[val]);
#endif
				state = BACKWARD;
				break;
			}
		}

	return(state);
}
