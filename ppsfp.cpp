
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

/*-----------------------------------------------------------------------
	ppsfp.c
	Contains all subroutines for the parallel pattern single
	fault propagation.
	Based on Dominator concept.
------------------------------------------------------------------------*/
#include "stdafx.h"
#include <stdio.h>

#include "ppsfp.h"

#include "parameter.h" 
#include "define.h"    
#include "macro.h"

#define output0 output

extern GATEPTR *g_net;
extern int g_iNoGate;
extern STACKPTR g_pEventListStack;
extern STACKTYPE g_freeGatesStack,	/* fault free simulation */
				 g_faultyGatesStack,   	  /* list of faulty gates */
				 g_evalGatesStack, 		  /* STEM_LIST to be simulated */
				 g_activeStemStack;  	   /* list of active stems */
extern STACKTYPE g_stack;
extern GATEPTR *g_dynamicStack;
extern int g_iSStack, g_iDStack;
extern level g_iAllOne;
extern level BITMASK[BITSIZE];
extern status g_iUpdateFlag, g_iUpdateFlag2;
extern int g_iMaxLevel;

/* macros for parallel gate evaluation */
/* one input gate */
//pgate_eval1
#define evalGateWithInputs1(pGate, iValue) \
	iValue = (pGate->type == NOT || pGate->type == NAND || pGate->type == NOR)? ~pGate->inList[0]->output1 : \
	pGate->inList[0]->output1

/* two input gates */
//pgate_eval2
#define evalGateWithInputs2(pGate, iValue) \
	switch (pGate->type) \
	{ \
	   case AND: iValue = pGate->inList[0]->output1 & pGate->inList[1]->output1; break; \
	   case NAND: iValue = ~(pGate->inList[0]->output1 & pGate->inList[1]->output1); break; \
	   case OR: iValue = pGate->inList[0]->output1 | pGate->inList[1]->output1; break; \
	   case NOR: iValue = ~(pGate->inList[0]->output1 | pGate->inList[1]->output1); break; \
	   case XOR: iValue = pGate->inList[0]->output1 ^ pGate->inList[1]->output1; break; \
	   case XNOR: iValue = ~(pGate->inList[0]->output1 ^ pGate->inList[1]->output1); break; \
	}

/* 3-input gate */
//pgate_eval3
#define evalGateWithInputs3(pGate,iValue) \
	switch (pGate->type) \
	{ \
	   case AND: iValue = pGate->inList[0]->output1 & pGate->inList[1]->output1 & pGate->inList[2]->output1; break; \
	   case NAND: iValue = ~(pGate->inList[0]->output1 & pGate->inList[1]->output1 & pGate->inList[2]->output1); break; \
	   case OR: iValue = pGate->inList[0]->output1 | pGate->inList[1]->output1 | pGate->inList[2]->output1; break; \
	   default: iValue = ~(pGate->inList[0]->output1 | pGate->inList[1]->output1 | pGate->inList[2]->output1); \
	}

/* more than 2-inputs */
//pgate_evalx
#define evalGateWithInputsX(pGate, iValue, i) \
switch(pGate->type) { \
   case AND: \
	  iValue=pGate->inList[0]->output1&pGate->inList[1]->output1&pGate->inList[2]->output1; \
	  for(i=3;i<pGate->inCount;i++) iValue&=pGate->inList[i]->output1; break; \
   case NAND: \
	  iValue=pGate->inList[0]->output1&pGate->inList[1]->output1&pGate->inList[2]->output1; \
	  for(i=3;i<pGate->inCount;i++) iValue&=pGate->inList[i]->output1; \
	  iValue=~iValue; break; \
   case OR: \
	  iValue=pGate->inList[0]->output1|pGate->inList[1]->output1|pGate->inList[2]->output1; \
	  for(i=3;i<pGate->inCount;i++) iValue|=pGate->inList[i]->output1; break; \
   case NOR: \
	  iValue=pGate->inList[0]->output1|pGate->inList[1]->output1|pGate->inList[2]->output1; \
	  for(i=3;i<pGate->inCount;i++) iValue|=pGate->inList[i]->output1; \
	  iValue=~iValue; \
	  break; \
   case XOR: iValue=pGate->inList[0]->output1^pGate->inList[1]->output1; break; \
   case XNOR: iValue=~(pGate->inList[0]->output1^pGate->inList[1]->output1); break; \
}

/* more than 4-inputs */
//pgate_eval4
#define evalGateWithInputs4(pGate, iValue, i) \
switch(pGate->type) { \
   case AND: \
	  iValue=pGate->inList[0]->output1&pGate->inList[1]->output1&pGate->inList[2]->output1&pGate->inList[3]->output1; \
	  for(i=4;i<pGate->inCount;i++) iValue&=pGate->inList[i]->output1; break; \
   case NAND: \
	  iValue=pGate->inList[0]->output1&pGate->inList[1]->output1&pGate->inList[2]->output1&pGate->inList[3]->output1; \
	  for(i=4;i<pGate->inCount;i++) iValue&=pGate->inList[i]->output1; \
	  iValue=~iValue; break; \
   case OR: \
	  iValue=pGate->inList[0]->output1|pGate->inList[1]->output1|pGate->inList[2]->output1|pGate->inList[3]->output1; \
	  for(i=4;i<pGate->inCount;i++) iValue|=pGate->inList[i]->output1; break; \
   default: \
	  iValue=pGate->inList[0]->output1|pGate->inList[1]->output1|pGate->inList[2]->output1|pGate->inList[3]->output1; \
	  for(i=4;i<pGate->inCount;i++) iValue|=pGate->inList[i]->output1; \
	  iValue=~iValue; \
}

