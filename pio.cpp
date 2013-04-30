
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

/*------------------------------------------------------------------
	filename:  pio.c
	This file contains all subroutines needed for input/output
	of atalanta and fsim results.
--------------------------------------------------------------------*/
#include "stdafx.h"
#include <stdio.h>

#include "pio.h"
#include "parameter.h" 
#include "define.h"    
#include "macro.h"

extern char chCctMode;
extern GATEPTR *g_net;
extern int *g_PrimaryIn, *g_PrimaryOut, *g_iHeadGateIndex;
extern level BITMASK[BITSIZE];

#define chars_per_line 60
#define chars_per_column 50
#define number_of_spaces 5

/* macros for the bit operation of a computer word */
#define setbit(word,n) word&=BITMASK[n]
#define resetbit(word,n) word|=(~BITMASK[n])
#define checkbit(word,n) ((word&BITMASK[n])!=ALL0)
#define putbit(fp,word,n) if(checkbit(word,n)) putc('1',fp); else putc('0',fp)

/*	printinputs
	print nth bit test pattern of the circuit input
*/
void printinputs(FILE *fp, int iNoPI, int nth_bit)
{
	register int j;
	for (j = 0; j < iNoPI; j++)
		putbit(fp, g_net[j]->output1, nth_bit);
}

/*	printoutputs
	print nth bit output response of the circuit
*/
void printoutputs(FILE *fp, int iNoPO, int nth_bit)
{
	register int j;
	for (j = 0; j < iNoPO; j++)
		putbit(fp, g_net[g_PrimaryOut[j]]->output1, nth_bit);
}

#define TEST_PER_LINE 13

void print_test_topic(FILE *fpFile, int iNoPI, int iNoPO, char strCctPathFileName[])
{
	register int i;
	register GATEPTR pGate;

	fprintf(fpFile, "* Name of circuit:  %s\n", strCctPathFileName);
	fprintf(fpFile, "* Primary inputs :\n  ");
	for (i = 0; i < iNoPI; i++)
	{
#ifdef ISCAS85_NETLIST_MODE //Never come here !!
// 		if (chCctMode == ISCAS89)
// 		{
// 			fprintf(fpFile, "%s ", g_net[i]->hash->symbol);
// 		}
// 		else
// 		{
// 			fprintf(fpFile, "%5d ", g_net[i]->gid);
// 		}
#else
		fprintf(fpFile, "%s ", g_net[i]->hash->symbol);
#endif
		if (((i + 1) mod TEST_PER_LINE == 0) || (i == iNoPI - 1))
		{
			fprintf(fpFile, "\n  ");
		}
	}
	fprintf(fpFile, "\n");
	fprintf(fpFile, "* Primary outputs:\n  ");
	for (i = 0; i < iNoPO; i++)
	{
#ifdef ISCAS85_NETLIST_MODE //Never come here !!
// 		if (chCctMode == ISCAS89)
// 		{
// 			gut = g_net[g_PrimaryOut[i]];
// 			if (gut->type == PO && gut->inCount == 1)
// 			{
// 				gut = gut->inList[0];
// 			}
// 			fprintf(fpFile, "%s ", gut->hash->symbol);
// 		}
// 		else
// 		{
// 			fprintf(fpFile, "%5d ", g_net[g_PrimaryOut[i]]->gid);
// 		}
#else
		/*
							  fprintf(fpFile,"%s ",net[primaryout[i]]->symbol->symbol);
						*/
		pGate = g_net[g_PrimaryOut[i]];
		if (pGate->type == PO && pGate->inCount == 1) //pGate->type == PO --------> pGate->inCount == 1
		{
			pGate = pGate->inList[0];
		}
		//else if (pGate->type == PO && pGate->inCount != 1) //Never comes here
		//{
			//STOP***************************STOP
			//int aaa = 1;
		//}
		fprintf(fpFile, "%s ", pGate->hash->symbol);
#endif
		if (((i + 1) mod TEST_PER_LINE == 0) || (i == iNoPI - 1))
		{
			fprintf(fpFile, "\n  ");
		}
	}
	fprintf(fpFile, "\n\n* Test patterns and fault free responses:\n\n");
}

/*	printio
	prints nth bit test patterns and outputs of the circuit
*/
void printio(FILE *fpFile, int iNoPI, int iNoPO, int nth_bit, int iNoTestPattern)
{
	fprintf(fpFile, "%4d: ", iNoTestPattern);
	printinputs(fpFile, iNoPI, nth_bit);
	fprintf(fpFile, " ");
	printoutputs(fpFile, iNoPO, nth_bit);
	fprintf(fpFile, "\n");
}

