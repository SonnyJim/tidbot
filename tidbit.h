void recall_tidbit (const char *tidbit, const char *target);
void store_tidbit (const char *tidbit, const char *bittid);
void forget_tidbit (const char *tidbit, const char *target);
void check_tidbit (const char **params, const char *target, const char *channel);

char* random_machine (const char *target, const char *channel);
void flip (const char *channel);
void flip_add (const char *params);
void lock (const char *channel);
void multiball (const char *channel);
void skillshot (const char *channel);

void choose (const char *params, const char *channel);
