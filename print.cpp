
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

		   Major change in log file format, H. K. Lee, 6/30/1997
 
***********************************************************************/

/*---------------------------------------------------------------------
	print.c
	prints head lines of atalanta and
	test pattern generation results.
----------------------------------------------------------------------*/
#include "stdafx.h"
#include <stdio.h>

#include "print.h"
#include "parameter.h" 
#include "define.h"    
#include "macro.h"

#include "pio.h"

extern char rptmode;
extern char compact;
extern int g_iRPTStopLimit, iseed, g_iMaxCompact;
extern double lfMemSize;
extern struct FAULT **g_pFaultList;

//extern void printfault();
//int print_undetected_faults();

char g_strTitle[] = "******   SUMMARY OF TEST PATTERN GENERATION RESULTS   ******";

/*------print_atpg_head---------------------------------------*/
void print_atpg_head(FILE *fp)
{
	fprintf(fp, "\t*******************************************************\n");
	fprintf(fp, "\t*                                                     *\n");
	fprintf(fp, "\t*          Welcome to atalanta (version 2.0)          *\n");
	fprintf(fp, "\t*                                                     *\n");
	fprintf(fp, "\t*               Dong S. Ha (ha@vt.edu)                *\n");
	fprintf(fp, "\t*            Web: http://www.ee.vt.edu/ha             *\n");
	fprintf(fp, "\t*  Virginia Polytechnic Institute & State University  *\n");
	fprintf(fp, "\t*                                                     *\n");
	fprintf(fp, "\t*******************************************************\n");
	fprintf(fp, "\n");
}


void print_atpg_result(FILE *fpFile, char *strCctFileName, int iNoGate, int iNoPI, int iNoPO,
	int iMaxLevelAdd2, int iMaxBackTrack1, int iMaxBackTrack2, int bPhase2, int iNoPatterns, int iNoPatternsAfterCompact,
	int g_iNoFault, int iNoDetected, int iNoRedundant, int iNoBackTrack, int iNoShuffle, double lfInitTime,
	double lfSimTime, double lfFanTime, double lfRunTime, char cMode, double lfMemSize)
