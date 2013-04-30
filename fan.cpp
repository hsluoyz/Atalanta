
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
 
	   Add dynamic unique path sensitization, H. K. Lee, 1/20/'94
 
		atalanta: version 2.0   	 H. K. Lee, 6/30/1997
		   Added diagnostic mode, H. K. Lee, 6/30/1997
 
***********************************************************************/

/*-----------------------------------------------------------------
	filename fan.c
	implements the FAN algorithm.
------------------------------------------------------------------*/
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>

#include "learn.h"
#include "sim.h"
#include "fan.h"

#include "parameter.h" 
#include "define.h"    
#include "macro.h"

extern GATEPTR *g_net;
extern int *g_PrimaryIn, *g_PrimaryOut, *g_iHeadGateIndex;
extern char learnmode, logmode;
extern level g_iTruthTable1[MAXGTYPE][ATALEVEL];
extern level g_iTruthTable2[MAXGTYPE][ATALEVEL][ATALEVEL];
extern STACKTYPE g_unjustStack,   /* set of unjustified lines */
				 g_initObjStack,   		  /* set of initial objectives */
				 g_curObjStack,   		  /* set of current objectives */
				 g_fanObjStack,			  /* set of fanout objectives */
				 g_headObjStack,   		  /* set of head objectives */
				 g_finalObjStack,  		  /* set of final objectives */
				 g_DfrontierStack,  		  /* set of Dfrotiers */
				 g_stack; 			   /* stack for backtracing */
extern struct ROOTTREE g_tree;
extern STACKPTR g_pEventListStack;
extern int mac_i;
extern char gen_all_pat, no_faultsim;
extern int g_iNoPatternsForOneTime;
extern int ntest_each_limit;
extern FILE *g_fpTestFile, *g_fpLogFile;
//extern void Dprintio();

int SELECTMODE = 0;	/* 0: easiest first, 1: hardest first */

/* constant for the dynamic unique path sensitization */
int dy_id = INFINITY;
GATEPTR dyn_dom[MAXGATE];

/* macros for the gate evaluation using the truthtable */
#define gate_eval1(g,v,f,i) \
	if(g->inCount==1) v=g_iTruthTable1[g->type][g->inList[0]->output]; \
	else if(g->inCount==2) \
	   v=g_iTruthTable2[g->type][g->inList[0]->output][g->inList[1]->output];\
	else { \
	   f = (g->type==NAND) ? AND : \
		   (g->type==NOR) ? OR : g->type; \
	   v=g_iTruthTable2[f][g->inList[0]->output][g->inList[1]->output];\
	   for(i=2;i<g->inCount;i++) \
		  v=g_iTruthTable2[f][v][g->inList[i]->output]; \
	   v=(g->type==NAND||g->type==NOR)? A_NOT(v) : v; \
	}

/*	initNetAndFreach
	Initialization of flags and data structures.
	changed=0, all set=empty.
	Identifies all reachable gates from the faulty line.
*/
void initNetAndFreach(int iNoGate, GATEPTR pFaultyGate, int iMaxDPI) //init_net
{
	//INPUT:  pFaultyGate
	//OUTPUT:  freach
	register int i, j;
	register GATEPTR pGate;

	/* clear changed ochange and set freach */
	for (i = 0; i < iNoGate; i++)
	{
		if (is_free(g_net[i]))
		{
			set(g_net[i]->changed); //don't pay attention to LFREE gates
		}
		else
		{
			reset(g_net[i]->changed);
		}
		reset(g_net[i]->freach);
		g_net[i]->output = X;
		g_net[i]->xpath = 1;
	}

	/* clear all sets */
	for (i = 0; i < iMaxDPI; i++)
	{
		clearevent(i);
	}
	clear(g_DfrontierStack);
	clear(g_unjustStack);
	clear(g_initObjStack);
	clear(g_curObjStack);
	clear(g_fanObjStack);
	clear(g_headObjStack);
	clear(g_finalObjStack);
	clear(g_stack);
	clear(g_tree);

	/* flag all reachable gates from the faulty gate */
	pushevent(pFaultyGate);
	for (i = pFaultyGate->dpi; i < iMaxDPI; i++)
		while (!is_empty(g_pEventListStack[i]))
		{
			pGate = popevent(i);
			reset(pGate->changed); //changed == 0 ------------> Need to handle
			set(pGate->freach); //All outputs of pFaultyGate --------------> freach = 1
			for (j = 0; j < pGate->outCount; j++)
			{
				pushevent(pGate->outList[j]);
			}
		}
}

/*	refFaultyGateOutput
	box 1 of the FAN algorithm.
	Defines input and output values of the faulty gate.
	Returns the level of the highest level gate in which
	backward implication is required.
	If a CONFLICT condition (redundant faults) occurs,
	returns (-1).
*/
int refFaultyGateOutput(FAULTPTR pFault) //set_faulty_gate
{
	//INPUT:  pFault
	//OUTPUT:  iLastDPI & g_stack & many gates's ouput
	register int i, iLastDPI;
	register GATEPTR pGate;
	level iOriginalLineValue, iLineValue;
	GATEPTR pUpflowGate;

	iLastDPI = 0;
	pGate = pFault->gate;
	pUpflowGate = (pFault->line == OUTFAULT) ? pGate : pGate->inList[pFault->line]; //get the upflow gate
	iLineValue = (pFault->type == SA0) ? D : DBAR; //D: 1/0     DBAR: 0/1

	/* input stuck-at faults */
	if (pFault->line >= 0) /////////////pUpflowGate -> pFault -> pGate
	{
		/* input line pFault */
		pUpflowGate->output = (iLineValue == D) ? ONE : ZERO;  /* faulty line */ //get pUpflowGate's output!!
		push(g_stack, pUpflowGate);
		//output change ------> push into g_stack !!
		switch (pGate->type)
		{
		case AND:
		case NAND:
			set(pGate->changed);
			for (i = 0; i< pGate->inCount; i++)
			{
				if (i != pFault->line) //For every other inputs
				{
					if (pGate->inList[i]->output == X)
					{
						pGate->inList[i]->output = ONE; //activate other inputs
						push(g_stack, pGate->inList[i]);
					}
					else if (pGate->inList[i]->output != ONE)
					{
						return(-1); //conflict occurs!
					}
				}
			}
			pGate->output = (pGate->type == NAND) ? A_NOT(iLineValue) : iLineValue; //get pGate's output!!
			push(g_stack, pGate);
			break;
		case OR:
		case NOR:
			set(pGate->changed);
			for (i = 0; i< pGate->inCount; i++)
			{
				if (i != pFault->line) //For every other inputs
				{
					if (pGate->inList[i]->output == X)
					{
						pGate->inList[i]->output = ZERO; //activate other inputs
						push(g_stack, pGate->inList[i]);
					}
					else if (pGate->inList[i]->output != ZERO)
					{
						return(-1); //conflict occurs!
					}
				}
			}
			pGate->output = (pGate->type == NOR) ? A_NOT(iLineValue) : iLineValue; //get pGate's output!!
			push(g_stack, pGate);
			break;
		case NOT:
			pGate->output = A_NOT(iLineValue);
			push(g_stack, pGate);
			break;
		case BUFF:
		case PO:
			pGate->output = iLineValue;
			push(g_stack, pGate);
			break;
		case XOR:
		case XNOR:
			break; //where's code?? no support for XOR and XNOR?
		}

		/* schedule events */
		if (pGate->output != X)
		{
			schedule_output(pGate); //lead to refFaultyGateOutput
		}
		for (i = 0; i< pGate->inCount; i++)
		{
			if (pGate->inList[i]->output != X) //Always != X
			{
				iLastDPI = max(pGate->inList[i]->dpi, iLastDPI);
				schedule_input(pGate, i); //lead to refFaultyGateOutput
			}
		}
	}
	else /////////////pGate -> pFault
	{
		/* output line pFault */
		//pGate->output = (iLineValue == D) ? ONE : ZERO;
		
		pGate->output = iLineValue;
		push(g_stack, pGate);
		schedule_output(pGate);

		if (is_head(pGate)) //pGate->outCount >= 2 (FANOUT)
		{
			set(pGate->changed);
		}
		
		//pGate->outCount == 1
		else if (pGate->inCount == 1) ////////////////////pGate->inList[0] ----> pGate ----> pFault 
		{
			set(pGate->changed);
			iOriginalLineValue = (iLineValue == D) ? ONE : ZERO;
			pGate->inList[0]->output = g_iTruthTable1[pGate->type][iOriginalLineValue]; //get input gate's output
			push(g_stack, pGate->inList[0]);
			schedule_input(pGate, 0);
			iLastDPI = pGate->inList[0]->dpi;
		}
		else if ((iLineValue == D && (pGate->type == AND || pGate->type == NOR)) || //Why ???
			(iLineValue == DBAR && (pGate->type == NAND || pGate->type == OR))) //(pGate->inCount >= 2)
		{
			set(pGate->changed);
			iOriginalLineValue = (pGate->type == AND || pGate->type == NAND) ? ONE : ZERO;
			for (i = 0; i< pGate->inCount; i++)
			{
				if (pGate->inList[i]->output == X)
				{
					pGate->inList[i]->output = iOriginalLineValue;
					iLastDPI = max(pGate->inList[i]->dpi, iLastDPI);
					push(g_stack, pGate->inList[i]);
					schedule_input(pGate, i);
				}
			}		
		}
		else
		{
			pushevent(pGate);
			iLastDPI = pGate->dpi;
		}
	}

	return(iLastDPI);
}

