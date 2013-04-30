
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
	file name: define_fault_list.c
	This file constructs fault list after collapsing equivalent
	faults.
-------------------------------------------------------------------*/
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "hash.h"
#include "define_fault_list.h"

#include "parameter.h" 
#include "define.h"    
#include "macro.h"

extern GATEPTR *g_net;
extern STACKTYPE g_stack;
extern struct FAULT **g_pFaultList;

extern char rptmode;
extern HASHPTR pArrSymbolTable[];
// extern HASHPTR FindHash();
// extern void fatalerror();

/*	set_all_fault_list
	Sets up the fault list data structure.
	Equivalent faults are collapsed.
*/
int initStemFaultsAndFaultList(int iNoGate, int iNoStemGates, GATEPTR *pStemGates) //set_all_fault_list
{
	//OUTPUT: iNoFault
	register GATEPTR pGate;
	register FAULTPTR pFault;
	register int i, j;
	FAULTPTR pPreFault;
	fault_type iFaultType;
	int iNoFault, iNoStemFaultAdd1, iNoFaultTemp;
	/**********************************
	//int *piTest, size;
	//FAULTTYPE *curr;

	//piTest = (int*) malloc(sizeof(int));
	//size = sizeof(FAULTTYPE); 
	//curr = (FAULTTYPE *)calloc(1, sizeof(FAULTTYPE));

	//Never Used !
	**********************************/
	iNoFault = 0;
	
	pPreFault = (FAULTPTR)malloc(sizeof(FAULTTYPE));
	pPreFault->next = NULL;

	for (i = 0; i < iNoGate; i++)
	{
		/* input line faults */
		pGate = g_net[i];
		pGate->pfault = pPreFault;
		pGate->nfault = 0; // 0 also has a pPreFault object !!!

		/* if the input of the gate has more than one fanouts, 
									add a s-a-1 for each AND/NAND,
									  a s-a-0 for each OR/NOR and
									  a s-a-0 and s-a-1 for other gates. */

		if (pGate->inCount > 1) //>= 2 inputs !!
		{
			iFaultType = (pGate->type == AND || pGate->type == NAND) ? SA1 :
						/*(pGate->type == OR || pGate->type == NOR)*/ SA0;
			for (j = 0; j< pGate->inCount; j++)
			{
				if (pGate->inList[j]->outCount > 1) //FANOUT !!
				{
					pFault = (FAULTPTR)malloc(sizeof(FAULTTYPE));
					pFault->gate = pGate;
					pFault->type = iFaultType;
					pFault->line = j;
					pFault->next = NULL;
					iNoFault++;
					pGate->nfault++;
					pPreFault->next = pFault;
					//(pGate->pfault) ---> ... ---> ... ---> pPreFault ---> pFault
					if (pPreFault == pGate->pfault) //For 1st time !!
					{
						pFault->previous = pFault; //don't point to pPreFault for 1st time
					}
					else
					{
						pFault->previous = pPreFault;
					}
					pPreFault = pFault;
					
					/* case of high level gates */
					if (pGate->type > PI) //XOR XNOR PO, add both SA1 & SA0 faults !!
					{
						ALLOCATE(pFault, FAULTTYPE, 1);
						pFault->gate = pGate;
						pFault->type = (iFaultType == SA1) ? SA0 : SA1;
						pFault->line = j;
						pFault->next = NULL;
						iNoFault++;
						pGate->nfault++;
						pPreFault->next = pFault;
						//(pGate->pfault) ---> ... ---> ... ---> pPreFault ---> pFault
						if (pPreFault == pGate->pfault)
						{
							pFault->previous = pFault;
						}
						else
						{
							pFault->previous = pPreFault;
						}
						pPreFault = pFault;
					}
				}
			}
		}

		/* output fault assignment:
									add s-a-1 and s-a-0 faults for each fanout stem or
									primary output. */

		//(output==1) & (CoInput || PO)
		if ((pGate->outCount == 1) && (pGate->outList[0]->inCount > 1 || pGate->outList[0]->type == PO))
		{
			//1)CoInput
			//pGate ----------------->pGate->outList[0]
			//Other Gates ------------>pGate->outList[0]

			//2)PO
			//pGate ----------------->pGate->outList[0] (PO)
			iFaultType = (pGate->outList[0]->type == OR || pGate->outList[0]->type == NOR) ? SA0 :
						/*(pGate->outList[0]->type == AND || pGate->outList[0]->type == NAND)*/ SA1;
			ALLOCATE(pFault, FAULTTYPE, 1);
			pFault->gate = pGate;
			pFault->type = iFaultType;
			pFault->line = OUTFAULT;
			pFault->next = NULL;
			iNoFault++;
			pGate->nfault++;
			pPreFault->next = pFault;
			//(pGate->pfault) ---> ... ---> ... ---> pPreFault ---> pFault
			if (pPreFault == pGate->pfault)
			{
				pFault->previous = pFault;
			}
			else
			{
				pFault->previous = pPreFault;
			}
			pPreFault = pFault;
			
			/* case of high level gates */
			if (pGate->outList[0]->type > PI) //XOR XNOR NOT PO, add both SA1 & SA0 faults !!
			{
				pFault = (FAULTPTR)malloc(sizeof(FAULTTYPE));
				pFault->gate = pGate;
				pFault->type = (iFaultType == SA1) ? SA0 : SA1;
				pFault->line = OUTFAULT;
				pFault->next = NULL;
				iNoFault++;
				pGate->nfault++;
				pPreFault->next = pFault;
				//(pGate->pfault) ---> ... ---> ... ---> pPreFault ---> pFault
				if (pPreFault == pGate->pfault)
				{
					pFault->previous = pFault;
				}
				else
				{
					pFault->previous = pPreFault;
				}
				pPreFault = pFault;
			}
		}
		//FANOUT
		//add both SA0 & SA1 !!!
		else if (pGate->outCount > 1)
		{
			pFault = (FAULTPTR)malloc(sizeof(FAULTTYPE));
			pFault->gate = pGate;
			pFault->type = SA1;
			pFault->line = OUTFAULT;
			pFault->next = NULL;
			iNoFault++;
			pGate->nfault++;
			pPreFault->next = pFault;
			//(pGate->pfault) ---> ... ---> ... ---> pPreFault ---> pFault
			if (pPreFault == pGate->pfault)
			{
				pFault->previous = pFault;
			}
			else
			{
				pFault->previous = pPreFault;
			}
			pPreFault = pFault;
			pFault = (FAULTPTR)malloc(sizeof(FAULTTYPE));
			pFault->gate = pGate;
			pFault->type = SA0;
			pFault->line = OUTFAULT;
			pFault->next = NULL;
			iNoFault++;
			pGate->nfault++;
			pPreFault->next = pFault;
			//(pGate->pfault) ---> ... ---> ... ---> pPreFault ---> pFault
			if (pPreFault == pGate->pfault)
			{
				pFault->previous = pFault;
			}
			else
			{
				pFault->previous = pPreFault;
			}
			pPreFault = pFault;
		}
		//PO & CoInput
		//add both SA0 & SA1 !!!
		else if (pGate->type == PO && pGate->inList[0]->outCount > 1)
		{
			//pGate->inList[0] --------------> pGate (PO)
			//pGate->inList[0] --------------> Other Gates
			pFault = (FAULTPTR)malloc(sizeof(FAULTTYPE));
			pFault->gate = pGate;
			pFault->type = SA1;
			pFault->line = 0;
			pFault->next = NULL;
			iNoFault++;
			pGate->nfault++;
			pPreFault->next = pFault;
			//(pGate->pfault) ---> ... ---> ... ---> pPreFault ---> pFault
			if (pPreFault == pGate->pfault)
			{
				pFault->previous = pFault;
			}
			else
			{
				pFault->previous = pPreFault;
			}
			pPreFault = pFault;
			ALLOCATE(pFault, FAULTTYPE, 1);
			pFault->gate = pGate;
			pFault->type = SA0;
			pFault->line = 0;
			pFault->next = NULL;
			iNoFault++;
			pGate->nfault++;
			pPreFault->next = pFault;
			//(pGate->pfault) ---> ... ---> ... ---> pPreFault ---> pFault
			if (pPreFault == pGate->pfault)
			{
				pFault->previous = pFault;
			}
			else
			{
				pFault->previous = pPreFault;
			}
			pPreFault = pFault;
		}

		//get pPreFault back !! So MAGIC !!
		pPreFault = pGate->pfault;
		if (pGate->pfault->next == NULL) //NO Fault generated!!
		{
			pGate->pfault = NULL; //Still the pPreFault object !!
		}
		else //New Fault !!
		{
			pGate->pfault = pPreFault->next;
		}
		pPreFault->next = NULL;
	}	/* end for */

	free((char*)pPreFault);

	/* create the fault_list and
	   enumerate faults in each fanout free region */
	g_pFaultList = (FAULTPTR *)malloc((unsigned)(sizeof(FAULTPTR) * iNoFault));
	clear(g_stack);
	iNoFaultTemp = 0;
	for (i = iNoStemGates - 1; i >= 0; i--)
	{
		push(g_stack, pStemGates[i]);
		iNoStemFaultAdd1 = 1;
		while (!is_empty(g_stack))
		{
			pGate = pop(g_stack);
			for (pFault = pGate->pfault; pFault != NULL; pFault = pFault->next)
			{
				//Spread within the FFR !!
				g_pFaultList[iNoFaultTemp++] = pFault; //Core sentence
				iNoStemFaultAdd1++; //Core sentence
			}
			for (j = 0; j< pGate->inCount; j++)
			{
				if (pGate->inList[j]->outCount == 1) //With FFR
				{
					push(g_stack, pGate->inList[j]);
				}
			}
		}
		pStemGates[i]->dfault = (FAULTPTR *)malloc((unsigned)(iNoStemFaultAdd1 * sizeof(FAULTPTR)));
	}
	//OUTPUT: pStemGates[i]->dfault && g_pFaultList

	if (iNoFault == iNoFaultTemp)
	{
		return(iNoFault);
	}
	else
	{
		//ERROR !!
		return(-1);
	}
}

