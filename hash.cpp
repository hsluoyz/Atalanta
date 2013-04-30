
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

/*------------------------------------------------------------------
	filename hash.c

	contains hash fuctions needed for a symbol table

	keyvalue() returns key-value of a given symbol.
	InitHash() initializes hash arrays into NULL.
	FindHash() finds a symbol for a given hash table.
	InsertHash() adds a symbol to the given hash table.
	Find_and_Insert_Hash() searches and inserts 
	a symbol to the hash table.
	CheckHash() checks a given hash entry is right or not.
	ReHash() rehashes the given entry if it is not correct.
--------------------------------------------------------------------*/
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
/*#ifdef WIN32*/
	#include <string.h> //for windows
// #else
// 	#include <strings.h> //for linux
// #endif

#include "error.h"
#include "hash.h"

#include "parameter.h" 
#include "define.h"    
#include "macro.h"

#define EDIGIT 4
#define BASIS ('Z'-'A'+19)

#define NOT_ALLOCATED (-1)	/* index of non-allocated gate */
#define is_white_space(c) (c==' ' || c=='\n'|| c=='\t')
#define is_delimiter(c) (c=='=' || c==',' || c=='(' || c==')')
#define is_valid(c) ((c>='0' && c<='9') || \
			 (c>='A' && c<='Z') || \
			 (c>='a' && c<='z') || \
			 (c=='_') || \
			 (c=='.'))

/*------keyvalue---------------------------------------------------
	computes the key value of a given string.
-------------------------------------------------------------------*/
int keyvalue(char s[])
{
	register char c;
	register int i, j, val = 0;
	int multi = 1;

	i = 0;
	while ((c = s[i]) != '\0')
	{
		if (c >= '0' && c <= '9')
		{
			j = c - '0' + 1;
		}
		else if (c >= 'a' && c <= 'z')
		{
			j = c - 'a' + 11;
		}
		else if (c >= 'A' && c <= 'Z')
		{
			j = c - 'A' + 11;
		}
		else if (c == '_')
		{
			j = 'Z' - 'A' + 12;
		}
		else if (c == '#')
		{
			j = 'Z' - 'A' + 13;
		}
		else if (c == '@')
		{
			j = 'Z' - 'A' + 14;
		}
		else if (c == '$')
		{
			j = 'Z' - 'A' + 15;
		}
		else if (c == '/')
		{
			j = 'Z' - 'A' + 16;
		}
		else if (c == '<' || c == '>')
		{
			j = 'Z' - 'A' + 17;
		}
		else if (c == '[' || c == ']')
		{
			j = 'Z' - 'A' + 18;
		}
		else
		{
			j = 'Z' - 'A' + 18;
		}

		val += j * multi;
		i++;
		if (i mod EDIGIT == 0)
		{
			multi = 1;
		}
		else
		{
			multi *= BASIS;
		}
	}

	return(val);
}

/*------InitHash--------------------------------------------------*/
void InitHash(HASHPTR pArrSymbolTable[], register int iSize)
{
	while (--iSize >= 0)
		pArrSymbolTable[iSize] = NULL;
}

/*------FindHash----------------------------------------------------
	search the given hash table to find a given string.
	inputs: symbol; string to be examined
		index; index number of the gate array
	outputs: returns the hash pointer. If not found, returns NULL.
	remark: if key==0, computes keyvalue, else uses given key value
------------------------------------------------------------------*/
HASHPTR FindHash(HASHPTR pArrSymbolTable[], int iSize, char symbol[], int key)
{
	struct HASH *hp;
	int h;

	/* symbol coding */
	if (key == 0)
	{
		key = keyvalue(symbol);
	}

	/* hash function: h(val) = val mod size */
	h = key mod iSize;

	/* check whether the symbol is hashed */
	hp = pArrSymbolTable[h];
	while (hp != NULL)
	{
		if (key == hp->key)
		{
			if (strcmp(hp->symbol, symbol) == 0)
			{
				break;
			}
		}
		hp = hp->next;
	}

	return(hp);
}

/*------astrcpy: allocate and copy a string -----------------*/
char * astrcpy(char *d, char *s)
{
	int length;

	length = strlen(s);
	if ((d = MALLOC(char, length + 1)) != NULL)
	{
		strcpy(d, s);
	}
	return(d);
}

/*------ hashalloc: allocates and initializes a type HASH ------*/
HASHPTR hashalloc()
{
	HASHPTR hash;

	if ((hash = MALLOC(HASHTYPE, 1)) != NULL)
	{
		hash->key = 0;
		/*  	hash->index=0; */
		hash->gate = NULL;
		hash->symbol = NULL;
		hash->next = NULL;
	}

	return(hash);
}

/*------InsertHash-------------------------------------------------
	adds a symbol with index at the symbol table.
	A new symbol is added at the top of the linked list.
	inputs: symbol; string to be examined
		index; index number
	outputs: symbol table pointer of the symbol
------------------------------------------------------------------*/
HASHPTR InsertHash(HASHPTR table[], int size, char symbol[], int key)
{
	register struct HASH *hp;
	register int h;

	/* symbol coding */
	if (key == 0)
	{
		key = keyvalue(symbol);
	}

	h = key mod size;

	if ((hp = hashalloc()) == NULL)
	{
		printFatalError(MEMORYERROR);
	}
	hp->key = key;
	hp->next = table[h];
	if ((hp->symbol = astrcpy(hp->symbol, symbol)) == NULL)
	{
		printFatalError(MEMORYERROR);
	}
	table[h] = hp;

	return(hp);
}

/*------Find_and_Insert_Hash--------------------------------------*/
HASHPTR Find_and_Insert_Hash(HASHPTR table[], int size, char symbol[], int key)
{
	struct HASH *hp;

	if (key == 0)
	{
		key = keyvalue(symbol);
	}

	if ((hp = FindHash(table, size, symbol, key)) == NULL)
	{
		hp = InsertHash(table, size, symbol, key);
	}
	return(hp);
}