/*	getFaultyGateOutput
	Evaluates the faulty gate function and returns the output value.
*/
level getFaultyGateOutput(register GATEPTR pGate, FAULTPTR pFault) //faulty_gate_eval
{
	//Precondition:  pGate == pFault->gate
	//OUTPUT:  pGate->output
	register int i, j;
	register level iVaule;
	//level fval;
	logic iGateType;

	if (pGate->inCount == 0) //pGate -> pFault
	{
		return(pGate->output);
	}

	if (pFault->line == OUTFAULT) //(pGate->inList[0]) -> pGate -> pFault
	{
		j = 0;
		iVaule = pGate->inList[0]->output;
	}
	else //(pGate->inList[j]) -> pFault -> pGate
	{
		j = pFault->line;
		iVaule = pGate->inList[j]->output;
		if (iVaule == ZERO && pFault->type == SA1)
		{
			iVaule = DBAR; //iVaule = pGate->input
		}
		else if (iVaule == ONE && pFault->type == SA0)
		{
			iVaule = D; //iVaule = pGate->input
		}
	}

	if (pGate->inCount == 1)
	{
		iVaule = g_iTruthTable1[pGate->type][iVaule]; //iVaule = pGate->output
	}
	else //pGate->inCount > 1
	{
		iGateType = (pGate->type == NAND) ? AND : (pGate->type == NOR) ? OR : pGate->type;
		for (i = 0; i < j; i++)
			iVaule = g_iTruthTable2[iGateType][iVaule][pGate->inList[i]->output];
		for (++i; i< pGate->inCount; i++)
			iVaule = g_iTruthTable2[iGateType][iVaule][pGate->inList[i]->output];
		if (pGate->type == NAND || pGate->type == NOR)
		{
			iVaule = A_NOT(iVaule);
		}
	}

	if (pFault->line == OUTFAULT) //pGate -> pFault
	{
		if (iVaule == ZERO && pFault->type == SA1)
		{
			iVaule = DBAR;
		}
		else if (iVaule == ONE && pFault->type == SA0)
		{
			iVaule = D;
		}
	}

	return(iVaule);
}

/*	eval
	Evaluate good circuit in forward and backward completely.
	Set changed flag if the gate is evaluated permanently.
	Push the evaluated gate to stack for backtracking.
	Schedule next events.
*/
status eval(register GATEPTR pGate, FAULTPTR pFault)
{
	register int i, j;
	register level iValue, iTempValue;
	int iNumX;
	logic iTempValue2;
	GATEPTR *pGateInList;

	reset(pGate->changed);
	pGateInList = pGate->inList;

	/* if a line is a head line, stop */
	if (is_head(pGate))
	{
		set(pGate->changed);
		return(FORWARD);
	}

	/* faulty pGate evaluation */
	if (pGate == pFault->gate)
	{
		iValue = getFaultyGateOutput(pGate, pFault);
		if (iValue == X)
		{
			if (pGate->output != X)
			{
				for (i = iNumX = 0; i< pGate->inCount; i++)
					if (pGateInList[i]->output == X)
					{
						iNumX++; j = i;
					}
				if (iNumX == 1)
				{
					/* backward implication */
					iValue = (pGate->output == D) ? ONE : (pGate->output == DBAR) ? ZERO : pGate->output;
					iValue = g_iTruthTable1[pGate->type][iValue];
					switch (pGate->type)
					{
					case XOR:
					case XNOR:
						iTempValue = (j == 0) ? pGateInList[1]->output : pGateInList[0]->output;
						if (iTempValue == ONE)
						{
							iValue = g_iTruthTable1[NOT][iValue];
						}
						break;
					}
					pGateInList[j]->output = iValue;
					set(pGate->changed);
					push(g_stack, pGateInList[j]);
					schedule_input(pGate, j);
					return(BACKWARD);
				}
				else
				{
					push(g_unjustStack, pGate);
				}
			}
		}
		else if (iValue == pGate->output)
		{
			set(pGate->changed);
		}
		else if (pGate->output == X)
		{
			/* forward imp */
			set(pGate->changed);
			pGate->output = iValue;
			push(g_stack, pGate);
			schedule_output(pGate);
		}
		else
		{
			return(CONFLICT);
		}
		return(FORWARD);
	}

	/* fault free pGate evaluation */
	gate_eval1(pGate, iValue, iTempValue2, i);

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
		/* forward implication */
		pGate->output = iValue;
		push(g_stack, pGate);
		set(pGate->changed);
		schedule_output(pGate);
		return(FORWARD);
	}

	if (iValue != X)
	{
		return(CONFLICT);
	}		/* conflict */

	/* backward implication */
	switch (pGate->type)
	{
	case AND:
	case NAND:
	case OR:
	case NOR:
		iTempValue = (pGate->type == AND || pGate->type == NOR) ? ONE : ZERO;
		if (pGate->output == iTempValue)
		{
			set(pGate->changed);
			for (i = 0; i< pGate->inCount; i++)
				if (pGateInList[i]->output == X)
				{
					pGateInList[i]->output = g_iTruthTable1[pGate->type][iTempValue];
					push(g_stack, pGateInList[i]);
					schedule_input(pGate, i);
				}
			i = BACKWARD;
		}
		else
		{
			for (i = iNumX = 0; i< pGate->inCount; i++)
				if (pGateInList[i]->output == X)
				{
					iNumX++; j = i;
				}
			if (iNumX == 1)
			{
				pGateInList[j]->output = g_iTruthTable1[pGate->type][pGate->output];
				set(pGate->changed);
				push(g_stack, pGateInList[j]);
				schedule_input(pGate, j);
				i = BACKWARD;
			}
			else
			{
				push(g_unjustStack, pGate);
				i = FORWARD;
			}
		}
		break;
	case BUFF:
	case NOT:
	case PO:
		pGateInList[0]->output = g_iTruthTable1[pGate->type][pGate->output];
		set(pGate->changed);
		push(g_stack, pGateInList[0]);
		schedule_input(pGate, 0);
		i = BACKWARD;
		break;
	case XOR:
	case XNOR:
		for (i = iNumX = 0; i< pGate->inCount; i++)
			if (pGateInList[i]->output == X)
			{
				iNumX++; j = i;
			}
		if (iNumX == 1)
		{
			iTempValue = (j == 0) ? pGateInList[1]->output : pGateInList[0]->output;
			iValue = g_iTruthTable1[pGate->type][pGate->output];
			if (iTempValue == ONE)
			{
				iValue = g_iTruthTable1[NOT][iValue];
			}
			pGateInList[j]->output = iValue;
			set(pGate->changed);
			push(g_stack, pGateInList[j]);
			schedule_input(pGate, j);
			i = BACKWARD;
		}
		else
		{
			push(g_unjustStack, pGate);
			i = FORWARD;
		}
		break;
	}

