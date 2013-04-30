
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
 
		atalanta: version 2.0   	 H. K. Lee, 6/30/1997

		   Linked HOPE with ATALANTA
 
***********************************************************************/

/*----------------------------------------------------------------- 
	lsim.c
	contains all subroutines necessary for 
	zero gate delay event driven logic simulation.

	list all modifications below:
	original: 8/15/1991	Hyung K. Lee   compiled and checked
-------------------------------------------------------------------*/
#include "stdafx.h"
#include <stdio.h>

#include "lsim.h"

#include "parameter.h"
#include "define.h"
#include "macro.h"

extern int g_iNoGate, g_iNoPI, g_iNoPO, g_iNoFF, g_iMaxLevel, g_iPOLevel, g_iPPOLevel;
extern int *g_PrimaryIn, *g_PrimaryOut, *g_FlipFlop;
extern GATEPTR *g_net;
extern STACKTYPE *g_pEventListStack;
extern level g_PIValues[];
extern char initialmode;

#define twoBitsDifferent(V1, V2) (V1[0] != V2[0] || V1[1] != V2[1])
#define twoBitsCopy(Dest, Sour) Dest[0] = Sour[0]; Dest[1] = Sour[1];
level g_TABLE[Z + 1][2] =
{
	{ALL1,ALL0}, {ALL0,ALL1}, {ALL0,ALL0}, {ALL1,ALL1}
};

extern level g_truthTable1[MAXGTYPE][MAXLEVEL];
extern level g_truthTable2[MAXGTYPE][MAXLEVEL][MAXLEVEL];
#define valueDifferent(V1, V2) (V1 != V2)
#define valueCopy(Dest, Sour) Dest = Sour

/*-----GoodSim------------------------------------------------------
	Performs good circuit logic simulation.
	Input patterns are stored in InVal[i].
	Current Flip-Flop values in inputs of each FF.
	Logic values in GVs of each gate structure are
	updated and old values are eliminated.
------------------------------------------------------------------*/
void GoodSim_HOPE(int iNoPatterns) //GoodSim
{
	//NO PARAMETER !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//
	//
	//
	//
	//iNoPatterns is current generated patterns in [tgen_sim]
	//iNoPatterns is 2 in [shuffle_hope]
	//DO NOT give you the 1st time in [shuffle_hope]
	register int i, j;
	register GATEPTR pGate, pTempGate;
	level iValue;

	/* schedule events in flip-flops 
	   --- FFs are modeled as Delay Elements */
	if (iNoPatterns == 1) //The 1st Time !!
	{
		//STOP************************************STOP
		for (i = 0; i < g_iNoFF; i++)
		{
			pGate = g_net[g_FlipFlop[i]];
			iValue = (initialmode == '0') ? ZERO : (initialmode == '1') ? ONE : X;
			if (valueDifferent(pGate->SGV, iValue))
			{
				valueCopy(pGate->SGV, iValue);
				twoBitsCopy(pGate->GV, g_TABLE[iValue]);
				pushGateOutputs2(pGate, j, pTempGate);
			}
		}
	}
	else //The 2rd Time and so on ..
	{
		//STOP************************************STOP
		for (i = 0; i < g_iNoFF; i++)
		{
			pGate = g_net[g_FlipFlop[i]];
			valueCopy(iValue, pGate->inList[0]->SGV);
			if (valueDifferent(pGate->SGV, iValue))
			{
				valueCopy(pGate->SGV, iValue);
				twoBitsCopy(pGate->GV, g_TABLE[iValue]);
				pushGateOutputs2(pGate, j, pTempGate);
			}
		}
	}

	/* schedule event in primary inputs */
	for (i = 0; i < g_iNoPI; i++)
	{
		//InVal == 0 | 1 | X | Z
		pGate = g_net[g_PrimaryIn[i]];
		if (valueDifferent(pGate->SGV, g_PIValues[i]))
		{
			valueCopy(pGate->SGV, g_PIValues[i]);
			twoBitsCopy(pGate->GV, g_TABLE[g_PIValues[i]]);
			pushGateOutputs2(pGate, j, pTempGate);
		}
	}

	for (i = 0; i < g_iPPOLevel; i++)
	{
		while (!is_empty(g_pEventListStack[i]))
		{
			pGate = pop(g_pEventListStack[i]);
			reset(pGate->changed);

			/* gate evaluation */
			//
			if (pGate->inCount == 1)
			{
				//InGates' SGV + Gate's type ----------> Gate's SGV
				iValue = g_truthTable1[pGate->type][pGate->inList[0]->SGV]; //Core Sentence
				
				valueCopy(pGate->SGV, iValue);
				twoBitsCopy(pGate->GV, g_TABLE[iValue]);
				pushGateOutputs2(pGate, j, pTempGate);
			}
			else //pGate->inCount >= 2
			{
				//InGates' SGV + Gate's type ----------> Gate's SGV
				iValue = g_truthTable1[pGate->type][pGate->inList[0]->SGV]; //Core Sentence
				for (j = 1; j< pGate->inCount; j++)
				{
					iValue = g_truthTable2[pGate->type][iValue][pGate->inList[j]->SGV];
				}
				if (valueDifferent(pGate->SGV, iValue))
				{
					valueCopy(pGate->SGV, iValue);
					twoBitsCopy(pGate->GV, g_TABLE[iValue]);
					pushGateOutputs2(pGate, j, pTempGate);
				}
			}
		}
	}

	/* Pseudo-Primary outputs */
	while (!is_empty(g_pEventListStack[g_iPPOLevel]))
	{
		//STOP************************************STOP
		pGate = pop(g_pEventListStack[g_iPPOLevel]);
		reset(pGate->changed);
	}
}

