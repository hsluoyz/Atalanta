
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

		   Integrated HOPE with ATALANTA
 
***********************************************************************/

/**************************** HISTORY **********************************

	hope: version 1.0

	Original: H. K. Lee, 8/15/1991
	Updated: H. K. Lee, 12/31/1991

	hope: version 1.1

	Added functional fault injection: H. K. Lee, 3/15/1993
	Added static & dynamic fault grouping: H. K. Lee, 3/15/1993
	Changed parser: H. K. Lee, 7/31/1993

***********************************************************************/

/*----------------------------------------------------------------- 
	fsim.c
	contains all subroutines necessary for 
	zero gate delay fault simulation of HOPE:

	version 0: applies heuristics for only single event faults.
		   converts single event faults into stemIndex faults and
		   applies candidacy test for a stemIndex fault:
		   If a dominator exists, simulates to the stemIndex of the
		   immediate dominator. Otherwise, checks the gates
		   immediately following the stemIndex.
		   final version of HOPE in paper.

	version 1: Added functional fault injection,
		   static fault grouping by FFRs and
		   dynamic fault grouping.

-------------------------------------------------------------------*/
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "fsim.h"

#include "parameter.h"
#include "define.h"
#include "macro.h"

extern int g_iNoGate, g_iLastGate, g_iNoPI, g_iNoPO, g_iNoFF, g_iMaxLevel, g_iPOLevel, g_iPPOLevel;
extern int* g_PrimaryIn, * g_PrimaryOut, * g_FlipFlop;
extern char xdetectmode;
extern GATEPTR* g_net;
extern STACKTYPE* g_pEventListStack, g_stack1, g_stack2;
extern level BITMASK[];
extern level g_truthTable1[MAXGTYPE][MAXLEVEL];
extern level g_truthTable2[MAXGTYPE][MAXLEVEL][MAXLEVEL];
extern FAULTPTR g_pHeadFault, g_pCurrentFault, g_pTailFault, * g_pFaultList;
extern void printFatalError();
extern int ntest;
extern level g_TABLE[Z + 1][2];
#ifdef DIAGNOSIS
extern char dropmode, dictmode, diagmode;
extern FILE* diagfile;
extern void Print_Faulty_Values();
#endif

FAULTPTR g_pPotentialFault;
int dynamic_order_flag = 0;

extern STEMTYPE* g_pStems;
extern int g_iNoStem, g_iNoRStem;
#define UNSIMULATED 32
#define SIMULATED 33
#define is_simulated(gate,val) (g_pStems[gate->stemIndex].flag[val])

#define twoBitsDifferent(V1,V2) ((V1[0]!=V2[0]) || (V1[1]!=V2[1]))
#define twoBitsCopy(Dest,Sour) Dest[0]=Sour[0]; Dest[1]=Sour[1]
#define is_outfault(f) (f->line<0)

typedef struct FLINK* FLINKPTR;
typedef struct FLINK
{
	FAULTPTR fault; FLINKPTR next;
} FLINKTYPE;


typedef struct _SUT
{
	GATEPTR gate;			/* stem of the faulty gate */
	level faultType;			/* value of the stem simulated */
	int faultLine;			/* faulty line */
	FLINKPTR extra;			/* extra faults simulated */
	EVENTTYPE* event;
	int gateType;
	int papa;
	level Val[2];
} _SUT_TYPE;
_SUT_TYPE g_SUTList[SIZE_OF_FUT];


FAULTPTR g_FUTList[SIZE_OF_FUT];
int g_iFUT;


typedef struct _IN_OUT_GATES
{
	int last;
	GATEPTR list[SIZE_OF_FUT];
} _IN_OUT_GATES_TYPE;
_IN_OUT_GATES_TYPE g_InGatesStack, g_OutGatesStack;

extern EVENTPTR g_headEvent, g_tailEvent;
FAULTPTR g_undetectableFault;			/* undetectable fault */

int g_iGroupID = 0;				/* used in fault simulation */
int g_iNoDetected;

#ifdef DIAGNOSIS
int diag_id = 0;
#endif


typedef struct _SSTEMS
{
	int stem;
	level val;
} _SSTEMS_TYPE;
_SSTEMS_TYPE g_pSStems[3000];


int g_iSStem = (-1);
int g_iStemIndex = (-1);
level ssval;

/*------InitFaultSim------------------------------------------------
	Initializes flags for the fault simulation
-------------------------------------------------------------------*/
void initFaultSim_HOPE() //InitFaultSim
{
	register int i;
	register GATEPTR pGate;
	register FAULTPTR pFault, pNextFault;
	EVENTPTR pEvent;

	for (i = 0; i < g_iNoGate; i++)
	{
		pGate = g_net[i];
		pGate->SGV = X;
		setPairToX(pGate->GV[0], pGate->GV[1]);
		setPairToX(pGate->FV[0], pGate->FV[1]);
		pGate->Gid = g_iGroupID;
		reset(pGate->changed);
	}

	for (pFault = g_pHeadFault; pFault->next != NULL; pFault = pFault->next)
	{
		pNextFault = pFault->next;
		if (pNextFault->detected != REDUNDANT)
		{
			pNextFault->detected = UNDETECTED; //Recover detected faults !!
		}
		while (pNextFault->event != NULL) //Remove All Events !!
		{
			pEvent = pNextFault->event;
			pNextFault->event = pEvent->next;
			FREE(pEvent);
		}
#ifdef DIAGNOSIS
		pNextFault->diag_id = diag_id;
#endif
	}
	g_pTailFault = pFault; //Already ??
	g_pPotentialFault = pFault; //g_pPotentialFault = g_pTailFault

	for (i = 1; i <= g_iNoStem; i++)
	{
		g_pStems[i].fault[ZERO] = g_pStems[i].fault[X] = g_pStems[i].fault[ONE] = NULL;
		g_pStems[i].flag[ZERO] = g_pStems[i].flag[X] = g_pStems[i].flag[ONE] = UNSIMULATED;
	}

	ALLOCATE(g_undetectableFault, FAULTTYPE, 1);
	g_undetectableFault->gate = 0;
	g_undetectableFault->line = 0;
	g_undetectableFault->type = 0;
	g_undetectableFault->detected = UNDETECTED;
	g_undetectableFault->next = NULL;
	g_undetectableFault->event = NULL;

	initEventList(g_headEvent, g_tailEvent);

	clear(g_stack1);
	clear(g_stack2);
}