/* macros for event scheduling */
//pschedule_output
#define pushGateOutputs(pGate, i) \
for (i = 0; i < pGate->outCount; i++) \
{ \
   g_pTempGate = pGate->outList[i]; \
   if (!g_pTempGate->changed) \
   { \
	  push(g_pEventListStack[g_pTempGate->dpi], g_pTempGate); \
	  set(g_pTempGate->changed); \
   } \
}

/* macro to delete a fault */
#define delete_fault(pFault) \
	if (pFault->previous == pFault) \
	{ \
		pFault->gate->pfault = pFault->next; \
		if(pFault->next != NULL) \
		{ \
			pFault->next->previous = pFault->next; \
		} \
	} \
	else \
	{ \
		pFault->previous->next = pFault->next; \
		if(pFault->next != NULL) \
		{ \
			pFault->next->previous = pFault->previous; \
		} \
	}


GATEPTR g_pTempGate;

/*	update_all
	Dynamically updates all lists
		  --- faulty_gates, free_gates and eval_gates
	Input: current faulty_agtes
	Output: updated faulty_gates
	Note: Gates are stored in the forward levelized order.
*/

//STOP*************************STOP -> NO USE
void update_all(int npi)
{
	register int i, j, k;
	register GATEPTR gut;

	/* clear freach */
	for (i = 0; i <= g_freeGatesStack.last; i++)
		reset(g_freeGatesStack.list[i]->freach);
	for (i = 0; i < npi; i++)
		reset(g_net[i]->freach);

	/* faulty gates */
	j = g_faultyGatesStack.last + 1;
	clear(g_faultyGatesStack);
	for (i = 0; i < j; i++)
	{
		gut = g_faultyGatesStack.list[i];
		if (gut->nfault > 0)
		{
			push(g_faultyGatesStack, gut);
			while (!gut->freach)
			{
				push(g_stack, gut);
				set(gut->freach);
				if (gut->outCount == 1)
				{
					gut = gut->outList[0];
				}
				else if (gut->u_path != NULL)
				{
					gut = gut->u_path->ngate;
				}
			}
		}
	}

	/* evaluation gate list */
	j = g_evalGatesStack.last + 1;
	clear(g_evalGatesStack);
	for (i = 0; i < j; i++)
	{
		gut = g_evalGatesStack.list[i];
		if (gut->freach)
		{
			push(g_evalGatesStack, gut);
		}
	}

	/* fault free gate list */
	while (!is_empty(g_stack))
	{
		gut = pop(g_stack);
		for (i = 0; i< gut->outCount; i++)
			if (!gut->outList[i]->freach)
			{
				set(gut->outList[i]->freach);
				push(g_stack, gut->outList[i]);
			}
	}

	k = g_freeGatesStack.last;
	clear(g_freeGatesStack);
	for (i = 0; i <= k; i++)
	{
		gut = g_freeGatesStack.list[i];
		if (gut->freach)
		{
			reset(gut->freach);
			push(g_freeGatesStack, gut);
			for (j = 0; j< gut->inCount; j++)
				set(gut->inList[j]->freach);
		}
	}

	/* schedule of freach */
	for (i = 0; i < npi; i++)
		reset(g_net[i]->freach);

	g_iSStack = (-1);
	for (i = 0; i <= g_faultyGatesStack.last; i++)
	{
		gut = g_faultyGatesStack.list[i];
		while (!gut->freach)
		{
			set(gut->freach);
			g_dynamicStack[++g_iSStack] = gut;
			if (gut->outCount == 1)
			{
				gut = gut->outList[0];
			}
		}
	}
	g_iDStack = g_iSStack;
}

/*	update_free_gates
	Update the list free_gates which contains gates to be evaluated.
	Input: faulty_gates, list of faulty gates
		   free_gates, current list
	Output: free_gates, list is updated
	Notes: Gates are in the reverse levelized order.
		   Primary inputs are not included in free_gates.
*/
//STOP*************************STOP -> NO USE
void update_free_gates(int npi)
{
	int j, k;
	register int i;
	register GATEPTR gut;

	for (i = g_faultyGatesStack.last; i >= 0; i--)
	{
		g_stack.list[i] = g_faultyGatesStack.list[i];
		set(g_stack.list[i]->changed);
	}
	g_stack.last = g_faultyGatesStack.last;

	while (!is_empty(g_stack))
	{
		gut = pop(g_stack);
		for (i = 0; i< gut->outCount; i++)
			if (!gut->outList[i]->changed)
			{
				set(gut->outList[i]->changed);
				push(g_stack, gut->outList[i]);
			}
	}

	k = g_freeGatesStack.last;
	clear(g_freeGatesStack);
	for (i = 0; i <= k; i++)
	{
		gut = g_freeGatesStack.list[i];
		if (gut->changed)
		{
			push(g_freeGatesStack, gut);
			reset(gut->changed);
			for (j = 0; j< gut->inCount; j++)
				set(gut->inList[j]->changed);
		}
	}
	for (i = 0; i < npi; i++)
		reset(g_net[i]->changed);
}

