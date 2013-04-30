typedef struct GATE *GATEPTR;
typedef struct FAULT *FAULTPTR;
typedef struct STACK *STACKPTR;
typedef	int level;
typedef	int status;
#define max(a,b)			(((a) > (b)) ? (a) : (b))


void initNetAndFreach(int nog, GATEPTR faulty_gate, int maxdpi);
int refFaultyGateOutput(FAULTPTR fault);
level getFaultyGateOutput(register GATEPTR g, FAULTPTR cf);
status eval(register GATEPTR gate, FAULTPTR cf);
bool implyForwardAndBackward(int maxdpi, bool backward, int last, FAULTPTR cf);
int unique_sensitize(register GATEPTR gate, GATEPTR faulty_gate);
int dynamic_unique_sensitize(GATEPTR *Dfront, int nod, int maxdpi, GATEPTR *dom_array, GATEPTR faulty_gate);
GATEPTR closest_po(STACKPTR objective, int *pclose);
GATEPTR select_hardest(STACKPTR objective, int *pclose);
status backtrace(status state);
void find_final_objective(bool *backtrace_flag, bool fault_propagated_to_po, int nog, GATEPTR *last_Dfrontier);
bool Xpath(register GATEPTR gate);
void update_Dfrontier();
void restore_faults(FAULTPTR fal);
void justify_free_lines(int npi, FAULTPTR of, FAULTPTR cf);
bool backtrack(GATEPTR faulty_gate, int *last, int nog);
void propFault2Headline(FAULTPTR cf);
status fan(int iNoGate, int iMaxLevelAdd2, int iNoPI, int iNoPO, FAULTPTR pCurrentFault, int iMaxBackTrack, int *piNoBackTrack);
status fan1(int nog, int maxdpi, int npi, int npo, FAULTPTR cf, int maxbacktrack, int *nbacktrack);