// FILE *fpFile;
// char *strCctPathFileName2;	/* circuit strCctPathFileName2 */
// int iNoGate;		/* number of gate */
// int iNoPI;	/* number of primary inputs */
// int iNoPO;	/* number of primary outputs */
// int iMaxLevelAdd2;	/* max level */
// int iMaxBackTrack1;	/* max backtrack limit for phase1*/
// int iMaxBackTrack2;	/* max backtrack limit for phase2*/
// int bPhase2;	/* phase 2 used */
// int iNoPatterns;	/* number of test patterns before test compaction */
// int iNoPatternsAfterCompact;	/* number of test patterns after test compaction */
// int g_iNoFault;	/* number of total faults */
// int iNoDetected;		/* number of detected faults */
// int iNoRedundant;	/* number of redundant faults */
// int iNoBackTrack;	/* number of backtracks (total) */
// int iNoShuffle;	/* number of shuffles*/
// double lfInitTime,lfSimTime,lfFanTime,lfRunTime;
// char cMode;	/* If 'y', prints redundant and undetected faults */
// int lfMemSize;	/* lfMemSize used*/
{
	int iNoPatternsAfterCompact2; //i no use
	/* int j; */
	//FAULTPTR f; //no use
	//iNoPatternsAfterCompact2 = (int)(iNoPatternsAfterCompact / 7.7);
	iNoPatternsAfterCompact2 = iNoPatternsAfterCompact;

	///////////////////////////////////////////////////////////////////////////////////////
	fprintf(fpFile, "%s\n", g_strTitle);
	fprintf(fpFile, "1. Circuit structure\n");
	fprintf(fpFile, "   Name of the circuit                       : %s\n", strCctFileName);
	fprintf(fpFile, "   Number of primary inputs                  : %d\n", iNoPI);
	fprintf(fpFile, "   Number of primary outputs                 : %d\n", iNoPO);
	fprintf(fpFile, "   Number of gates                           : %d\n", iNoGate - iNoPI - iNoPO);
	fprintf(fpFile, "   Level of the circuit                      : %d\n", iMaxLevelAdd2 - 3);


	///////////////////////////////////////////////////////////////////////////////////////
	fprintf(fpFile, "\n");
	fprintf(fpFile, "2. ATPG parameters\n");
	fprintf(fpFile, "   Test pattern generation Mode              : ");
	if (rptmode == 'n')
	{
		fprintf(fpFile, "DTPG + TC\n");
	}
	else if (rptmode == 'y')
	{
		fprintf(fpFile, "RPT + DTPG + TC\n");
	}
	if (rptmode == 'y')
	{
		fprintf(fpFile, "   Limit of random patterns (packets)        : %d\n", g_iRPTStopLimit);
	}
	if (!bPhase2)
	{
		fprintf(fpFile, "   Backtrack limit                           : %d\n", iMaxBackTrack1);
	}
	else
	{
		fprintf(fpFile, "   Backtrack limit (Phase 1 test generation) : %d\n", iMaxBackTrack1);
		fprintf(fpFile, "   Backtrack limit (Phase 2 test generation) : %d\n", iMaxBackTrack2);
	}
	fprintf(fpFile, "   Initial random number generator seed      : %d\n", iseed);
	fprintf(fpFile, "   Test pattern compaction Mode              : ");
	if (compact == 'n')
	{
		fprintf(fpFile, "NONE \n");
	}
	else if (compact == 'r')
	{
		fprintf(fpFile, "REVERSE \n");
	}
	else if (compact == 's')
	{
		fprintf(fpFile, "REVERSE + SHUFFLE \n");
	}
	if (compact == 's')
	{
		fprintf(fpFile, "   Limit of shuffling compaction             : %d\n", g_iMaxCompact);
		fprintf(fpFile, "   Number of shuffles                        : %d\n", iNoShuffle);
	}


	///////////////////////////////////////////////////////////////////////////////////////
	fprintf(fpFile, "\n");
	fprintf(fpFile, "3. Test pattern generation results\n");
	if (compact == 'n')
	{
		fprintf(fpFile, "   Number of test patterns                   : %d\n", iNoPatterns);
	}
	else //COMPACT !!!
	{
		fprintf(fpFile, "   Number of test patterns before compaction : %d\n", iNoPatterns);
		fprintf(fpFile, "   Number of test patterns after compaction  : %d\n", iNoPatternsAfterCompact2);
	}
	fprintf(fpFile, "   Fault coverage                            : %.3lf %%\n", (double) iNoDetected / (double) g_iNoFault * 100.0);
	fprintf(fpFile, "   Number of collapsed faults                : %d\n", g_iNoFault);
	fprintf(fpFile, "   Number of identified redundant faults     : %d\n", iNoRedundant);
	fprintf(fpFile, "   Number of aborted faults                  : %d\n", g_iNoFault - iNoDetected - iNoRedundant);
	fprintf(fpFile, "   Total number of backtrackings             : %d\n", iNoBackTrack);


	///////////////////////////////////////////////////////////////////////////////////////
	fprintf(fpFile, "\n");
	fprintf(fpFile, "4. Memory used                               : %.3lf MB\n", lfMemSize);


	///////////////////////////////////////////////////////////////////////////////////////
	fprintf(fpFile, "\n");
	fprintf(fpFile, "5. CPU time\n");
	fprintf(fpFile, "   Initialization                            : %.3lf Secs\n", lfInitTime);
	fprintf(fpFile, "   Fault simulation                          : %.3lf Secs\n", lfSimTime);
	fprintf(fpFile, "   FAN                                       : %.3lf Secs\n", lfFanTime);
	fprintf(fpFile, "   Total                                     : %.3lf Secs\n", lfRunTime);

	/*
	   if(cMode=='y' && iNoDetected<g_iNoFault) {
		  if(iNoRedundant > 0)
			 fprintf(fpFile,"\n* List of identified redundant faults:\n");
		  for(i=0;i<g_iNoFault;i++) {
			 f=faultlist[i];
			 if(f->detected==REDUNDANT) {
			printfault(fpFile,f,0);
		 }
		  }
		  if(g_iNoFault-iNoDetected-iNoRedundant > 0)
			 fprintf(fpFile,"\n* List of aborted faults:\n");
		  for(i=0;i<g_iNoFault;i++) {
		 f=faultlist[i];
		 if(f->detected==PROCESSED) {
			printfault(fpFile,f,0);
			 }
		  }
	   }
	*/
	if (cMode == 'y' && iNoDetected < g_iNoFault)
	{
		print_undetected_faults(fpFile, 's', 'y', 1);
	}
}