void print_log_topic(FILE *fp, char name[])
{
	fprintf(fp, "* Log file for the circuit %s.\n", name);
	fprintf(fp, "* Number of faults detected by each test pattern:\n\n");
}

/*	pget_test
	Reads test input files and stores in parallel.
	Returns number of test patterns read.
	Returns 0 if EOF is encountered.
*/
int pget_test(FILE *fp, level input[], int npi, int nbit)
{
	register int i;
	register int c;
	int ntest;
	unsigned mask1 = ~(ALL1 << 1);
	bool valid = FALSE;

	i = 0;
	for (ntest = 0; ntest < nbit; ntest++)
	{
		while ((c = getc(fp)) != EOF)
		{
			switch (c)
			{
			case '*':
				while ((c = getc(fp)) != '\n')
					if (c == EOF)
					{
						break;
					} break;
			case ':':
				valid = TRUE; break;
			case '0':
				if (valid)
				{
					input[i++] <<= 1;
				} break;
			case '1':
				if (valid)
				{
					input[i] <<= 1; input[i++] |= mask1;
				} break;
			}
			if (i == npi)
			{
				i = 0; valid = FALSE; break;
			}
		}
		if (c == EOF)
		{
			break;
		}
	}
	if (i > 0 && i < npi)
	{
		return(0);
	}
	return(ntest);
}

/*	printfault: prints a fault 	*/
char fault2str[][4] =
{
	"/0", "/1", "/0", "/1"
};
void printfault(FILE *fpFile, FAULTPTR pFault, bool bModeDisplayStatus)
{
	GATEPTR pGate;

#ifdef ISCAS85_NETLIST_MODE
// 	if (chCctMode == ISCAS89)
// 	{
// 		if (pFault->line >= 0)
// 		{
// 			pGate = pFault->gate->inList[pFault->line];
// 			fprintf(fpFile, "%s", pGate->hash->symbol);
// 			fprintf(fpFile, "->");
// 		}
// 		fprintf(fpFile, "%s %s", pFault->gate->hash->symbol, fault2str[pFault->type]);
// 	}
// 	else
// 	{
// 		if (pFault->line < 0)
// 		{
// 			fprintf(fpFile, "Output line s-a-%d ", pFault->type);
// 		}
// 		else
// 		{
// 			fprintf(fpFile, "Input line %d s-a-%d ", pFault->line + 1, pFault->type);
// 		}
// 		fprintf(fpFile, "of gate %d ", pFault->gate->gid);
// 	}
#else
	if (pFault->line >= 0) //pFault->gate->inList[pFault->line] --------> pFault -------> pFault->gate
	{
		pGate = pFault->gate->inList[pFault->line];
		fprintf(fpFile, "%s", pGate->hash->symbol);
		fprintf(fpFile, "->");
	}
	fprintf(fpFile, "%s %s", pFault->gate->hash->symbol, fault2str[pFault->type]);
	//OUTPUT:   "Symbol1 -> Symbol2 SA1 detected"
#endif
	if (bModeDisplayStatus)
	{
		switch (pFault->detected)
		{
		case DETECTED:
			fprintf(fpFile, " detected"); break;
		case UNDETECTED:
			fprintf(fpFile, " undetected"); break;
		case PROCESSED:
			fprintf(fpFile, " aborted"); break;
		case REDUNDANT:
			fprintf(fpFile, " redundant");
		}
	}
	fprintf(fpFile, "\n");
}


/* The following subroutines are necessary for debugging or
   exteraction of net lists --- not used in normal mode */

#define logic_type(gut) \
switch(gut->type) { \
case AND: if(gut->inCount==1)printf("buff");else printf("and "); break; \
case NAND: if(gut->inCount==1)printf("not ");else printf("nand"); break; \
case OR: if(gut->inCount==1)printf("buff");else printf("or  "); break; \
case NOR: if(gut->inCount==1)printf("not ");else printf("nor "); break; \
case XOR: if(gut->inCount==1)printf("buff");else printf("xor "); break; \
case PI: printf("input");}

