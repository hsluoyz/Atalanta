
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
		
		   Changed output display, M. Chan, 3/10/1996
		   Added flexiblities on reading cct name, M. Chan, 4/10/1996 

		atalanta: version 2.0   	 H. K. Lee, 6/30/1997
 
***********************************************************************/

/*----------------------------------------------------------------- 
	filename read_cct.c
	This file contains all subroutines necessary for
	construction of circuit data structures for sequencial
	ISCAS89 benchmark circuits
-------------------------------------------------------------------*/
#include "stdafx.h"
#include <stdio.h>
//#include <strings.h> //for linux only, change to windows edition as below
#include <string.h>
#include <stdlib.h>

#include "read_cct.h"
#include "error.h"
#include "hash.h"

#include "parameter.h"
#include "define.h"
#include "macro.h"

extern GATEPTR *g_net;
extern int *g_PrimaryIn, *g_PrimaryOut, *g_FlipFlop, *g_iHeadGateIndex;
extern int g_iNoGate, g_iNoPI, g_iNoPO, g_iNoFF, g_iMaxLevel;
extern char fn_to_string[][MAXGTYPE + 3];
//extern void fatalerror();
//extern void InitHash();
//extern HASHPTR FindHash(), Find_and_Insert_Hash();

HASHPTR pArrSymbolTable[HASHSIZE];
int g_iMaxFanOut = 0;
GATEPTR pGateNext = (GATEPTR)NULL;

#define NOT_ALLOCATED (-1)	/* index of non-allocated gate */
#define is_white_space(c) (c==' ' || c=='\n'|| c=='\t')
#define is_delimiter(c) (c=='=' || c==',' || c=='(' || c==')')
#define is_valid(c) ((c>='0' && c<='9') || \
			 (c>='A' && c<='Z') || \
			 (c>='a' && c<='z') || \
			 (c=='_') || \
			 (c=='.'))
#define is_comment(c) (c=='#')

/*------getsymbol--------------------------------------------------
	reads next symbol from the given circuit file
	inputs : file	the name of input file
	outputs: s	symbol string output
		 returns delimiter following symbol
		 =   gate name
		 (   gate type
		 ,   input signal --- will be continued
		 )   input --- end of input
-------------------------------------------------------------------*/
char getsymbol(FILE *file, register char *s)
{
	register char c;
	int comm = 0;

	while ((c = getc(file)) != EOF)
	{
		if (is_comment(c))
		{
			comm = 1; continue;
		}
		if (comm == 1)
		{
			if (c == '\n')
			{
				comm = 0;
			}
			continue;
		}
		if (is_white_space(c))
		{
			continue;
		}
		if (is_delimiter(c))
		{
			break;
		}
		*s++ = c;
	}
	*s = EOS;
	return(c);
}