/*	add_fault
	Add a fault to the linked list of each gate.
*/
void addFaultToGate(FAULTPTR pFault) //add_fault
{
	//add pGate->pfault!!!
	GATEPTR pGate; FAULTPTR pOldFault;

	pGate = pFault->gate;
	if (pGate->pfault == NULL) //The first fault
	{
		pGate->pfault = pFault;
		pFault->previous = pFault;
		pGate->nfault = 1;
	}
	else //Not the first
	{
		pOldFault = pGate->pfault;
		while (pOldFault->next != NULL)
			pOldFault = pOldFault->next;
		pOldFault->next = pFault;
		pFault->previous = pOldFault;
		pGate->nfault++;
	}
	pFault->next = NULL;
}

/*	restore_detected_fault_list
	Restores the fault list for test compaction.
	Does not restore redundant faults.
*/
int restoreUndetectedState_FSIM(int iNoFault) //restore_detected_fault_list
{
	register FAULTPTR pFault;
	register int i, iNoRestored;

	iNoRestored = 0;
	for (i = 0; i < iNoFault; i++)
	{
		pFault = g_pFaultList[i];
		if (pFault->detected == DETECTED)
		{
			pFault->detected = UNDETECTED;
			addFaultToGate(pFault);
			iNoRestored++;
		}
	}
	return(iNoRestored);
}

