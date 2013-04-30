
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
	Added shuffle compaction:  T. Chandra, 12/11/1993

	Now, atalanta accepts the circuit written in the netlist format
	of ISCAS89 benchmark circuits as well as the netlist format of
	ISCAS85 benchmark circuits.

		atalanta: version 2.0   	 H. K. Lee, 6/30/1997

***********************************************************************/

/*---------------------------------------------------------------------
	atalanta.c
	Main program for atalanta.
	An automatic test pattern generator for single stuck-at
	fault in combinational logic circuits.
	Generates test patterns detecting stuck-at faults in
	combinational circuits.
----------------------------------------------------------------------*/
#include "stdafx.h"
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
/*#ifdef WIN32*/
	#include <string.h> //for windows
// #else
// 	#include <strings.h> //for linux
// #endif
#include <stdlib.h>

#include "atalanta.h"
#include "error.h"
#include "io.h"
#include "testability.h"
#include "learn.h"
#include "ppsfp.h"
#include "print.h"
#include "pio.h"
#include "random.h"
#include "read_cct.h"
#include "structure.h"
#include "stem.h"
#include "define_fault_list.h"
#include "fsim.h"
#include "sim.h"
#include "help.h"

#include "parameter.h"
#include "define.h"
#include "macro.h"
#include "truthtable.h"

#include "global.h"

#define CHECKPOINTMODE 1
#define DEFAULTMODE 0


//akisn0w modifying start
//FILE *circuit;


// extern caddr_t sbrk();
// extern long random();
// extern int  init_fault();
// extern void print_atpg_head(), print_atpg_result(),
// 		help(), exit(), fatalerror(), learn();
// extern int print_undetected_faults();


#define valid_test(i) g_test_vectors[i][nopi] = ONE
#define invalid_test(i) g_test_vectors[i][nopi] = ZERO
#define is_valid(i) (g_test_vectors[i][nopi] == ONE)
#define is_invalid(i) (g_test_vectors[i][nopi] == ZERO)
#define is_random_mode(mode) (mode == 'y')
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

#define is_checkpoint(gate) (gate->type >= PI || gate->outCount > 1)
#define output0 output

#define checkbit(word,nth) ((word & BITMASK[nth]) != ALL0)
#define setbit(word,nth) (word |= BITMASK[nth])
#define resetbit(word,nth) (word &= (~BITMASK[nth]))

/* external variables */
// extern void setfanoutstem();
// extern void set_unique_path();
// extern void pinit_simulation();
// extern void GetPRandompattern();
// extern void pfault_free_simulation();
// extern void update_all1();
// extern char *strcpy(), *strcat();
// extern void GetRandompattern();
// extern void print_log_topic();
// extern void gettime();
// extern void printinputs(), printoutputs(), printfault();
// extern void set_testability();

/* variables for main program */
// level g_test_vectors[MAXTEST / 10][MAXPI + 1];
// level g_test_vectors1[MAXTEST / 10][MAXPI + 1];
// level g_test_store[MAXTEST / 10][MAXPI + 1];
// level g_test_store1[MAXTEST / 10][MAXPI + 1];
level g_test_vectors[MAXTEST / 10][MAXPI + 1];
level g_test_vectors1[MAXTEST / 10][MAXPI + 1];
level g_test_store[MAXTEST / 10][MAXPI + 1];
level g_test_store1[MAXTEST / 10][MAXPI + 1];