/*	pinit_simulation
	Initializes necessary flags and counters.
	changed=0, observe=ALL0 clear g_pEventListStack 
*/
void initGateStackAndFreach(int iNoGate, int iMaxLevelAdd2, int iNoPI) //pinit_simulation
{
	register int i;
	register GATEPTR pGate;

	/* clear changed ochange and set freach */
	for (i = 0; i < iNoGate; i++)
	{
		reset(g_net[i]->changed); 
		reset(g_net[i]->freach);
		g_net[i]->cobserve = ALL0;
		g_net[i]->observe = ALL0;
	}

	/* clear all sets */
	for (i = 0; i < iMaxLevelAdd2; i++)
	{
		clear(g_pEventListStack[i]);
	}
	clear(g_stack);
	clear(g_freeGatesStack);
	clear(g_faultyGatesStack);
	clear(g_evalGatesStack);
	clear(g_activeStemStack);

	/* initialize all stacks */
	for (i = 0; i < iNoGate; i++)
	{
		pGate = g_net[i]; 
		if (pGate->nfault > 0) //Faulty
		{
			push(g_faultyGatesStack, pGate); //Core sentence
		}
		if (pGate->outCount != 1) //PO && FANOUT
		{
			push(g_evalGatesStack, pGate); //Core sentence
		}
	}
	for (i = iNoGate - 1; i >= iNoPI; i--) //NO PI
	{
		push(g_freeGatesStack, g_net[i]);
	}

	/* schedule freach */
	g_iSStack = (-1);
	for (i = 0; i <= g_faultyGatesStack.last; i++)
	{
		pGate = g_faultyGatesStack.list[i];
		while (!pGate->freach)
		{
			set(pGate->freach); //Core sentence
			g_dynamicStack[++g_iSStack] = pGate; //Core sentence
			if (pGate->outCount == 1) //Spread freach within the FFR !!
			{
				pGate = pGate->outList[0];
			}
		}
	}
	g_iDStack = g_iSStack;
}

/* restoreOutput1FromOutput: Restores fault free values */
void restoreOutput1FromOutput() //restore_fault_free_va1lue
{
	register GATEPTR pGate;

	while (!is_empty(g_stack))
	{
		pGate = pop(g_stack);
		pGate->output1 = pGate->output;
		//output1:  Fault Value
		//outout:   Sim Value
	}
}

/*	pfault_free_simulation
	Perform fault free simulation.
	Only gates in free_gates are evaluated
	Inputs: test vectors should be put to output1 and output0
	Outputs: output0 and output1
	Notes: Resets freach flag for the fault dropping
*/
void evalGatesFromFreeStack() //pfault_free_simula1tion
{
	//INPUT:  g_freeGatesStack
	//OUTPUT: pGate->output & pGate->output1
	register int i, j;
	register GATEPTR pGate;
	register level iValue;

	for (i = g_freeGatesStack.last; i >= 0; i--) //g_freeGatesStack ---------> Not PI
	{
		//PI's output is just the input !!
		pGate = g_freeGatesStack.list[i];
		switch (pGate->inCount)
		{
		case 1:
			evalGateWithInputs1(pGate, pGate->output1);
			break;
		case 2:
			evalGateWithInputs2(pGate, pGate->output1);
			break;
		case 3:
			evalGateWithInputs3(pGate, pGate->output1);
			break;
		default: // >= 4
			evalGateWithInputs4(pGate, iValue, j);
			pGate->output1 = iValue;
			break;
		}
		pGate->output = pGate->output1;
	}
}

/*	pfault_simulation
	Performs fault simulation of the stem fault to the Dominator.
	
	Inputs:
		gut: gate under test
		observe: observability of the gate under test
		Dominator: Dominator gate
		maxdpi: depth of evaluation stack (circuit)
	Output: observe word of fault simulation

	Notes:
		1. If observe=ALL0, no simulation.
		   If observe=ALL1, simulates both s-a-1 and s-a-0
		   If observe=~free_value, simulates s-a-1
		   If observe=free_value, simulates s-a-0
		   Else, simulates observe^free_value
		2. If Dominator=NULL, simulates upto primary outputs.
*/
level faultSimForStemGate(register GATEPTR pGate, level iObserve, GATEPTR pDomiGate, int iMaxDPI) //pfault_simulation
// register GATEPTR pGate;		/* faulty gate */
// level iObserve;		/* determines faulty value of the pGate */
// GATEPTR pDomiGate;
// int iMaxDPI;		/* depth of event list, number of output */
{
	//Fault0_Simulation's Step 1:  pGate->output  == pGate->output 1
	
	//INPUT:  pGate, pGate->observe, pGate->u_path->ngate (or NULL)
	//OUTPUT:  pGate->observe (from dominator or PO)
	register int i, j;
	register level iValue;

	//iObserve = pGate->observe, iObserve is the difference between Fault-Free and Faulty !!
	if (iObserve == ALL0)
	{
		return(iObserve);
	}
	
	pGate->output1 ^= iObserve; //Differentiate with pGate->output !!!
	//pGate->output & pGate->observe ------> pGate->output1
	
	push(g_stack, pGate); //g_stack is the changed pGate->output1 !!!
	pushGateOutputs(pGate, i);
	if (pGate->type != PO)
	{
		iObserve = ALL0; //Recalculate !!
	}
	if (pDomiGate != NULL)
	{
		iMaxDPI = pDomiGate->dpi + 1;
	}

	/* evaluate event list */
	//for (i = pGate->dpi + 1; i < g_iMaxLevel + 2; i++)
	for (i = pGate->dpi + 1; i < iMaxDPI; i++) //This optimization is NO BIG USE !!!
	{
		while (!is_empty(g_pEventListStack[i])) //EXIT1-2:  NO pDominator !!!
		{
			pGate = pop(g_pEventListStack[i]);
			reset(pGate->changed);

			//Recalculate pGate's output
			switch (pGate->inCount)
			{
			case 1:
				evalGateWithInputs1(pGate, iValue); //NOT support D/DBAR, support PPSFP
				break;
			case 2:
				evalGateWithInputs2(pGate, iValue);
				break;
			default:
				evalGateWithInputsX(pGate, iValue, j);
			}


			//EXIT2:  pDominator !!!	---> update stem observe from pDominator
			if (pGate == pDomiGate) //pDomiGate CANNOT be PO !!!
			{
				restoreOutput1FromOutput();
				return(iObserve | (iValue ^ pGate->output1)); //Collect all output dismatch to iObserve !!
			}

			
			//EXIT1-1:  NO pDominator !!!	---> update stem observe from POs (Union Operation)
			/* else if */
			if (pGate->type == PO)
			{
				iObserve |= (iValue ^ pGate->output1); //Collect all output dismatch to iObserve !!
			}

			//NORMAL
			/* else if */
			if (iValue != pGate->output1) //If the output1 updates, refresh the output1 and push pGate into event list !!
			{
				pushGateOutputs(pGate, j);
				pGate->output1 = iValue;
				push(g_stack, pGate);
			}
		}
	}
	//EXIT1-3:  NO pDominator !!!
	restoreOutput1FromOutput();
	return(iObserve);
}

