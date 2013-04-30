
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
#include "stdafx.h"
#include "macro.h"


// int tgen()
// {
//    reset(phase2);
//    fantime=0;
//    fault_selection_mode=DEFAULTMODE;
//    lastfault=nof;
//    all_one=~(ALL1<<1);
// 
//    while(!done) {
// 
//  	 if(maxbacktrack==0) break;
// 
//  	 /* select any undetected and untried fault */
//  	 pcurrentfault=NULL;
//  	 switch(fault_selection_mode) {
// 	 case CHECKPOINTMODE:
// 		while(--lastfault>=0)
// 		   if(is_undetected(faultlist[lastfault])) {
// 		  pcurrentfault=faultlist[lastfault];
// 		  gut=pcurrentfault->gate;
// 		  if(pcurrentfault->line!=OUTFAULT)
// 			 gut=gut->inList[pcurrentfault->line];
// 		  if(is_checkpoint(gut)) break;
// 		  pcurrentfault=NULL;
// 		   }
// 		if(pcurrentfault==NULL) {
// 		fault_selection_mode=DEFAULTMODE;
// 		lastfault=nof;
// 		}
// 		break;
// 
//  		default:
// 		while(--lastfault>=0)
//  			  if(is_undetected(faultlist[lastfault])) {
// 			  pcurrentfault=faultlist[lastfault];
// 			  break;
// 		   }
// 		if(pcurrentfault==NULL) set(done);
//  	 }
// 
//  	 if(pcurrentfault==NULL) continue;
//  	 gut=pcurrentfault->gate;
// 
//  	 gettime(&minutes,&seconds,&runtime2);
//  	 fantime-=runtime2;
// 
//  	 /* test pattern generation using fan */
//  	 state =
// 	 fan(nog,LEVEL,nopi,nopo,pcurrentfault,maxbacktrack,&nbacktrack);
//  	 tbacktrack+=nbacktrack;
// 
//  	 gettime(&minutes,&seconds,&runtime2);
//  	 fantime+=runtime2;
// 
//  	 if(state==TEST_FOUND) {
// 	 /* fault is detected, delete the detected fault from fault list */
// 	 pcurrentfault->detected=PROCESSED;
// 
// /*	 pcurrentfault->detected=DETECTED;
// 	 gut->nfault--;
// 	 ndetect++;
// */
// 	 /* assign random zero and ones to the unassigned bits */
//  		ntest++;
// 	 for(j=0;j<nopi;j++) {
// 		switch(net[j]->output) {
// 		   case ZERO: resetbit(test_vectors[npacket][j],nbit);
// 					  net[j]->output1=ALL0;
// 			  break;
// 		   case ONE : setbit(test_vectors[npacket][j],nbit);
// 					  net[j]->output1=ALL1;
// 			  break;
// 		   default  : ran=(int)random()&01;
// 			  if(ran != 0)
// 				 setbit(test_vectors[npacket][j],nbit);
// 			  else resetbit(test_vectors[npacket][j],nbit);
// 			  net[j]->output1=ran;
// 		}
// 		reset(net[j]->changed);
// 		reset(net[j]->freach);
// 		net[j]->cobserve=ALL0;
// 		net[j]->output0=net[j]->output1;
// 	 }
// 	 if(++nbit==maxbits) {nbit=0; npacket++;}
// 	 clear(stack);
// 
// 	 /* fault simulation */
// 	 fault_profile[0]=0;
// 	 ndetect +=
// 		Fault0_Simulation(nog,LEVEL,nopi,nopo,nstem,stem,1,fault_profile);
// 	 if(pcurrentfault->detected!=DETECTED) {
// 		printf("Error in test generation\n");
// 	 }
//  	 }
// 
//  	 else if(state==NO_TEST) {
// 	 /* redundant faults */
// 	 pcurrentfault->detected=REDUNDANT;
// 	 nredundant++;
// 	 delete_fault(pcurrentfault);
// 	 if(--gut->nfault==0) set(update_flag);
//  	 }
// 
//  	 else {
// 	 /* over backtracking */
// 	 noverbacktrack++;
// 	 pcurrentfault->detected=PROCESSED;
//  	 }
//    }

