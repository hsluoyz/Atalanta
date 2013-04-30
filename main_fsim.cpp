
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
	fsim.c
	Main program for FSIM.
	FSIM performs fault simulation for combinational circuits.
	Test patterns can be read from a user supplied file or
	generated randomly.
	Collapses the equivalent faults and fault coverage is
	computed based on the collapsed fault set.
-----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
//#include <strings.h> //for linux only, change to windows edition as below
#include <string.h>

#include "atalanta.h"
#include "help_fsim.h"
#include "random.h"
#include "io.h"
#include "read_cct.h"
#include "error.h"
#include "structure.h"
#include "define_fault_list.h"
#include "ppsfp.h"
#include "pio.h"
#include "print.h"

#include "main_fsim.h"
#include "parameter.h"
#include "define.h"
#include "macro.h"
#include "global.h"

#define DEFAULT_RLIMIT 224
#define is_random_mode(mode) (mode=='y')
#define is_fanout(gate) (gate->outCount>1)
#define output0	output

/* external functions */
// extern void GetPRandompattern();
// extern void pinit_simulation();
// extern void pfault_free_simulation();
// extern void printinputs(),print_log_topic(),printoutputs(),printfault();
// char *strcpy(), *strcat();
// extern void update_all(), update_all1();
// extern void gettime();
// extern void setfanoutstem();
// extern void set_unique_path();
// extern caddr_t sbrk();

/* default parameters setting */
char name1_f[100] = "",name2_f[100] = "",name3_f[100] = "",commandfile_f[100];
char namecct_f[100] = "";
char faultname_f[100] = "";
char inputmode_f = 'd';		/* default mode */
char rptmode_f = 'y';		/* RPT mode ON */
char logmode_f = 'n';		/* LOG off */
char helpmode_f = 'q';  			/* On-line help mode */
char cctmode_f = ISCAS89;   		/* input circuit format */
int iseed_f = 0;			/* initial random seed */
int randomlimit_f = DEFAULT_RLIMIT;	/* limit of random patterns */
int maxbit_f = BITSIZE;		/* number of bits for one operation */
char faultmode_f = 'd';
char compact_f;
int maxcompact_f;
int msize_f;
level LFSR_f[MAXPI];
FILE *faultfile_f;
int Group_Id_f;

/*
	main
	Main program of FSIM.
*/
void main2(int argc, char *argv[])
{
	register int i, j;
	int nbit;
	int maxdpi, nstem, ndominator; //,LEVEL;
	int nof, ndetect, maxdetect, nredundant;
	int ntest;
	//FAULTPTR f;
	GATEPTR *stem;
	int fault_profile[BITSIZE];
	double inittime, runtime;
	double runtime1, minutes, seconds;
	double coverage;
	char c;

	nbit = maxbit_f;
	update_flag = FALSE;
	update_flag2 = FALSE;

	/********************************************************************
	 *  																*
	 *  				preprocess ---  								*
	 *  				input and output file interface 				*
	 *  																*
	 ********************************************************************/

	if (argc == 1)
	{
		helpmode_f = 'd';
	}
	else
	{
		for (i = 1; i < argc; i++)
		{
			if (argv[i][0] == '-')
			{
				if ((i = option_set(argv[i][1], argv, i, argc)) < 0)
				{
					helpmode_f = 'd'; break;
				}
			}
			else
			{
				strcpy(name1_f, argv[i]);
			}
		}
	}

	if (helpmode_f != 'q')
	{
		help_fsim(helpmode_f); exit(0);
	}

	if ((circuit = fopen(name1_f, "r")) == NULL)
	{
		fprintf(stderr, "Fatal error: no such file exists %s\n", name1_f);
		exit(0);
	}

	strcpy(namecct_f, name1_f);

	i = 0; j = 0;
	if (name3_f[0] == '\0')
	{
		while ((c = name1_f[i++]) != '\0')
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
				name3_f[j++] = c;
			}
		}
		name3_f[j] = '\0';
		strcat(name3_f, ".log");
	}

	if (rptmode_f == 'n')
	{
		if ((test = fopen(name2_f, "r")) == NULL)
		{
			fprintf(stderr, "Fatal error: %s file open error\n", name2_f);
			exit(0);
		}
	}

	if (logmode_f == 'y')
	{
		if ((logfile = fopen(name3_f, "w")) == NULL)
		{
			fprintf(stderr, "Fatal error: %s file open error\n", name3_f);
			exit(0);
		}
	}
	/*
	   if(logmode=='y') print_log_topic(logfile,namecct);
	*/

	if (cctmode_f == ISCAS85 && faultmode_f == 'f')
	{
		fprintf(stderr, "Fatal error in options:\n");
		fprintf(stderr, "The option -f can not combined with the option -I.\n");
		exit(0);
	}

	if (faultmode_f == 'f')
	{
		if ((faultfile_f = fopen(faultname_f, "r")) == NULL)
		{
			fprintf(stderr, "Fatal error: %s file open error\n", faultname_f);
			exit(0);
		}
	}

	if (rptmode_f == 'y')
	{
		iseed_f = Seed(iseed_f);
	}

	gettime(&minutes, &seconds, &runtime1);

	/*****************************************************************
	 *  															 *
	 *  				preprocess ---  							 *
	 *  				construction of data structures 			 *
	 *  															 *
	 *****************************************************************/

	if (cctmode_f == ISCAS89)
	{
		if (read_circuit(circuit, namecct_f) < 0)
		{
			fprintf(stderr, "Fatal error: Invalid circuit file.\n");
			exit(0);
		}
	}