/*------gatetype: returns type of the gate symbol------*/
int gatetype(char *symbol)
{
	int fn;

	if (strcmp(symbol, "NOT") == 0)
	{
		fn = NOT;
	}
	else if (strcmp(symbol, "AND") == 0)
	{
		fn = AND;
	}
	else if (strcmp(symbol, "NAND") == 0)
	{
		fn = NAND;
	}
	else if (strcmp(symbol, "OR") == 0)
	{
		fn = OR;
	}
	else if (strcmp(symbol, "NOR") == 0)
	{
		fn = NOR;
	}
	else if (strcmp(symbol, "DFF") == 0)
	{
		fn = DFF;
	}
	else if (strcmp(symbol, "XOR") == 0)
	{
		fn = XOR;
	}
	else if (strcmp(symbol, "XNOR") == 0)
	{
		fn = XNOR;
	}
	else if (strcmp(symbol, "BUFF") == 0)
	{
		fn = BUFF;
	}
	else if (strcmp(symbol, "BUF") == 0)
	{
		fn = BUFF;
	}
	else if (strcmp(symbol, "INPUT") == 0)
	{
		fn = PI;
	}
	else if (strcmp(symbol, "OUTPUT") == 0)
	{
		fn = PO;
	}
	else if (strcmp(symbol, "not") == 0)
	{
		fn = NOT;
	}
	else if (strcmp(symbol, "and") == 0)
	{
		fn = AND;
	}
	else if (strcmp(symbol, "nand") == 0)
	{
		fn = NAND;
	}
	else if (strcmp(symbol, "or") == 0)
	{
		fn = OR;
	}
	else if (strcmp(symbol, "nor") == 0)
	{
		fn = NOR;
	}
	else if (strcmp(symbol, "dff") == 0)
	{
		fn = DFF;
	}
	else if (strcmp(symbol, "xor") == 0)
	{
		fn = XOR;
	}
	else if (strcmp(symbol, "xnor") == 0)
	{
		fn = XNOR;
	}
	else if (strcmp(symbol, "buff") == 0)
	{
		fn = BUFF;
	}
	else if (strcmp(symbol, "buf") == 0)
	{
		fn = BUFF;
	}
	else if (strcmp(symbol, "input") == 0)
	{
		fn = PI;
	}
	else if (strcmp(symbol, "output") == 0)
	{
		fn = PO;
	}
	else
	{
		fn = (-1);
	}

	return(fn);
}

char * spc_to_und(char *buf)
{
	int i = 0;
	while (buf[i] == ' ')
		buf++;

	while (buf[i] != '\0')
	{
		if (buf[i] == ' ')
		{
			buf[i] = '_';
		}
		i++;
	}
	return (buf);
}

#define get_string(file,string) \
if(fscanf(file,"%s",string)<GOOD) fatalerror(CIRCUITERROR)