void print_gate(register GATEPTR gut) //never used
{
	register int i;

	/*
	   printf("%s: ",gut->symbol->symbol);
	*/
	if (gut->type == PI)
	{
		printf("input(%d)", gut->index);
	}
	else
	{
		printf("%d = ", gut->index);
		logic_type(gut); 
		printf("(");
		for (i = 0; i< gut->inCount; i++)
			if (i == 0)
			{
				printf("%d", gut->inList[i]->index);
			}
			else
			{
				printf(",%d", gut->inList[i]->index);
			}
		printf(")");
	}
	printf("\n");
}

void sprint_gate(register GATEPTR gut) //never used
{
	register int i;

	/*
	   printf("%s: ",gut->symbol->symbol);
	*/
	if (gut->type == PI)
	{
		printf("input(%s)", gut->hash->symbol);
	}
	else
	{
		printf("%s = ", gut->hash->symbol);
		logic_type(gut); 
		printf("(");
		for (i = 0; i< gut->inCount; i++)
			if (i == 0)
			{
				printf("%s", gut->inList[i]->hash->symbol);
			}
			else
			{
				printf(",%s", gut->inList[i]->hash->symbol);
			}
		printf(")");
	}
	printf("\n");
}

void print_faultlist(FILE *fp, FAULTPTR *flist, int no, bool mode)
{
	register int i;

	for (i = 0; i < no; i++)
	{
		printfault(fp, flist[i], mode);
	}
}

/*
void print_output_cone(gut,maxdpi,mode)
register GATEPTR gut; int maxdpi;
char mode;
{
   register int i,j;

   push(g_pEventListStack[gut->dpi],gut);
   set(gut->changed);
   for(i=gut->dpi;i<maxdpi;i++)
	  while(!is_empty(g_pEventListStack[i])) {
	 gut=pop(g_pEventListStack[i]);
	 reset(gut->changed);
	 if(mode=='s') sprint_gate(gut);
	 else print_gate(gut);
	 for(j=0;j<gut->outCount;j++) {
		if(!gut->outList[j]->changed) {
		   push(g_pEventListStack[gut->outList[j]->dpi],gut->outList[j]);
		   set(gut->outList[j]->changed);
		}
	 }
	  }
}

void print_input_cone(gut,mode)
register GATEPTR gut;
char mode;
{
   register int i,j;

   push(g_pEventListStack[gut->dpi],gut);
   set(gut->changed);
   for(i=gut->dpi;i>=0;i--)
	  while(!is_empty(g_pEventListStack[i])) {
	 gut=pop(g_pEventListStack[i]);
	 reset(gut->changed);
	 if(mode=='s') sprint_gate(gut);
	 else print_gate(gut);
	 for(j=0;j<gut->inCount;j++) {
		if(!gut->inList[j]->changed) {
		   push(g_pEventListStack[gut->inList[j]->dpi],gut->inList[j]);
		   set(gut->inList[j]->changed);
		}
	 }
	  }
}
*/


/*----------------------------------------------------------------- 
	filename print.c
	This file contains all subroutines necessary to print out
	circuit structure, fault list and test patterns to the
	standard output. --- will be changed to report to a file.
-------------------------------------------------------------------*/

extern int g_iNoGate, g_iNoPI, g_iNoPO, g_iNoFF, g_iMaxLevel;
extern int *g_PrimaryIn, *g_PrimaryOut, *g_FlipFlop;
extern GATEPTR *g_net;
extern level BITMASK[];
extern char fn_to_string[][MAXGTYPE + 3], level_to_string[][MAXLEVEL + 1], fault_to_string[][3];
extern level parallel_to_level[][2];
extern FAULTPTR g_pHeadFault;
extern int g_iGroupID;

#define printgatetype(file,type) fprintf(file,"%s",fn_to_string[type])
#define printfaulttype(file,type) fprintf(file,"%s",fault_to_string[type])
#define printlevel(file,type) fprintf(file,"%s",level_to_string[type])
#define aprintgate(file,gate) \
{ printgate(file,gate,'s'); fprintf(file," ; "); printgate(file,gate,'n'); }

/*------prints gate symbol or index--------------------------------*/
void printgatename(FILE *fp, register GATEPTR gate, char wmode)
{
	if (wmode == 's')
	{
		if (gate->hash == NULL)
		{
			fprintf(fp, "TEMP%d", gate->index);
		}
		else
		{
			fprintf(fp, "%s", gate->hash->symbol);
		}
	}
	else
	{
		fprintf(fp, "%d", gate->index);
	}
}

