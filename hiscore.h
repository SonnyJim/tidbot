void hiscore_add_nick (const char *nick);
void hiscore_initialise_nicks (const char *nicks);
void hiscore_init (void);
void hiscore_add_score (const char *nick, long score);
void hiscore_print_scores (const char *target, const char *channel);
void hiscore_load (const char *target);
void hiscore_save (const char *target);

