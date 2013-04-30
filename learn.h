typedef int status;
typedef struct GATE *GATEPTR;
typedef	int level;

status leval(register GATEPTR gate);
bool impval(int maxdpi, bool backward, int last);
void learn(int nog, int maxdpi);
void learn_node(int maxdpi, int node, level val);
void store_learn(GATEPTR gut, level val);
status imply_learn(register GATEPTR gut, register level val);
status imply_learn1(register GATEPTR gut, register level val);