/*------prints PI,PO,PPI or PPO gatenames--------------------------*/
void printionames(FILE *fp, int array[], register int n, char *head, register char wmode, register char iomode)
{
	GATEPTR gut; int i;

	for (i = 0; i < n; i++)
	{
		gut = g_net[array[i]];
		if (iomode == 'i')
		{
			if (gut->inCount > 0)
			{
				gut = gut->inList[0];
			}
		}
		printgatename(fp, gut, wmode);
		fprintf(fp, " ");
		if (((i + 1) mod TEST_PER_LINE == 0) || (i == n - 1))
		{
			fprintf(fp, "\n");
			if (i < n - 1)
			{
				fprintf(fp, "%s", head);
			}
		}
	}
} 

/*------print out gate structure in symbolic or neumeric notations---*/
void printgate(FILE *fp, register GATEPTR gate, char wmode)
{
	register int i;

	if (gate->type == PI)
	{
		printgatetype(fp, gate->type);
		fprintf(fp, "(");
		printgatename(fp, gate, wmode);
		fprintf(fp, ")");
	}
	else
	{
		printgatename(fp, gate, wmode);
		fprintf(fp, " = ");
		if (gate->inCount == 1)
		{
			if (gate->type == AND || gate->type == OR)
			{
				fprintf(fp, "BUFF");
			}
			else if (gate->type == NAND || gate->type == NOR)
			{
				fprintf(fp, "NOT");
			}
			else
			{
				printgatetype(fp, gate->type);
			}
		}
		else
		{
			printgatetype(fp, gate->type);
		}
		putc('(', fp);
		for (i = 0; i< gate->inCount; i++)
		{
			printgatename(fp, gate->inList[i], wmode);
			if (i< gate->inCount - 1)
			{
				putc(',', fp);
			}
		}
		putc(')', fp);
	}
}

/*------converts a parallel logic format to a single logic level------*/
level logiclevel(register level V0, register level V1, register int n)
{
	V0 = ((V0 & BITMASK[n]) == ALL0) ? ZERO : ONE;
	V1 = ((V1 & BITMASK[n]) == ALL0) ? ZERO : ONE;
	return(parallel_to_level[V0][V1]);
}

/*------prints values of PIs,POs,PPIs or PPOs-------------------------
	if iomode=='i', prints input values
	else prints output values.
---------------------------------------------------------------------*/
void printiovalues(FILE *fp, int array[], int n, char iomode, char gmode, int bit)
{
	register int j;
	register GATEPTR gut;

	for (j = 0; j < n; j++)
	{
		gut = g_net[array[j]];
		if (iomode == 'i')
		{
			if (gut->inCount > 0)
			{
				gut = gut->inList[0];
			}
		}
		if (gmode == 'g' || gut->Gid != g_iGroupID)
		{
			printlevel(fp, logiclevel(gut->GV[0], gut->GV[1], bit));
		}
		else
		{
			printlevel(fp, logiclevel(gut->FV[0], gut->FV[1], bit));
		}
	}
}

/*------prints inputs and outputs of a given gate--------------------*/
void printgatevalues(FILE *fp, register GATEPTR gut, int n, char gmode)
{
	register int i;
	register GATEPTR g;

	if (gmode == 'g' || gut->Gid != g_iGroupID)
	{
		printlevel(fp, logiclevel(gut->GV[0], gut->GV[1], n));
	}
	else
	{
		printlevel(fp, logiclevel(gut->FV[0], gut->FV[1], n));
	}
	fprintf(fp, " = ");
	printgatetype(fp, gut->type);
	fprintf(fp, "(");
	for (i = 0; i< gut->inCount; i++)
	{
		g = gut->inList[i];
		if (gmode == 'g' || g->Gid != g_iGroupID)
		{
			printlevel(fp, logiclevel(g->GV[0], g->GV[1], n));
		}
		else
		{
			printlevel(fp, logiclevel(g->FV[0], g->FV[1], n));
		}
		if (i< gut->inCount - 1)
		{
			putc(',', fp);
		}
	}
	putc(')', fp);
}

