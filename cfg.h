#include "tidbot.h"
#define NUM_CFG_OPTS 12

typedef struct{
	char cfg_file[2048];
	char server[2048];
	char port[16];
	char channel[64];
	char nick[64];
	char username[16];
	char realname[16];
	char server_connect_msg[2048];
	char server_connect_nick[16];
	char server_connect_delay[6];
	char channel_connect_msg[2048];
	char channel_connect_nick[16];
	char channel_connect_delay[6];
} cfg ; 

cfg irc_cfg;

int verbose;
int use_default_cfg;

int cfg_load (void);

