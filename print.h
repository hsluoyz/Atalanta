

void print_atpg_head(FILE *fp);
void print_atpg_result(FILE *fpFile, char *strCctPathFileName2, int iNoGate, int iNoPI, int iNoPO,
					   int iMaxLevelAdd2, int iMaxBackTrack1, int iMaxBackTrack2, int bPhase2, int iNoPatterns, int iNoPatternsAfterCompact,
					   int g_iNoFault, int iNoDetected, int iNoRedundant, int iNoBackTrack, int iNoShuffle, double lfInitTime,
					   double lfSimTime, double lfFanTime, double lfRunTime, char cMode, double lfMemSize);
void print_undetected_faults(FILE *fp, char symbol, char rfaultmode, int flag);
void print_sim_head(FILE *fp);
void print_sim_result(FILE *fp, char *name, int ng, int npi, int npo, int mlev, char *nametest, int nt, int nof, int nd, double inittime, double simtime, double runtime, char mode);
