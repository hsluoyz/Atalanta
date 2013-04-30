
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
 
		atalanta: version 1.1   	 H. K. Lee, 10/5/1992
		atalanta: version 2.0   	 H. K. Lee, 6/30/1997
 
***********************************************************************/

/*----------------------------------------------------------------------
	atpg.h
	Define global data structure for atalanta.
-----------------------------------------------------------------------*/
#ifndef 		__ATALANTA_ATPG_H__
#define 		__ATALANTA_ATPG_H__

#define		INCLUDE_HOPE	'y'
#define		ATALANTA	'y'

/* This flag should be commented out during a normal compilation. */
/* #define		DEBUG		'y' */

#ifdef INCLUDE_HOPE
#define		output		FV[0]
#define		output1		FV[1]
#endif

#define 		MAXGTYPE		100	/* logic gates */
#define 		MAXLEVEL		4   	/* logic levels for hope */
#define 		ATALEVEL		5   	/* logic levels for atalanta */

#define 		AND 			0
#define 		NAND			1
#define 		OR  			2
#define 		NOR 			3
#define 		PI  			4 ///////////
#define 		XOR 			5
#define 		XNOR			6
#define 		DFF 			7
#define 		DUMMY   		8
#define 		BUFF			9
#define 		NOT 			10
#define 		PO  			20 ///////////
#define 		FAULTY  		50

#define 		ZERO			0
#define 		ONE 			1
#define 		X   			2
#define			D				3
#define			DBAR			4

#define 		Z   			3
#define 		ALL0			0
#define 		ALL1			(~0)
#define 		MASK0   		(~(ALL1<<1))

#define 		SA0 			0   			/* stuck-at fault */
#define 		SA1 			1
#define 		SAX 			2
#define 		OUTFAULT		(-1)

/* circuit netlist format */
#define	ISCAS85		'5'
#define	ISCAS89		'9'

/* status */
#define 		FALSE   		0   			/* boolean */
#define 		TRUE			1

#define 		GOOD			1   			/* status */

#define 		UNDETECTED  	0
#define 		DETECTED		1
#define 		XDETECTED   	2   			/* Potentially detected */
#define 		REDUNDANT   	3
#define 	PROCESSED	4

#define 	EMPTY		(-1)
#define 	FORWARD		0
#define 	BACKWARD	1
#define 	CONFLICT	2
#define 	TEST_FOUND	0
#define 	OVER_BACKTRACK	1
#define 	NO_TEST		2

#define 	mod		%

typedef		int logic;
typedef		int level;
typedef		int fault_type;
//typedef		int	boolean;
typedef		int status;
typedef		int line_type;

/* typedef enum {LFREE,HEAD,BOUND} line_type; */
#define		LFREE		0
#define		HEAD		1
#define		BOUND		2

typedef struct HASH *HASHPTR;
typedef struct GATE *GATEPTR;
typedef struct FAULT *FAULTPTR;
typedef struct link *LINKPTR;
typedef struct EVENT *EVENTPTR;

typedef struct HASH
{
	int key;					 /* variable of the given symbol */
	GATEPTR gate;
	struct HASH *next;  		 /* next string */
	char *symbol;   			 /* symbolic name */
} HASHTYPE;

/* gate data structure */
typedef struct GATE
{
	int index;
	logic type;		/* type of gate */
	short inCount;		/* number of fanins */
	struct GATE **inList;	/* fan-in list */
	short outCount;		/* number of fan-outs */
	struct GATE **outList;	/* fan-out list */
	short dpi, dpo;		/* depth from PIs & POs */
	status changed;		/* flag for event queue operations */
	HASHPTR hash;		/* pointer to the symbol table */
	int freach1;		/* is reached from faulty gate */
	int freach;		/* is reached from faulty gate */
	status xpath;		/* x path exists or not */
	line_type ltype;	/* head line, free line, bound line */
	int numzero, numone;	/* expected number of zeros and ones */
	LINKPTR u_path;		/* unique path */
	LINKPTR u_path1;	/* unique path */
	int cont0, cont1;	/* 0 and 1 controllability */
	int fosIndex;		/* fanout stemIndex indication */
	int nfault;
	level cobserve, observe;	/* cumulated and local detectabilities */
	FAULTPTR pfault, *dfault;/* fault list */
#ifdef LEARNFLG
	struct LEARN *plearn;
#endif
	GATEPTR next;
#ifdef ISCAS85_NETLIST_MODE
	int gid;		/* gate identification */
#endif
#ifdef INCLUDE_HOPE
	level GV[2];			/* Good value; (V0,V1) */
	level FV[2], Gid;   	 /* Faulty Value; (V0,V1,Gid) */
	int stemIndex;   			/* indication of the stems */
	status sense;   		/* simulated or not --- version 3 */
	level SGV;  			/* Good Value for SPF */
#else
	level output;		//FV[0] /* output value for fan */
	level output1;		//FV[1] /* output value for fsim */
#endif
} GATETYPE;

/* fault list data structure */
typedef struct FAULT
{
	struct GATE *gate;	/* faulty gate */
	int line;		/* faulty line, -1 if output fault */
	fault_type type;	/* fault type */
	int detected;	/* detected or not */
	level observe;		/* detectability */
	struct FAULT *next;	/* pointer to the next fault */
	struct FAULT *previous;	/* pointer to the previous fault */
#ifdef INCLUDE_HOPE
	int npot;		/* # potentially detected --- HOPE */
	EVENTPTR event;		/* event list --- HOPE */
#endif
} FAULTTYPE;

typedef struct link
{
	GATEPTR ngate;
	struct link *next;
} LINKTYPE;

typedef struct LEARN
{
	/* for learning */
	struct LEARN *next;
	int node;
	char sval, tval;
} LEARNTYPE;

typedef struct EDEN
{
	/* constant values */
	struct EDEN *next;
	int node;
	char val;
} EDENTYPE;


typedef struct STACK
{
	/* LIFO stack */
	int last;
	struct GATE **list;
} STACKTYPE, *STACKPTR;


/* Decision tree data structure */
typedef struct TREENODE
{
	GATEPTR gate;		/* pointer to the gate */
	bool flag;		/* flag for backtracking */
	int pstack;		/* pointer for implication */
} TREETYPE, *TREEPTR;

struct ROOTTREE
{
	/* tree for backtracking */
	int last;
	TREEPTR list;
};

struct FLIST
{
	/* fault list */
	int last;
	FAULTPTR *list;
};

#ifdef INCLUDE_HOPE
typedef struct EVENT
{
	/* for HOPE */
	int node;   			/* event node */
	level value;			/* v0 & v1 */
	struct EVENT *next; 			/* pointer to next event */
} EVENTTYPE;

typedef struct STEM *STEMPTR;
typedef struct STEM
{
	int gate;
	int dominatorIndex;
	int checkup;
	FAULTPTR fault[3];
	short flag[3];
} STEMTYPE;
#endif

/* mask bits for parallel pattern simulation */
#define BITSIZE	32		/* word size */
#define ALL0	0		/* (0,0,...,0) */
#define ALL1	(~0)		/* (1,1,...,1) */
#define MASK1	(ALL1<<1)	/* (1,1,...,1,0) */

#ifdef WIN32
	typedef void* caddr_t; //for windows
	#define random() rand()
	#define srandom(arg) srand(arg)
#endif // WIN32

#endif /* __ATALANTA_ATPG_H__ */