/*	check_redundant_faults
	Identifies redundant faults in which more than two inputs of
	a gate are connected to one gate simultaneously and deletes
	the faults from the fault list.

	Input: nog, the number of gates of the circuit
	Output: number of identified redundant faults
	Note: changed flags of all gates should be reset to 0
		  prior to call this function.
*/

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

int deleteRedundantFaults(int iNoGate) //check_redundant_faults
{
	register int i, j;
	int iNoRedundant;
	register GATEPTR pFanoutGate;
	FAULTPTR pFault;

	iNoRedundant = 0;
	for (i = 0; i < iNoGate; i++)
	{
		if (g_net[i]->outCount > 1) //for every FANOUT pFanoutGate !!!
		{
			pFanoutGate = g_net[i]; //FANOUT
			for (j = 0; j< pFanoutGate->outCount; j++)
			{
				//changed = 0
				(pFanoutGate->outList[j]->changed)++;
			}
			
			for (j = 0; j< pFanoutGate->outCount; j++)
			{
				if (pFanoutGate->outList[j]->changed > 1) //for every pFanoutGate->outList[j] && changed > 1
				{
					for (pFault = pFanoutGate->outList[j]->pfault; pFault != NULL; pFault = pFault->next)
					{
						if (pFault->line >= 0)
						//pFanoutGate->outList[j] == (pFault->gate)
						//(pFault->gate->inList[pFault->line]) ----> pFault ----> (pFault->gate)

						
						//pFanoutGate ------------------------------> pFault ----> pFanoutGate->outList[j]
						//Other Gates ------------------------->pFault ----> pFanoutGate->outList[j]
						{
							if (pFault->gate->inList[pFault->line] == pFanoutGate) //pFanoutGate is 2rd gate which faults!!
							{
								pFault->detected = REDUNDANT;
								if (--pFault->gate->nfault > 0) //At least 1 fault remains !!
								{
									//ALWAYS !!
									delete_fault(pFault);
								}
								iNoRedundant++;
							}
						}
					}
				}
				reset(pFanoutGate->outList[j]->changed); //change to 0 instead of 1 !!
			}
		}
	}
	return(iNoRedundant);
}


