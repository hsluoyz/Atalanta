
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

/*---------------------------------------------------------------------

	file name: help.c

		Displays online help manual.
-----------------------------------------------------------------------*/
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "help_fsim.h"

extern char *getenv(), * strcpy(), * strcat();

#define DELI '|'
#define FP stdout
#define FP1 stderr

char *prog_fsim = "fsim";
char *commandline_fsim = "fsim";
char *helpfilename_fsim = "man";
char helpfile_fsim[250] = "";
char *MANENV_FSIM = "FSIM_MAN";

void help_fsim(char mode)
{
	FILE *fp;
	char c;
	char *envptr;

	if (mode != 'd')
	{
		if ((envptr = (char*)getenv(MANENV_FSIM)) == NULL)
		{
			strcpy(helpfile_fsim, helpfilename_fsim);
		}
		else
		{
			strcpy(helpfile_fsim, envptr);
			strcat(helpfile_fsim, "/");
			strcat(helpfile_fsim, helpfilename_fsim);
		}

		if ((fp = fopen(helpfile_fsim, "r")) == NULL)
		{
			fprintf(FP1, "\n\nError:\tCannot find the on-line help file ");
			fprintf(FP1, "\"%s\" of %s\n", helpfilename_fsim, prog_fsim);
			if (envptr != NULL)
			{
				fprintf(FP1, "\tunder the directory \"%s\".\n\n", envptr);
			}
			else
			{
				fprintf(FP1, "under the current directory.\n\n");
				fprintf(FP1, "Since the environment variable %s ", MANENV_FSIM);
				fprintf(FP1, "is not defined,\n");
				fprintf(FP1, "the current directory is searched.");
			}
			fprintf(FP1, "Please check your environment variables");
			fprintf(FP1, " usng \"env\".\n");
			fprintf(FP1, "The variable %s should be set to", MANENV_FSIM);
			fprintf(FP1, " the directory of %s manual.\n\n", prog_fsim);
			if (envptr != NULL)
			{
				fprintf(FP1, "If the variable %s is incorrect,\n", MANENV_FSIM);
			}
			else
			{
				fprintf(FP1, "Please, ");
			}
			fprintf(FP1, "set the environment variable %s using\n\n", MANENV_FSIM);
			fprintf(FP1, "        setenv %s directory-of-%s-manual\n", MANENV_FSIM, prog_fsim);
			fprintf(FP1, "\nThen, try the help command again.\nThanks.\n");
			return;
		}
	}

	/* header of on-line-manual */
	if (mode != 'd')
	{
		while ((c = getc(fp)) != EOF)
		{
			if (c == DELI)
			{
				break;
			} putc(c, FP);
		}
		if (c == EOF)
		{
			return;
		}
	}

	switch (mode)
	{
	case 'a':
	case 'A':
		/* all */
		while ((c = getc(fp)) != EOF)
		{
			if (c != DELI)
			{
				putc(c, FP);
			}
		} 
		break;
	case 'g':
	case 'G':
		/* user's guide */
		while ((c = getc(fp)) != EOF)
		{
			if (c == DELI)
			{
				break;
			} putc(c, FP);
		}
		break;
	case 'n':
	case 'N':
		/* netlist format */
		while ((c = getc(fp)) != EOF)
		{
			if (c == DELI)
			{
				break;
			}
		}
		if (c == EOF)
		{
			return;
		}
		while ((c = getc(fp)) != EOF)
		{
			if (c == DELI)
			{
				break;
			} putc(c, FP);
		}
		break;
	case 't':
	case 'T':
		while ((c = getc(fp)) != EOF)
		{
			if (c == DELI)
			{
				break;
			}
		}
		if (c == EOF)
		{
			return;
		}
		while ((c = getc(fp)) != EOF)
		{
			if (c == DELI)
			{
				break;
			}
		}
		if (c == EOF)
		{
			return;
		}
		while ((c = getc(fp)) != EOF)
		{
			if (c == DELI)
			{
				break;
			} putc(c, FP);
		}
		break;
	default:
		fprintf(FP1, "Invalid command line options.\n");
		fprintf(FP1, "To see on-line manual, use the following commands:\n");
		fprintf(FP1, "\tFor the user's guide, type \"%s ", commandline_fsim);
		fprintf(FP1, "-h g\"\n");
		fprintf(FP1, "\tFor the netlist format, type \"%s ", commandline_fsim);
		fprintf(FP1, "-h n\"\n");
		fprintf(FP1, "\tFor the test file format, type \"%s ", commandline_fsim);
		fprintf(FP1, "-h t\"\n");
		fprintf(FP1, "\tFor the entire manual, type \"%s ", commandline_fsim);
		fprintf(FP1, "-h a\"\n");
	}

	if (fp != NULL && mode != 'd')
	{
		fclose(fp);
	}
}