/* getFaultObserveFromInput: faulty gate evaluation */
level getFaultObserveFromInput(FAULTPTR pFault, register GATEPTR pGate) //feval
{
	//INPUT:  pFault & pGate->inList
	//OUTPUT:  pGate's (input) observe
	
	//pFault is supposed to be an input fault !!
	//pFault -------------> pGate
	register level iValue;
	register int i;
	GATEPTR pInGate;

	pInGate = pGate->inList[pFault->line]; //(pInGate ---> pFault) ---> pGate
	if ((iValue = pInGate->output1 ^ (pFault->type == SA0 ? ALL0 : ALL1)) == ALL0)
	{
		//pInGate->output1 == SA0 | SA1, NO hope to detect.
		return(iValue); //iValue = ALL0
	}


	//pInGate ---> (pFault ---> pGate)
	//Other Gates ----------->pGate
	if (pGate->inCount == 2)
	{
		if (pGate->type <= NAND) //AND NAND
		{
			return(iValue & (pFault->line == 0 ? pGate->inList[1]->output1 : pGate->inList[0]->output1));
		}
		else if (pGate->type <= NOR) //OR NOR
		{
			return(iValue & (pFault->line == 0 ? ~pGate->inList[1]->output1 : ~pGate->inList[0]->output1));
		}
		else
		{
			return(iValue);
		}
	}

	//pGate->inCount == 1 && pGate->inCount > 2
	pInGate->output1 = pGate->inList[0]->output;
	switch (pGate->type)
	{
	case AND:
	case NAND:
		for (i = 1; i< pGate->inCount; i++)
			iValue &= pGate->inList[i]->output1;
		break;
	case OR:
	case NOR:
		for (i = 1; i< pGate->inCount; i++)
			iValue &= (~pGate->inList[i]->output1);
		break;
	}
	pInGate->output1 = pInGate->output; //Why this
	return(iValue);
}

/*	pcheck_fault
	Determine the detectability of a fault at the stem  of the gate.
	If stem is a PO, enumerates number of detected faults

	Inputs: gut->observe: detectability array of gut
	Outputs: cumulative detectability of the faults
	Note: Pushes detectable faults to stem buffer
*/

level updateFaultObserveByFFRGate(register GATEPTR pGate, FAULTPTR **pppFault, level iStemObserve) //pcheck_fault
// register GATEPTR pGate;
// FAULTPTR **pppFault;		/* pointer of stem buffer */
// level iObserve;		/* cumulative detectability of the stem */
{
	//INPUT:  pGate
	//OUTPUT:  ppFault & iObserve & pFault->observe

	//pFault->type + pGate->output1 ---> pFault->observe
	
	FAULTPTR pFault;
	level iTempObserve;

	for (pFault = pGate->pfault; pFault != NULL; pFault = pFault->next) //For every pFault
	{
		iTempObserve = pGate->observe;
		/* output fault */
		if (pFault->line == OUTFAULT) //pGate ---> pFault ---> iTempObserve
		{
			iTempObserve &= ((pFault->type == SA0) ? pGate->output1 : ~pGate->output1);
		}
		/* input fault */
		else //pFault ---> pGate
		{
			iTempObserve &= getFaultObserveFromInput(pFault, pGate);
		}

		pFault->observe = iTempObserve;
		if (iTempObserve != ALL0)
		{
			**pppFault = pFault; //The faults that contribute for observe
			(*pppFault)++;
			//*ppFault = pFault;
			//ppFault++;
			iStemObserve |= iTempObserve; //Union between different faults for one gate !!
		}
	}
	return(iStemObserve);
}