/*-----InjectFault-------------------------------------------------
	Inject a given fault by modifying the circuit structure.
	For each fault, inject two temprary gates (a DUMMY gate
	and an AND/OR gate for stuck-at 1/0 fault) at the end of
	netlist and modifies existing net connection.
	Inputs: f	fault to be injected
	Outputs: returns the last injected gate (AND or OR)
	Remarks: Increments nog by two.
------------------------------------------------------------------*/

GATEPTR injectFault(GATEPTR pGate, int iFaultType, int iFaultLine, register int iBit) //InjectFault
{
	register GATEPTR pEventGate;
	register int i;
	GATEPTR pTempGate;
	register EVENTTYPE* pEvent;
	int iValue[2];

	if (pGate->type < FAULTY)
	{
		g_SUTList[iBit].gateType = pGate->type;
		g_SUTList[iBit].papa = (-1);
	}
	else
	{
		g_SUTList[iBit].gateType = g_SUTList[pGate->type - FAULTY].gateType;
		g_SUTList[iBit].papa = pGate->type - FAULTY;
	}
	pGate->type = iBit + FAULTY;

	/* Flip-Flops */
	for (pEvent = g_tailEvent->next = g_SUTList[iBit].event; pEvent != NULL; pEvent = pEvent->next)
	{
		pEventGate = g_net[pEvent->node];
		if (pEventGate->Gid != g_iGroupID)
		{
			pEventGate->Gid = g_iGroupID;
			twoBitsCopy(pEventGate->FV, pEventGate->GV);
		}


		if (checkBitIs0(pEvent->value, 0))
		{
			pEventGate->FV[0] = resetBit(pEventGate->FV[0], iBit);
		}
		else
		{
			pEventGate->FV[0] = setBit(pEventGate->FV[0], iBit);
		}


		if (checkBitIs0(pEvent->value, 1))
		{
			pEventGate->FV[1] = resetBit(pEventGate->FV[1], iBit);
		}
		else
		{
			pEventGate->FV[1] = setBit(pEventGate->FV[1], iBit);
		}


		for (i = 0; i< pEventGate->outCount; i++)
		{
			pTempGate = pEventGate->outList[i];
			if (!pTempGate->changed)
			{
				pushGate(pTempGate);
				set(pTempGate->changed);
			}
		}
		g_tailEvent = pEvent;
	}

	/* Faulty pGate */
	if (iFaultLine < 0)
	{
		/* output line fault */
		if (pGate->Gid != g_iGroupID)
		{
			pGate->Gid = g_iGroupID;
			twoBitsCopy(pGate->FV, pGate->GV);
		}
		twoBitsCopy(iValue, pGate->FV);
		switch (iFaultType)
		{
		case SA0:
			iValue[0] |= BITMASK[iBit];
			iValue[1] &= ~BITMASK[iBit];
			break;
		case SA1:
			iValue[0] &= ~BITMASK[iBit];
			iValue[1] |= BITMASK[iBit];
			break;
		default:
			iValue[0] &= ~BITMASK[iBit];
			iValue[1] &= ~BITMASK[iBit];
			break;
		}
		if (twoBitsDifferent(iValue, pGate->FV))
		{
			twoBitsCopy(pGate->FV, iValue);
			for (i = 0; i< pGate->outCount; i++)
			{
				pTempGate = pGate->outList[i];
				if (!pTempGate->changed)
				{
					pushGate(pTempGate);
					set(pTempGate->changed);
				}
			}
		}
	}
	else
	{
		/* input line fault */
		if (pGate->changed)
		{
			return(NULL);
		}

		pTempGate = pGate->inList[iFaultLine];
		if (pTempGate->Gid == g_iGroupID)
		{
			iValue[0] = pTempGate->FV[0] & BITMASK[iBit];
			iValue[1] = pTempGate->FV[1] & BITMASK[iBit];
		}
		else
		{
			iValue[0] = pTempGate->GV[0] & BITMASK[iBit];
			iValue[1] = pTempGate->GV[1] & BITMASK[iBit];
		}
		iValue[0] = (iValue[0] != ALL0) ? ZERO : (iValue[1] != ALL0) ? ONE : X;
		if (iValue[0] != iFaultType)
		{
			pushGate(pGate);
			set(pGate->changed);
		}
	}
	return(NULL);
}