#ifdef LEARNFLG
	if (learnmode == 'y' && pGate->plearn != NULL)
	{
		if (pGate->output == ZERO || pGate->output == ONE)
		{
			switch (imply_learn(pGate, pGate->output))
			{
			case BACKWARD:
				i = BACKWARD; break;
			case CONFLICT:
				i = CONFLICT; break;
			}
		}
	}
#endif
	return(i);
}

/*	implyForwardAndBackward
	box 3 of the FAN algorithm.
	Forward and backward implication.
*/
bool implyForwardAndBackward(int iMaxDPI, bool bBackward, int iLastDPI, FAULTPTR pFault) //imply
{
	register int i, iStartDPI;
	status iStatus;
	GATEPTR pGate;

	if (bBackward)
	{
		iStartDPI = iLastDPI;
	}
	else
	{
		iStartDPI = 0;
	}
	
	while (TRUE)
	{
		/* backward implication */
		if (bBackward)
		{
			for (i = iStartDPI; i >= 0; i--) //clear all stacks from 0 to iStartDPI
			{
				while (!is_empty(g_pEventListStack[i]))
				{
					pGate = pop(g_pEventListStack[i]);
					if ((iStatus = eval(pGate, pFault)) == CONFLICT)
					{
						return(FALSE); //conflict!!
					}
				}
			}
		}
		/* forward implication */
		reset(bBackward);
		for (i = 0; i < iMaxDPI; i++)
		{
			while (!is_empty(g_pEventListStack[i]))
			{
				if ((iStatus = eval(pop(g_pEventListStack[i]), pFault)) == CONFLICT)
				{
					return(FALSE); //conflict!!
				}
				else if (iStatus == BACKWARD)
				{
					iStartDPI = i - 1;
					set(bBackward);
					break;
				}
			}
			if (bBackward)
			{
				break;
			}
		}
		if (!bBackward)
		{
			break;
		}
	}
	return(TRUE);
}

/*	unique_sensitize
	Box 6 in Fig 9 of the FAN algorithm.
	Predetermines all neccessary inputs of the uniquely
	sensitizable path.
	Returns -1 if no sensitized values exist.
		Otherwise, returns the highest depth for backward implication.
*/

int unique_sensitize(register GATEPTR pGate, GATEPTR pFaultyGate)
{
	register int i;
	register LINKPTR pLink;
	register GATEPTR pNextGate;
	int iLastDPI;		/* the largest depth of sensitized lines */
	bool bFlag;
	level iValue;

	iLastDPI = (-1);

	/* sensitize the current pGate */
	if (pGate != pFaultyGate)
	{
		//pFaultyGate ----------> pGate
		//other Gates ---iValue---> pGate
		iValue = (pGate->type == AND || pGate->type == NAND) ? ONE :
			(pGate->type == OR || pGate->type == NOR) ? ZERO : X;
			
		if (iValue != X)
		{
			for (i = 0; i< pGate->inCount; i++)
				if (pGate->inList[i]->output == X)
				{
					pGate->inList[i]->output = iValue;
					iLastDPI = max(pGate->inList[i]->dpi, iLastDPI);
					push(g_stack, pGate->inList[i]);
					schedule_input(pGate, i);
				}
		}
	}

	/* sensitize pNextGate path */
	while (pGate != NULL)
	{
		if (pGate->outCount == 0)
		{
			break;
		}
		else if (pGate->outCount == 1) //pGate -> pNextGate
		{
			pNextGate = pGate->outList[0];
		}
		else if (pGate->u_path == NULL)
		{
			break;
		}
		else
		{
			pNextGate = pGate->u_path->ngate;
		}
		//pGate -------------------> pNextGate
		//pNextGate->inList[i] ---iValue---> pNextGate
		iValue = (pNextGate->type == AND || pNextGate->type == NAND) ? ONE :
			(pNextGate->type == OR || pNextGate->type == NOR) ? ZERO :
				X;
		if (iValue != X) //////////if iValue == X, then pNextGate can't have multiple outputs!!!
		{
			if (pGate->outCount == 1)
			{
				/* one fanout */
				for (i = 0; i< pNextGate->inCount; i++)
					if (pNextGate->inList[i] != pGate && pNextGate->inList[i]->output == X)
					{
						pNextGate->inList[i]->output = iValue;
						iLastDPI = max(pNextGate->inList[i]->dpi, iLastDPI);
						push(g_stack, pNextGate->inList[i]);
						schedule_input(pNextGate, i);
					}
			}
			else //pGate->outCount > 1
			{
				/* multiple fanout */
				for (i = 0; i< pNextGate->inCount; i++)
					if (pNextGate->inList[i]->output == X)
					{
						set(bFlag);
						for (pLink = pGate->u_path->next; pLink != NULL; pLink = pLink->next)
							if (pNextGate->inList[i] == pLink->ngate)
							{
								reset(bFlag);
								break;
							}
						if (bFlag)
						{
							pNextGate->inList[i]->output = iValue;
							iLastDPI = max(pNextGate->inList[i]->dpi, iLastDPI);
							push(g_stack, pNextGate->inList[i]);
							schedule_input(pNextGate, i);
						}
					}
			}
		}
		pGate = pNextGate;
	}
	return(iLastDPI);
}

/*------dynamic_unique_sensitize---------------------------------
	Dynamic unique path sensitization
	Finds dynamic dominators for the given D-frontier gates
	and assigns mandatory signals.

	Algorithm:
	   This is a three step algorithm finding dynamic dominators.
	   1. Propagate all D-frontier gates in the forward order.
	   2. Trace back in the backward order ---
		  This step finds all X-paths.
	   3. Find dynamic dominators by tracing X-paths.

	   Assigns non-controlling values to inputs of dynamic
	   dominators which can be be reachable from D-frontier gates.

	Implemented by H. K. Lee, 1/20/1994
---------------------------------------------------------------------*/

