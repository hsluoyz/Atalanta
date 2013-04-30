typedef struct GATE *GATEPTR;

void allocateStack1And2();
int computeLevels();
void allocateEventListStacks();
void updateGateIndexByLevel();
int updateGateIndexByLevel(GATEPTR *net, int n, int nopi, int nopo, int noff, GATEPTR *stack);
int addPOGates();
void addSpareGates();