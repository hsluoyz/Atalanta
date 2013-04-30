
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
 
		Changed Parser and added on-line manual: H. K. Lee, 10/5/1992
		Now, atalanta accepts the circuit written in the netlist format
		of ISCAS89 benchmark circuits as well as the netlist format of
		ISCAS85 benchmark circuits.
 
		atalanta: version 2.0   	 H. K. Lee, 6/30/1997
 
***********************************************************************/

/*--------------------------------------------------------------------- 
	filename io.c
	gettime() returns the CPU time.
-----------------------------------------------------------------------*/
#include "stdafx.h"
#include <stdio.h>
#include <sys/types.h>
#ifdef WIN32
	#include <windows.h>
	#include <psapi.h>
	#pragma comment(lib, "psapi.lib")
#else
	#include <sys/times.h>
#endif
#include <time.h>
#include <stdlib.h>

#include "error.h"

#include "io.h"
#include "parameter.h" 
#include "define.h"    
#include "macro.h"

//#define NULL 0

extern GATEPTR *g_net;
extern int *g_PrimaryIn, *g_PrimaryOut, *g_iHeadGateIndex;
//extern char *strcpy();
//extern void fatalerror();
extern int g_iMaxLevel, g_iPOlevel;
extern int *depth_array;
extern GATEPTR *g_dynamicStack;
extern STACKTYPE g_freeGatesStack,	/* fault free simulation */
				 g_faultyGatesStack,   	  /* list of faulty gates */
				 g_evalGatesStack, 		  /* STEM_LIST to be simulated */
				 g_activeStemStack;  	   /* list of active stems */
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
extern char gen_all_pat;

#ifdef ISCAS85_NETLIST_MODE

int lineindex[MAXLINE];
char namelist[MAXGATE][10];
char line[MAXSTRING];