#ifdef ISCAS85_NETLIST_MODE
	else if (!circin(&nog, &nopi, &nopo))
	{
		fprintf(stderr, "Fatal error: Invalid circuit file\n");
		exit(0);
	}
#endif

	fclose(circuit);
	nodummy = add_PO();

	if (nog <= 0 || nopi <= 0 || nopo <= 0)
	{
		fprintf(stderr, "Fatal error: Invalid circuit file\n");
		exit(0);
	}

	if (cctmode_f == ISCAS89)
	{
		ALLOCATE(stack.list, GATEPTR, nog);
		clear(stack);

#ifdef INCLUDE_HOPE
		allocate_stacks();
		maxdpi = compute_level();
		allocate_event_list();
		levelize();
		iLastGate = nog - 1;
#else
		if (levelize(net, nog, nopi, nopo, noff, stack.list) < 0)
		{
			fprintf(stderr, "Fatal error: Invalid circuit file.\n");
			exit(0);
		}
#endif

		if (noff > 0)
		{
			fprintf(stderr, "Error: Invalid type DFF is defined.\n");
			exit(0);
		}
	}
	else
	{
		stack.list = NULL;
	}

#ifdef INCLUDE_HOPE
	set_cct_parameters(nog, nopi);
	maxdpi = POlevel + 1;
#else
	maxdpi = set_cct_parameters(nog, nopi);