int dynamic_unique_sensitize(GATEPTR *Dfront, int nod, int maxdpi, GATEPTR *dom_array, GATEPTR faulty_gate)
{
	int ndom = 0, ngate;
	int dy_id2;
	register int i, j, k; //,l,m,n;
	register GATEPTR gut, next; //gate, g not used
	//GATEPTR Dominator;
	int ndominator = 0,no_dom = 0; //new_dom not used
	int flag = FALSE;
	int debug = TRUE;
	int v1,send = -1;
	GATEPTR xpo[MAXPO]; int nxpo;

	/* pass 1: D-frontier propagation */
	++dy_id;
	for (i = nxpo = 0; i <= nod; i++)
	{
		gut = Dfront[i];
		if (gut->freach1 < dy_id)
		{
			push(g_pEventListStack[gut->dpi], gut);
			gut->freach1 = dy_id;
		}
	}
	for (i = 0; i < maxdpi; i++)
	{
		while (!is_empty(g_pEventListStack[i]))
		{
			gut = pop(g_pEventListStack[i]);
			if (gut->type == PO)
			{
				xpo[nxpo++] = gut;
			}
			for (j = 0; j< gut->outCount; j++)
			{
				next = gut->outList[j];
				if ((next->output == X) && (next->freach1 < dy_id))
				{
					push(g_pEventListStack[next->dpi], next);
					next->freach1 = dy_id;
				}
			}
		}
	}

	/* pass 2: Backward propagation --- X-path */
	dy_id2 = dy_id + 1;
	for (i = 0; i < nxpo; i++)
	{
		gut = xpo[i];
		gut->freach1 = dy_id2;
		for (j = 0; j< gut->inCount; j++)
		{
			next = gut->inList[j];
			if (next->freach1 == dy_id)
			{
				push(g_pEventListStack[next->dpi], next);
				next->freach1 = dy_id2;
			}
		}
	}
	for (i = maxdpi - 1; i >= 0; i--)
		while (!is_empty(g_pEventListStack[i]))
		{
			gut = pop(g_pEventListStack[i]);
			for (j = 0; j< gut->inCount; j++)
			{
				next = gut->inList[j];
				if (next->freach1 == dy_id)
				{
					push(g_pEventListStack[next->dpi], next);
					next->freach1 = dy_id2;
				}
			}
		}

	/* pass 3: Compute dominators */
	dy_id = dy_id2 + 1;
	for (i = ngate = 0; i <= nod; i++)
	{
		gut = Dfront[i];
		push(g_pEventListStack[gut->dpi], gut);
		ngate++;
		gut->freach1 = dy_id;
	}
	for (i = k = 0; i < maxdpi; i++)
	{
		if (ngate == 1 && g_pEventListStack[i].last == 0)
		{
			dom_array[k++] = g_pEventListStack[i].list[0];
		}
		while (!is_empty(g_pEventListStack[i]))
		{
			gut = pop(g_pEventListStack[i]);
			ngate--;
			if (gut->type == PO)
			{
				ngate = INFINITY; break;
			}
			for (j = 0; j< gut->outCount; j++)
			{
				next = gut->outList[j];
				if (next->freach1 == dy_id2)
				{
					push(g_pEventListStack[next->dpi], next);
					next->freach1 = dy_id;
					ngate++;
				}
			}
		}
		if (ngate == INFINITY)
		{
			break;
		}
	}

	/* Assign non-controlling values to dominators */
	send = (-1);
	while (--k >= 0)
	{
		gut = dom_array[k];
		/*
									printf("dominator: gut=%d #Dfrontier=%d\n",gut->index,nod+1);
									*/
		if (gut == faulty_gate)
		{
			continue;
		}
		v1 = (gut->type == AND || gut->type == NAND) ? ONE : (gut->type == OR || gut->type == NOR) ? ZERO : X;
		if (v1 != X)
		{
			for (i = 0; i< gut->inCount; i++)
			{
				next = gut->inList[i];
				if (next->freach1 < dy_id && next->output == X)
				{
					next->output = v1;
					/*
																										printf("\tmandatory signal assignment: gut=%d val=%d\n", next->index, v1);
																										*/
					send = max(next->dpi, send);
					push(g_stack, next);
					schedule_input(gut, i);
				}
			}
		}
	}

	return(send);
}


/*	closest_po
	Finds the gate closiest from a primary output.
*/
GATEPTR closest_po(STACKPTR pObjectiveStack, int *piClose)
{
	register int i, iDPO;
	register GATEPTR pGate;

	if (ptr_is_empty(pObjectiveStack))
	{
		return(NULL);
	}
	*piClose = pObjectiveStack->last;
	iDPO = pObjectiveStack->list[*piClose]->dpo;
	for (i = (pObjectiveStack->last) - 1; i >= 0; i--)
		if (pObjectiveStack->list[i]->dpo < iDPO)
		{
			iDPO = pObjectiveStack->list[i]->dpo;
			*piClose = i;
		}
	pGate = pObjectiveStack->list[*piClose];
	return(pGate);
}

/*	select_hardest
	Finds the hardest gate to satisfy
*/
GATEPTR select_hardest(STACKPTR pObjectiveStack, int *piClose)
{
	register int i, iDPO;
	register GATEPTR pGate;

	if (ptr_is_empty(pObjectiveStack))
	{
		return(NULL);
	}
	*piClose = pObjectiveStack->last;
	iDPO = pObjectiveStack->list[*piClose]->dpo;
	for (i = (pObjectiveStack->last) - 1; i >= 0; i--)
		if (pObjectiveStack->list[i]->dpo > iDPO)
		{
			iDPO = pObjectiveStack->list[i]->dpo;
			*piClose = i;
		}
	pGate = pObjectiveStack->list[*piClose];
	return(pGate);
}

/*	backtrace
	Fiqure 8, of the FAN algorithm.
	Multiple backtrace.
*/
status backtrace(status iState)
{
	int i;
	register int j;
	register level v1;
	GATEPTR a_curr_obj, *input;
	int n0, n1, nn0, nn1;
	int easiest, easy_cont;

	/* box 1: Initialization of objective and its logic level */
	if (iState == 81)
	{
		copy(g_initObjStack, g_curObjStack, i);
		for (i = 0; i <= g_initObjStack.last; i++)
		{
			a_curr_obj = g_initObjStack.list[i];
			switch (a_curr_obj->output)
			{
			case ZERO:
			case DBAR:
				setline(a_curr_obj, 1, 0); break;	/* unjustified lines */
			case ONE:
			case D:
				setline(a_curr_obj, 0, 1); break;
			default:
				/* Dfrontier */
				switch (a_curr_obj->type)
				{
				case AND:
				case NOR:
					setline(a_curr_obj, 0, 1); break;
				case NAND:
				case OR:
					setline(a_curr_obj, 1, 0); break;
				case XOR:
				case XNOR:
					setline(a_curr_obj, 1, 0); break;
				}
			}
		}
		iState = 82;
	}

	while (TRUE)
	{
		switch (iState)
		{
			/* Box 2,3,4 of figure 8 */
		case 82:
			if (is_empty(g_curObjStack)) 		/* box 2 */
			{
				if (is_empty(g_fanObjStack))
				{
					iState = 103;
				}	/* box 4 */
				else
				{
					iState = 86;
				}
			}
			else
			{
				a_curr_obj = pop(g_curObjStack);	/* box 3 */
				iState = 85;
			}
			break;

			/* Box 5,9,10,11,12 of figure 8 */
		case 85:
			if (is_head(a_curr_obj))
			{
				/* box 5 */
				push(g_headObjStack, a_curr_obj);	/* box 12 */
			}
			else
			{
				/* box 9,10,11 */
				switch (a_curr_obj->type)
				{
				case AND:
					n0 = a_curr_obj->numzero;
					n1 = a_curr_obj->numone;
					v1 = ZERO;
					break;
				case OR:
					n0 = a_curr_obj->numzero;
					n1 = a_curr_obj->numone;
					v1 = ONE;
					break;
				case NAND:
					n0 = a_curr_obj->numone;
					n1 = a_curr_obj->numzero;
					v1 = ZERO;
					break;
				case NOR:
					n0 = a_curr_obj->numone;
					n1 = a_curr_obj->numzero;
					v1 = ONE;
					break;
				case NOT:
					n0 = a_curr_obj->numone;
					n1 = a_curr_obj->numzero;
					v1 = X;
					break;
				case XOR:
					j = 0;
					if ((v1 = a_curr_obj->inList[j]->output) == X)
					{
						v1 = a_curr_obj->inList[++j]->output;
					}
					if (v1 == ONE)
					{
						n0 = a_curr_obj->numone;
						n1 = a_curr_obj->numzero;
					}
					else
					{
						n0 = a_curr_obj->numzero;
						n1 = a_curr_obj->numone;
					}
					v1 = X;
					break;
				case XNOR:
					j = 0;
					if ((v1 = a_curr_obj->inList[j]->output) == X)
					{
						v1 = a_curr_obj->inList[++j]->output;
					}
					if (v1 == ZERO)
					{
						n0 = a_curr_obj->numone;
						n1 = a_curr_obj->numzero;
					}
					else
					{
						n0 = a_curr_obj->numzero;
						n1 = a_curr_obj->numone;
					}
					v1 = X;
					break;
				default:
					/* BUFF, PO, PI */
					n0 = a_curr_obj->numzero;
					n1 = a_curr_obj->numone;
					v1 = X;
					break;
				}

				/* Find the easiest input. */
				input = a_curr_obj->inList;
				easy_cont = INFINITY;
				easiest = 0;
				if (v1 == ZERO)
				{
					/* and, nand */
					for (i = 0; i< a_curr_obj->inCount; i++)
						if (input[i]->output == X)
						{
							if (easy_cont > input[i]->cont0)
							{
								easy_cont = input[i]->cont0;
								easiest = i;
							}
						}
				}
				else
				{
					/* or, nor, xor,xnor */
					for (i = 0; i< a_curr_obj->inCount; i++)
						if (input[i]->output == X)
						{
							if (easy_cont > input[i]->cont1)
							{
								easy_cont = input[i]->cont1;
								easiest = i;
							}
						}
				}

				for (i = 0; i< a_curr_obj->inCount; i++)
				{
					if (input[i]->output == X)
					{
						if (i == easiest)
						{
							nn0 = n0; nn1 = n1;
						}
						else if (v1 == ZERO)
						{
							nn0 = 0; nn1 = n1;
						}
						else if (v1 == ONE)
						{
							nn0 = n0; nn1 = 0;
						}
						else
						{
							/* xor,xnor */
							if (n0 > n1)
							{
								nn0 = n0; nn1 = n1;
							}
							else
							{
								nn0 = n1; nn1 = n0;
							}
						}

						if (nn0 > 0 || nn1 > 0)
						{
							if (is_fanout(input[i]))
							{
								if (input[i]->numzero == 0 && input[i]->numone == 0)
								{
									push(g_fanObjStack, input[i]);
								}
								input[i]->numzero += nn0;
								input[i]->numone += nn1;
							}
							else
							{
								setline(input[i], nn0, nn1);
								push(g_curObjStack, input[i]);
							}
						}
					}
				} /* for */
			}

			iState = 82;
			break;

			/* Box 6,7,8 of figure 8 */
		case 86:
			a_curr_obj = closest_po(&g_fanObjStack, &i);	/* box 6 */
			delete(g_fanObjStack, i);
			if (a_curr_obj->output != X)
			{
				iState = 82; break;
			}
			if (is_reachable_from_fault(a_curr_obj))
			{
				/* box 7 */
				iState = 85; break;
			}
			if (!is_conflict(a_curr_obj))
			{
				/* box 8 */
				iState = 85; break;
			}
			push(g_finalObjStack, a_curr_obj);		/* box 12 in figure 10 */
			iState = 93;
			break;

		default:
			return(iState);
		}
	}
}