/*  	circin
		Reads an ISCAS85 benchmark circuit file
		and constructs internal data stuructures.
 
		The following information of the netlist is set in circin:
		1. index field : identification of gates (same as array index)
		2. type   	 : gate type
		3. ninput    : number of fan-in lines
		4. inlis	 : list of fan-in lines
		5. noutput   : number of fan-out lines
		6. outlis    : list of fan-out lines
		7. po   	 : set if the gate is a primary output
 
		Inputs: circuit input file (circuit)
		Outputs: data structures and
			 nog: number of gate
			 nopi: number of primary inputs
			 nopout: number of primary outputs
 
		Note: Circuit format is the same as ISCAS85 benchmark circuits.
			  The circuit description should be topologically sorted,
			  All gates are re-numbered in sequential order from
			  0 to nog-1.
*/
bool circin(int *nog, int *nopi, int *nopout)
{
	register int i, j;
	int lineno, nfout, nfin;
	int currentline = 0;
	int inputs[20];
	char name[10], gtype[5], fromline[10];

	*nog = 0;
	*nopi = 0;
	*nopout = 0;

	ALLOCATE(g_net, GATEPTR, MAXGATE);
	ALLOCATE(g_PrimaryIn, int, MAXPI);
	ALLOCATE(g_PrimaryOut, int, MAXPO);
	ALLOCATE(g_iHeadGateIndex, int, MAXPI);

	while (fscanf(g_fpCctFile, "%80s", line) != EOF)
	{
		if (line[0] == '*')
		{
			fgets(line, 80, g_fpCctFile);
		} /* comment lines */
		else
		{
			/* read gate descriptions in the order of
																	   line_number, label, gtype, # of fanout, # of fanin */
			sscanf(line, "%5d", &lineno);
			fscanf(g_fpCctFile, "%9s%5s", name, gtype);
			if (strcmp(gtype, "from") == 0) 		 /* fan-out branch */
			{
				fscanf(g_fpCctFile, "%9s", fromline);
			}
			else
			{
				fscanf(g_fpCctFile, "%d%d", &nfout, &nfin);
			}
			while (getc(g_fpCctFile) != '\n');

			/* if gate type is from, search fanin lines
																	  and skip fanout lines from the gate list */
			if (strcmp(gtype, "from") == 0)
			{
				for (i = currentline; i >= 0; i--)
					if (strcmp(fromline, namelist[i]) == 0)
					{
						lineindex[lineno] = g_net[i]->index;

						break;
					}
			}

			/* else, construct gate structutes */
			else
			{
				strcpy(namelist[currentline], name); /* store label */
				g_net[currentline] = (GATEPTR)malloc(sizeof(GATETYPE));
				lineindex[lineno] = currentline;
				g_net[currentline]->index = currentline;/* internal netlist */
				g_net[currentline]->gid = lineno;   /* actual netlist */
				g_net[currentline]->inCount = nfin;
				if (nfin != 0)
				{
					g_net[currentline]->inList = (GATEPTR *)malloc((unsigned)(sizeof(GATEPTR) * nfin));
				}
				g_net[currentline]->outCount = nfout;
#ifdef LEARNFLG
				g_net[currentline]->plearn = NULL;
#endif
				if (nfout != 0)
				{
					g_net[currentline]->outList = (GATEPTR *)malloc((unsigned)(sizeof(GATEPTR) * nfout));
					for (i = 0; i < nfout; i++)
						g_net[currentline]->outList[i] = NULL;
				}
				if (strcmp(gtype, "inpt") == 0)
				{
					g_net[currentline]->type = PI;
					g_PrimaryIn[*nopi] = currentline;
					(*nopi)++;
				}
				else
				{
					if (strcmp(gtype, "and") == 0)
					{
						g_net[currentline]->type = AND;
					}
					else if (strcmp(gtype, "nand") == 0)
					{
						g_net[currentline]->type = NAND;
					}
					else if (strcmp(gtype, "or") == 0)
					{
						g_net[currentline]->type = OR;
					}
					else if (strcmp(gtype, "nor") == 0)
					{
						g_net[currentline]->type = NOR;
					}
					else if (strcmp(gtype, "not") == 0)
					{
						g_net[currentline]->type = NAND;
					}
					else if (strcmp(gtype, "xor") == 0)
					{
						g_net[currentline]->type = XOR;
					}
					else if (strcmp(gtype, "buff") == 0)
					{
						g_net[currentline]->type = AND;
					}
					else if (strcmp(gtype, "buf") == 0)
					{
						g_net[currentline]->type = AND;
					}
					else
					{
						return(FALSE);
					}

					/* get intput list */
					for (i = 0; i < nfin; i++)
						fscanf(g_fpCctFile, "%6d", &inputs[i]);
					fgets(line, 80, g_fpCctFile);

					/* convert input index into internal and check fan-out list */
					for (i = 0; i < nfin; i++)
					{
						g_net[currentline]->inList[i] = g_net[lineindex[inputs[i]]];
						for (j = 0; j< g_net[lineindex[inputs[i]]]->outCount; j++)
							if (g_net[lineindex[inputs[i]]]->outList[j] == NULL)
							{
								g_net[lineindex[inputs[i]]]->outList[j] = g_net[currentline];
								break;
							}
					}
				}
				if (nfout == 0)
				{
					g_PrimaryOut[*nopout] = currentline;
					/*  			net[currentline]->po=TRUE; */
					(*nopout)++;
				}
				/*  		   else net[currentline]->po=FALSE; */
				currentline++;
			}
		}
	}    
	*nog = currentline;

	return(TRUE);
}

#endif