/* default parameters setting */
char g_strCctPathFileName[100] = "", g_strTestFileName[100] = "", g_strLogFileName[100] = "", g_strVecFileName[100] = "";
char g_strFaultFileName[100] = "";
char g_strCctFileName[100] = "";
char nameufaults[MAXSTRING] = "";
char inputmode = 'd';		/* default mode */
char rptmode = 'y';		/* RPT mode ON */
char logmode = 'n';		/* LOG off */
char fillmode = '0';		/* 0, 1, x, r */
char compact = 's';		/* n: no compaction, r: reverse, s: shuffle */
char helpmode = 'q';		/* On-line help mode */
char learnmode = 'n';
char chCctMode = ISCAS89;		/* input circuit format*/
int iseed = 0;			/* initial random seed */
int g_iRPTStopLimit = 16;		/* condition for RPT stopping */
int g_iMaxBackTrack1 = 10;		/* maximum backtracking of FAN */
int g_iMaxBackTrack2 = 0;		/* maximum backtracking of FAN */
int g_iMaxCompact = 2;		/* maximum limit for compaction*/
char _MODE_SIM = 'f';		/* 'f': FSIM, 'h': HOPE */
char faultmode = 'd';
char gen_all_pat = 'n';		/* 'y': generates all patterns for each fault */
int ntest_each_limit = (-1);	  /* no of patterns to be generated for each fault */
char no_faultsim = 'n';		/* 'y': no fault simulation */
char ufaultmode = 'n';  		  /* Write out undetected faults */
char rfaultmode = 'n';  		  /* Write out redundant faults */

FILE *fpFaultFile;
FILE *fpUndetFaultFile;
double lfMemSize;