/*	find_final_objective
	Figure 10 of the FAN algorithm.
	Finds a final objective.
*/

void find_final_objective(bool *backtrace_flag, bool fault_propagated_to_po, int nog, GATEPTR *last_Dfrontier)
{
	int i;
	register GATEPTR p;
	register status state;


	if (*backtrace_flag)
	{
		state = 107;
	}	/* box 1 */
	else if (is_empty(g_fanObjStack))
	{
		state = 103;
	}	/* box 2 */
	else
	{
		state = 86;
	}
	while (TRUE)
		switch (state)
		{
		case 103:
			/* box 3,4,5,6 */
			if (is_empty(g_headObjStack))
			{
				state = 107;
			}	/* box 3 */
			else
			{
				p = g_headObjStack.list[0];
				for (i = 1; i <= g_headObjStack.last; i++)
					g_headObjStack.list[i - 1] = g_headObjStack.list[i];
				delete_last(g_headObjStack);
				if (p->output == X)
				{
					/* box 4,5 */
					push(g_finalObjStack, p);		/* box 6 */
					state = 93;
				}
				else
				{
					state = 103;
				}
			}
			break;
		case 107:
			/* box 7,8,9,10,11 */
			reset(*backtrace_flag);	/* box 7 */
			for (i = 0; i < nog; i++)
			{
				/* initialization */
				g_net[i]->numzero = 0;
				g_net[i]->numone = 0;
			}
			clear(g_initObjStack);
			clear(g_curObjStack);
			clear(g_fanObjStack);
			clear(g_headObjStack);
			clear(g_finalObjStack);
			if (!is_empty(g_unjustStack))
			{
				/* box 8 */
				copy(g_unjustStack, g_initObjStack, i);	/* box 9 */
				if (fault_propagated_to_po)
				{
					/* box 10 */
					(*last_Dfrontier) = NULL;
					state = 81;
					break;
				}
			}
			switch (SELECTMODE)
			{
			case 0:
				/* easiest D first */
				(*last_Dfrontier) = closest_po(&g_DfrontierStack, &i);
				push(g_initObjStack, (*last_Dfrontier));
				break;
			case 1:
				(*last_Dfrontier) = select_hardest(&g_DfrontierStack, &i);
				push(g_initObjStack, (*last_Dfrontier));
				break;
			case 2:
				(*last_Dfrontier) = closest_po(&g_DfrontierStack, &i);
				if (is_empty(g_unjustStack))
				{
					push(g_initObjStack, (*last_Dfrontier));
				}
				break;
			case 3:
				(*last_Dfrontier) = select_hardest(&g_DfrontierStack, &i);
				if (is_empty(g_unjustStack))
				{
					push(g_initObjStack, (*last_Dfrontier));
				}
				break;
			}

			state = 81;
			break;
		case 86:
		case 81:
			state = backtrace(state);
			break;
		default:
			return;				/* exit */
		}
}

/*	Xpath
	Finds the existence of an X-path recursively.
	Returns 0 if no X-path exist,
		1 if unknown (not checked yet) and
		2 if an X-path exist.
*/
bool Xpath(register GATEPTR pGate)
{
	register int i;

	/* base step --- if no X-path exist, return FALSE  */
	if (pGate->output != X || pGate->xpath == 0)
	{
		pGate->xpath = 0;
		return(FALSE);
	}

	/* base step --- if an X-path exist, return TRUE */
	if ((pGate->type == PO) || (pGate->xpath == 2))
	{
		pGate->xpath = 2;
		return(TRUE);
	}

	/* induction step --- else, go to next step */
	for (i = 0; i < pGate->outCount; i++)
		if (Xpath(pGate->outList[i]))
		{
			pGate->xpath = 2;
			return(TRUE);
		}
	pGate->xpath = 0;
	return(FALSE);
} 

/*	update_Dfrontier
	Updates Dfrontier and checks X-path exist or not.
*/
void update_Dfrontier()
{
	register int i, j;
	register GATEPTR pGate;
	//int iFirst; //No use !!

	//iFirst = INFINITY;
	for (i = 0; i <= g_DfrontierStack.last;)
	{
		pGate = g_DfrontierStack.list[i];
		switch (pGate->output)
		{
		case D:
		case DBAR:
			for (j = 0; j< pGate->outCount; j++)
			{
				push(g_DfrontierStack, pGate->outList[j]);
			}
			delete(g_DfrontierStack, i);
			break;

			
		case X: //Keep X gates
			//if (pGate->index < iFirst)
			//{
			//	iFirst = pGate->index;
			//}
			i++;
			break;

			
		default:
			/* 1 or 0 */
			delete(g_DfrontierStack, i);
		}
	}
}