/*	set_cct_parameters
	Set several circuit parameters (ltype) of the net data structure.
	Computes the level (dpi) of each gate.
	Allocates space for g_pEventListStack and various objectives.
	Inputs	: data structures from circin +
		  nog: number of inputs
		  npi: numeber of primary inputs
	Outputs : ltype (line type among HEAD,BOUND and FREE)
		  dpi (distance from primary inputs)
		  maxdpi: maximum depth of the circuit
	Note	: should be called after circin
*/
#ifdef INCLUDE_HOPE
int setCctParameters(int iNoGate, int iNoPI) //set_cct_parameters
{
	//OUTPUT:  HEAD LFREE BOUUND 
	register int i, j; //,depth; not used
	int iHeadCnt = 0;

	/* define line type (free,head,bound) and distance from input */
	if (gen_all_pat == 'y')
	{
		//STOP*************************STOP
		for (i = 0; i < iNoGate; i++)
			g_net[i]->ltype = (g_net[i]->type == PI) ? HEAD : BOUND;
		iHeadCnt = iNoPI;
	}
	else //Default !!!
	{
		for (i = 0; i < iNoGate; i++)
		{
			g_net[i]->ltype = LFREE; //First assignment !!! --------------> LFREE
			if (g_net[i]->type != PI)
			{
				for (j = 0; j< g_net[i]->inCount; j++)
				{
					if (!is_free(g_net[i]->inList[j]))
					{
						g_net[i]->ltype = BOUND; //HEAD | BOUND -> * ------------>HEAD | BOUND -> BOUND
						//break;
						//We can break here !!!
					}
				}
			}
			if (is_free(g_net[i]) && (g_net[i]->outCount != 1))
			{
				g_net[i]->ltype = HEAD; //LFREE + FANOUT(PO) ------------> HEAD (FANOUT)
			}


			//HEAD
			if (is_head(g_net[i]))
			{
				iHeadCnt++;
			}
			//BOUND
			else if (is_bound(g_net[i])) //(FREE -> BOUND) -------------> (HEAD(NOT FANOUT) -> BOUND)
			{
				for (j = 0; j< g_net[i]->inCount; j++)
				{
					if (is_free(g_net[i]->inList[j]))
					{
						g_net[i]->inList[j]->ltype = HEAD;
						iHeadCnt++;
					}
				}
			}
		}
	}

	//PI has nothing to do with HEAD, NO USE !!!
	for (i = 0; i < iNoPI; i++)
	{
		//STOP*********************************************STOP
		g_iHeadGateIndex[i] = (-1);
	}

	j = iHeadCnt;
	for (i = iNoGate - 1; i >= 0; i--)
	{
		if (is_head(g_net[i]))
		{
			g_iHeadGateIndex[--j] = i;
		}
	}

	/* alloacate space for sets (needed for the fan algorithm) */
	ALLOCATE(g_unjustStack.list, GATEPTR, MAXOBJ);
	ALLOCATE(g_initObjStack.list, GATEPTR, MAXOBJ);
	ALLOCATE(g_curObjStack.list, GATEPTR, MAXOBJ);
	ALLOCATE(g_fanObjStack.list, GATEPTR, MAXOBJ);
	ALLOCATE(g_headObjStack.list, GATEPTR, iHeadCnt);
	ALLOCATE(g_finalObjStack.list, GATEPTR, MAXOBJ);
	ALLOCATE(g_DfrontierStack.list, GATEPTR, MAXOBJ);
	if (g_stack.list == NULL)
	{
		ALLOCATE(g_stack.list, GATEPTR, iNoGate);
	}
	ALLOCATE(g_tree.list, TREETYPE, MAXTREE);

	return(g_iMaxLevel);
}

