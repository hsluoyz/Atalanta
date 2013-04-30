typedef struct HASH *HASHPTR;

int keyvalue(char s[]);
void InitHash(HASHPTR pArrSymbolTable[], register int size);
HASHPTR FindHash(HASHPTR pArrSymbolTable[], int iSize, char symbol[], int key);
char *astrcpy(char *d, char *s);
HASHPTR hashalloc();
HASHPTR InsertHash(HASHPTR table[], int size, char symbol[], int key);
HASHPTR Find_and_Insert_Hash(HASHPTR table[], int size, char symbol[], int key);