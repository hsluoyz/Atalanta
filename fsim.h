typedef struct GATE *GATEPTR;
typedef	int level;
typedef struct FAULT *FAULTPTR;

void initFaultSim_HOPE();
GATEPTR injectFault(GATEPTR gate, int ftype, int fline, register int bit);
void Faulty_Gate_Eval(GATEPTR gut, level *Val);
void FaultSim(int start, int stop, register int Gid);
GATEPTR SSimToDominator(register GATEPTR gut, register GATEPTR dom, register int Gid);
GATEPTR checkStemOutputNeedUpdate(register GATEPTR gut, register int Gid);
GATEPTR SSimToStem(register GATEPTR gut, register int Gid);
int DropDetectedFaults();
void StoreFaultyStatus();
void RemoveFault();
void RestoreCircuits();
GATEPTR CheckSingleEvent(FAULTPTR f, GATEPTR gut, register int Gid);
FAULTPTR selectNextFaults(FAULTPTR cf);
FAULTPTR SelectOneFault(FAULTPTR cf);
int FaultSim_HOPE();