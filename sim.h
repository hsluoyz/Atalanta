#include "parameter.h"

typedef struct GATE *GATEPTR;
typedef int level;

int random_fsim(int iNoGate, int iNoPI, int iNoPO, int iMaxLevelAdd2, int iStem, GATEPTR *pStem, level *LFSR, int limit, int maxbit, int maxdetect, int *ntest, int *npacket, int *nbit, FILE *test);

int random_hope(int nopi, int nopo, level *LFSR, int limit, int maxbit, int maxdetect, int *ntest, int *npacket, int *nbit, FILE *test);

int random_sim(int nog, int nopi, int nopo, int LEVEL, int nstem, GATEPTR *stem, level *LFSR, int limit, int maxbit, int maxdetect, int *ntest, int *npacket, int *nbit, FILE *test);

int tgen_sim(int nog, int LEVEL, int nopi, int nopo, int nstem, GATEPTR *stem, int ntest, int profile[]);

void fill_patterns_fsim(char mode, int npacket, int nbit, int nopi);

void fill_patterns_hope(char mode, int npacket, int nbit, int nopi);

void fill_patterns(char mode, int npacket, int nbit, int nopi);

int testgen(int nog, int nopi, int nopo, int LEVEL, int maxbits, int nstem, GATEPTR *stem, int maxbacktrack, int phase, int *nredundant, int *noverbacktrack, int *nbacktrack, int *ntest, int *npacket, int *nbit, double *fantime, FILE *test);

void randomizePatterns_FSIM(level test_st[][MAXPI + 1], level test_vect[][MAXPI + 1], int pack, int no_bit, int no_pi);

int reverse_fsim(int nog, int nopi, int nopo, int LEVEL, int nstem, GATEPTR *stem, int nof, int *ndet, int npacket, int nbit, int maxbits, FILE *test);

int shuffle_fsim(int nog, int nopi, int nopo, int LEVEL, int nstem, GATEPTR *stem, int nof, int *nshuf, int *ndet, int npacket, int nbit, int maxbits, FILE *test);

void randomizePatterns_HOPE(level test_st0[][MAXPI + 1], level test_st1[][MAXPI + 1], level test_vec0[][MAXPI + 1], level test_vec1[][MAXPI + 1], int pack, int no_bit, int no_pi);

int reverse_hope(int nog, int nopi, int nopo, int nof, int *ndet, int npacket, int nbit, int maxbits, FILE *test);

int shuffle_hope(int nog, int nopi, int nopo, int nof, int *nshuf, int *ndet, int npacket, int nbit, int maxbits, FILE *test);

int compact_test(int nog, int nopi, int nopo, int LEVEL, int nstem, GATEPTR *stem, int nof, int *nshuf, int *ndet, int npacket, int nbit, int maxbits, FILE *test);

void Dprintio(FILE *test, int nopi, int nopo, int no);