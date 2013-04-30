
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
	macro.h
	Define macro functions for atalanta.
-----------------------------------------------------------------------*/

#ifndef 		__ATALANTA_MACRO_H__
#define 		__ATALANTA_MACRO_H__

/* character substitution */
#define 		EOS 	'\0'	/* End of string */
#define 		CR  	'\n'	/* carriage return */
#define 		TAB 	'\t'	/* tab */

/* assignment & comparison */
#define set(var) (var=TRUE)
#define reset(var) (var=FALSE)
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X,Y) ((X) > (Y) ? (Y) : (X))
//#define max(a,b) ((a) > (b) ? (a) : (b))
//#define min(a,b) ((a) > (b) ? (b) : (a))
#define ABS(X) ( X >= 0 ? X : (-1)*X )
#define A_NOT(var)     ((var==ONE) ? ZERO :  \
			   (var==ZERO) ? ONE :  \
			   (var==D) ? DBAR : \
			   (var==DBAR) ? D : X)

/* macros for LIFO stack operation */
#define EMPTY (-1)
#define push(s,ele) s.list[++(s.last)]=ele
#define pop(s) s.list[(s.last)--]
#define clear(s) s.last=EMPTY
#define is_empty(s) (s.last<0)
#define ptr_is_empty(s) (s->last<0)
#define delete_last(s) --(s.last)
#define delete(s,i) s.list[i]=s.list[(s.last)--]
#define copy(s1,s2,i) s2.last=s1.last;\
			  for(i=s1.last;i>=0;i--) s2.list[i]=s1.list[i]

/* macros for event queue operation */
#define pushevent(gate) \
	if (!gate->changed) \
	{ \
		push(g_pEventListStack[gate->dpi], gate); \
		set(gate->changed); \
	}
#define popevent(depth) pop(g_pEventListStack[depth])
#define clearevent(depth) clear(g_pEventListStack[depth])
#define schedule_output(pGate) \
	for (mac_i = 0; mac_i < pGate->outCount; mac_i++) \
	   pushevent(pGate->outList[mac_i])
#define schedule_input(gate,i) pushevent(gate->inList[i]) \
				 schedule_output(gate->inList[i])

#define pushGate(pGate) push(g_pEventListStack[pGate->dpi], pGate)
#define pushGateOutputs2(pGate, i, pOutGate) \
	for (i = 0; i < pGate->outCount; i++) \
	{ \
		pOutGate = pGate->outList[i]; \
		if (!pOutGate->changed) \
	   	{ \
			set(pOutGate->changed); \
			pushGate(pOutGate); \
		} \
	}


#define is_head(line) (line->ltype==HEAD)
#define is_free(line) (line->ltype==LFREE)
#define is_bound(line) (line->ltype==BOUND)
#define is_fanout(line) (line->outCount>1)
#define is_reachable_from_fault(line) (line->freach)
#define is_unspecified(line) (line->output==X)
#define is_justified(line) (line->changed)
#define is_unjustified(line) ((!line->changed)&&(line->output!=X))
#define is_D_propagated(line) (line->output==D || line->output==DBAR)

#define is_undetected(fault) (fault->detected==UNDETECTED)
#define is_detected(fault) (fault->detected==DETECTED)
#define is_redundant(fault) (fault->detected==REDUNDANT)

/* macros for tree operation */
#define current_node g_tree.list[g_tree.last]
#define is_flagged(node) node.flag

/* macros for multiple backtrace */
#define setline(line,n0,n1) line->numzero=n0; \
	line->numone=n1
#define resetline(line) setline(line,0,0)
#define is_conflict(line) (line->numzero>0 && line->numone>0)


/********** Macros for memory menagement **********/
//extern char *malloc(), *calloc(), *realloc(); for windows
extern char *calloc(), * realloc();

#define MALLOC(type,number) \
(type *)malloc((unsigned)(sizeof(type)*(number)))
#define CALLOC(type,number) (type *)calloc((unsigned)(sizeof(type)),(unsigned)(number))
#define REALLOC(pointer,type,number) \
pointer = (type *)realloc((char *)pointer,((unsigned)sizeof(type),(unsigned)(number)))
#define MFREE(pointer) \
{ if((pointer)!=NULL) free((char *)pointer); }
#define FREE(pointer) \
{ if((pointer)!=NULL) free((char *)pointer); }
#define ALLOCATE(pointer,type,number) \
if((pointer=MALLOC(type,number))==NULL) printFatalError(MEMORYERROR)
#define CALLOCATE(pointer,type,number) \
if((pointer=CALLOC(type,number))==NULL) printFatalError(MEMORYERROR)


/************ Macros for hope ***************************/
/* macros handling the event list */
#define initEventList(h,t) \
{ ALLOCATE(t,EVENTTYPE,1); t->next=NULL; h=t; }
#define reseteventlist(h,t,var1) \
{ while(h!=t) { var1=h; h=h->next; FREE(var1); }
#define remove(elist) \
{ \
	g_tailEvent->next=elist; \
	elist=NULL; \
	while(g_tailEvent->next!=NULL) \
		g_tailEvent=g_tailEvent->next; \
}
#define create(e) \
{ if(g_headEvent==g_tailEvent) {ALLOCATE(e,EVENTTYPE,1); } else { e=g_headEvent; g_headEvent=g_headEvent->next; }}

#define setBit(word, p) (word | BITMASK[p])
#define resetBit(word, p) (word & (~BITMASK[p]))
#define checkBitIs0(word, p) ((word & BITMASK[p]) == ALL0)
#define setPairTo0(V0, V1) V0=ALL1; V1=ALL0
#define setPairTo1(V0, V1) V0=ALL0; V1=ALL1
#define setPairToX(V0, V1) V0=V1=ALL0
#define setPairToZ(V0, V1) V0=V1=ALL1
#define checkPair(V0, V1) ((V0==ALL1 && V1==ALL0) ? ZERO : \
			   (V0==ALL0 && V1==ALL1) ? ONE : \
			   (V0==ALL0 && V1==ALL0) ? X : Z)