/*******************************************************************
The following routines are used to read fault directory.
The file format is the same as that of ISCAS89 fault list file.
**********************************************************************/

#define is_white_space(c) (c==' ' || c=='-' || c=='\t' || c=='\n')
#define is_head_symbol(c) (c=='/' || c=='>')
#define is_valid(c) ((c>='0' && c<='9') || \
			 (c>='A' && c<='Z') || \
			 (c>='a' && c<='z') || \
			 (c=='[' || c==']') || \
			 (c=='_'))

/*------getfaultsymbol-------------------------------------------------
	reads next symbol from the given fault file
	inputs : file	the name of input file
	outputs: s	symbol string output
		returns head character of s unless EOF is encountered.
-------------------------------------------------------------------*/
#define	EOS	'\0'

char getfaultsymbol(FILE *file, char s[])
{
	char c;
	int n = 0;
	status valid = FALSE;

	while ((c = getc(file)) != EOF)
	{
		if (is_white_space(c))
		{
			if (valid)
			{
				break;
			}
			else
			{
				continue;
			}
		}
		if (is_head_symbol(c))
		{
			s[n++] = c; continue;
		}
		if (is_valid(c))
		{
			s[n++] = c; set(valid);
		}
		else
		{
			printFatalError(FAULTERROR);
		}
	}
	s[n] = EOS;
	if (c == EOF)
	{
		return(c);
	}
	else
	{
		return(s[0]);
	}
}