/*	restore_faults
	Sets up primary inputs of free faults.
*/
void restore_faults(FAULTPTR pFault)
{
	register int i, j;
	register GATEPTR pGate;
	int k;
	level value, gtype;

	clear(g_fanObjStack);
	pGate = pFault->gate;
	if (pFault->line != OUTFAULT)
	{
		pGate = pGate->inList[pFault->line]; //inList[pFault->line] -> pFault -> pGate
	}
	push(g_fanObjStack, pGate);

	while (TRUE)
	{
		pGate = pGate->outList[0];
		for (i = 0; i< pGate->inCount; i++)
			if (pGate->inList[i]->output == ZERO || pGate->inList[i]->output == ONE)
			{
				push(g_fanObjStack, pGate->inList[i]);
			}
		if (is_head(pGate))
		{
			break;
		}
	}

	while (!is_empty(g_fanObjStack))
	{
		pGate = pop(g_fanObjStack);    
		if (pGate->output == D)
		{
			pGate->output = ONE;
		}
		else if (pGate->output == DBAR)
		{
			pGate->output = ZERO;
		}
		if (!(pGate->type == PI || pGate->output == X))
		{
			clear(g_curObjStack);
			push(g_curObjStack, pGate);
			while (!is_empty(g_curObjStack))
			{
				pGate = pop(g_curObjStack);
				switch (pGate->type)
				{
				case PI:
					break;
				case XOR:
					pGate->inList[0]->output = ZERO;
					push(g_curObjStack, pGate->inList[0]);
					for (j = 1; j< pGate->inCount; j++)
					{
						pGate->inList[j]->output = pGate->output;
						push(g_curObjStack, pGate->inList[j]);
					}
					break;
				case XNOR:
					pGate->inList[0]->output = ONE;
					push(g_curObjStack, pGate->inList[0]);
					for (j = 1; j< pGate->inCount; j++)
					{
						pGate->inList[j]->output = pGate->output;
						push(g_curObjStack, pGate->inList[j]);
					}
					break;
				case PO:
				case BUFF:
				case NOT:
					pGate->inList[0]->output = g_iTruthTable1[pGate->type][pGate->output];
					push(g_curObjStack, pGate->inList[0]);
					break;
				default:
					/* and,or,nor,nand */
					value = g_iTruthTable1[pGate->type][pGate->output];
					gtype = (pGate->type == AND || pGate->type == NAND) ? ONE : ZERO;
					if (value == gtype)
					{
						for (j = 0; j< pGate->inCount; j++)
						{
							pGate->inList[j]->output = value;
							push(g_curObjStack, pGate->inList[j]);
						}
					}
					else
					{
						k = 0;
						for (j = 1; j< pGate->inCount; j++)
							if (pGate->inList[j]->dpi< pGate->inList[k]->dpi)
							{
								k = j;
							}
						pGate->inList[k]->output = value;
						push(g_curObjStack, pGate->inList[k]);
					} 
					break;
				} /* switch */
			} /* while */
		} /* if */
	} /* while */
}

/*	justify_free_lines
	Justifies free lines of the circuit.
*/
void justify_free_lines(int iNoPI, FAULTPTR pFault, FAULTPTR pCurFault)
{
	register int i, j, k;
	register GATEPTR pGate;
	level iValue, gtype;

	for (i = 0; i < iNoPI; i++)
	{
		if (g_iHeadGateIndex[i] < 0) //never happen!!
		{
			break;
		}
		pGate = g_net[g_iHeadGateIndex[i]];
		if (pGate == pCurFault->gate && pFault != NULL)
		{
			restore_faults(pFault);
			continue;
		}
		if (pGate->output == D)
		{
			pGate->output = ONE;
		}
		else if (pGate->output == DBAR)
		{
			pGate->output = ZERO;
		}
		if (!(pGate->type == PI || pGate->output == X)) //pGate->type != PI && pGate->output != X
		{
			//	  	headlines1[i]=p->output;
			clear(g_curObjStack);
			push(g_curObjStack, pGate);
			while (!is_empty(g_curObjStack))
			{
				pGate = pop(g_curObjStack);
				switch (pGate->type)
				{
				case PI:
					break;
				case XOR:
					pGate->inList[0]->output = ZERO;
					push(g_curObjStack, pGate->inList[0]);
					for (j = 1; j< pGate->inCount; j++)
					{
						pGate->inList[j]->output = pGate->output;
						push(g_curObjStack, pGate->inList[j]);
					}
					break;
				case XNOR:
					pGate->inList[0]->output = ONE;
					push(g_curObjStack, pGate->inList[0]);
					for (j = 1; j< pGate->inCount; j++)
					{
						pGate->inList[j]->output = pGate->output;
						push(g_curObjStack, pGate->inList[j]);
					}
					break;
				case PO:
				case BUFF:
				case NOT:
					pGate->inList[0]->output = g_iTruthTable1[pGate->type][pGate->output];
					push(g_curObjStack, pGate->inList[0]);
					break;
				default:
					/* and,or,nor,nand */
					iValue = g_iTruthTable1[pGate->type][pGate->output];
					gtype = (pGate->type == AND || pGate->type == NAND) ? ONE : ZERO;
					if (iValue == gtype)
					{
						for (j = 0; j< pGate->inCount; j++)
						{
							pGate->inList[j]->output = iValue;
							push(g_curObjStack, pGate->inList[j]);
						}
					}
					else
					{
						k = 0;
						for (j = 1; j< pGate->inCount; j++)
							if (pGate->inList[j]->dpi< pGate->inList[k]->dpi)
							{
								k = j;
							}
						pGate->inList[k]->output = iValue;
						push(g_curObjStack, pGate->inList[k]);
					} 
					break;
				} /* switch */
			} /* while */
		} /* if */
	} /* for */
}


/*	backtrack
	Backtracs from current tree node.
	Returns FALSE if the tree is empty.
*/
bool backtrack(GATEPTR faulty_gate, int *last, int nog)
{
	register GATEPTR p;
	register int i, j;
	level value;

	while (!is_empty(g_tree))
		if (is_flagged(current_node))
		{
			delete_last(g_tree);
		}
		else
		{
			/* update & remove duplicate unjustified lines */
			for (i = g_unjustStack.last; i >= 0; i--)
			{
				p = g_unjustStack.list[i];
				if (is_justified(p))
				{
					delete(g_unjustStack, i);
				}
				else
				{
					set(p->changed);
					push(g_finalObjStack, p);
				}
			}
			while (!is_empty(g_finalObjStack))
				reset(pop(g_finalObjStack)->changed);

			/* restore and schedule events */
			value = A_NOT(current_node.gate->output);
			set(current_node.flag);
			for (i = current_node.pstack; i <= g_stack.last; i++)
			{
				p = g_stack.list[i];
				p->output = X;
				reset(p->changed);
				for (j = 0; j< p->outCount; j++)
					reset(p->outList[j]->changed);
			}
			*last = 0;
			for (i = current_node.pstack + 1; i <= g_stack.last; i++)
			{
				p = g_stack.list[i];
				for (j = 0; j< p->outCount; j++)
				{
					if (p->outList[j]->output != X)
					{
						if (!is_justified(p->outList[j]))
						{
							push(g_unjustStack, p->outList[j]);
						}
						pushevent(p->outList[j]);
					}
					if (*last< p->outList[j]->dpi)
					{
						*last = p->outList[j]->dpi;
					}
				}
			}
			g_stack.last = current_node.pstack;
			current_node.gate->output = value;
			if (is_head(current_node.gate))
			{
				set(current_node.gate->changed);
			}
			else
			{
				pushevent(current_node.gate);
			}
			schedule_output(current_node.gate);

			/* update Dfrontier */
			clear(g_DfrontierStack);
			push(g_DfrontierStack, faulty_gate);
			update_Dfrontier();

			/* update unjustified set */
			for (i = g_unjustStack.last; i >= 0; i--)
				if (g_unjustStack.list[i]->output == X)
				{
					delete(g_unjustStack, i);
				}

			/* reset xpath */
			for (i = faulty_gate->index; i < nog; i++)
				g_net[i]->xpath = 1;
			return(TRUE);
		}
	return(FALSE);
}


/*	propFault2Headline
	Propagates the fault into a head line.
	Changes the fault into equivalent fault in the head line.
*/
void propFault2Headline(FAULTPTR pFault) //faulty_line_is_free
{
	//PRECONDITION:  is_free(pGate) == TRUE
	//OUTPUT:  pFault & pGate->output
	register int i;
	register level iValue;
	register GATEPTR pGate;

	pGate = pFault->gate;
	
	if (pFault->line != OUTFAULT)
	{
		//					  pFault -------> pGate
		pGate = pGate->inList[pFault->line]; //pGate to upper side
		//pGate -------> pFault
	}
	pGate->output = (pFault->type == SA0) ? D : DBAR;


	//Always: is_free(pGate) --------------> pGate->outCount == 1
	push(g_stack, pGate->outList[0]);
	while (!is_empty(g_stack))
	{
		pGate = pop(g_stack);
		iValue = (pGate->type == AND || pGate->type == NAND) ? ONE : ZERO;
		for (i = 0; i< pGate->inCount; i++) //UP (Always:  pGate->inCount == 1)
		{
			//Execute only once for s9234tc.bench
			if (pGate->inList[i]->output == X)
			{
				pGate->inList[i]->output = iValue;
			}
			else
			{
				pGate->output = g_iTruthTable1[pGate->type][pGate->inList[i]->output]; //Always:  i == 0
			}
		}	
		if (is_free(pGate))
		{
			push(g_stack, pGate->outList[0]); //DOWN
		}
	}
	
	set(pGate->changed);
	push(g_stack, pGate);
	schedule_output(pGate);
	pFault->gate = pGate;
	pFault->line = OUTFAULT; //pGate -------> pFault
	pFault->type = (pGate->output == D) ? SA0 : SA1; //Why this?
}