/*	pcheck_po
	Enumerates number of detected faults.

	Inputs: gut->observe: grobal detectability array of gut
	Outputs: number of detected faults
	Note:
		gut->observe should contain grobal detectability wrt
		primary outputs of the circuit.
*/
int updateFaultObserveByPOGate(register GATEPTR pGate, status *piUpdateFlag, int iBit, int iArrProfile[]) //pcheck_po
{
	//INPUT:  pGate
	//OUTPUT:  iNoDetected & iArrProfile & pFault->detected

	//pFault->type + pGate->output1 ---> pFault->observe
	
	register int i;
	int iNoDetected = 0;
	FAULTPTR pFault;
	level iTempObserve;

	for (pFault = pGate->pfault; pFault != NULL; pFault = pFault->next) //For every pFault
	{
		iTempObserve = pGate->observe;
		/* output fault */
		if (pFault->line == OUTFAULT) //pGate ---> pFault ---> iTempObserve
		{
			iTempObserve &= ((pFault->type == SA0) ? pGate->output1 : ~pGate->output1);
		}
		/* input fault */
		else //pFault ---> pGate
		{
			iTempObserve &= getFaultObserveFromInput(pFault, pGate);
		}

		//pFault->observe = iTempObserve; //NO Need to update, detection already finished !!
		if (iTempObserve != ALL0)
		{
			pFault->detected = DETECTED;
			iNoDetected++;
			for (i = iBit - 1; i >= 0; i--) //Always only execute i = 0 condition
			{
				if ((iTempObserve & BITMASK[i]) != ALL0) //iTempObserve[0] == 1
				{
					++iArrProfile[i];
					break;
				}
			}
			if (--pGate->nfault == 0)
			{
				set(*piUpdateFlag); //pGate->nfault == 0
				//No delete, special handling needed
			}
			else
			{
				delete_fault(pFault); //Only remove the pointer from the gate !!
			}
		}
	}
	return(iNoDetected);
}

/*	ftp_reverse
	Simulates fan-out free regions using critical path tracing.

	Inputs:
		stem: a fan-out stem to be evaluated
	Outputs:
		sets observe of each fault, each gate and dominator of the stem.
		observe: cumulative observability of the stem, i.e.,
			 ORs of each fault detectability and dominator obserbility.
			 If, stem is a primary output, returns detected faults.

	Notes:
		1. freach should be set before calling,
		   the ftp_reverse is performed for only those gates
		2. Try to use buffers instead of flags
		3. stem->observe should be set before calling
*/
int updateWholeStemObserveAndDetect(GATEPTR pStemGate, status *piUpdateFlag, status bIsPO, int iBit, int iArrNoDetected[]) //ftp_reverse
{
	//INPUT:  pStemGate
	//OUTPUT:  iNoDetected |(iObserve(pStemGate->observe) & ppFault)
	//iBit == 1
	//iNoDetected = iArrNoDetected[0]
	
	//update the observe of the whole stem !!!
	
	register int i, j;
	register GATEPTR pGate, pInGate;
	register level iStemObserve, iValue;
	int iNoDetected = 0;
	FAULTPTR *ppFault;

	iStemObserve = ALL0;
	ppFault = pStemGate->dfault;

	//g_stack = NULL
	push(g_stack, pStemGate);
	while (!is_empty(g_stack))
	{
		pGate = pop(g_stack);
		//
		//***************Update whole stem's faulty observe !!***************
		//
		if (pGate->nfault > 0)
		{
			if (bIsPO) //PO
			{
				//Always:  iBit == 1
				iNoDetected += updateFaultObserveByPOGate(pGate, piUpdateFlag, iBit, iArrNoDetected);
				//pGate->nfault == 0 ----> *piUpdateFlag == 1
			}
			else //FFR Gates (FANOUT or other Gate in stem)
			{
				iStemObserve = updateFaultObserveByFFRGate(pGate, &ppFault, iStemObserve);
				//pFault->type + pGate->output1 ---> pFault->observe
				//pGate->observe |= UNION(pFault->observe)
				
				//Only faults that contribute to observe remains in ppFault
			}
		}

		//No use for PO
		if (pGate->cobserve != ALL0)
		{
			iStemObserve |= (pGate->observe & pGate->cobserve);
			//The Same As:
			//iStemObserve |= pGate->cobserve;
		}
		//pStemGate->observe = iObserve !!!



		//
		//***************Update whole stem's fault-free observe !!***************
		//down gate's output1 + observe ---> up gate's observe
		
		//inputs == 1 !!
		if (pGate->inCount == 1)
		{
			pInGate = pGate->inList[0];
			if ((pInGate->outCount == 1) && pInGate->freach) //Within FFR !!
			{
				pInGate->observe = pGate->observe; //*****Algorithm*****
				push(g_stack, pInGate);
			}
		}
		//inputs == 2 !!
		else if (pGate->inCount == 2)
		{
			pInGate = pGate->inList[0];
			if ((pInGate->outCount == 1) && pInGate->freach) //Within FFR !!
			{
				switch (pGate->type)
				{
				case AND:
				case NAND:
					pInGate->observe = pGate->observe & pGate->inList[1]->output1; //*****Algorithm*****
					break;
				case OR:
				case NOR:
					pInGate->observe = pGate->observe & ~(pGate->inList[1]->output1); //*****Algorithm*****
					break;
				default:
					pInGate->observe = pGate->observe;
				}
				if (pInGate->observe != ALL0)
				{
					push(g_stack, pInGate);
				}
			}
			pInGate = pGate->inList[1];
			if ((pInGate->outCount == 1) && pInGate->freach) //Within FFR !!
			{
				switch (pGate->type)
				{
				case AND:
				case NAND:
					pInGate->observe = pGate->observe & pGate->inList[0]->output1; //*****Algorithm*****
					break;
				case OR:
				case NOR:
					pInGate->observe = pGate->observe & ~(pGate->inList[0]->output1); //*****Algorithm*****
					break;
				default:
					pInGate->observe = pGate->observe;
				}
				if (pInGate->observe != ALL0)
				{
					push(g_stack, pInGate);
				}
			}
		}
		//inputs >= 3 !!
		else
		{  
			for (i = 0; i< pGate->inCount; i++)
			{
				pInGate = pGate->inList[i];
				if ((pInGate->outCount == 1) && pInGate->freach) //Within FFR !!
				{
					pInGate->output1 = ~pInGate->output1; //*****Algorithm*****
					if (pGate->inCount == 1) //Why?? Impossible!!!  pGate->inCount >= 3
					{
						//STOP***************************STOP
						evalGateWithInputs1(pGate, iValue);
					}
					else
					{
						evalGateWithInputsX(pGate, iValue, j);
					}
					pInGate->observe = (iValue ^ pGate->output1) & pGate->observe; //*****Algorithm*****
					pInGate->output1 = pInGate->output; //Restore output1
					if (pInGate->observe != ALL0)
					{
						push(g_stack, pInGate);
					}
				}
			}
		}
	}
	*ppFault = NULL;
	if (bIsPO)
	{
		return(iNoDetected);
	}
	else
	{
		return(iStemObserve);
	}
}