/*------readfaults------------------------------------------------
	reads fault list and constructs fault list data structure
	inputs: file	fault directory file
	outputs: fault list
------------------------------------------------------------------*/
int readFaults(FILE *fpFaultFile, int g_iNoGate, int iStem, GATEPTR *pStem) //readfaults
{
	//OUTPUT:  g_pFaultList
	GATEPTR gut;
	FAULTPTR f;
	int from, to, line, type;
	HASHPTR h;
	int i, j;
	//char c;
	char s[MAXSTRING];
	int nfault, n, nof;

	nfault = 0;
	for (i = 0; i < g_iNoGate; i++)
	{
		gut = g_net[i];
		gut->pfault = (FAULTPTR)NULL;
		gut->nfault = 0;
	}

	while (getfaultsymbol(fpFaultFile, s) != EOF)
	{
		if (is_valid(s[0]))
		{
			if ((h = FindHash(pArrSymbolTable, HASHSIZE, s, 0)) == NULL)
			{
				fprintf(stderr, "Error in fault file:");
				fprintf(stderr, "%s is not defined\n", s);
				printFatalError(FAULTERROR);
			}
			if ((to = h->gate->index) < 0)
			{
				printFatalError(FAULTERROR);
			}
			gut = g_net[to];
			line = (-1);
		}
		else if (s[0] == '>')
		{
			from = to;
			if ((h = FindHash(pArrSymbolTable, HASHSIZE, &s[1], 0)) == NULL)
			{
				fprintf(stderr, "Error in fault file:");
				fprintf(stderr, "%s is not defined\n", s);
				printFatalError(FAULTERROR);
			}
			if ((to = h->gate->index) < 0)
			{
				printFatalError(FAULTERROR);
			}
			gut = g_net[to];
			for (i = 0; i< gut->inCount; i++)
				if (gut->inList[i]->index == from)
				{
					line = i; break;
				}
		}
		else if (s[0] == '/')
		{
			if (s[1] == '1')
			{
				type = SA1;
			}
			else
			{
				type = SA0;
			}
			if (line >= 0)
			{
				type = (type == SA1) ? SA1 : SA0;
			}

			ALLOCATE(f, FAULTTYPE, 1);
			f->gate = gut;
			f->line = line;
			f->type = type;
			f->previous = f;
			if ((f->next = gut->pfault) != NULL)
			{
				f->next->previous = f;
			}
			gut->pfault = f;
			nfault++;
			gut->nfault++;
			/*  	 while((c=getc(file)) != '\n') if(c==EOF) break; */
		}
		else
		{
			printFatalError(FAULTERROR);
		}
	}

	/* create the fault_list and
	   enumerate faults in each fanout free region */
	g_pFaultList = (FAULTPTR *)malloc((unsigned)(sizeof(FAULTPTR) * nfault));
	nof = 0;
	clear(g_stack);
	for (i = iStem - 1; i >= 0; i--)
	{
		push(g_stack, pStem[i]);
		n = 1;
		while (!is_empty(g_stack))
		{
			gut = pop(g_stack);
			for (f = gut->pfault; f != NULL; f = f->next)
			{
				g_pFaultList[nof++] = f; n++;
			}
			for (j = 0; j< gut->inCount; j++)
				if (gut->inList[j]->outCount == 1)
				{
					push(g_stack, gut->inList[j]);
				}
		}
		pStem[i]->dfault = (FAULTPTR *)malloc((unsigned)(n * sizeof(FAULTPTR)));
	}

	if (nfault == nof)
	{
		return(nfault);
	}
	else
	{
		return(-1);
	}
}

#ifdef INCLUDE_HOPE

/*----------------------------------------------------------------- 
	The following routines creates the fault list for HOPE
-------------------------------------------------------------------*/
extern int g_iNoGate, g_iNoPI, g_iNoPO, g_iNoFF, g_iMaxLevel, g_iNoFault;
extern int *g_PrimaryIn, *g_PrimaryOut, *g_FlipFlop;
extern GATEPTR *g_net;
extern STACKTYPE g_stack1, g_stack2, *g_pEventListStack;
extern FAULTPTR g_pHeadFault, g_pCurrentFault, g_pTailFault, *g_pFaultList;
extern HASHPTR pArrSymbolTable[];
// extern HASHPTR FindHash();
// extern void fatalerror();

FAULTPTR g_pEvenHeadFault, g_pEvenTailFault, g_pOddHeadFault, g_pOddTailFault;
int g_parityForGate[MAXGTYPE] =
{
	   0,	  1,	 0,		 1,		 0,		 0,		  0, 	 0, 	   0,		  0,	 1,
}; //AND	NAND	OR		NOR	PI		XOR	XNOR	DFF		DUMMY		BUFF	NOT
int inverseParity[2][2] =
{
	{0,1}, {1,0}
};


/*------init_fault_list--------------------------------------------
	initializes fault list including headfault,tailfault
	headfault indicates one dummy fault
-------------------------------------------------------------------*/
void initFaultList() //init_fault_list
{
	ALLOCATE(g_pHeadFault, FAULTTYPE, 1);
	g_pHeadFault->gate = NULL;
	g_pHeadFault->next = NULL;
	g_pHeadFault->event = NULL;
	g_pEvenHeadFault = g_pEvenTailFault = g_pOddHeadFault = g_pOddTailFault = NULL;
	g_pTailFault = g_pHeadFault;
}