/*------read_circuit-------------------------------------------------
		Reads a circuit file.
	The netlist format is the same as that of ISCAS89 benchmark
	netlists except that:

			1) The first line should start with # followed by
		   the name of the circuit, and
			2) Comment lines which starts with # can be inserted
		   in any part of the circuit netlist.

	Constructs basic data stuructures
		and allocates necessary memory spaces.

	Users should provide the following information;
	1. index field : identification of gates (same as array index)
	2. fn   	 : gate type
	3. ninput    : number of fan-in lines
	4. inlis	 : list of fan-in lines
	5. noutput   : number of fan-out lines
	6. outlis    : list of fan-out lines
	7. name		 : pointer to the hash table

	inputs: circuit input file (circuit)
	outputs: data structures

	Note: The circuit description is topologically sorted  in the
		  levelized order after parsing.
	Laveling order: primary input starts 0 to n-1  (n inputs)
			flip/flops starts n to n+m-1   (m f/f)
			gate starts n+m to n+m+l-1     (l gates)
---------------------------------------------------------------------*/
int readCircuit(FILE *fpCctFile, char strCctName[]) //read_circuit
{
	//INPUT:  fpCctFile
	//OUTPUT:  strCctName
	register int i, j;
	register char c;
	register GATEPTR pGate;
	register HASHPTR pHash;
	int iNoGate, iNoPI, iNoPO, iNoFF;
	char strSymbol[MAXSTRING];
	char buf[MAXSTRING], *bufptr;  /* store a line of text */
	int fn, iGatesFanIn;
	int iNetSize;
	GATETYPE *pGate2;
	GATEPTR pArrGatesFanIn[MAXFIN + 100];
	GATEPTR pArrGatesPO[MAXPO + 100];
	int iErrors = 0;

	/* The first line should start with # followed by
	   the name of the circuit */
	while ((c = getc(fpCctFile)) != EOF)
		if (c == '#' || c == '\n')
		{
			break;
		}
	if (c == EOF)
	{
		return((-1));
	}
	if (c == '#')
	{
		if (!fgets(buf, MAXSTRING, fpCctFile))
		{
			printFatalError(CIRCUITERROR);
		}
		buf[strlen(buf) - 1] = '\0';
		bufptr = spc_to_und(buf);
		strcpy(strCctName, bufptr);
	}

	InitHash(pArrSymbolTable, HASHSIZE);

	/* Pass 1:
	Adds the gate symbols to symbol_tbl[] and
	counts # of gates, pi's, po's, ff's
	*/
	iNoPI = iNoPO = iNoFF = iNoGate = 0;
	iGatesFanIn = 0;
	pGateNext = (GATEPTR)NULL;

	while ((c = getsymbol(fpCctFile, strSymbol)) != EOF)
	{
		switch (c)
		{
		case '=' :
			/* a new gate */
			pHash = Find_and_Insert_Hash(pArrSymbolTable, HASHSIZE, strSymbol, 0);
			if ((pGate = pHash->gate) == NULL)
			{
				ALLOCATE(pGate, GATETYPE, 1);
				pHash->gate = pGate;
				pGate->hash = pHash;
				pGate->next = pGateNext;
				pGateNext = pGate;
			}
			break;
		case '(' :
			/* gate type */
			if ((fn = gatetype(strSymbol)) < 0)
			{
				fprintf(stderr, "Error: Gate type %s is not valid\n", strSymbol);
				return(-1);
			}
			break;
		case ',':
			/* fanin list */
			pHash = Find_and_Insert_Hash(pArrSymbolTable, HASHSIZE, strSymbol, 0);
			if ((pGate2 = pHash->gate) == NULL)
			{
				ALLOCATE(pGate2, GATETYPE, 1);
				pHash->gate = pGate2;
				pGate2->hash = pHash;
				pGate2->index = (-1);
				pGate2->next = pGateNext;
				pGateNext = pGate2;
			}
			pArrGatesFanIn[iGatesFanIn++] = pGate2;
			break;
		case ')':
			/* terminator, fanin list */
			pHash = Find_and_Insert_Hash(pArrSymbolTable, HASHSIZE, strSymbol, 0);
			if ((pGate2 = pHash->gate) == NULL)
			{
				ALLOCATE(pGate2, GATETYPE, 1);
				pHash->gate = pGate2;
				pGate2->hash = pHash;
				pGate2->index = (-1);
				pGate2->next = pGateNext;
				pGateNext = pGate2;
			}
			switch (fn)
			{
			case PI:
				iNoPI++;
				pGate2->index = iNoGate++;
				pGate2->inCount = 0;
				pGate2->inList = (GATEPTR *)NULL;
				pGate2->type = PI;
				pGate2->outCount = 0;
				pGate2->outList = (GATEPTR *)NULL;
				break;
			case PO:
				pArrGatesPO[iNoPO++] = pGate2;
				break;
			default:
				pArrGatesFanIn[iGatesFanIn++] = pGate2;

				switch (fn)
				{
				case DFF:
					iNoFF++;
					break;
				case XOR:
				case XNOR:
					if (iGatesFanIn != 2)
					{
						fprintf(stderr, "Error: %d-input %s gate is not supported\n", iGatesFanIn, fn_to_string[fn]);
						return (-1);
					}
				}

				if (pGate == NULL)
				{
					fprintf(stderr, "Error: Syntax error in the circuit file\n");
					return(-1);
				}
				pGate->index = iNoGate++;
				pGate->type = fn;
				if ((pGate->inCount = iGatesFanIn) == 0)
				{
					pGate->inList = NULL;
				}
				else
				{
					ALLOCATE(pGate->inList, GATEPTR, pGate->inCount);
				}
				for (i = 0; i < iGatesFanIn; i++)
					pGate->inList[i] = pArrGatesFanIn[i];
				pGate->outCount = 0;
				pGate->outList = (GATEPTR *)NULL;

				iGatesFanIn = 0;
				pGate = (GATEPTR)NULL;
				break;
			}
		}
	}

	/* Pass 2: Construct the circuit data structure */
	iNetSize = iNoGate + iNoPO + iNoFF + SPAREGATES;
	ALLOCATE(g_net, GATEPTR, iNetSize);
	ALLOCATE(g_PrimaryIn, int, iNoPI);
	ALLOCATE(g_PrimaryOut, int, iNoPO);
	ALLOCATE(g_FlipFlop, int, iNoFF);
#ifdef ATALANTA
	ALLOCATE(g_iHeadGateIndex, int, iNoPI);
#endif
	int cc = 0;
	for (pGate = pGateNext; pGate != NULL; pGate = pGate->next)
	{
		//fprintf(stderr,"%d ",cc++);
	}
	g_iNoGate = g_iNoPI = g_iNoPO = g_iNoFF = 0;
	for (pGate = pGateNext; pGate != NULL; pGate = pGate->next)
	{
		if (pGate->index < 0)
		{
			fprintf(stderr, "Error: floating net %s\n", pGate->hash->symbol);
			fprintf(stderr, "Workaround. You have to take one of the two actions:\n");
			fprintf(stderr, "   1. Remove all the floating input and associated gates, or\n");
			fprintf(stderr, "   2. Make each floating input a primary output.\n");
			return(-1);
		}
		g_net[pGate->index] = pGate;
		//fprintf(stderr,"%s ",cg->symbol->symbol);
		g_iNoGate++;
	}

	for (i = g_iNoGate; i < iNetSize; i++)
		g_net[i] = (GATEPTR)NULL;
	fprintf(stderr, "before\n");
	if (g_iNoGate != iNoGate)
	{
		fprintf(stderr, "%d,%d\n", iNoGate, g_iNoGate);
		fprintf(stderr, "Error in read_circuit\n");
		return(-1);
	}
	fprintf(stderr, "after\n");
	/* Pass 3: Compute fanout list */
	for (i = 0; i < g_iNoGate; i++)
	{
		pGate = g_net[i];
#ifdef LEARNFLG
		pGate->plearn = NULL;
#endif
		for (j = 0; j < pGate->inCount; j++)
			pGate->inList[j]->outCount++;
		switch (pGate->type)
		{
		case PI:
			g_PrimaryIn[g_iNoPI++] = i; break;
		case DFF:
			g_FlipFlop[g_iNoFF++] = i; break;
		}
	}
	for (i = 0; i < iNoPO; i++)
		g_PrimaryOut[g_iNoPO++] = pArrGatesPO[i]->index;
	for (i = 0; i < g_iNoGate; i++)
	{
		pGate = g_net[i];
		if (pGate->outCount > 0)
		{
			ALLOCATE(pGate->outList, GATEPTR, pGate->outCount);
			g_iMaxFanOut = MAX(g_iMaxFanOut, pGate->outCount);
			pGate->outCount = 0;
		}
	}

	for (i = 0; i < g_iNoGate; i++)
	{
		pGate = g_net[i];
		for (j = 0; j< pGate->inCount; j++)
			pGate->inList[j]->outList[(pGate->inList[j]->outCount)++] = pGate;
	}

	for (i = 0; i < g_iNoGate; i++)
	{
		pGate = g_net[i];
		if (pGate->outCount > 0)
		{
			continue;
		}
		for (j = 0; j < iNoPO; j++)
			if (pGate == pArrGatesPO[j])
			{
				break;
			}
		if (j == iNoPO)
		{
			fprintf(stderr, "Error: floating output '%s' detected!\n", pGate->hash->symbol); 
			iErrors++;
		}
	}
	if (iErrors > 0)
	{
		fprintf(stderr, "Workaround. You have to take one of the two actions:\n");
		fprintf(stderr, "   1. Remove all the floating output and associated gates, or\n");
		fprintf(stderr, "   2. Make each floating output a primary output.\n");
		return (-1);
	}

	if (iNoGate == g_iNoGate)
	{
		return(g_iNoGate);
	}
	else
	{
		return (-1);
	}
}