/*	updateFaultyStack
	updates the list, faulty_gates, which contains faulty gates
	Input: current faulty_agtes
	Output: updated faulty_gates
	Notes: gates are stored in the forward levelized order
*/
void updateFaultyStack() //update_faulty_gates
{
	register int i, iFaultyGatesStackSizeAdd1;
	register GATEPTR pGate;

	iFaultyGatesStackSizeAdd1 = g_faultyGatesStack.last;
	clear(g_faultyGatesStack);
	for (i = 0; i <= iFaultyGatesStackSizeAdd1; i++)
	{
		pGate = g_faultyGatesStack.list[i];
		if (pGate->nfault > 0)
		{
			push(g_faultyGatesStack, pGate);
		}
	}
}

/*	updateEvalStackByFaultyStack
	Updates stem_list which contains stems to be evaluated.
	Inputs: faulty_gates, eval_gates
	Outputs: eval_gates updated
*/
void updateEvalStackByFaultyStack() //update_eval_ga1tes
{
	register int i, j;
	register GATEPTR pStemGate;

	/* set freach from each faulty gate to its stem */
	for (i = 0; i <= g_faultyGatesStack.last; i++)
	{
		pStemGate = g_net[g_faultyGatesStack.list[i]->fosIndex];
		//changed == 0 at first !!
		while (!pStemGate->changed)
		{
			set(pStemGate->changed);
			if (pStemGate->u_path != NULL)
			{
				pStemGate = g_net[pStemGate->u_path->ngate->fosIndex];
			}
		}
	}

	/* set eval_gates based on previous stacks */
	//pStemGate->changed == 1 ------------> g_evalGatesStack
	j = g_evalGatesStack.last;
	clear(g_evalGatesStack);
	for (i = 0; i <= j; i++)
	{
		pStemGate = g_evalGatesStack.list[i];
		if (pStemGate->changed)
		{
			push(g_evalGatesStack, pStemGate);
			reset(pStemGate->changed);
		}
	}
}

/*	update_all1
	updates all lists --- faulty_gates and eval_gates

	Input: current faulty_agtes
	Output: updated faulty_gates

	Notes: gates are stored in the forward levelized order
*/

void updateFaultyAndDynamicStack(int iNoPI) //update_all1
{
	//OUTPUT:  g_faultyGatesStack & g_dynamicStack
	register int i, iFaultyGatesStackSizeAdd1;
	register GATEPTR pGate;

	/* clear freach */
	for (i = 0; i <= g_freeGatesStack.last; i++) //NOT PI !!
	{
		reset(g_freeGatesStack.list[i]->freach);
	}
	for (i = 0; i < iNoPI; i++) //PI
	{
		reset(g_net[i]->freach);
	}
	//ALL freach cleared !!!

	/* faulty gates */
	iFaultyGatesStackSizeAdd1 = g_faultyGatesStack.last + 1;
	clear(g_faultyGatesStack); //clear!!!
	g_iSStack = (-1); //g_dynamicStack = NULL
	for (i = 0; i < iFaultyGatesStackSizeAdd1; i++)
	{
		pGate = g_faultyGatesStack.list[i];
		if (pGate->nfault > 0) //Filter some detected faults !!
		{
			push(g_faultyGatesStack, pGate);
			while (!pGate->freach)
			{
				set(pGate->freach);
				g_dynamicStack[++g_iSStack] = pGate; //pGate->freach == TRUE ------> g_dynamicStack
				if (pGate->outCount == 1) //Within FFR !!
				{
					pGate = pGate->outList[0];
				}
			}
		}
	}

	/* evaluation gate list */
	updateEvalStackByFaultyStack(); //g_faultyGatesStack -------> g_evalGatesStack
	g_iDStack = g_iSStack;
}

/*	Fault0_Simulation
	Main routine for the fault simuslation.
	Simulates each gates strictly in the forward order.
	Uses dominator information to avoid duplicate calculation
	of gate profiles.
	Inputs: circuit structure
	Outputs: number of detected faults
	Notes:
		1. Input patterns should be set in the output0 
		   and output1 of primary input gates.
		2. ALL1 should be set according to number of bits used
		3. Modified for the atalanta
*/

