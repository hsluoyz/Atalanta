typedef	int fault_type;
typedef struct FAULT *FAULTPTR;
typedef struct GATE *GATEPTR;

int initStemFaultsAndFaultList(int iNoGate, int iStem, GATEPTR *pStem);
void addFaultToGate(FAULTPTR f);
int restoreUndetectedState_FSIM(int nof);
int deleteRedundantFaults(int nog);
char getfaultsymbol(FILE *file, char s[]);
int readFaults(FILE *file, int nog, int no_stem, GATEPTR *stem);
void initFaultList();
void insertFaultForGate(GATEPTR gut, int line, fault_type type);
void insertFaultsForGateByLine(GATEPTR gut, int line);
void initFFRFaults(GATEPTR gut);
void initFaultsForPOByDFS(GATEPTR parent, GATEPTR child);
void initFaultList_HOPE();
void readFaults_HOPE(FILE *file);
int restoreUndetectedState_HOPE(int nof);