#else
int setCctParameters(int iNoGate, int iNoPI)
{
	register int i, j, depth;
	int maxdpi = 0;
	int iHeadCnt = 0;

	/* define line type (free,head,bound) and distance from input */
	for (i = 0; i < iNoGate; i++)
	{
		g_net[i]->ltype = LFREE;
		depth = (-1);
		if (g_net[i]->type != PI)
		{
			for (j = 0; j< g_net[i]->inCount; j++)
			{
				if (!is_free(g_net[i]->inList[j]))
				{
					g_net[i]->ltype = BOUND;
				}
				depth = max(g_net[i]->inList[j]->dpi, depth);
			}
		}
		if (is_free(g_net[i]) && (g_net[i]->outCount != 1))
		{
			g_net[i]->ltype = HEAD;
		}
		g_net[i]->dpi = depth + 1;
		if (is_head(g_net[i]))
		{
			iHeadCnt++;
		}
		if (is_bound(g_net[i]))
		{
			for (j = 0; j< g_net[i]->inCount; j++)
				if (is_free(g_net[i]->inList[j]))
				{
					g_net[i]->inList[j]->ltype = HEAD;
					iHeadCnt++;
				}
		}
		maxdpi = max(g_net[i]->dpi, maxdpi);
	}

	/* allocate memory for g_pEventListStack and reset event counter */
	maxdpi++;
	g_pEventListStack = (STACKPTR)malloc((unsigned)(sizeof(STACKTYPE) * maxdpi));
	depth_array = (int*)malloc((unsigned)(sizeof(int) * maxdpi));
	for (i = 0; i < maxdpi; i++)
		g_pEventListStack[i].last = 0;

	for (i = 0; i < iNoPI; i++)
		g_iHeadGateIndex[i] = (-1);
	j = iHeadCnt;
	for (i = iNoGate - 1; i >= 0; i--)
	{
		if (is_head(g_net[i]))
		{
			g_iHeadGateIndex[--j] = i;
		}
		/* count the number of gates in each depth */
		(g_pEventListStack[g_net[i]->dpi].last)++;
	}

	/* allocate space for each event list */
	for (i = 0; i < maxdpi; i++)
	{
		g_pEventListStack[i].list = (GATEPTR *)malloc((unsigned)(sizeof(GATEPTR) * g_pEventListStack[i].last));
		depth_array[i] = g_pEventListStack[i].last;
		clear(g_pEventListStack[i]);
	}

	/* alloacate space for sets (needed for the fan algorithm) */
	ALLOCATE(g_unjustStack.list, GATEPTR, MAXOBJ);
	ALLOCATE(g_initObjStack.list, GATEPTR, MAXOBJ);
	ALLOCATE(g_curObjStack.list, GATEPTR, MAXOBJ);
	ALLOCATE(g_fanObjStack.list, GATEPTR, MAXOBJ);
	ALLOCATE(g_headObjStack.list, GATEPTR, iHeadCnt);
	ALLOCATE(g_finalObjStack.list, GATEPTR, MAXOBJ);
	ALLOCATE(g_DfrontierStack.list, GATEPTR, MAXOBJ);
	if (g_stack.list == NULL)
	{
		ALLOCATE(g_stack.list, GATEPTR, iNoGate);
	}
	ALLOCATE(g_tree.list, TREETYPE, MAXTREE);

	return(maxdpi);
}
#endif

/*	allocate_dynamic_buffers
	Allocates dynamic buffers needed for fsim.
*/
bool allocateStacks(int iNoGate) //allocate_dynamic_buffers
{
	if ((g_faultyGatesStack.list = (GATEPTR *)malloc((unsigned)(sizeof(GATEPTR) * iNoGate))) == NULL)
	{
		return(FALSE);
	}
	if ((g_freeGatesStack.list = (GATEPTR *)malloc((unsigned)(sizeof(GATEPTR) * iNoGate))) == NULL)
	{
		return(FALSE);
	}
	if ((g_evalGatesStack.list = (GATEPTR *)malloc((unsigned)(sizeof(GATEPTR) * iNoGate))) == NULL)
	{
		return(FALSE);
	}
	if ((g_activeStemStack.list = (GATEPTR *)malloc((unsigned)(sizeof(GATEPTR) * iNoGate))) == NULL)
	{
		return(FALSE);
	}
	if ((g_dynamicStack = (GATEPTR *)malloc((unsigned)(sizeof(GATEPTR) * iNoGate))) == NULL)
	{
		return(FALSE);
	}
	return(TRUE);
}

//Dschedule_output
#define outputs2EventList(pGate, i, iDFrontierCnt, pOutGate) \
	for (i = 0; i < pGate->outCount; i++) \
	{ \
		pOutGate = pGate->outList[i]; \
		if (!pOutGate->changed) \
		{ \
			push(g_pEventListStack[pOutGate->dpi], pOutGate); \
			iDFrontierCnt++; \
			set(pOutGate->changed); \
		} \
	}