int Fault0_Simulation(int iNoGate, int iMaxDPI, int iNoPI, int iNoPO, int iStem, GATEPTR pStem[],
	int iBit, int iArrNoDetected[])
{
	//Return iNoDetected
	//iNoDetected = iArrNoDetected[0]
	//Always !!  iBit == 1
	//g_evalGatesStack -----> g_activeStemsStack & pGate->observe
	register int i;
	register int j; //DIFFERENCE//
	register GATEPTR pGate, pDomiGate;
	FAULTPTR pFault, *ppFault;
	int iNoDetected = 0;
	level iValue; //DIFFERENCE//Add for Fault1_Simulation !!!

	/* step 1: fault free simulation */
	//INPUT:  g_freeGatesStack
	//OUTPUT:  pGate->output & pGate->output1
	for (i = g_freeGatesStack.last; i >= 0; i--) //g_freeGatesStack is in REVERSED order !!
	{
		pGate = g_freeGatesStack.list[i]; //Without FANOUT
		switch (pGate->inCount)
		{
		case 1:
			evalGateWithInputs1(pGate, pGate->output1); break;
		case 2:
			evalGateWithInputs2(pGate, pGate->output1); break;
		case 3:
			evalGateWithInputs3(pGate, pGate->output1); break;
		default:
			evalGateWithInputs4(pGate, iValue, j);
			pGate->output1 = iValue;
		}
		pGate->output = pGate->output1;
		reset(pGate->freach);
		reset(pGate->changed);
	}

	/* step 2: fault dropping */
	//Drop freach to pGate's only son (Within FFR !!!)
	for (i = 0; i <= g_faultyGatesStack.last; i++)
	{
		pGate = g_faultyGatesStack.list[i];
		while (!pGate->freach)
		{
			set(pGate->freach); //Core Sentence
			pGate->cobserve = ALL0; //Core Sentence
			if (pGate->outCount == 1) //Within FFR !!!
			{
				pGate = pGate->outList[0];
			}
		}
	}

	/* step 3: fault simulation in the forward order */
	clear(g_activeStemStack); //g_activeStemsStack only used in Fault0/1_Simulation
	//g_evalGatesStack -------------------> outCount == 0 || fANOUT(outCount >= 2)
	for (i = 0; i <= g_evalGatesStack.last; i++) //don't pop g_evalGatesStack
	{
		pGate = g_evalGatesStack.list[i];
		if (!pGate->freach) //g_evalGatesStack <--------> freach
		{
			continue;
		}
		pGate->observe = g_iAllOne; //DIFFERENCE//0x00000001


		//output == 0 (PO)		bIsPO == TRUE
		if (pGate->type == PO)
		{
			//DIFFERENCE//iBit == 1
			iNoDetected += updateWholeStemObserveAndDetect(pGate, &g_iUpdateFlag, TRUE, iBit, iArrNoDetected); //pGate->nfault == 0 ----> g_iUpdateFlag = 1
		}

		
		//output >= 2 (FANOUT,)		bIsPO == FALSE
		else if ((pGate->observe = updateWholeStemObserveAndDetect(pGate, &g_iUpdateFlag, FALSE, iBit, iArrNoDetected)) != ALL0) //g_iUpdateFlag no change!!!
		{
			//DIFFERENCE//iBit == 1
			pDomiGate = (pGate->u_path == NULL) ? NULL : pGate->u_path->ngate;
			if ((pGate->observe = faultSimForStemGate(pGate, pGate->observe, pDomiGate, iMaxDPI)) != ALL0)
			{
				push(g_activeStemStack, pGate); //g_activeStemsStack:  Need to simulate!!!


				//Update pDomiGate->cobserve, seems like useless !!!
				if (pDomiGate != NULL)
				{
					pDomiGate->observe = ALL0;
					if (pDomiGate->freach)
					{
						pDomiGate->cobserve |= pGate->observe; //Core sentence
					}
					else
					{
						set(pDomiGate->freach);
						pDomiGate->cobserve = pGate->observe; //Core sentence
						
						if (pDomiGate->outCount == 1) //Refresh pDomiGate to its only son and freach them !!
						{
							pDomiGate = pDomiGate->outList[0];
							while (!pDomiGate->freach)
							{
								set(pDomiGate->freach);
								if (pDomiGate->outCount == 1)
								{
									pDomiGate = pDomiGate->outList[0];
								}
							}
						}
					}
				}
			}
		}
	}

	/* step 4: determine global detectability and detected faults */
	while (!is_empty(g_activeStemStack))
	{
		pGate = pop(g_activeStemStack); //pGate is a stem !!
		if (pGate->u_path != NULL)
		{
			pDomiGate = pGate->u_path->ngate;
			pGate->observe &= (pDomiGate->observe & g_net[pDomiGate->fosIndex]->observe); //Core sentence
		}
		if (pGate->observe != ALL0)
		{
			ppFault = pGate->dfault; //Traverse pGate->dfault!!!
			while ((pFault = (*ppFault)) != NULL) //For every pFault in pGate->dfault
			{
				if ((pFault->observe & pGate->observe) != ALL0) //Core sentence
				{
					//pFault detected !!!
					pFault->observe &= pGate->observe;
					for (i = iBit - 1; i >= 0; i--) //iBit == 1
					{
						if ((pFault->observe & BITMASK[i]) != ALL0)
						{
							++iArrNoDetected[i];
							break;
						}
					}
					pFault->detected = DETECTED;
					iNoDetected++;
					if (--(pFault->gate->nfault) == 0)
					{
						set(g_iUpdateFlag);
					}
					else
					{
						delete_fault(pFault);
					}
				}
				ppFault++;
			}
		}
	}

	/* if event occurs, update fault free lines */
	if (g_iUpdateFlag) //pGate->nfault == 0
	{
		updateFaultyStack();
		updateEvalStackByFaultyStack();
		/*  	update_free_gates(iNoPI); */
		reset(g_iUpdateFlag);
	}
	return(iNoDetected);
}