/* Gate Evaluation */
#define FEVAL(gut,Val,j,v,temp,GGID) \
	switch(gut->type) { \
	case NOT: \
		   if(gut->inList[0]->Gid==GGID) { \
			  Val[0]=gut->inList[0]->FV[1]; \
			  Val[1]=gut->inList[0]->FV[0]; \
		   } else { \
			  Val[0]=gut->inList[0]->GV[1]; \
			  Val[1]=gut->inList[0]->GV[0]; \
		   } \
		   break; \
	case AND: \
	   if(gut->inList[0]->Gid==GGID) { \
		  twoBitsCopy(Val,gut->inList[0]->FV); \
	   } else { twoBitsCopy(Val,gut->inList[0]->GV); } \
	   for(j=1;j<gut->inCount;j++) \
		  if(gut->inList[j]->Gid==GGID) { \
			 Val[0] |= gut->inList[j]->FV[0]; \
			 Val[1] &= gut->inList[j]->FV[1]; \
		  } else { \
			 Val[0] |= gut->inList[j]->GV[0]; \
			 Val[1] &= gut->inList[j]->GV[1]; \
		  } \
	   break; \
	case NAND: \
	   if(gut->inList[0]->Gid==GGID) { \
		  twoBitsCopy(Val,gut->inList[0]->FV); \
	   } else { twoBitsCopy(Val,gut->inList[0]->GV); } \
	   for(j=1;j<gut->inCount;j++) \
		  if(gut->inList[j]->Gid==GGID) { \
			 Val[0] |= gut->inList[j]->FV[0]; \
			 Val[1] &= gut->inList[j]->FV[1]; \
		  } else { \
			 Val[0] |= gut->inList[j]->GV[0]; \
			 Val[1] &= gut->inList[j]->GV[1]; \
		  } \
	   v=Val[0]; Val[0]=Val[1]; Val[1]=v; \
	   break; \
	case OR: \
	   if(gut->inList[0]->Gid==GGID) { \
		  twoBitsCopy(Val,gut->inList[0]->FV); \
	   } else { twoBitsCopy(Val,gut->inList[0]->GV); } \
	   for(j=1;j<gut->inCount;j++) \
		  if(gut->inList[j]->Gid==GGID) { \
			 Val[0] &= gut->inList[j]->FV[0]; \
			 Val[1] |= gut->inList[j]->FV[1]; \
		  } else { \
			 Val[0] &= gut->inList[j]->GV[0]; \
			 Val[1] |= gut->inList[j]->GV[1]; \
		  } \
	   break; \
	case NOR: \
	   if(gut->inList[0]->Gid==GGID) { \
		  twoBitsCopy(Val,gut->inList[0]->FV); \
	   } else { twoBitsCopy(Val,gut->inList[0]->GV); } \
	   for(j=1;j<gut->inCount;j++) \
		  if(gut->inList[j]->Gid==GGID) { \
			 Val[0] &= gut->inList[j]->FV[0]; \
			 Val[1] |= gut->inList[j]->FV[1]; \
		  } else { \
			 Val[0] &= gut->inList[j]->GV[0]; \
			 Val[1] |= gut->inList[j]->GV[1]; \
		  } \
	   v=Val[0]; Val[0]=Val[1]; Val[1]=v; \
	   break; \
	case XOR: \
	   if(gut->inList[0]->Gid==GGID) { \
		  twoBitsCopy(Val,gut->inList[0]->FV); \
	   } else { twoBitsCopy(Val,gut->inList[0]->GV); } \
	   for(j=1;j<gut->inCount;j++) { \
		  temp=gut->inList[j]; \
		  v=Val[0]; \
		  if(temp->Gid==GGID) { \
			 Val[0] = (v&temp->FV[0])|(Val[1]&temp->FV[1]); \
			 Val[1] = (Val[1]&temp->FV[0])|(v&temp->FV[1]); \
		  } else { \
			 Val[0] = (v&temp->GV[0])|(Val[1]&temp->GV[1]); \
			 Val[1] = (Val[1]&temp->GV[0])|(v&temp->GV[1]); \
		  } \
	   } break; \
	case XNOR: \
	   if(gut->inList[0]->Gid==GGID) { \
		  twoBitsCopy(Val,gut->inList[0]->FV); \
	   } else { twoBitsCopy(Val,gut->inList[0]->GV); } \
	   for(j=1;j<gut->inCount;j++) { \
		  temp=gut->inList[j]; \
		  v=Val[0]; \
		  if(temp->Gid==GGID) { \
			 Val[0] = (v&temp->FV[0])|(Val[1]&temp->FV[1]); \
			 Val[1] = (Val[1]&temp->FV[0])|(v&temp->FV[1]); \
		  } else { \
			 Val[0] = (v&temp->GV[0])|(Val[1]&temp->GV[1]); \
			 Val[1] = (Val[1]&temp->GV[0])|(v&temp->GV[1]); \
		  } \
	   } \
	   v=Val[0]; Val[0]=Val[1]; Val[1]=v; \
	   break; \
	case DUMMY: case PO: case BUFF: \
	   if(gut->inList[0]->Gid==GGID) { \
		  twoBitsCopy(Val,gut->inList[0]->FV); \
	   } else { twoBitsCopy(Val,gut->inList[0]->GV); } \
	   break; \
		default: \
	   Faulty_Gate_Eval(gut,Val); \
	   break; \
	}

#endif /* __ATALANTA_MACRO_H__ */