void Faulty_Gate_Eval(GATEPTR pGate, level* iValue)
{
	register int i;
	register int iBit;
	int iGateType;
	level iTempValue;
	GATEPTR pInGate;

	iGateType = pGate->type;
	for (iBit = iGateType - FAULTY,pGate->type = g_SUTList[iBit].gateType; iBit >= 0; iBit = g_SUTList[iBit].papa)
	{
		if (g_SUTList[iBit].faultLine >= 0)
		{
			pInGate = pGate->inList[g_SUTList[iBit].faultLine];
			if (pInGate->Gid != g_iGroupID)
			{
				twoBitsCopy(pInGate->FV, pInGate->GV);
				pInGate->Gid = g_iGroupID;
			}
			if (pInGate->changed < 3)
			{
				twoBitsCopy(g_SUTList[iBit].Val, pInGate->FV);
				pInGate->changed += 3;
			}
			switch (g_SUTList[iBit].faultType)
			{
			case SA0:
				pInGate->FV[0] |= BITMASK[iBit];
				pInGate->FV[1] &= ~BITMASK[iBit];
				break;
			case SA1:
				pInGate->FV[0] &= ~BITMASK[iBit];
				pInGate->FV[1] |= BITMASK[iBit];
				break;
			default:
				pInGate->FV[0] &= ~BITMASK[iBit];
				pInGate->FV[1] &= ~BITMASK[iBit];
				break;
			}
		}
	}
	FEVAL(pGate, iValue, i, iTempValue, pInGate, g_iGroupID);
	for (iBit = iGateType - FAULTY,pGate->type = iGateType; iBit >= 0; iBit = g_SUTList[iBit].papa)
	{
		if (g_SUTList[iBit].faultLine >= 0)
		{
			pInGate = pGate->inList[g_SUTList[iBit].faultLine];
			if (pInGate->changed >= 3)
			{
				twoBitsCopy(pInGate->FV, g_SUTList[iBit].Val);
				pInGate->changed -= 3;
			}
		}
		else
		{
			switch (g_SUTList[iBit].faultType)
			{
			case SA0:
				iValue[0] |= BITMASK[iBit];
				iValue[1] &= ~BITMASK[iBit];
				break;
			case SA1:
				iValue[0] &= ~BITMASK[iBit];
				iValue[1] |= BITMASK[iBit];
				break;
			default:
				iValue[0] &= ~BITMASK[iBit];
				iValue[1] &= ~BITMASK[iBit];
				break;
			}
		}
	}
}

/*------FaultSim---------------------------------------------------
	Performs fault simulation for the given fault set based
	on the good values of the circuit.

	Next time events are stored at g_pEventListStack[0].
	Inputs: Gid	group-id of the current fault set
------------------------------------------------------------------*/
void FaultSim(int start, int stop, register int Gid)
{
	register GATETYPE* gut;
	register int i, j;
	register GATETYPE* temp;
	level Val[2], v;

	for (i = start; i < stop; i++)
		while (!is_empty(g_pEventListStack[i]))
		{
			gut = pop(g_pEventListStack[i]);
			reset(gut->changed);
			FEVAL(gut, Val, j, v, temp, Gid);
			if (twoBitsDifferent(gut->GV, Val))
			{
				twoBitsCopy(gut->FV, Val);
				gut->Gid = Gid;
				for (j = gut->outCount - 1; j >= 0; j--)
				{
					temp = gut->outList[j];
					if (!temp->changed)
					{
						pushGate(temp);
						set(temp->changed);
					}
				}
			}
		}
}

level g_faultType2Value[SAX + 1] =
{
	ZERO,	ONE,	X
	//SA0	SA1		SAX
};
level g_value2Pair[Z + 1][2] =
{
	{ALL1,ALL0}, {ALL0,ALL1}, {ALL0,ALL0}, {ALL1,ALL1}
};

GATEPTR SSimToDominator(register GATEPTR pGate, register GATEPTR pDomiGate, register int iGID)
{
	//OUTPUT:  pDomiGate !!
	register int i, j;
	register GATEPTR pTempGate;
	register level iValue;
	int iEnd;

	iEnd = (pDomiGate == NULL) ? g_iPPOLevel : pDomiGate->dpi + 1;

	for (i = 0; i< pGate->outCount; i++)
	{
		pTempGate = pGate->outList[i];
		if (!pTempGate->changed)
		{
			pushGate(pTempGate);
			set(pTempGate->changed);
		}
	}

	
	for (i = (pGate->dpi >= g_iPPOLevel) ? 1 : pGate->dpi; i < iEnd; i++)
	{
		while (!is_empty(g_pEventListStack[i]))
		{
			pGate = pop(g_pEventListStack[i]);
			reset(pGate->changed);
			iValue = (pGate->inList[0]->Gid == iGID) ? g_truthTable1[pGate->type][pGate->inList[0]->FV[0]] : g_truthTable1[pGate->type][pGate->inList[0]->SGV];
			if (pGate->inCount > 1)
			{
				for (j = 1; j< pGate->inCount; j++)
				{
					iValue = (pGate->inList[j]->Gid == iGID) ? g_truthTable2[pGate->type][iValue][pGate->inList[j]->FV[0]] : g_truthTable2[pGate->type][iValue][pGate->inList[j]->SGV];
				}
			}
			if (iValue != pGate->SGV)
			{
				pGate->FV[0] = iValue;
				pGate->Gid = iGID;
				if (pGate == pDomiGate)
				{
					return(pGate);
				}
				for (j = 0; j< pGate->outCount; j++)
				{
					pTempGate = pGate->outList[j];
					if (!pTempGate->changed)
					{
						pushGate(pTempGate);
						set(pTempGate->changed);
					}
				}
			}
		}
	}
	return(NULL);
}

/*------CheckStem----------------------------------------------------
	Candidacy test for a single event fault at a stemIndex
	Checks the gates immediately following the stemIndex
	Faulty stemIndex should be updated before calling
-------------------------------------------------------------------*/
GATEPTR checkStemOutputNeedUpdate(register GATEPTR pStemGate, register int iGid) //CheckStem
{
	//OUTPUT:  pStemGate or NULL
	register int i, j;
	register level iValue;
	register GATEPTR pOutGate;

	for (i = 0; i< pStemGate->outCount; i++)
	{
		pOutGate = pStemGate->outList[i];
		iValue = (pOutGate->inList[0]->Gid == iGid) ? g_truthTable1[pOutGate->type][pOutGate->inList[0]->FV[0]] :
			g_truthTable1[pOutGate->type][pOutGate->inList[0]->SGV];
		for (j = 1; j< pOutGate->inCount; j++)
		{
			iValue = (pOutGate->inList[j]->Gid == iGid) ?
				g_truthTable2[pOutGate->type][iValue][pOutGate->inList[j]->FV[0]] :
				g_truthTable2[pOutGate->type][iValue][pOutGate->inList[j]->SGV];
		}
		if (iValue != pOutGate->SGV)
		{
			return(pStemGate);
		}
	}
	return(NULL);
}