/*------insert_fault------------------------------------------------
	inserts a fault at the end of the fault list
	used for fault ordering. Parity of each fault is
	checked and stored in even and odd fault list.
	inputs: gut; faulty gate
		line; faulty line, if (-1), output fault
		type; fault type
	outputs: evenhead,eventail,oddhead,oddtail
-------------------------------------------------------------------*/
void insertFaultForGate(GATEPTR pGate, int iLineType, fault_type iFaultType) //insert_fault
{
	int iParity;
	FAULTPTR pFault;

	iParity = (pGate->changed >= 2) ? pGate->changed - 2 : pGate->changed;
	if (iLineType < 0)
	{
		iParity = inverseParity[g_parityForGate[pGate->type]][iParity];
	}

	ALLOCATE(pFault, FAULTTYPE, 1);
	pFault->gate = pGate;
	pFault->line = iLineType;
	pFault->type = iFaultType;
	pFault->next = NULL;
	pFault->event = NULL;
	pFault->npot = 0;

	g_iNoFault++;

	if ((iParity == 0 && iFaultType == SA0) || (iParity == 1 && iFaultType == SA1))
	{
		if (g_pEvenTailFault == NULL)
		{
			g_pEvenHeadFault = g_pEvenTailFault = pFault;
		}
		else
		{
			g_pEvenTailFault->next = pFault;
			g_pEvenTailFault = pFault;
		}
	}
	else //(iParity == 0 && iFaultType == SA1) || (iParity == 1 && iFaultType == SA0)
	{
		if (g_pOddTailFault == NULL)
		{
			g_pOddHeadFault = g_pOddTailFault = pFault;
		}
		else
		{
			g_pOddTailFault->next = pFault;
			g_pOddTailFault = pFault;
		}
	}
}

/*------default_line_fault-----------------------------------------
	defines default stuck-at faults for the given line
	inputs: gut; gate under consideration
		line; faulty line of the gate, if (-1), output fault
	outputs: none
-------------------------------------------------------------------*/
void insertFaultsForGateByLine(GATEPTR pGate, int iLineType) //default_line_fault
{
	GATEPTR pInGate, pOutGate;

	if (iLineType < 0)
	{
		/* output iLineType fault */
		if (pGate->type == DUMMY || pGate->type == PO)
		{
			return; //PO Eliminated !!
		}

		if (pGate->outCount != 1) //FANOUT
		{
			insertFaultForGate(pGate, OUTFAULT, SA0);
			insertFaultForGate(pGate, OUTFAULT, SA1);
		}
		else //NORMAL Gate
		{
			//pGate ------------> pOutGate
			//Other Gates --------> pOutGate
			pOutGate = pGate->outList[0];
			if (pOutGate->type == DUMMY)
			{
				//STOP***************************STOP
				pOutGate = pOutGate->outList[0];
			}
			
			switch (pOutGate->type)
			{
			case AND:
			case NAND:
				if (pOutGate->inCount > 1)
				{
					insertFaultForGate(pGate, OUTFAULT, SA1);
				}
				break;
			case OR:
			case NOR:
				if (pOutGate->inCount > 1)
				{
					insertFaultForGate(pGate, OUTFAULT, SA0);
				}
				break;
			case XOR:
			case XNOR:
			case DFF:
			case PO:
				insertFaultForGate(pGate, OUTFAULT, SA0);
				insertFaultForGate(pGate, OUTFAULT, SA1);
				break;
			default:
				break;	/* dummy, not, buff */
			}
		}
	}
	else
	{
		//STOP*****************************************STOP
		/* input iLineType fault */

		//pInGate ------------> pGate
		//pInGate ------------->Other Gates
		pInGate = pGate->inList[iLineType];
		if (pInGate->type == DUMMY || pInGate->type == PO)
		{
			//STOP***************************STOP
			pInGate = pInGate->inList[0];
		}
		if (pInGate->outCount > 1)
		{
			switch (pGate->type)
			{
			case AND:
			case NAND:
				if (pGate->inCount > 1)
				{
					insertFaultForGate(pGate, iLineType, SA1);
				}
				break;
			case OR:
			case NOR:
				if (pGate->inCount > 1)
				{
					insertFaultForGate(pGate, iLineType, SA0);
				}
				break;
			case XOR:
			case XNOR:
			case DFF:
			case PO:
				insertFaultForGate(pGate, iLineType, SA0);
				insertFaultForGate(pGate, iLineType, SA1);
				break;
			default:
				break;	/* dummy, not, buffer */
			}
		}
	}
}

#define setParity(pGate, iParity) \
	pGate->changed = inverseParity[g_parityForGate[pGate->type]][iParity]
#define mark(pGate) pGate->changed += 2
#define isNotMarked(pGate) pGate->changed < 2
#define isStem(pGate) \
	((pGate->outCount != 1) || (pGate->outList[0]->type == DFF))