/*	Fault1_Simulation
	Main routine for the fault simulation
	Simulates each gates strictly in the forward order
	Uses dominator information to avoid duplicate calculation
	of gate profiles
	Inputs: circuit structure
	Outputs: number of detected faults
	Notes:
		1. Input patterns should be set in the output0 
		   and output1 of primary input gates.
		2. ALL1 should be set according to number of bits used
*/
int Fault1_Simulation(int iNoGate, int iMaxDPI, int iNoPI, int iNoPO, int iStem, GATEPTR pStem[],
	int iBit, int iArrNoDetected[])
{
	//Return iNoDetected
	//iNoDetected = iArrNoDetected[0]
	//Always !!  iBit == 1
	//g_evalGatesStack -----> g_activeStemsStack & pGate->observe
	register int i;
	register GATEPTR pGate, pDomiGate;
	FAULTPTR pFault, *ppFault;
	int iNoDetected = 0;

	/* step 3: fault simulation in the forward order */
	clear(g_activeStemStack); //g_activeStemsStack only used in Fault0/1_Simulation
	//g_evalGatesStack -------------------> outCount == 0 || fANOUT(outCount >= 2)
	for (i = 0; i <= g_evalGatesStack.last; i++) //don't pop g_evalGatesStack
	{
		pGate = g_evalGatesStack.list[i];
		if (!pGate->freach) //g_evalGatesStack <--------> freach
		{
			continue;
		}
		pGate->observe = g_iAllOne; //DIFFERENCE//g_iAllOne has iBit's "1" in the right!!!

		
		//output == 0 (PO)		bIsPO == TRUE
		if (pGate->type == PO)
		{
			iNoDetected += updateWholeStemObserveAndDetect(pGate, &g_iUpdateFlag, TRUE, iBit, iArrNoDetected); //pGate->nfault == 0 ----> g_iUpdateFlag = 1
			//g_iUpdateFlag ONLY used in Fault0_Simulation !!!
		}


		//output >= 2 (FANOUT)		bIsPO == FALSE
		else if ((pGate->observe = updateWholeStemObserveAndDetect(pGate, &g_iUpdateFlag, FALSE, iBit, iArrNoDetected)) != ALL0) //g_iUpdateFlag no change!!!
		{
			pDomiGate = (pGate->u_path == NULL) ? NULL : pGate->u_path->ngate;
			if ((pGate->observe = faultSimForStemGate(pGate, pGate->observe, pDomiGate, iMaxDPI)) != ALL0)
			{
				push(g_activeStemStack, pGate); //g_activeStemsStack:  Need to simulate!!!

				
				//Update pDomiGate->cobserve, seems like useless !!!
				if (pDomiGate != NULL)
				{
					pDomiGate->observe = ALL0;
					if (pDomiGate->freach)
					{
						pDomiGate->cobserve |= pGate->observe; //Core sentence
					}
					else
					{
						set(pDomiGate->freach);
						g_dynamicStack[++g_iDStack] = pDomiGate; //DIFFERENCE//Add for Fault1_Simulation !!!
						pDomiGate->cobserve = pGate->observe; //Core sentence
						
						if (pDomiGate->outCount == 1) //Refresh pDomiGate to its only son and freach them !!
						{
							pDomiGate = pDomiGate->outList[0];
							while (!pDomiGate->freach)
							{
								set(pDomiGate->freach);
								g_dynamicStack[++g_iDStack] = pDomiGate; //DIFFERENCE//Add for Fault1_Simulation !!!
								pDomiGate->cobserve = ALL0; //Core sentence //DIFFERENCE//Add for Fault1_Simulation !!!
								if (pDomiGate->outCount == 1)
								{
									pDomiGate = pDomiGate->outList[0];
								}
							}
						}
					}
				}
			}
		}
	}

	/* step 4: determine global detectability and detected faults */
	while (!is_empty(g_activeStemStack))
	{
		pGate = pop(g_activeStemStack); //pGate is a stem !!
		if (pGate->u_path != NULL)
		{
			pDomiGate = pGate->u_path->ngate;
			pGate->observe &= (pDomiGate->observe & g_net[pDomiGate->fosIndex]->observe); //Core sentence
		}
		if (pGate->observe != ALL0)
		{
			ppFault = pGate->dfault; //Traverse pGate->dfault!!!
			while ((pFault = (*ppFault)) != NULL) //For every pFault in pGate->dfault
			{
				if ((pFault->observe & pGate->observe) != ALL0) //Core sentence
				{
					//pFault detected !!!
					pFault->observe &= pGate->observe;
					for (i = iBit - 1; i >= 0; i--) // 1 <= iBit <= 32
					{
						if ((pFault->observe & BITMASK[i]) != ALL0)
						{
							++iArrNoDetected[i];
							break;
						}
					}
					pFault->detected = DETECTED;
					iNoDetected++;
					if (--(pFault->gate->nfault) == 0)
					{
						set(g_iUpdateFlag);
					}
					else
					{
						delete_fault(pFault);
					}
				}
				ppFault++;
			}
		}
	}

	return(iNoDetected);
}