GATEPTR SSimToStem(register GATEPTR pGate, register int iGid) //No use by now, good for me!!!
{
	GATEPTR pTempGate;
	register int i;
	register level iValue;

	/* Check 2: stemIndex */
	while (pGate->stemIndex <= 0)
	{
		pGate = pGate->outList[0]; //why so sure pGate has outList[0]?
		iValue = (pGate->inList[0]->Gid == iGid) ? g_truthTable1[pGate->type][pGate->inList[0]->FV[0]] :
			g_truthTable1[pGate->type][pGate->inList[0]->SGV];
		if (pGate->inCount > 1)
			for (i = 1; i< pGate->inCount; i++)
				iValue = (pGate->inList[i]->Gid == iGid) ? g_truthTable2[pGate->type][iValue][pGate->inList[i]->FV[0]] : g_truthTable2[pGate->type][iValue][pGate->inList[i]->SGV];
		if (iValue == pGate->SGV)
			return(NULL);
		pGate->FV[0] = iValue; pGate->Gid = iGid;
	}

	if (g_pStems[pGate->stemIndex].checkup < 1)
		return(pGate);
	if (is_simulated(pGate, pGate->FV[0]) != UNSIMULATED)
		return(pGate);

	/* Check whether the stemIndex can be propagated to next gate or not */
	if ((i = g_pStems[pGate->stemIndex].dominatorIndex) > 0)
	{
		/* dominator */
		if ((pTempGate = SSimToDominator(pGate, g_net[i], iGid)) != NULL)
		{
			g_iStemIndex = pGate->stemIndex;
			ssval = pGate->FV[0];
			return(pTempGate);
		}
	}
	else
	{
		/* no dominator */
		if (checkStemOutputNeedUpdate(pGate, iGid) != NULL)
			return(pGate);
	}
	g_pStems[pGate->stemIndex].flag[pGate->FV[0]] = SIMULATED;
	g_pStems[pGate->stemIndex].fault[pGate->FV[0]] = g_undetectableFault;
	push(g_stack1, pGate);
	return(NULL);
}

/*------DropDetectedFaults----------------------------------------------
	Computes signature of primary outputs and determines
	detected faults.
	Outputs: returns the number of faults detected.
-----------------------------------------------------------------------*/
int DropDetectedFaults()
{
	register int i;
	register level iSigDetection = ALL0;	/* signature for detection */
	register level iSigPotential = ALL0;	/* signature for potential detection */
	register GATEPTR pGate;
	register FAULTPTR pFault;

	while (!is_empty(g_pEventListStack[g_iPOLevel]))
	{
		pGate = pop(g_pEventListStack[g_iPOLevel]);
		reset(pGate->changed);
		if (pGate->inList[0]->Gid == g_iGroupID)
		{
			twoBitsCopy(pGate->FV, pGate->inList[0]->FV); //Fault Value
		}
		else
		{
			twoBitsCopy(pGate->FV, pGate->inList[0]->GV); //Good Value
		}
		pGate->Gid = g_iGroupID;

		//STOP*********************************************************STOP
		if (pGate->type >= FAULTY)
		{
			for (i = pGate->type - FAULTY; i >= 0; i = g_SUTList[i].papa) //Why??
			{
				switch (g_SUTList[i].faultType)
				{
				case SA0:
					pGate->FV[0] |= BITMASK[i];
					pGate->FV[1] &= ~BITMASK[i];
					break;
				case SA1:
					pGate->FV[0] &= ~BITMASK[i];
					pGate->FV[1] |= BITMASK[i];
					break;
				default:
					pGate->FV[0] &= ~BITMASK[i];
					pGate->FV[1] &= ~BITMASK[i];
					break;
				}
			}
			if (!twoBitsDifferent(pGate->FV, pGate->GV))
				continue;
		}

		iSigDetection |= (pGate->GV[0] & ~pGate->GV[1] & ~pGate->FV[0] & pGate->FV[1]) |
		(~pGate->GV[0] & pGate->GV[1] & pGate->FV[0] & ~pGate->FV[1]);
		//GV[0]	GV[1]		FV[0]		FV[1]
		//    1		   0			   0			   1
		//    0		   1			   1			   0
#ifndef ATALANTA
		//STOP************************************************STOP
		iSigPotential |= (pGate->GV[0] ^ pGate->GV[1]) & (~pGate->FV[0] & ~pGate->FV[1]);
#endif
	}
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
	if (iSigDetection != ALL0)
		for (i = 0; i < g_iFUT; i++)	/* detected */
			if ((iSigDetection & BITMASK[i]) != ALL0)
			{
				pFault = g_FUTList[i];
				pFault->detected = DETECTED;
				g_iNoDetected++;
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
#ifdef DIAGNOSIS
	//STOP************************************************STOP
				pFault->diag_id = diag_id;
				if (dropmode == 'y')
				{
					remove(pFault->event);
				}
#else
				remove(pFault->event);
#endif
			}
#ifndef ATALANTA
	//STOP************************************************STOP
	if (iSigPotential != ALL0)
		for (i = 0; i < g_iFUT; i++)	/* potentially detected */
			if ((iSigPotential & BITMASK[i]) != ALL0)
			{
				pFault = g_FUTList[i];
				pFault->npot += 1;
				if (pFault->detected == UNDETECTED)
				{
					pFault->detected = XDETECTED;
					if (xdetectmode == 'y')
					{
						g_iNoDetected++;
#ifdef DIAGNOSIS
	//STOP************************************************STOP
						pFault->diag_id = diag_id;
						if (dropmode == 'y')
						{
							remove(pFault->event);
						}
#else
						remove(pFault->event);
#endif
					}
				}
			}
#endif
	return(g_iNoDetected);
}