/*	initDominators
	Finds the immediate dominators of all fanout stems.
	Inputs	: circuit structure +
		  nog (number of gates), maxdpi(maximum depth)
	Outputs	: dominators (u_path)
*/
int initFanoutGateDominators(int g_iNoGate, int iMaxLevelAdd2) /* depth of event list, number of output */ //set_dominator
{
	//OUTPUT:  iNoDominator
	register int i, j;
	register GATEPTR pGate, pTempGate;
	GATEPTR pDomiGate;
	int iGateCount;
	int iNoDominator = 0;

	for (i = g_iNoGate - 1; i >= 0; i--)
	{
		pGate = g_net[i];
		if (pGate->outCount <= 1)
		{
			pGate->u_path = NULL;
		}
		else //outCount >= 2 (FANOUT)
		{
			iGateCount = 0;
			outputs2EventList(pGate, j, iGateCount, pTempGate); //pGate ----> g_pEventListStack & iGateCount
			//OUTPUT:  g_pEventListStack & iGateCount (changed)
			for (j = pGate->dpi + 1; j < iMaxLevelAdd2; j++)
			{
				while (!is_empty(g_pEventListStack[j])) //For every level stack
				{
					pDomiGate = pop(g_pEventListStack[j]);
					reset(pDomiGate->changed);
					if (iGateCount <= 0)
					{
						continue;
					}
					iGateCount--;
					
					if (iGateCount == 0) //LAST ONE !!
					{
						pGate->u_path = (LINKPTR)malloc(sizeof(LINKTYPE));
						pGate->u_path->ngate = pDomiGate;
						iNoDominator++; //EXIT!!!
						break;
					}
					else //iGateCount != 0   NOT LAST ONE !!
					{
						//PO
						if (pDomiGate->outCount == 0)
						{
							pGate->u_path = NULL; //blocked
							iGateCount = 0; //Empty g_pEventListStack[j] &&&&&&&& End while !!
						}
						
						//FANOUT
						else if (pDomiGate->outCount > 1)
						{
							if (pDomiGate->u_path == NULL)
							{
								pGate->u_path = NULL; //blocked
								iGateCount = 0; //Empty g_pEventListStack[j] &&&&&&&& End while !!
							}
							else
							{
								pTempGate = pDomiGate->u_path->ngate; //next is u_path->ngate!!
								if (!pTempGate->changed)
								{
									push(g_pEventListStack[pTempGate->dpi], pTempGate);
									set(pTempGate->changed);
									iGateCount++;
								}
							}
						}
						
						//NORMAL GATE
						else //pDomiGate->outCount == 1
						{
							if (!pDomiGate->outList[0]->changed)
							{
								pTempGate = pDomiGate->outList[0];
								push(g_pEventListStack[pTempGate->dpi], pTempGate);
								set(pTempGate->changed);
								iGateCount++;
							}
						}
					}
				}
			}
		}
	}
	return(iNoDominator);
}

/*	set_unique_path
	For every fanout stem which has a dominator,
	this routine finds the unique path and property of the dominator.
	Used in FAN.
	Should be called after setting dominators.
*/

#define setFreach(pGate, i) pGate->freach = i
#define checkFreach(pGate, i) pGate->freach == i

void initUniquePath(int iNoGate, int iMaxDPI) /* depth of event list, number of output */ //set_unique_path
{
	//OUTPUT:  freach
	register int i, j;
	register GATEPTR pGate, pInGate, pOutGate;
	LINKPTR pUPath;
	int iGateCount, k;

	for (i = iNoGate - 1; i >= 0; i--) //For every FANOUT
	{
		pGate = g_net[i];
		if (pGate->u_path == NULL) //pGate->output <= 1
		{
			continue;
		}

		//pGate is a FANOUT has owns a pDominator !!!
		iGateCount = 0;
		setFreach(pGate, i);

		for (j = 0; j< pGate->outCount; j++) //For every output of pGate
		{
			pOutGate = pGate->outList[j];
			if (!pOutGate->changed)
			{
				push(g_pEventListStack[pOutGate->dpi], pOutGate);
				set(pOutGate->changed);
				iGateCount++;
			}
		}
		
		for (j = pGate->dpi + 1; j < iMaxDPI; j++) //For every g_pEventListStack after index pGate->dpi
		{
			while (!is_empty(g_pEventListStack[j]))
			{
				pGate = pop(g_pEventListStack[j]);
				reset(pGate->changed);
				setFreach(pGate, i);
				iGateCount--;
				
				if (iGateCount == 0) //EXIT for iteration of pGate !!!
				{
					//pGate == g_net[i]->u_path->ngate !!!
					pUPath = g_net[i]->u_path; //g_net[i] == original pGate			
					for (k = 0; k < pGate->inCount; k++)
					{
						pInGate = pGate->inList[k]; //pInGate ---> pGate
						if (checkFreach(pInGate, i))
						{
							pUPath->next = (LINKPTR)malloc(sizeof(LINKTYPE));
							pUPath = pUPath->next;
							pUPath->ngate = pInGate;
							pUPath->next = NULL;
						}
					}
					break;
				}
				
				for (k = 0; k< pGate->outCount; k++)
				{
					pOutGate = pGate->outList[k];
					if (!pOutGate->changed)
					{
						push(g_pEventListStack[pOutGate->dpi], pOutGate);
						set(pOutGate->changed);
						iGateCount++;
					}
				}
			}
		}
	}
}