/*------prints head lines of the log file----------------------------*/
/*
void print_log_topic(fp,name,wmode)
FILE *fp; char name[], wmode;
{
   fprintf(fp,"* Log files for test pattern generation.\n");
   fprintf(fp,"* name of the circuit: %s\n",name);
   fprintf(fp,"* primary inputs: ");
   printionames(fp,primaryin,nopi,"                  ",wmode,'o');
   fprintf(fp,"* pseudo-primary inputs: ");
   printionames(fp,flip_flops,noff,"                         ",wmode,'o');
   fprintf(fp,"* primary outputs: ");
   printionames(fp,primaryout,nopo,"                   ",wmode,'o');
   fprintf(fp,"* pseudo-primary outputs: ");
   printionames(fp,flip_flops,noff,"                          ",wmode,'i');
   fprintf(fp,"number of faults detected by each test pattern.\n\n");
}

void printfault(fp,f,wmode)
FILE *fp;
register FAULTPTR f;
char wmode;
{
   register GATEPTR gut;

   if(f->line>=0) {
	  for(gut=f->gate->inList[f->line];gut->symbol==NULL;gut=gut->inList[1]);
	  printgatename(fp,gut,wmode);
	  fprintf(fp,"->");
   }
   printgatename(fp,f->gate,wmode);
   fprintf(fp," ");
   printfaulttype(fp,f->type);
}
*/

void printfaultlist(FILE *fp, bool wmode)
{
	register FAULTPTR f;

	for (f = g_pHeadFault->next; f != NULL; f = f->next)
	{
		printfault(fp, f, wmode);
		fprintf(fp, "\n");
	}
}

#define TEST_PER_L 13

/*
void print_test_topic(fp,npi,npo,name)
FILE *fp;
int npi,npo;
char name[];
{
   int i;
   GATEPTR gut;

   fprintf(fp,"* Name of circuit: %s\n",name);
   fprintf(fp,"* Primary inputs :\n  ");
   for(i=0;i<npi;i++) {
	  gut=net[primaryin[i]];
	  printgatename(fp,gut,'s'); fprintf(fp," ");
	  if(((i+1) mod TEST_PER_L == 0) || (i==npi-1))
	 fprintf(fp,"\n  ");
   }
   fprintf(fp,"\n");
   fprintf(fp,"* Primary outputs:\n  ");
   for(i=0;i<npo;i++) {
	  gut=net[primaryout[i]];
	  printgatename(fp,gut,'s'); fprintf(fp," ");
	  if(((i+1) mod TEST_PER_L == 0) || (i==npi-1))
	 fprintf(fp,"\n  ");
   }
   fprintf(fp,"\n\n* Test patterns and fault free responses:\n\n");
}
*/

/*------print_net------------------------------------------------------
	prints the circuit structure
	if wmode=='a', prints in both symbolic and neumeric notations.
	if wmode=='s', prints in symbolic notations.
	else prints in neumeric notations.
-----------------------------------------------------------------------*/
void print_net(FILE *fp, char name[], char wmode)
{
	register int i, j;
	int number_of_gates[MAXGTYPE],nonot = 0;

	for (i = 0; i < MAXGTYPE; i++)
		number_of_gates[i] = 0;
	for (i = 0; i < g_iNoGate; i++)
		if (g_net[i]->inCount == 1)
		{
			if (g_net[i]->type == AND || g_net[i]->type == OR)
			{
				number_of_gates[BUFF]++;
			}
			else if (g_net[i]->type == NAND || g_net[i]->type == NOR)
			{
				nonot++;
			}
			else
			{
				number_of_gates[g_net[i]->type]++;
			}
		}
		else
		{
			number_of_gates[g_net[i]->type]++;
		}

	fprintf(fp, "# %s\n", name);		/* name */
	fprintf(fp, "# %d inputs\n", g_iNoPI);
	fprintf(fp, "# %d outputs\n", g_iNoPO);
	fprintf(fp, "# %d D-type flipflops\n", g_iNoFF);
	fprintf(fp, "# %d inverters\n", nonot);
	fprintf(fp, "# %d gates", g_iNoGate - g_iNoPI - g_iNoFF - nonot);
	j = 0;
	for (i = 0; i < MAXGTYPE; i++)
		if (number_of_gates[i] > 0)
		{
			if (j == 0)
			{
				fprintf(fp, " (%d %ss", number_of_gates[i], fn_to_string[i]);
				j++;
			}
			else
			{
				fprintf(fp, " + %d %ss", number_of_gates[i], fn_to_string[i]);
			}
		}
	fprintf(fp, ")\n\n");

	for (i = 0; i < g_iNoPI; i++)
		if (wmode == 'a')
		{
			printgate(fp, g_net[g_PrimaryIn[i]], 's'); fprintf(fp, " ; ");
			printgate(fp, g_net[g_PrimaryIn[i]], 'n'); putc(CR, fp);
		}
		else
		{
			printgate(fp, g_net[g_PrimaryIn[i]], wmode); putc(CR, fp);
		}
	putc(CR, fp);
	for (i = 0; i < g_iNoPO; i++)
	{
		fprintf(fp, "OUTPUT(");
		if (wmode == 'a')
		{
			printgatename(fp, g_net[g_PrimaryOut[i]], 's');
			fprintf(fp, ") ; OUTPUT(");
			printgatename(fp, g_net[g_PrimaryOut[i]], 'n');
		}
		else
		{
			printgatename(fp, g_net[g_PrimaryOut[i]], wmode);
		}
		fprintf(fp, ")\n");
	}
	putc(CR, fp);

	for (i = 0; i < g_iNoGate; i++)
		if (g_net[i]->type != PI)
		{
			if (wmode == 'a')
			{
				printgate(fp, g_net[i], 's'); fprintf(fp, " ; ");
				printgate(fp, g_net[i], 'n');
			}
			else
			{
				printgate(fp, g_net[i], wmode);
			}
			putc(CR, fp);
		}
}