/*------ Sets fault list for the given FFRs ------*/
void initFFRFaults(GATEPTR pGate) //FFR_Fault
{
	//pGate == [STEM Gate]
	//OUTPUT:  g_pEvenHeadFault etc.. && g_iNoFault
	register GATEPTR pInGate;
	register int i;

	clear(g_stack1);
	g_pEvenHeadFault = g_pEvenTailFault = g_pOddHeadFault = g_pOddTailFault = NULL;

	push(g_stack1, pGate);
	while (!is_empty(g_stack1))
	{
		pGate = pop(g_stack1);
		insertFaultsForGateByLine(pGate, OUTFAULT); //Core sentence	g_pEvenHeadFault etc. updated !!
		for (i = 0; i< pGate->inCount; i++)
		{
			pInGate = pGate->inList[i];
			if (isStem(pInGate))
			{
				insertFaultsForGateByLine(pGate, i); //Core sentence
			}
			else
			{
				push(g_stack1, pInGate); //Within FFR !!
			}
		}
	}

	//g_pTailFault ---> g_pEvenHeadFault ---> g_pEvenTailFault ---> g_pOddHeadFault ---> g_pOddTailFault
	if (g_pEvenTailFault == NULL)
	{
		g_pEvenHeadFault = g_pOddHeadFault;
		g_pEvenTailFault = g_pOddTailFault;
	}
	else
	{
		g_pEvenTailFault->next = g_pOddHeadFault;
	}
	if ((g_pEvenTailFault = (g_pOddTailFault == NULL) ? g_pEvenTailFault : g_pOddTailFault) != NULL)
	{
		//g_pEvenTailFault = g_pOddTailFault (if g_pOddTailFault != NULL)
		g_pTailFault->next = g_pEvenHeadFault;
		g_pTailFault = g_pEvenTailFault;
		//g_pTailFault ---> g_pEvenHeadFault ---> g_pEvenTailFault ---> g_pOddHeadFault ---> g_pOddTailFault
	}
}

/*------DFS_po--------------------------------------------------------
	Subroutine of DFS search from primary outputs
---------------------------------------------------------------------*/
void initFaultsForPOByDFS(GATEPTR pParentGate, GATEPTR pGate) //DFS_po
{
	//ALWAYS:  pParentGate == NULL
	//Initially: pGate == PO Gate (Every PO executes this method)
	int i;

	/* preWORK */
	setParity(pGate, (pParentGate == NULL ? 0 : pParentGate->changed - 2)); //The same as:
	//pGate->changed = parity_of_gate[pGate->type]; //NAND & NOR & NOT ---> changed = TRUE
	
	mark(pGate); //pGate->changed += 2

	if (isStem(pGate)) //PO & FANOUT
	{
		initFFRFaults(pGate);
	}

	/* Go into children */
	for (i = 0; i< pGate->inCount; i++)
	{
		/* preWORK for input lines */
		if (isNotMarked(pGate->inList[i]))
		{
			initFaultsForPOByDFS(pGate, pGate->inList[i]);
		}
	}
}

/*------DFS_faults----------------------------------------------------
	defines fault list in the order of DFS from primary outputs
	use recursive version DFS routines.
	main of DFS_po.
---------------------------------------------------------------------*/
void initFaultList_HOPE() //FWD_faults
{
	register int i;
	register GATEPTR pGate;

	for (i = 0; i < g_iNoGate; i++)
	{
		reset(g_net[i]->changed);
	}

	initFaultList();

	/* Primary Outputs */
	for (i = 0; i < g_iNoPO; i++)
	{
		pGate = g_net[g_PrimaryOut[i]];
		initFaultsForPOByDFS(NULL, pGate);
	}

	/* count faults and copy */
	//
	/////////////// [g_pHeadFault, g_pTailFault] ---> g_pFaultList /////////////
	//
	g_iNoFault = 0; //g_iNoFault already has the CORRECT value !!
	g_pCurrentFault = g_pHeadFault;
	while (g_pCurrentFault->next != NULL)
	{
		g_iNoFault++;
		g_pCurrentFault = g_pCurrentFault->next;
	}

	ALLOCATE(g_pFaultList, FAULTPTR, g_iNoFault);
	i = 0;
	g_pCurrentFault = g_pHeadFault;

	while (g_pCurrentFault->next != NULL)
	{
		g_pFaultList[i++] = g_pCurrentFault->next;
		g_pCurrentFault = g_pCurrentFault->next;
	}
	g_pTailFault = g_pFaultList[g_iNoFault - 1]; ////g_pTailFault already has the CORRECT value !!
}