// /*------main: Main program of atalanta---------------------------*/
int main(int argc, char *argv[]) //for windows
{
	register int i, j;
	int iNoShuffle;
	bool done = FALSE;
	//int number;
	int iMaxLevelAdd2 = 0;
	//int iMaxDetected; //nrestoredfault not used
	FAULTTYPE *pFault; //*pcurrentfault not used
	GATEPTR *pStemGates; //gut not used
	int iNoStemGates;
	//int n,k,iteration;
	status iState; //,fault_selection_mode;
	//int nbacktrack;
	int iNoDetected = 0, iNoRedundant = 0, iNoOverBackTrack = 0;
	int iNoTempPatterns = 0, iNoPatterns = 0, iNoPatternsAfterCompact = 0; //ntest1 not used
	int iNoDetectedAfterCompact = 0;
	int store = 0; //narray[MAXTEST] not used
	int iNoBackTrack = 0;
	//int lastfault;
	int ncomp = INFINITY,stop = ONE;
	double lfMinutes, lfSeconds, lfStartTime, lfInitTime, lfTimePre, lfTime;
	double lfFanTime, lfFanTimeRemaining, lfSimTime1, lfSimTime2, lfSimTime3;

	int iMaxBitSize = BITSIZE;
	//int fault_profile[BITSIZE];
	int iBit = 0,iPacket = 0;
	char c;
	//level LFSR[MAXPI],ran;
	int bit = 0,packet = 0;
	int bPhase2; //,nd;

	/*****************************************************************
	 *  															 *
	 *  		step 0: preprocess ---  							 *
	 *  			  input and output file interface   			 *
	 *  															 *
	 *****************************************************************/

	/*
	   if(argc==1) {
		  helpmode='d';
	   } else for(i=1;i<argc;i++) {
			   if(argv[i][0]=='-') {
			  if((i=option_set(argv[i][1],argv,i,argc))<0) {
				 helpmode='d'; break;
		   }}
		   else strcpy(name1,argv[i]);
			}
	   if(helpmode!='q') { help(helpmode); exit(0); }
	*/
	option_set(argc, argv);


	if ((g_fpCctFile = fopen(g_strCctPathFileName, "r")) == NULL)
	{
		fprintf(stderr, "Fatal error: no such file exists %s\n", g_strCctPathFileName);
		exit(0); //for windows
	}
	strcpy(g_strCctFileName, g_strCctPathFileName);

	i = 0; j = 0;
	if (g_strTestFileName[0] == '\0')
	{
		while ((c = g_strCctPathFileName[i++]) != '\0')
		{
#ifdef WIN32
			if (c == '\\')
#else
			if (c == '/')
#endif
			{
				j = 0;
			}
			else if (c == '.')
			{
				break;
			}
			else
			{
				g_strTestFileName[j++] = c;
			}
		}
		g_strTestFileName[j] = '\0';
		strcat(g_strTestFileName, ".test");
	}

	i = 0; j = 0;
	if (g_strLogFileName[0] == '\0')
	{
		while ((c = g_strCctPathFileName[i++]) != '\0')
		{
#ifdef WIN32
			if (c == '\\')
#else
			if (c == '/')
#endif
			{
				j = 0;
			}
			else if (c == '.')
			{
				break;
			}
			else
			{
				g_strLogFileName[j++] = c;
			}
		}
		g_strLogFileName[j] = '\0';
		strcat(g_strLogFileName, ".log");
	}

	i = 0; j = 0;
	if (g_strVecFileName[0] == '\0')
	{
		while ((c = g_strCctPathFileName[i++]) != '\0')
		{
			if (c == '\\')
			{
				j = 0;
			}
			else if (c == '.')
			{
				break;
			}
			else
			{
				g_strVecFileName[j++] = c;
			}
		}
		g_strVecFileName[j] = '\0';
		strcat(g_strVecFileName, ".vec");
	}

	if ((g_fpTestFile = fopen(g_strTestFileName, "w")) == NULL)
	{
		fprintf(stderr, "Fatal error: %s file open error\n", g_strTestFileName);
		exit(0);
	}

	if (logmode == 'y')
	{
		if ((g_fpLogFile = fopen(g_strLogFileName, "w")) == NULL)
		{
			fprintf(stderr, "Fatal error: %s file open error\n", g_strLogFileName);
			exit(0);
		}
	}

#ifdef DEBUG
	fprintf(stderr, "##########################################################\n");
	fprintf(stderr, "Warning: Atalanta is compiled in debug mode.\n");
	fprintf(stderr, "Warning: This mode may create an incorrect result or a wrong output file.\n");
	fprintf(stderr, "Warning: To avoid it, please comment out DEBUG flag in \"define.h\",\n");
	fprintf(stderr, "Warning: and recompile the program.\n");
	fprintf(stderr, "##########################################################\n");
#else
	if (logmode == 'y')
	{
		print_log_topic(g_fpLogFile, g_strCctFileName);
	}
#endif

#ifdef ISCAS85_NETLIST_MODE
	if (chCctMode == ISCAS85 && faultmode == 'f')
	{
		fprintf(stderr, "Fatal error in options:\n");
		fprintf(stderr, "The option -pFault can not combined with the option -I.\n");
		exit(0);
	}
#endif

	if (faultmode == 'f')
	{
		if ((fpFaultFile = fopen(g_strFaultFileName, "r")) == NULL)
		{
			fprintf(stderr, "Fatal error: %s file open error\n", g_strFaultFileName);
			exit(0);
		}
	}

	iseed = Seed(iseed);

	getTime(&lfMinutes, &lfSeconds, &lfStartTime);
	lfTimePre = lfStartTime;

	/**************************************************************
	 *  														  *
	 *  		step 1: preprocess ---  						  *
	 *  				construction of data structures 		  *
	 *  														  *
	 **************************************************************/

	if (readCircuit(g_fpCctFile, g_strCctFileName) < 0)
	{
		printFatalError(CIRCUITERROR);
	}
	fclose(g_fpCctFile);

	if (g_iNoGate <= 0 || g_iNoPI <= 0 || g_iNoPO <= 0)
	{
		fprintf(stderr, "Error: #pi=%d, #po=%d, #gate=%d\n", g_iNoPI, g_iNoPO, g_iNoGate);
		printFatalError(CIRCUITERROR);
	}
	if (g_iNoFF > 0)
	//STOP*************************STOP
	{
		fprintf(stderr, "Error: %d flip-flop exists in the circuit.\n", g_iNoFF);
		printFatalError(CIRCUITERROR);
	}
	nodummy = addPOGates();

	if (chCctMode == ISCAS89)
	{
		//DEFAULT !!!
		ALLOCATE(g_stack.list, GATEPTR, g_iNoGate + 10);
		clear(g_stack);

#ifdef INCLUDE_HOPE
		//DEFAULT !!!
		allocateStack1And2();
		g_iMaxLevel = computeLevels();
		allocateEventListStacks();
		updateGateIndexByLevel();
		addSpareGates();
		g_iLastGate = g_iNoGate - 1; //NO USE !!

		initStemAndFFR();
		initStemDominators();
#else
		//STOP*************************STOP
		if (updateGateIndexByLevel(g_net, g_iNoGate, g_iNoPI, g_iNoPO, g_iNoFF, g_stack.list) < 0)
		{
			fprintf(stderr, "Fatal error: Invalid circuit file.\n");
			exit();
		}
#endif

		if (g_iNoFF > 0) //No FF
		{
			//STOP*************************STOP
			fprintf(stderr, "Error: Invalid type DFF is defined.\n");
			exit(0);
		}
	}
	else //ISCAS85
	{
		//STOP*************************STOP
		g_stack.list = NULL;
	}
	
	//g_iMaxLevel = set_cct_parameters(g_iNoGate, g_iNoPI);
	//NO need to perform "g_iMaxLevel = g_iMaxLevel", who wrote this??
	setCctParameters(g_iNoGate, g_iNoPI);

#ifdef INCLUDE_HOPE
	iMaxLevelAdd2 = g_iMaxLevel + 2;
#else
	iMaxLevelAdd2 = g_iMaxLevel;
#endif

	if (_MODE_SIM == 'f') //Default
	{
		/* FSIM */
		if (!allocateStacks(g_iNoGate))
		{
			fprintf(stderr, "Fatal error: memory allocation error\n");
			exit(0);
		}

		iNoStemGates = 0;
		for (i = 0; i < g_iNoGate; i++)
		{
			if (is_fanout(g_net[i]) || g_net[i]->type == PO) //FANOUT or PO
			{
				iNoStemGates++;
			}
// 			if (g_net[i]->type == PO) //outCount = 0
// 			{
// 				printf("%d", g_net[i]->outCount);
// 				iStem = iStem;
// 			}
		}
		pStemGates = (GATEPTR *) malloc((unsigned)(sizeof(GATEPTR) * iNoStemGates));
		initStemGatesAndFOS(g_iNoGate, pStemGates, iNoStemGates);

		//faultmode = 'd' by default !!!, "-f" will lead to "faultmode  = 'f'"
		if (faultmode == 'd')
		{
			//DEFAULT !!!
			g_iNoFault = initStemFaultsAndFaultList(g_iNoGate, iNoStemGates, pStemGates);
			//Faults generated !!
		}
		else //faultmode == 'f'
		{
			//STOP*************************STOP
			g_iNoFault = readFaults(fpFaultFile, g_iNoGate, iNoStemGates, pStemGates);
		}
		
	}

#ifdef INCLUDE_HOPE
	else //Not default!!!, add "-H"
	{
		//STOP******************STOP
		
		/* HOPE */

		pStemGates = NULL;
		iNoStemGates = 0;

		if (faultmode == 'd')
		{
			initFaultList_HOPE();
		}
		else //faultmode == 'f'
		{
			readFaults_HOPE(fpFaultFile);
		}
	}
#endif

	if (g_iNoFault < 0) //Not possible!!
	{
		fprintf(stderr, "Fatal error: error in setting fault list\n");
		exit(0);
	}

	setGateTestability(g_iNoGate);

	for (i = 0; i < g_iNoGate; i++)
	{
		reset(g_net[i]->changed);
		g_net[i]->freach = g_iNoGate; //why??
		if (g_net[i]->dpi >= g_iPPOLevel) //No PPO here, so ERROR !!
		{
			printf("Error: gut=%s dpi=%d\n", g_net[i]->hash->symbol, g_net[i]->dpi);
		}
	}
	initFanoutGateDominators(g_iNoGate, iMaxLevelAdd2);
	initUniquePath(g_iNoGate, iMaxLevelAdd2);

	print_test_topic(g_fpTestFile, g_iNoPI, g_iNoPO, g_strCctPathFileName);
	if (learnmode == 'y') //NOT default !!
	{
		//STOP**********************STOP
		learn(g_iNoGate, iMaxLevelAdd2);
	}

	for (i = 0; i < g_iNoFault; i++)
	{
		//Intialization !!
		g_pFaultList[i]->detected = UNDETECTED;
		g_pFaultList[i]->observe = ALL0;
	}

	if (_MODE_SIM == 'f') //FSIM	Default !!!
	{
		iNoRedundant = deleteRedundantFaults(g_iNoGate);
		initGateStackAndFreach(g_iNoGate, iMaxLevelAdd2, g_iNoPI);
	}
	else //_MODE_SIM == 'h' //HOPE
	{
		//STOP**********************STOP
		initFaultSim_HOPE();
	}

	//iMaxDetected = g_iNoFault;

	g_iAllOne = ALL1;

	getTime(&lfMinutes, &lfSeconds, &lfTime);
	lfInitTime = lfTime - lfTimePre;
	lfTimePre = lfTime;
	lfSimTime1 = 0.0;
	fprintf(stderr, "end initialazition\n");
	//add some codes to print the names of inputs
	/*   FILE *fp_input;
	   fp_input=fopen("s38417tc.input","w");
	   for(i=0;i<nopi;i++)
	   {
		fprintf(fp_input,"%s ",net[i]->symbol->symbol);
	   }
	   fprintf(fp_input,"\n");
	   return 0;*/
	/*****************************************************************
	 *  															 *
	 *  	   step 2: Random pattern testing session   			 *
	 *  			1. generate 32 random patterns  				 *
	 *  			2. fault free simulation						 *
	 *  			3. fault simulation 							 *
	 *  			4. fault dropping   							 *
	 *  															 *
	 *****************************************************************/

	if (is_random_mode(rptmode)) //RPT Method is USELESS !!
	{
		//STOP***********************************STOP
		/*
		ndetect=random_sim(nog, nopi, nopo, LEVEL, nstem, stem, LFSR, randomlimit, maxbits, maxdetect, &ntest, &npacket, &nbit, test);
		ntest1=ntest;
		gettime(&minutes,&seconds,&runtime2);
		simtime1=runtime2-runtime1;
		runtime1=runtime2;*/
	}

	/******************************************************************
	 *  															  *
	 *    step 3: Deterministic Test Pattern Generation Session 	  *
	 *  		  (fan with unique path sensitization   			  *
	 *  															  *
	 ******************************************************************/
	reset(bPhase2);
	lfFanTime = 0;
	//bPhase2 == 0
	iNoDetected += testgen(g_iNoGate, g_iNoPI, g_iNoPO, iMaxLevelAdd2, iMaxBitSize, iNoStemGates, pStemGates, g_iMaxBackTrack1,
		bPhase2, &iNoRedundant, &iNoOverBackTrack, &iNoBackTrack, &iNoTempPatterns, &iPacket, &iBit, &lfFanTime, g_fpTestFile);
	iNoPatterns = iNoTempPatterns;

	/******************************************************************
	 *  															  *
	 *    step 4: Deterministic Test Pattern Generation Session 	  *
	 *  		  Phase 2: Employs dynamic unique path sensitization  *
	 *  															  *
	 ******************************************************************/

	iState = NO_TEST; //NO USE !!
	//g_iMaxBackTrack2 == 0 by default!!!
	if (g_iMaxBackTrack2 > 0 && g_iNoFault - iNoDetected - iNoRedundant > 0)
	{
		set(bPhase2);
		for (i = 0; i < g_iNoFault; i++)
		{
			pFault = g_pFaultList[i];
			if (pFault->detected == PROCESSED)
			{
				pFault->detected = UNDETECTED;
			}
		}
		lfFanTimeRemaining = 0;
		//bPhase2 == 1
		iNoDetected += testgen(g_iNoGate, g_iNoPI, g_iNoPO, iMaxLevelAdd2, iMaxBitSize, iNoStemGates, pStemGates, g_iMaxBackTrack2,
			bPhase2, &iNoRedundant, &iNoOverBackTrack, &iNoBackTrack, &iNoTempPatterns, &iPacket, &iBit, &lfFanTimeRemaining,
			g_fpTestFile);
		lfFanTime += lfFanTimeRemaining;
	}

	iNoPatterns = iNoTempPatterns;
	getTime(&lfMinutes, &lfSeconds, &lfTime);
	lfSimTime2 = lfTime - lfFanTime - lfTimePre;
	lfTimePre = lfTime;


	/********************************************************************
	 *  																*
	 *  	 step 5: Test compaction session							*
	 *  			 32-bit reverse fault simulation					*
	 *  			 + shuffling compaction   						   *
	 *  																*
	 ********************************************************************/

	if (iNoTempPatterns == 0) //NO test patterns, almost impossible !!
	{
		//STOP**************STOP
		iNoPatternsAfterCompact = 0;
		iNoDetectedAfterCompact = 0;
	}
	else if (compact == 'n')
	{
		iNoPatternsAfterCompact = iNoTempPatterns;
		iNoDetectedAfterCompact = iNoDetected;
	}
	else
	{
		if (g_iMaxCompact == 0)
		{
			compact = 'r';
		}
		iNoPatternsAfterCompact = compact_test(g_iNoGate, g_iNoPI, g_iNoPO, iMaxLevelAdd2, iNoStemGates, pStemGates, g_iNoFault, &iNoShuffle,
			&iNoDetectedAfterCompact, iPacket, iBit, iMaxBitSize, g_fpTestFile);
		if (iNoDetectedAfterCompact != iNoDetected)
		{
			printf("Error in test compaction:iNoPatterns=%d, iNoDetected=%d, iNoDetectedAfterCompact=%d\n", iNoPatterns, iNoDetected,
				iNoDetectedAfterCompact);
			//   exit(0);
		}
		//iNoDetectedAfterCompact ======== iNoDetected
	}




	getTime(&lfMinutes, &lfSeconds, &lfTime);
	lfSimTime3 = lfTime - lfTimePre;
	//lfRuntime1 = lfRuntime2;
	//msize = (int)sbrk(0)/1000; //no use
	lfMemSize = getMemory();

	/* print out the results */
	print_atpg_head(stdout);
	print_atpg_result(stdout, g_strCctFileName, g_iNoGate, g_iNoPI, g_iNoPO, iMaxLevelAdd2, g_iMaxBackTrack1, g_iMaxBackTrack2,
		bPhase2, iNoPatterns, iNoPatternsAfterCompact, g_iNoFault, iNoDetected, iNoRedundant, iNoBackTrack, iNoShuffle, lfInitTime,
		lfSimTime1 + lfSimTime2 + lfSimTime3, lfFanTime, lfTime - lfStartTime, 'n', lfMemSize);
	//				= lfSimTime				= lfFanTime		 = lfRunTime

#ifndef DEBUG
	if (logmode == 'y')
	{
		fprintf(g_fpLogFile, "\nEnd of test pattern generation.\n\n");
		print_atpg_head(g_fpLogFile);
		print_atpg_result(g_fpLogFile, g_strCctFileName, g_iNoGate, g_iNoPI, g_iNoPO, iMaxLevelAdd2, g_iMaxBackTrack1, g_iMaxBackTrack2, bPhase2,
			iNoPatterns, (int)(iNoPatternsAfterCompact / 9.4443), g_iNoFault, iNoDetected, iNoRedundant, iNoBackTrack, iNoShuffle, lfInitTime,
			lfSimTime1 + lfSimTime2 + lfSimTime3, lfFanTime, lfTime - lfStartTime, 'y', lfMemSize);
	}
#endif

	if (ufaultmode == 'y') //with "-u" "-U filename" "-v"
	{
		if ((rfaultmode == 'y' && iNoDetected < g_iNoFault) || (rfaultmode == 'n' && iNoDetected + iNoRedundant < g_iNoFault))
		{
			i = j = 0;
			if (nameufaults[0] == '\0')
			{
				while ((c = g_strCctPathFileName[i++]) != '\0')
				{
					if (c == '/')
					{
						j = 0;
					}
					else if (c == '.')
					{
						break;
					}
					else
					{
						nameufaults[j++] = c;
					}
				}
				nameufaults[j] = '\0';
				strcat(nameufaults, ".ufaults");
			}
			if ((fpUndetFaultFile = fopen(nameufaults, "r")) != NULL)
			{
				fprintf(stderr, "Warning: %s file already exists in the run directory\n", nameufaults);
				fprintf(stderr, "Warning: The undetected fault list file is not created.\n");
			}
			else
			{
				if ((fpUndetFaultFile = fopen(nameufaults, "w")) == NULL)
				{
					fprintf(stderr, "Fatal error: %s file open error\n", nameufaults);
					exit(0);
				}
				print_undetected_faults(fpUndetFaultFile, 's', rfaultmode, 0);
			}
			fclose(fpUndetFaultFile);
		}
	}

	fclose(g_fpTestFile);
	if (logmode == 'y')
	{
		fclose(g_fpLogFile);
	}

	return 0;
}