/*------count_events-----------------------------------------------------
	Counts numbers of next events (PPIs which are different from
	fault free circuits).
	Input:	f; fault
	Output: returns the number of events
------------------------------------------------------------------------*/
int count_events(register FAULTPTR f)
{
	int i = 0;
	EVENTPTR e;

	for (e = f->event; e != NULL; e = e->next)
		i++;
	return(i);
}

void print_event(FILE *file, FAULTPTR f, char mode)
{
	EVENTPTR e;

	if (count_events(f) > 0)
	{
		for (e = f->event; e != NULL; e = e->next)
		{
			printgate(file, g_net[e->node], mode);
			fprintf(file, ":");
			switch (e->value)
			{
			case 0:
				fprintf(file, "x  "); break;
			case 1:
				fprintf(file, "0  "); break;
			case 2:
				fprintf(file, "1  "); break;
			}
		}
	}
	fprintf(file, "\n");
}

void print_event_tree(FILE *file, EVENTPTR event, char mode)
{
	EVENTPTR e;

	if (event != NULL)
	{
		for (e = event; e != NULL; e = e->next)
		{
			printgatename(file, g_net[e->node], mode);
			fprintf(file, "=");
			switch (e->value)
			{
			case 0:
				fprintf(file, "x "); break;
			case 1:
				fprintf(file, "0 "); break;
			case 2:
				fprintf(file, "1 "); break;
			}
		}
	}
	fprintf(file, "\n");
}

void DFSWalk(GATEPTR child, char iomode)
{
	int i;

	set(child->changed);

	/* Go into children */
	if (iomode == 'i')
	{
		for (i = 0; i< child->outCount; i++)
		{
			if (!child->outList[i]->changed)
			{
				DFSWalk(child->outList[i], iomode);
			}
		}
	}
	else
	{
		for (i = 0; i< child->inCount; i++)
		{
			if (!child->inList[i]->changed)
			{
				DFSWalk(child->inList[i], iomode);
			}
		}
	}
}

int FindUnobservableGates(FILE *fp, status wflag, status iomode)
{
	register int i,count = 0;

	for (i = 0; i < g_iNoGate; i++)
		reset(g_net[i]->changed);
	if (iomode == 'i')
	{
		for (i = 0; i < g_iNoPI; i++)
			DFSWalk(g_net[g_PrimaryIn[i]], iomode);
	}
	else
	{
		for (i = 0; i < g_iNoPO; i++)
			DFSWalk(g_net[g_PrimaryOut[i]], iomode);
	}

	for (i = 0; i < g_iNoGate; i++)
		if (!g_net[i]->changed)
		{
			count++;
		}

	if (fp != NULL && count > 0)
	{
		if (iomode == 'i')
		{
			fprintf(fp, "Number of gates which are unreachable from PI = %d\n", count);
		}
		else
		{
			fprintf(fp, "Number of gates which are unreachable to any PO = %d\n", count);
		}
		for (i = 0; i < g_iNoGate; i++)
			if (g_net[i]->changed == 0 && g_net[i]->type == DFF)
			{
				printgate(fp, g_net[i], wflag); fprintf(fp, "\n");
			}
	}

	for (i = 0; i < g_iNoGate; i++)
		reset(g_net[i]->changed);

	return(count);
}