/*	setfanoutstem
	For fault simulation.
	Identifies fanout stems and builds fanout free region.
*/
void initStemGatesAndFOS(int iNoGate, GATEPTR *pStemGates, int iNoStemGates) //setfanoutstem
{
	//OUTPUT:  pStemGates & fos
	register int i, j;
	register GATEPTR pGate;

	j = 0;
	for (i = 0; i < iNoGate; i++)
	{
		if (g_net[i]->outCount != 1) //FANOUT or PO
		{
			pStemGates[j++] = g_net[i];
		}
	}

	clear(g_stack);
	//j == iNoStemGates
	for (i = 0; i < iNoStemGates; i++)
	{
		push(g_stack, pStemGates[i]);
		while (!is_empty(g_stack))
		{
			pGate = pop(g_stack);
			pGate->fosIndex = pStemGates[i]->index; //Core sentence
			for (j = 0; j< pGate->inCount; j++)
			{
				if (pGate->inList[j]->outCount == 1) //Spread fosIndex within the FFR !!
				{
					push(g_stack, pGate->inList[j]);
				}
			}
		}
	}
}

/*	gettime: Gets CPU time used by the program. */
void getTime(double *usertime, double *systemtime, double *total)
{
#ifdef WIN32
	LARGE_INTEGER m_liPerfFreq = {0};
	LARGE_INTEGER m_liPerfStart = {0};
	QueryPerformanceFrequency(&m_liPerfFreq);
	QueryPerformanceCounter(&m_liPerfStart);

	*total = (double) m_liPerfStart.QuadPart / m_liPerfFreq.QuadPart;
	*usertime = *total;
	*systemtime = *total;
#else
	struct tms timesbuffer;
	time_t totaltime;
	time_t utime;
	time_t stime;
	
	times(&timesbuffer);
	utime = timesbuffer.tms_utime;
	stime = timesbuffer.tms_stime;
	totaltime = timesbuffer.tms_utime + timesbuffer.tms_stime; /* In 60th seconds */
	*usertime = (double) utime / 60.0;
	*systemtime = (double) stime / 60.0;
	*total = (double) totaltime / 60.0;
#endif // WIN32
}

double getMemory()
{
	double fMaxUse;

#ifdef WIN32
	//进程句柄
	HANDLE hProcess = GetCurrentProcess();
	//HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, dwProcessId);
	if (NULL != hProcess)
	{
		//内存情况
		PROCESS_MEMORY_COUNTERS pmc;
		if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
		{
			//double fCurUse = double(pmc.PeakWorkingSetSize / (1024.0*1024.0));
			fMaxUse = (double) (pmc.WorkingSetSize / (1024.0 * 1024.0));
			//TRACE("[info] 主程序内存使用情况，当前所用内存：%.4fM，最大曾用：%.4fM\r\n", fCurUse, fMaxUse);
		}
		CloseHandle(hProcess);
	}
	return fMaxUse;
#else
	return fMaxUse;
#endif // WIN32
}