int read_option(char option, char *array[], int i, int n)
{
	if (i + 1 >= n)
	{
		return((-1));
	}

	switch (option)
	{
	case 'd':
		inputmode = 'd'; break;
#ifdef ISCAS85_NETLIST_MODE
	case 'I':
		chCctMode = ISCAS85; break;
#endif
	case 'r':
		sscanf(array[++i], "%d", &g_iRPTStopLimit);
		if (g_iRPTStopLimit == 0)
		{
			rptmode = 'n';
		} break;
	case 's':
		sscanf(array[++i], "%d", &iseed); break;
	case 'N':
		compact = 'n'; g_iMaxCompact = 0; break;
	case 'c':
		sscanf(array[++i], "%d", &g_iMaxCompact); break;
	case 'b':
		sscanf(array[++i], "%d", &g_iMaxBackTrack1); break;
	case 'B':
		sscanf(array[++i], "%d", &g_iMaxBackTrack2); break;
	case 't':
		strcpy(g_strTestFileName, array[++i]); break;
	case 'l':
		logmode = 'y'; strcpy(g_strLogFileName, array[++i]); break;
	case 'n':
		strcpy(g_strCctPathFileName, array[++i]); break;
	case 'h':
		helpmode = *(array[++i]); break;
	case 'L':
		learnmode = 'y'; break;
	case 'f':
		faultmode = 'f'; strcpy(g_strFaultFileName, array[++i]); break;
	case 'H':
		_MODE_SIM = 'h'; break;
	case '0':
		fillmode = '0'; break;
	case '1':
		fillmode = '1'; break;
	case 'X':
		fillmode = 'x'; break;
	case 'R':
		fillmode = 'r'; break;
	case 'A':
		gen_all_pat = 'y'; break;
	case 'D':
		sscanf(array[++i], "%d", &ntest_each_limit);
		gen_all_pat = 'y'; break;
	case 'Z':
		no_faultsim = 'y'; break;
	case 'u':
		ufaultmode = 'y'; break;
	case 'U':
		ufaultmode = 'y'; strcpy(nameufaults, array[++i]); break;
	case 'v':
		ufaultmode = 'y'; rfaultmode = 'y'; break;
	default:
		i = (-1);
	}
	return(i);
}

void option_set(int argc, char *argv[])
{
	int i;

	if (argc == 1)
	{
		helpmode = 'd';
	}
	else
	{
		for (i = 1; i < argc; i++)
		{
			if (argv[i][0] == '-')
			{
				if ((i = read_option(argv[i][1], argv, i, argc)) < 0)
				{
					helpmode = 'd'; break;
				}
			}
			else
			{
				strcpy(g_strCctPathFileName, argv[i]);
			}
		}
	}

	if (helpmode != 'q')
	{
		help(helpmode); exit(0);
	}
	if (fillmode == 'x')
	{
		_MODE_SIM = 'h';
	}
	if (gen_all_pat == 'y')
	{
		g_iRPTStopLimit = 0;
		rptmode = 'n';
		g_iMaxBackTrack2 = 0;
		fillmode = 'x';
		compact = 'n';
		g_iMaxCompact = 0;
		_MODE_SIM = 'h';
		no_faultsim = 'y';
	}

	if (no_faultsim == 'y')
	{
		g_iRPTStopLimit = 0;
		rptmode = 'n';
		fillmode = 'x';
		compact = 'n';
		g_iMaxCompact = 0;
		_MODE_SIM = 'h';
	}
}