/*------readfaults_hope-------------------------------------------
	reads fault list and constructs fault list data structure
	inputs: file	fault directory file
	outputs: fault list
------------------------------------------------------------------*/
void readFaults_HOPE(FILE *file) //readfaults_hope
{
	GATEPTR gut;
	FAULTPTR f;
	int from, to, line, type;
	HASHPTR h;
	int i;
	char s[MAXSTRING];

	initFaultList();

	while (getfaultsymbol(file, s) != EOF)
	{
		if (is_valid(s[0]))
		{
			if ((h = FindHash(pArrSymbolTable, HASHSIZE, s, 0)) == NULL)
			{
				fprintf(stderr, "Error in fault file:");
				fprintf(stderr, "%s is not defined\n", s);
				printFatalError(FAULTERROR);
			}
			if ((to = h->gate->index) < 0)
			{
				printFatalError(FAULTERROR);
			}
			gut = g_net[to];
			line = OUTFAULT;
		}
		else if (s[0] == '>')
		{
			from = to;
			if ((h = FindHash(pArrSymbolTable, HASHSIZE, &s[1], 0)) == NULL)
			{
				fprintf(stderr, "Error in fault file:");
				fprintf(stderr, "%s is not defined\n", s);
				printFatalError(FAULTERROR);
			}
			if ((to = h->gate->index) < 0)
			{
				printFatalError(FAULTERROR);
			}
			gut = g_net[to];
			for (i = 0; i< gut->inCount; i++)
				if (gut->inList[i]->index == from)
				{
					line = i; break;
				}
		}
		else if (s[0] == '/')
		{
			if (s[1] == '1')
			{
				type = SA1;
			}
			else
			{
				type = SA0;
			}
			ALLOCATE(f, FAULTTYPE, 1);
			f->gate = gut;
			f->line = line;
			f->type = type;
			f->event = NULL;
			f->next = g_pHeadFault->next;
			g_pHeadFault->next = f;
		}
		else
		{
			printFatalError(FAULTERROR);
		}
	}

	/* count faults and copy */
	g_iNoFault = 0;
	g_pCurrentFault = g_pHeadFault;
	while (g_pCurrentFault->next != NULL)
	{
		g_iNoFault++; g_pCurrentFault = g_pCurrentFault->next;
	}

	ALLOCATE(g_pFaultList, FAULTPTR, g_iNoFault);
	i = 0;
	g_pCurrentFault = g_pHeadFault;
	while (g_pCurrentFault->next != NULL)
	{
		g_pFaultList[i++] = g_pCurrentFault->next;
		g_pCurrentFault = g_pCurrentFault->next;
	}
}

/*	restore_hope_fault_list
	Restores the fault list for test compaction.
	Does not restore redundant faults.
*/
extern EVENTPTR g_headEvent, g_tailEvent;

int restoreUndetectedState_HOPE(int iNoFault) //restore_hope_fault_list
{
	register FAULTPTR pFault, pTailFault;
	register int i, iNoRestored;

	iNoRestored = 0;
	pTailFault = g_pHeadFault;
	for (i = 0; i < iNoFault; i++)
	{
		pFault = g_pFaultList[i];
		if (pFault->detected == DETECTED)
		{
			//Restore the g_pHeadFault (global fault chain) !!
			pFault->detected = UNDETECTED;
			pTailFault->next = pFault;
			pTailFault = pFault;
			iNoRestored++;
		}
		if (pFault->event != NULL)
		{
			//Extract the events to global event chain !!
			g_tailEvent->next = pFault->event;
			while (g_tailEvent->next != NULL)
			{
				g_tailEvent = g_tailEvent->next;
			}
			pFault->event = NULL;
		}
	}
	pTailFault->next = NULL;
	g_pTailFault = pTailFault;
	return(iNoRestored);
}

#endif