/*------StoreFaultyStatus-------------------------------------------
	Stores faulty status to each node.
	Allocates frees memory status if necessary.
	Flip-Flops which need schedule are stored in g_pEventListStack[0].
------------------------------------------------------------------*/
void StoreFaultyStatus()
{
	register GATEPTR pGate;
	register int i, iGateIndex;
	EVENTPTR temp;
	FAULTPTR pFault;
	level iValueDifference;
	level iFaultValue[2];

	while (!is_empty(g_pEventListStack[g_iPPOLevel]))
	{
		pGate = pop(g_pEventListStack[g_iPPOLevel]);
		iGateIndex = pGate->index;
		reset(pGate->changed);

		if (pGate->inList[0]->Gid == g_iGroupID)
		{
			twoBitsCopy(iFaultValue, pGate->inList[0]->FV);
		}
		else
		{
			twoBitsCopy(iFaultValue, pGate->inList[0]->GV);
		}

		if (pGate->type >= FAULTY)
		{
			for (i = pGate->type - FAULTY; i >= 0; i = g_SUTList[i].papa)
			{
				if (g_SUTList[i].faultLine < 0)
					continue;
				switch (g_SUTList[i].faultType)
				{
				case SA0:
					iFaultValue[0] |= BITMASK[i];
					iFaultValue[1] &= ~BITMASK[i];
					break;
				case SA1:
					iFaultValue[0] &= ~BITMASK[i];
					iFaultValue[1] |= BITMASK[i];
					break;
				default:
					iFaultValue[0] &= ~BITMASK[i];
					iFaultValue[1] &= ~BITMASK[i];
					break;
				}
			}
		}

		pGate = pGate->inList[0];
		if ((iValueDifference = (pGate->GV[0] ^ iFaultValue[0]) | (pGate->GV[1] ^ iFaultValue[1])) == ALL0) //GV == iFaultValue
			continue;

		for (i = 0; i < g_iFUT; i++)
		{
			pFault = g_FUTList[i];
			if ((iValueDifference & BITMASK[i]) == ALL0)
				continue;
#ifdef DIAGNOSIS
	//STOP**************************************STOP
			if (dropmode == 'n' || pFault->detected == UNDETECTED ||
#else
			if (pFault->detected == UNDETECTED ||
#endif

#ifdef ATALANTA
				pFault->detected==PROCESSED ||
#endif
			//
			//if (pFault->detected == UNDETECTED || pFault->detected==PROCESSED ||
				(xdetectmode == 'n' && pFault->detected == XDETECTED))
			{
				//pFault->detected != DETECTED
				create(temp);
				temp->node = iGateIndex;
				temp->value = ALL0;
				if (!checkBitIs0(iFaultValue[0], i))
					temp->value = setBit(temp->value, 0);
				if (!checkBitIs0(iFaultValue[1], i))
					temp->value = setBit(temp->value, 1);
				temp->next = pFault->event;
				pFault->event = temp;		/* head of event */
				if ((iValueDifference = resetBit(iValueDifference, i)) == ALL0)
					break;
			}
		}
	}
}

#ifndef NEW_FAULT_INJECT
/*------RemoveFault-----------------------------------------------
	Restores the most current injected fault.
-----------------------------------------------------------------*/
void RemoveFault()
{
	register GATEPTR gut, prev, next;
	register int j, k;

	gut = g_net[g_iNoGate];
	prev = gut->inList[1];
	for (j = 0; j< gut->outCount; j++)
	{
		/* next gates */
		next = gut->outList[j];
		for (k = 0; k< next->inCount; k++)
			if (next->inList[k] == gut)
				next->inList[k] = prev;
	}
	if (prev->outCount == 1)
	{
		prev->outCount = gut->outCount;
		for (j = 0; j< gut->outCount; j++)
			prev->outList[j] = gut->outList[j];
	}
	else
		for (j = 0; j< prev->outCount; j++)
			if (prev->outList[j] == gut)
				prev->outList[j] = gut->outList[0];
	g_iNoGate--;
}

/*------RestoreCircuits--------------------------------------------
	Restores original circuits for all injected faults.
-------------------------------------------------------------------*/
//NO USE !!
//STOP****************************************STOP
void RestoreCircuits()
{
	while (--g_iNoGate > g_iLastGate)
		RemoveFault();
	g_iNoGate++;
}
#endif

/*------CheckSingleEvent----------------------------------------------------
	Performs candidacy test for a single event fault.
----------------------------------------------------------------------------*/
GATEPTR CheckSingleEvent(FAULTPTR pFault, GATEPTR pGate, register int iGID)
{
	register int j, k;
	EVENTPTR pEvent, e1;
	FAULTPTR g;
	FLINKTYPE* flink;

	register GATEPTR pDomiGate;
	register int i;
	register level iValue;

	while (pGate->stemIndex <= 0) //NOT Stem Gate !!
	{
		pGate = pGate->outList[0]; //Only 1 output !!
		iValue = (pGate->inList[0]->Gid == iGID) ? g_truthTable1[pGate->type][pGate->inList[0]->FV[0]] : g_truthTable1[pGate->type][pGate->inList[0]->SGV];
		if (pGate->inCount > 1)
		{
			for (i = 1; i< pGate->inCount; i++)
			{
				iValue = (pGate->inList[i]->Gid == iGID) ? g_truthTable2[pGate->type][iValue][pGate->inList[i]->FV[0]] : g_truthTable2[pGate->type][iValue][pGate->inList[i]->SGV];
			}
		}
		if (iValue == pGate->SGV)
		{
			return(NULL);
		}
		pGate->FV[0] = iValue;
		pGate->Gid = iGID;
	}

	//pGate is STEM GATE now !!
	if ((g_pStems[pGate->stemIndex].checkup >= 1) && (is_simulated(pGate, pGate->FV[0]) == UNSIMULATED))
	{
		if ((j = g_pStems[pGate->stemIndex].dominatorIndex) > 0)
		{
			/* dominator */
			if ((pDomiGate = SSimToDominator(pGate, g_net[j], iGID)) != NULL)
			{
				g_iStemIndex = pGate->stemIndex;
				ssval = pGate->FV[0];
				pGate = pDomiGate;
			}
			else
			{
				g_pStems[pGate->stemIndex].flag[pGate->FV[0]] = SIMULATED;
				g_pStems[pGate->stemIndex].fault[pGate->FV[0]] = g_undetectableFault;
				push(g_stack1, pGate);
				return(NULL);
			}
		}
		else if (checkStemOutputNeedUpdate(pGate, iGID) == NULL)
		{
			/* no dominator */
			g_pStems[pGate->stemIndex].flag[pGate->FV[0]] = SIMULATED;
			g_pStems[pGate->stemIndex].fault[pGate->FV[0]] = g_undetectableFault;
			push(g_stack1, pGate);
			return(NULL);
		}
	}

	if (pGate != NULL)
	{
		j = pGate->FV[0];
		if (pGate->type == PO)
		{
			/* PO */
			if ((k = checkPair(pGate->GV[0], pGate->GV[1])) != X)
			{
				if (j == X)
				{
#ifndef ATALANTA
					pFault->detected = XDETECTED;
					if (xdetectmode == 'y')
					{
						g_iNoDetected++;
#ifdef DIAGNOSIS
						pFault->diag_id = diag_id;
#endif
					}
#endif
				}
				else
				{
					pFault->detected = DETECTED;
					g_iNoDetected++;
#ifdef DIAGNOSIS
					pFault->diag_id = diag_id;
#endif
				}
			}
			if (g_iStemIndex > 0)
			{
				g_pStems[g_iStemIndex].flag[ssval] = SIMULATED;
				g_pStems[g_iStemIndex].fault[ssval] = pFault;
				push(g_stack1, g_net[g_pStems[g_iStemIndex].gate]);
			}
			pGate = NULL;
		}
		else if (pGate->outCount == 1)
		{
			/* PPO */
			create(pEvent);
			pEvent->node = pGate->outList[0]->index;
			pEvent->value = ALL0;
			if (!checkBitIs0(g_value2Pair[j][0], 0))
				pEvent->value = setBit(pEvent->value, 0);
			if (!checkBitIs0(g_value2Pair[j][1], 0))
				pEvent->value = setBit(pEvent->value, 1);
			pEvent->next = NULL;
			pFault->event = pEvent;
			if (g_iStemIndex > 0)
			{
				g_pStems[g_iStemIndex].flag[ssval] = SIMULATED;
				g_pStems[g_iStemIndex].fault[ssval] = pFault;
				push(g_stack1, g_net[g_pStems[g_iStemIndex].gate]);
			}
			pGate = NULL;
		}
		else
		{
			/* Check the stemIndex is already simulated */
			switch ((k = is_simulated(pGate, j)))
			{
			case UNSIMULATED:
				break;
			case SIMULATED:
				g = g_pStems[pGate->stemIndex].fault[j];
				for (pEvent = g->event; pEvent != NULL; pEvent = pEvent->next)
				{
					create(e1);
					e1->node = pEvent->node;
					e1->value = pEvent->value;
					e1->next = pFault->event;
					pFault->event = e1;
				}
				switch (g->detected)
				{
				case UNDETECTED:
					break;
				case DETECTED:
					pFault->detected = DETECTED;
					g_iNoDetected++;
#ifdef DIAGNOSIS
					pFault->diag_id = diag_id;
#endif
					break;

				default:
					/* XDETECTED */
#ifndef ATALANTA
					pFault->detected = XDETECTED;
					if (xdetectmode == 'y')
					{
						g_iNoDetected++;
#ifdef DIAGNOSIS
						pFault->diag_id = diag_id;
#endif
					}
#endif
					break;
				}
				if (g_iStemIndex > 0)
				{
					g_pStems[g_iStemIndex].flag[ssval] = SIMULATED;
					g_pStems[g_iStemIndex].fault[ssval] = g;
					push(g_stack1, g_net[g_pStems[g_iStemIndex].gate]);
				}
				pGate = NULL; break;
			default:
				/* pFault is already injected */
				ALLOCATE(flink, FLINKTYPE, 1);
				flink->fault = pFault;
				flink->next = g_SUTList[k].extra;
				g_SUTList[k].extra = flink;
				if (g_iStemIndex > 0)
				{
					g_pStems[g_iStemIndex].flag[ssval] = k;
					g_pStems[g_iStemIndex].fault[ssval] = pFault;
					g_pSStems[++g_iSStem].stem = g_iStemIndex;
					g_pSStems[g_iSStem].val = ssval;
					push(g_stack1, g_net[g_pStems[g_iStemIndex].gate]);
				}
				pGate = NULL;
			}
		}
	}
	return(pGate);
}

level LEVEL_TO_EVENT[4] =
{
	1, 2, 0, 3
};

/*------SelectNextFaults-------------------------------------------
	Selects next 32 faults to be simulated begginning cf->next.
	Schedules the event list for the faulty cite and PPIs.
	Inputs: cf	head of remainning fault list
	Outputs: returns next head fault
------------------------------------------------------------------*/
FAULTPTR selectNextFaults(FAULTPTR pFault) //SelectNextFaults
{
	register int i, j, iValue;
	register GATEPTR pGate;
	register FAULTPTR pNextFault;
	register EVENTPTR pEvent;
	//GATEPTR temp;

	/* Select upto 32 faults to be simulated and inject them */
	i = 0;
	clear(g_InGatesStack);
	clear(g_OutGatesStack);

	while (i < SIZE_OF_FUT) //SIZE_OF_FUT == 32
	{
		if ((pNextFault = pFault->next) == NULL)
		{
			break; //EXIT !!
		}

		if ((pNextFault->detected == DETECTED) || (pNextFault->detected == REDUNDANT) ||
			/*STOP***************************************STOP*/
			(xdetectmode == 'y' && pNextFault->detected == XDETECTED))
		{
			//Delete pNextFault !!
			if ((pFault->next = pNextFault->next) == NULL)
			{
				g_pTailFault = pFault; //g_pTailFault go back 1 !!
			}
			if (pNextFault == g_pPotentialFault)
			{
				g_pPotentialFault = pFault; //g_pPotentialFault go back 1 !!
			}
			continue;
		}

		/* Dynamic Fault Ordering */
		if (dynamic_order_flag == 1)
		{
			if (pFault == g_pPotentialFault || pNextFault == g_pPotentialFault)
			{
				dynamic_order_flag = 0;
			}
			else if (pNextFault->npot > 0)
			{
				pFault->next = pNextFault->next;
				g_pTailFault->next = pNextFault;
				g_pTailFault = pNextFault;
				pNextFault->next = NULL;
				continue;
			}
		}

		if (pNextFault->event == NULL)
		{
			/* single pEvent fault */
			/* Check 1: faulty gate output */
			pGate = pNextFault->gate;
			iValue = g_faultType2Value[pNextFault->type];

			//Output Fault !!
			if (pNextFault->line < 0)
			{
				if (pGate->SGV == iValue) //NO NEED to recalculate !!
				{
					pFault = pFault->next;
					continue;
				}
				//pGate->SGV != iValue
				//Here doesn't matter !!
			}
			//Input Fault !!
			else
			{
				if (pGate->inList[pNextFault->line]->SGV == iValue) //InGate's value == pNextFault's value
				//NO NEED to recalculate !!
				{
					pFault = pFault->next;
					continue;
				}

				if (pGate->type == DFF)
				{
					//STOP*************************************STOP
					create(pEvent);
					pEvent->node = pGate->index;
					pEvent->value = LEVEL_TO_EVENT[iValue];
					pEvent->next = NULL;
					pNextFault->event = pEvent;
					pFault = pFault->next; continue;
				}

				iValue = g_truthTable1[pGate->type][iValue];
				for (j = 0; j< pGate->inCount; j++)
				{
					if (j != pNextFault->line)
					{
						//InGate's SGV + type ---------> iValue
						iValue = g_truthTable2[pGate->type][iValue][pGate->inList[j]->SGV];
					}
				}
				if (iValue == pGate->SGV)
				{
					//Gate's value == calculated value, NO NEED to recalculate !!
					pFault = pFault->next;
					continue;
				}
				//pGate->SGV != iValue
				//Here doesn't matter !!
			}

			pGate->FV[0] = iValue; // 0 or 1 or 2
			pGate->Gid = ++g_iGroupID;
			g_iStemIndex = (-1);

			if ((pGate = CheckSingleEvent(pNextFault, pGate, g_iGroupID)) == NULL)
			{
				pFault = pFault->next;
				continue;
			}
			g_FUTList[i] = pNextFault;
			g_SUTList[i].gate = pGate;
			j = pGate->FV[0];
			g_SUTList[i].faultType = j; //Type v.s. Value ??
			g_SUTList[i].faultLine = OUTFAULT;
			g_SUTList[i].event = NULL;
			push(g_stack1, pGate);
			g_pStems[pGate->stemIndex].flag[j] = i;
			g_pStems[pGate->stemIndex].fault[j] = pNextFault;
			if (g_iStemIndex > 0)
			{
				g_pStems[g_iStemIndex].flag[ssval] = i;
				g_pStems[g_iStemIndex].fault[ssval] = pNextFault;
				g_pSStems[++g_iSStem].stem = g_iStemIndex;
				g_pSStems[g_iSStem].val = ssval;
				push(g_stack1, g_net[g_pStems[g_iStemIndex].gate]);
			}
		}
		else //pNextFault->pEvent != NULL
		{
			//Move the pEvent from pNextFault to g_SUTList !!
			g_FUTList[i] = pNextFault;
			g_SUTList[i].gate = pNextFault->gate;
			g_SUTList[i].faultType = pNextFault->type;
			g_SUTList[i].faultLine = pNextFault->line;
			g_SUTList[i].event = pNextFault->event;
			pNextFault->event = NULL;
		}

		i++;
		pFault = pFault->next;
	}

	if ((g_iFUT = i) == 0)
	{
		return(pFault);	/* end of fault simulation */
	}

	/* Schedule faulty events */
	g_iGroupID++;

	for (i = 0; i < g_iFUT; i++) // 1 <= g_iFUT <= SIZE_OF_FUT (32)
	{
		injectFault(g_SUTList[i].gate, g_SUTList[i].faultType, g_SUTList[i].faultLine, i);
	}

	return(pFault);
}

#ifdef DIAGNOSIS
/*------SelectOneFault-------------------------------------------
	Selects one fault and simulates.
	Used only for fault diagnosis.
	Does not emply parallel fault simulation techniques of hope.
	Prints faulty outputs for each fault.
	Inputs: cf	head of remainning fault list
	Outputs: returns next head fault
------------------------------------------------------------------*/
FAULTPTR SelectOneFault(FAULTPTR pFault)
{
	register int i, j, iValue;
	register GATEPTR pGate;
	register FAULTPTR pNextFault;
	register EVENTPTR pEvent;
	//GATEPTR temp;

	/* Select upto 32 faults to be simulated and inject them */
	i = 0;
	clear(g_InGatesStack);
	clear(g_OutGatesStack);

	while (i < SIZE_OF_FUT) //SIZE_OF_FUT == 32
	{
		if ((pNextFault = pFault->next) == NULL)
		{
			break; //EXIT !!
		}

		if ((pNextFault->detected == DETECTED) || (pNextFault->detected == REDUNDANT) ||
			/*STOP***************************************STOP*/
			(xdetectmode == 'y' && pNextFault->detected == XDETECTED))
		{
			//Delete pNextFault !!
			if ((pFault->next = pNextFault->next) == NULL)
			{
				g_pTailFault = pFault; //g_pTailFault go back 1 !!
			}
			if (pNextFault == g_pPotentialFault)
			{
				g_pPotentialFault = pFault; //g_pPotentialFault go back 1 !!
			}
			continue;
		}

		//Move the pEvent from pNextFault to g_SUTList !!
		g_FUTList[i] = pNextFault;
		g_SUTList[i].gate = pNextFault->gate;
		g_SUTList[i].faultType = pNextFault->type;
		g_SUTList[i].faultLine = pNextFault->line;
		g_SUTList[i].event = pNextFault->event;
		pNextFault->event = NULL;

		i++;
		pFault = pFault->next;
	}

	if ((g_iFUT = i) == 0)
	{
		return(pFault);	/* end of fault simulation */
	}

	/* Schedule faulty events */
	g_iGroupID++;

	for (i = 0; i < g_iFUT; i++) // 1 <= g_iFUT <= SIZE_OF_FUT (32)
	{
		injectFault(g_SUTList[i].gate, g_SUTList[i].faultType, g_SUTList[i].faultLine, i);
	}

	return(pFault);
}
#endif

/*------Simulation-------------------------------------------------
	Performs fault simulation for the given input vector
	based on the previous time frame good value.
	Returns the number of faults detected.
-------------------------------------------------------------------*/
int FaultSim_HOPE() //Simulation
{
	register int i;
	register FLINKTYPE* flink;
	register FAULTPTR pFault;
	register EVENTPTR pEvent, pTempEvent;
	GATEPTR pGate;

	g_iNoDetected = 0;
	g_pCurrentFault = g_pHeadFault;

#ifdef DIAGNOSIS //Never defined !!
	//STOP****************************************STOP
	if (dictmode == 'y')
	{
		dynamic_order_flag = 0;
	}
	else
#endif

	if (g_pCurrentFault == g_pPotentialFault)
	{
		dynamic_order_flag = 0;
	}
	else if (g_pCurrentFault->next == g_pPotentialFault)
	{
		dynamic_order_flag = 0;
	}
	else
	{
		dynamic_order_flag = 1;
	}

	while (g_pCurrentFault->next != NULL)
	{
		g_pCurrentFault = 
#ifdef DIAGNOSIS //Never defined !!
	//STOP****************************************STOP
		(diagmode == 'y') ? SelectOneFault(g_pCurrentFault) :
#endif
		selectNextFaults(g_pCurrentFault);
		//g_pCurrentFault = SelectNextFaults(g_pCurrentFault);


		if (g_iFUT == 0)
			break;
		FaultSim(0, g_iMaxLevel, g_iGroupID);		/* box 9 */
		DropDetectedFaults();			/* box 10 */
		StoreFaultyStatus();			/* box 11 */

		while (g_iSStem >= 0)
		{
			g_pStems[g_pSStems[g_iSStem].stem].flag[g_pSStems[g_iSStem].val] = SIMULATED;
			g_iSStem--;
		}

		for (i = 0; i < g_iFUT; i++)
		{
			g_SUTList[i].gate->type = g_SUTList[i].gateType;

			if (g_SUTList[i].event != NULL)
				continue;
			g_pStems[g_SUTList[i].gate->stemIndex].flag[g_SUTList[i].faultType] = SIMULATED;

			while ((flink = g_SUTList[i].extra) != NULL)
			{
				pFault = flink->fault;
				for (pEvent = g_FUTList[i]->event; pEvent != NULL; pEvent = pEvent->next)
				{
					create(pTempEvent);
					pTempEvent->node = pEvent->node;
					pTempEvent->value = pEvent->value;
					pTempEvent->next = pFault->event;
					pFault->event = pTempEvent;
				}
				switch (g_FUTList[i]->detected)
				{
				case DETECTED:
					pFault->detected = DETECTED;
					g_iNoDetected++;
#ifdef DIAGNOSIS
					pFault->diag_id = diag_id;
#endif
					break;
#ifndef ATALANTA
				case XDETECTED:
					pFault->detected = XDETECTED;
					if (xdetectmode == 'y')
					{
						g_iNoDetected++;
#ifdef DIAGNOSIS
						pFault->diag_id = diag_id;
#endif
					}
#endif
				}
				g_SUTList[i].extra = flink->next;
				FREE(flink);
			}
		}

#ifdef DIAGNOSIS
		if (diagmode == 'y')
			Print_Faulty_Values(diagfile, g_FUTList, g_iFUT, diag_id);
#endif
	}

	while (!is_empty(g_stack1))
	{
		pGate = pop(g_stack1);
		g_pStems[pGate->stemIndex].flag[0] = g_pStems[pGate->stemIndex].flag[1] = g_pStems[pGate->stemIndex].flag[2] = UNSIMULATED;
	}

	if (g_iGroupID > MAXINTEGER)
	{
		g_iGroupID = 0;
		for (i = 0; i < g_iNoGate; i++)
			g_net[i]->Gid = 0;
	}

	return(g_iNoDetected);
}
