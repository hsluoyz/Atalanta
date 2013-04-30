typedef struct GATE *GATEPTR;

bool circin(int *nog, int *nopi, int *nopout);
int setCctParameters(int iNoGate, int iNoPI);
bool allocateStacks(int iNoGate);
int initFanoutGateDominators(int nog, int maxdpi);
void initUniquePath(int nog, int maxdpi);
void initStemGatesAndFOS(int iNoGate, GATEPTR *pStem, int iStem);
void getTime(double *usertime, double *systemtime, double *total);
double getMemory();