/*	fan
	Fig 9 main flowchart of the FAN algorithm.
	Main for the FAN algorithm.
*/
status fan(int iNoGate, int iMaxLevelAdd2, int iNoPI, int iNoPO, FAULTPTR pCurrentFault, int iMaxBackTrack,
		   int *piNoBackTrack)
{
	//INPUT:  pCurrentFault
	//OUTPUT:  iState & g_iPatternsForOneTime & g_net[i] (PI)
	register int i;
	register GATEPTR pGate, pTempGate;
	int iLastDPI;
	GATEPTR pLastDFtrGate;
	bool bBackwardFlag, bBacktraceFlag, bFaultPropagatedToPO;
	bool bDfrontierChanged, bDone;
	status iState;
	FAULTPTR pOriginFault = NULL;

	*piNoBackTrack = 0;
	reset(bDone);
	reset(bBackwardFlag);
	set(bBacktraceFlag);
	reset(bFaultPropagatedToPO);
	pLastDFtrGate = NULL;

	pGate = pCurrentFault->gate;

	initNetAndFreach(iNoGate, pGate, iMaxLevelAdd2);		/* initializaiton */

	if (pCurrentFault->line != OUTFAULT) //pGate -----> pCurrentFault -----> pGate
	{
		//					  pCurrentFault -------> pGate
		pGate = pGate->inList[pCurrentFault->line]; //pGate to upper side
		//pGate -------> pCurrentFault
	}
	
	if (is_free(pGate)) //come here only once for s9234tc.bench,       pGate->outCount == 1
	{
		/* box 1 */
		pOriginFault = (FAULTPTR)malloc(sizeof(FAULTTYPE));
		pOriginFault->gate = pCurrentFault->gate;
		pOriginFault->line = pCurrentFault->line;
		pOriginFault->type = pCurrentFault->type;
		propFault2Headline(pCurrentFault);
		iLastDPI = 0;
	}
	else
	{
		iLastDPI = refFaultyGateOutput(pCurrentFault);
	}
	
	if (iLastDPI == (-1)) //Never come here !!
	{
		//STOP***********************STOP
		return(NO_TEST);
	}

	pGate = pCurrentFault->gate; //pGate comes back to pCurrentFault->gate
	
#ifdef LEARNFLG
	if (pGate->plearn != NULL && pGate->output != X) //Never come here !! pGate->plearn === NULL
	{
		//STOP***********************STOP
		switch (imply_learn1(pGate, pGate->output))
		{
		case CONFLICT:
			return(NO_TEST); break;
		case BACKWARD:
			iLastDPI = pGate->dpi; break;
		}
	}
#endif

	push(g_DfrontierStack, pGate);
	i = unique_sensitize(pGate, pGate);
	if ((iLastDPI = max(i, iLastDPI)) > 0)
	{
		set(bBackwardFlag);
	}
	iState = 93;

	/* main loop of fan algorithm */
	while (bDone == FALSE)
	{
		switch (iState)
		{
		case 93:
			/* box 3,4,5,6 */

			if (!implyForwardAndBackward(iMaxLevelAdd2, bBackwardFlag, iLastDPI, pCurrentFault))
			{
				/* box 3 */
				iState = 98;
				break;
			}
			if (pGate->output == ZERO || pGate->output == ONE)
			{
				iState = 98; break;
			}

			/* update unjustified lines and delete duplicated lines 
																	final_obj should be empty */
			for (i = g_unjustStack.last; i >= 0; i--)
			{
				pTempGate = g_unjustStack.list[i];
				if (is_justified(pTempGate))
				{
					delete(g_unjustStack, i);
				}
				else
				{
					set(pTempGate->changed);
					push(g_finalObjStack, pTempGate);
				}
			}
			while (!is_empty(g_finalObjStack))
				reset(pop(g_finalObjStack)->changed);

			/* check for backtrace */
			for (i = g_initObjStack.last; i >= 0; i--)
				if (is_justified(g_initObjStack.list[i]))
				{
					delete(g_initObjStack, i);
				}

			reset(bFaultPropagatedToPO);
			for (i = 0; i < iNoPO; i++)
				if (g_net[g_PrimaryOut[i]]->output == D || g_net[g_PrimaryOut[i]]->output == DBAR)
				{
					set(bFaultPropagatedToPO);
					break;
				}

			if (pLastDFtrGate != NULL)
			{
				if (pLastDFtrGate->output == X)
				{
					reset(bDfrontierChanged);
				}
				else
				{
					set(bDfrontierChanged);
				}
			}
			else
			{
				set(bDfrontierChanged);
			}

			if (is_empty(g_initObjStack) && bDfrontierChanged)	/* box 4, 4-1 */
			{
				set(bBacktraceFlag);
			}

			if (bFaultPropagatedToPO)
			{
				iState = 99;				/* box 4-3 */
				for (i = g_unjustStack.last; i >= 0; i--)
					if (is_unjustified(g_unjustStack.list[i]) && is_bound(g_unjustStack.list[i]))
					{
						iState = 97;
						break;
					}
			}
			else
			{
				/* box 5 */
				/* update Dfrontier */
				if (!is_empty(g_DfrontierStack))
				{
					update_Dfrontier();
				}
				for (i = pGate->index; i < iNoGate; i++)
					if (g_net[i]->xpath == 2)
					{
						g_net[i]->xpath = 1;
					}
				for (i = g_DfrontierStack.last; i >= 0; i--)
					if (!Xpath(g_DfrontierStack.list[i]))
					{
						delete(g_DfrontierStack, i);
					}

				if (is_empty(g_DfrontierStack))
				{
					iState = 98;
				}
				else if (g_DfrontierStack.last == 0)
				{
					/* box 6 */
					if ((iLastDPI = unique_sensitize(g_DfrontierStack.list[0], pGate)) > 0)
					{
						set(bBackwardFlag);
						iState = 93;
					}
					else if (iLastDPI == 0)
					{
						iState = 93;
					}
					else
					{
						iState = 97;
					}
				}
				else
				{
					iState = 97;
				}
			}
			break;

		case 97:
			/* box 7 */

			find_final_objective(&bBacktraceFlag, bFaultPropagatedToPO, iNoGate, &pLastDFtrGate);

			while (!is_empty(g_finalObjStack))
			{
				pTempGate = pop(g_finalObjStack);
				if (pTempGate->numzero > pTempGate->numone)
				{
					pTempGate->output = ZERO;
				}
				else
				{
					pTempGate->output = ONE;
				}
				(g_tree.last)++;
				current_node.gate = pTempGate;
				reset(current_node.flag);
				push(g_stack, pTempGate);
				if (is_head(pTempGate))
				{
					set(pTempGate->changed);
				}
				else
				{
					pushevent(pTempGate);
				}		/**** study ****/
				schedule_output(pTempGate);
				current_node.pstack = g_stack.last;
			}

			reset(bBackwardFlag);
			iState = 93;
			break;

		case 98:
			/* box 8 */

			if (is_flagged(current_node))
			{
				(*piNoBackTrack)++;
			}
			for (i = 0; i < iMaxLevelAdd2; i++)
				while (!is_empty(g_pEventListStack[i]))
					reset(pop(g_pEventListStack[i])->changed);
			if (*piNoBackTrack > iMaxBackTrack)
			{
				iState = OVER_BACKTRACK;
			}
			else if (backtrack(pGate, &iLastDPI, iNoGate))
			{
				if (iLastDPI > 0)
				{
					set(bBackwardFlag);
				}
				else
				{
					reset(bBackwardFlag);
				}
				set(bBacktraceFlag);
				pLastDFtrGate = NULL;
				iState = 93;
			}
			else
			{
				iState = NO_TEST;
			}
			break;

		case 99:
			justify_free_lines(iNoPI, pOriginFault, pCurrentFault);
			iState = TEST_FOUND;
			if (no_faultsim == 'y')
			{
				Dprintio(g_fpTestFile, iNoPI, iNoPO, ++g_iNoPatternsForOneTime);
				if (logmode == 'y')
				{
					Dprintio(g_fpLogFile, iNoPI, iNoPO, g_iNoPatternsForOneTime);
				}
				if (gen_all_pat == 'y')
				{
					if (ntest_each_limit <= 0 || g_iNoPatternsForOneTime < ntest_each_limit)
					{
						*piNoBackTrack = 0;
						iState = 98;
					}
				}
			}
			break;
		default:
			set(bDone);
		}
	}

	if (pOriginFault != NULL)
	{
		pCurrentFault->gate = pOriginFault->gate;
		pCurrentFault->line = pOriginFault->line;
		pCurrentFault->type = pOriginFault->type;
		free((char*)pOriginFault);
	}
	return(iState);
}