#endif

	if (!allocate_dynamic_buffers(nog))
	{
		fprintf(stderr, "Fatal error: memory allocation error\n");
		exit(0);
	}

	nstem = 0;
	for (i = 0; i < nog; i++)
		if (net[i]->outCount != 1)
		{
			nstem++;
		}
	stem = (GATEPTR *)malloc((unsigned)((sizeof(GATEPTR)) * nstem));
	setfanoutstem(nog, stem, nstem);

	nof = (faultmode_f == 'f') ? readfaults(faultfile_f, nog, nstem, stem) : set_all_fault_list(nog, nstem, stem);

	if (nof < 0)
	{
		fprintf(stderr, "Fatal error: error in setting fault list\n");
		exit(0);
	}

	for (i = 0; i < nog; i++)
	{
		reset(net[i]->changed);
		net[i]->freach = nog;
	}
	ndominator = set_dominator(nog, maxdpi);
	set_unique_path(nog, maxdpi);

	/*****************************************************************
	 *  															 *
	 *  			   Initialization of circuit parameters 		 *
	 *  															 *
	 *****************************************************************/

	for (i = 0; i < nof; i++)
	{
		faultlist[i]->detected = UNDETECTED;
		faultlist[i]->observe = ALL0;
	}

	nredundant = check_redundant_faults(nog);

	pinit_simulation(nog, maxdpi, nopi);
	ntest = 0;
	ndetect = 0;
	maxdetect = nof;

	all_one = (maxbit_f == BITSIZE) ? ALL1 : ~(ALL1 << maxbit_f);

	gettime(&minutes, &seconds, &runtime1);
	inittime = runtime1;

	/*******************************************************************
	*   															   *
	*		Main loop for the fault simulaiton. 			   *
	*		1. Read test patterns.  						   *
	*		2. Fault free simulation.   					   *
	*		3. Fault Simulation.				   *
	*		4. Update flags.				   *
	*   															   *
	********************************************************************/

	while (ntest < randomlimit_f)
	{
		/* Get a test pattern */
		if (rptmode_f == 'y')
		{
			GetPRandompattern(nopi, LFSR_f);
		}
		else
		{
			if ((nbit = pget_test(test, LFSR_f, nopi, maxbit_f)) == 0)
			{
				break;
			}
			all_one = (nbit == BITSIZE) ? ALL1 : ~(ALL1 << nbit);
		}

		/* fault free simulation */
		for (i = 0; i < nopi; i++)
			net[i]->output1 = net[i]->output0 = LFSR_f[i];

		pfault_free_simulation();

		/* fault simulation */
		for (i = 0; i < nbit; i++)
			fault_profile[i] = 0;
		ndetect += Fault1_Simulation(nog, maxdpi, nopi, nopo, nstem, stem, nbit, fault_profile);
		ntest += nbit;

		if (logmode_f == 'y')
		{
			ntest -= nbit;
			for (i = nbit - 1; i >= 0; i--)
			{
				fprintf(logfile, "test %4d: ", ++ntest);
				printinputs(logfile, nopi, i);
				fprintf(logfile, " ");
				printoutputs(logfile, nopo, i);
				fprintf(logfile, " %4d faults detected", fault_profile[i]);

				fprintf(logfile, "\n");
			}
		}

		if (ndetect >= maxdetect)
		{
			break;
		}

		/* dynamic scheduling of network flags */
		for (i = 0; i <= nsstack; i++)
			dynamic_stack[i]->cobserve = ALL0;
		if (update_flag)
		{
			if (logmode_f == 'y')
			{
				update_all1(nopi);
			}
			else
			{
				update_all(nopi);
			}
			reset(update_flag);
		}
		else
		{
			for (i = ndstack; i > nsstack; i--)
				dynamic_stack[i]->freach = 0;
		}
		ndstack = nsstack;
	}

	gettime(&minutes, &seconds, &runtime);
	coverage = (double)ndetect / (double)nof * 100.00;
	//msize = (int)sbrk(0)/1000; // no use
	msize_f = 0;

	/* print out the results */
	print_sim_head(stdout);
	print_sim_result(stdout, namecct_f, nog, nopi, nopo, maxdpi, name2_f, ntest, nof, ndetect, (float)inittime, (float)(runtime - inittime), (float)runtime, 'n');
	if (logmode_f == 'y')
	{
		/*
								  fprintf(logfile,"\nEnd of fault simulation.\n\n");
								  print_sim_head(logfile);
								  print_sim_result(logfile,namecct,nog,nopi,nopo,maxdpi,name2,
									 ntest,nof,ndetect,inittime,runtime-inittime,runtime,'y');
							*/
		fclose(logfile);
	}

	if (rptmode_f == 'n')
	{
		fclose(test);
	}
}


int option_set(char option, char *array[], register int i, register int n)
{
	if (i + 1 >= n)
	{
		return((-1));
	}

	switch (option)
	{
	case 'd':
		inputmode_f = 'd'; break;
	case 'I':
		cctmode_f = ISCAS85; break;
	case 'r':
		sscanf(array[++i], "%d", &randomlimit_f); 
		if (randomlimit_f == 0)
		{
			randomlimit_f = DEFAULT_RLIMIT;
		} break;
	case 's':
		sscanf(array[++i], "%d", &iseed_f); break;
	case 't':
		rptmode_f = 'n'; randomlimit_f = INFINITY;
		strcpy(name2_f, array[++i]); break;
	case 'l':
		logmode_f = 'y'; strcpy(name3_f, array[++i]); break;
	case 'n':
		strcpy(name1_f, array[++i]); break;
	case 'h':
		helpmode_f = *(array[++i]); break;
	case 'f':
		faultmode_f = 'f'; strcpy(faultname_f, array[++i]); break;
	default:
		i = (-1);
	}
	return(i);
}