extern int g_iNoFault;
void print_undetected_faults(FILE *fpFile, char cSymbol, char cRFaultMode, int bFlag)
{
	int i;
	int no_red, no_det, no_abort;
	FAULTPTR pFault;

	no_red = no_det = no_abort = 0;
	for (i = 0; i < g_iNoFault; i++)
	{
		pFault = g_pFaultList[i];
		switch (pFault->detected)
		{
		case REDUNDANT:
			no_red++; break;
		case PROCESSED:
			no_abort++; break;
		default:
			no_det++; break;
		}
	}
	if (cRFaultMode == 'y' && no_red > 0)
	{
		if (bFlag)
		{
			fprintf(fpFile, "\n* List of identified redundant faults:\n");
		}
		/* j=0; */
		for (i = 0; i < g_iNoFault; i++)
		{
			pFault = g_pFaultList[i];
			if (pFault->detected == REDUNDANT)
			{
				/* if(bFlag) fprintf(fpFile,"%4d. ",++j); */
				printfault(fpFile, pFault, 0);
			}
		}
	}
	if (no_abort > 0)
	{
		if (bFlag)
		{
			fprintf(fpFile, "\n* List of aborted faults:\n");
		}
		/* j=0; */
		for (i = 0; i < g_iNoFault; i++)
		{
			pFault = g_pFaultList[i];
			if (pFault->detected == PROCESSED)
			{
				/* if(bFlag) fprintf(fpFile,"%4d. ",++j); */
				printfault(fpFile, pFault, 0);
			}
		}
	}
}

char titlesim[] = "******       SUMMARY OF FAULT SIMULATION RESULTS      ******";

/*------print_sim_head---------------------------------------*/
void print_sim_head(FILE *fpFile)
{
	fprintf(fpFile, "*********************************************************\n");
	fprintf(fpFile, "*                                                       *\n");
	fprintf(fpFile, "*            Welcome to fsim (version 1.1)              *\n");
	fprintf(fpFile, "*                                                       *\n");
	fprintf(fpFile, "*                 Copyright (C) 1991,                   *\n");
	fprintf(fpFile, "*   Virginia Polytechnic Institute & State University   *\n");
	fprintf(fpFile, "*                                                       *\n");
	fprintf(fpFile, "*********************************************************\n");
	fprintf(fpFile, "\n");
}


void print_sim_result(FILE *fp, char *name, int ng, int npi, int npo, int mlev, char *nametest, int nt, int nof, int nd, double inittime, double simtime, double runtime, char mode)
// FILE *fp;
// char *name;	/* circuit name */
// int ng;		/* number of gate */
// int npi;	/* number of primary inputs */
// int npo;	/* number of primary outputs */
// int mlev;	/* max level */
// char *nametest;	/* name of test cct */
// int nt;		/* number of test patterns */
// int nof;	/* number of total faults */
// int nd;		/* number of detected faults */
// double inittime,simtime,runtime;
// char mode;	/* If 'y', prints redundant and undetected faults */
{
	int i;
	/* int j; */
	FAULTPTR f;

	fprintf(fp, "%s\n", titlesim);
	fprintf(fp, "1. Circuit structure\n");
	fprintf(fp, "   Name of the circuit                       : %s\n", name);
	fprintf(fp, "   Number of primary inputs                  : %d\n", npi);
	fprintf(fp, "   Number of primary outputs                 : %d\n", npo);
	fprintf(fp, "   Number of gates                           : %d\n", ng - npi - npo);
	fprintf(fp, "   Level of the circuit                      : %d\n", mlev - 3);
	fprintf(fp, "\n");
	fprintf(fp, "2. Simulation parameters\n");
	fprintf(fp, "   Simulation mode                           : ");
	if (rptmode == 'n')
	{
		fprintf(fp, "file (%s)\n", nametest);
	}
	else if (rptmode == 'y')
	{
		fprintf(fp, "random\n");
	}
	if (rptmode == 'y')
	{
		fprintf(fp, "   Initial random number generator seed      : %d\n", iseed);
	}
	fprintf(fp, "\n");
	fprintf(fp, "3. Simulation results\n");
	fprintf(fp, "   Number of test patterns applied           : %d\n", nt);
	fprintf(fp, "   Fault coverage                            : %.3lf %%\n", (double) nd / (double) nof * 100.0);
	fprintf(fp, "   Number of collapsed faults                : %d\n", nof);
	fprintf(fp, "   Number of detected faults                 : %d\n", nd);
	fprintf(fp, "   Number of undetected faults               : %d\n", nof - nd);

	fprintf(fp, "\n4. Memory used                               : %d Kbytes\n", lfMemSize);

	fprintf(fp, "\n5. CPU time\n");
	fprintf(fp, "   Initialization                            : %.3lf secs\n", inittime);
	fprintf(fp, "   Fault simulation                          : %.3lf secs\n", simtime);
	fprintf(fp, "   Total                                     : %.3lf secs\n", runtime);

	if (mode == 'y' && nd < nof)
	{
		fprintf(fp, "\n* List of undetected faults:\n");
		/*    j=0; */
		for (i = 0; i < nof; i++)
		{
			f = g_pFaultList[i];
			if (f->detected != DETECTED)
			{
				/*		fprintf(fp,"%4d. ",++j); */
				printfault(fp, f, 0);
			}
		}
	}
}