/*	fan1
	Fig 10 main flowchart of the FAN algorithm.
	Main for the FAN algorithm with the dynamic unique path sensitization.
*/
status fan1(int nog, int maxdpi, int npi, int npo, FAULTPTR cf, int maxbacktrack, int *nbacktrack)
{
	register int i;
	register GATEPTR gut, g;
	int last_dpi;
	GATEPTR last_Dfrontier;
	bool backward_flag, backtrace_flag, fault_propagated_to_po;
	bool Dfrontier_changed, done;
	status state;
	FAULTPTR original = NULL;

	*nbacktrack = 0;
	reset(done);
	reset(backward_flag);
	set(backtrace_flag);
	reset(fault_propagated_to_po);
	last_Dfrontier = NULL;

	gut = cf->gate;

	initNetAndFreach(nog, gut, maxdpi);		/* initializaiton */

	if (cf->line != OUTFAULT)
	{
		gut = gut->inList[cf->line];
	}
	if (is_free(gut))
	{
		/* box 1 */
		original = (FAULTPTR)malloc(sizeof(FAULTTYPE));
		original->gate = cf->gate;
		original->line = cf->line;
		original->type = cf->type;
		propFault2Headline(cf);
		last_dpi = 0;
	}
	else
	{
		last_dpi = refFaultyGateOutput(cf);
	}
	if (last_dpi == (-1))
	{
		return(NO_TEST);
	}

	gut = cf->gate;
#ifdef LEARNFLG
	if (gut->plearn != NULL && gut->output != X)
	{
		switch (imply_learn1(gut, gut->output))
		{
		case CONFLICT:
			return(NO_TEST); break;
		case BACKWARD:
			last_dpi = gut->dpi; break;
		}
	}
#endif

	push(g_DfrontierStack, gut);
	i = unique_sensitize(gut, gut);
	if ((last_dpi = max(i, last_dpi)) > 0)
	{
		set(backward_flag);
	}
	state = 93;

	/* main loop of fan algorithm */
	while (done == FALSE)
	{
		switch (state)
		{
		case 93:
			/* box 3,4,5,6 */

			if (!implyForwardAndBackward(maxdpi, backward_flag, last_dpi, cf))
			{
				/* box 3 */
				state = 98;
				break;
			}
			if (gut->output == ZERO || gut->output == ONE)
			{
				state = 98; break;
			}

			/* update unjustified lines and delete duplicated lines 
																	final_obj should be empty */
			for (i = g_unjustStack.last; i >= 0; i--)
			{
				g = g_unjustStack.list[i];
				if (is_justified(g))
				{
					delete(g_unjustStack, i);
				}
				else
				{
					set(g->changed);
					push(g_finalObjStack, g);
				}
			}
			while (!is_empty(g_finalObjStack))
				reset(pop(g_finalObjStack)->changed);

			/* check for backtrace */
			for (i = g_initObjStack.last; i >= 0; i--)
				if (is_justified(g_initObjStack.list[i]))
				{
					delete(g_initObjStack, i);
				}

			reset(fault_propagated_to_po);
			for (i = 0; i < npo; i++)
				if (g_net[g_PrimaryOut[i]]->output == D || g_net[g_PrimaryOut[i]]->output == DBAR)
				{
					set(fault_propagated_to_po);
					break;
				}

			if (last_Dfrontier != NULL)
			{
				if (last_Dfrontier->output == X)
				{
					reset(Dfrontier_changed);
				}
				else
				{
					set(Dfrontier_changed);
				}
			}
			else
			{
				set(Dfrontier_changed);
			}

			if (is_empty(g_initObjStack) && Dfrontier_changed)	/* box 4, 4-1 */
			{
				set(backtrace_flag);
			}

			if (fault_propagated_to_po)
			{
				state = 99;				/* box 4-3 */
				for (i = g_unjustStack.last; i >= 0; i--)
					if (is_unjustified(g_unjustStack.list[i]) && is_bound(g_unjustStack.list[i]))
					{
						state = 97;
						break;
					}
			}
			else
			{
				/* box 5 */
				/* update Dfrontier */
				if (!is_empty(g_DfrontierStack))
				{
					update_Dfrontier();
				}
				for (i = gut->index; i < nog; i++)
					if (g_net[i]->xpath == 2)
					{
						g_net[i]->xpath = 1;
					}
				for (i = g_DfrontierStack.last; i >= 0; i--)
					if (!Xpath(g_DfrontierStack.list[i]))
					{
						delete(g_DfrontierStack, i);
					}

				if (is_empty(g_DfrontierStack))
				{
					state = 98;
				}
				/*when dfrontier is not zero*/
				else
				{
					/* box 6 */
					if (dy_id >= INFINITY - 3)
					{
						for (i = 0; i < nog; i++)
							g_net[i]->freach1 = 0;
						dy_id = 0;
					}
					if ((last_dpi = dynamic_unique_sensitize(g_DfrontierStack.list, g_DfrontierStack.last, maxdpi, dyn_dom, gut)) > 0)
					{
						set(backward_flag);
						state = 93;
					}
					else if (last_dpi == 0)
					{
						state = 93;
					}
					else
					{
						state = 97;
					}
				}
			}
			break;

		case 97:
			/* box 7 */

			find_final_objective(&backtrace_flag, fault_propagated_to_po, nog, &last_Dfrontier);

			while (!is_empty(g_finalObjStack))
			{
				g = pop(g_finalObjStack);
				if (g->numzero > g->numone)
				{
					g->output = ZERO;
				}
				else
				{
					g->output = ONE;
				}
				(g_tree.last)++;
				current_node.gate = g;
				reset(current_node.flag);
				push(g_stack, g);
				if (is_head(g))
				{
					set(g->changed);
				}
				else
				{
					pushevent(g);
				}		/**** study ****/
				schedule_output(g);
				current_node.pstack = g_stack.last;
			}

			reset(backward_flag);
			state = 93;
			break;

		case 98:
			/* box 8 */

			if (is_flagged(current_node))
			{
				(*nbacktrack)++;
			}
			for (i = 0; i < maxdpi; i++)
				while (!is_empty(g_pEventListStack[i]))
					reset(pop(g_pEventListStack[i])->changed);
			if (*nbacktrack > maxbacktrack)
			{
				state = OVER_BACKTRACK;
			}
			else if (backtrack(gut, &last_dpi, nog))
			{
				if (last_dpi > 0)
				{
					set(backward_flag);
				}
				else
				{
					reset(backward_flag);
				}
				set(backtrace_flag);
				last_Dfrontier = NULL;
				state = 93;
			}
			else
			{
				state = NO_TEST;
			}
			break;

		case 99:
			justify_free_lines(npi, original, cf);
			state = TEST_FOUND;
		default:
			set(done);
		}
	}

	if (original != NULL)
	{
		cf->gate = original->gate;
		cf->line = original->line;
		cf->type = original->type;
		free((char*)original);
	}
	return(state);
}
