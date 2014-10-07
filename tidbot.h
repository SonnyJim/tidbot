#define DEFAULT_CFG_FILE  	"/.tidbot.cfg"
#define DEFAULT_IRC_SERVER  	"irc.choopa.net"
#define DEFAULT_IRC_PORT	"6667"
#define DEFAULT_IRC_CHANNEL  	"#pinball2"
#define DEFAULT_IRC_NICK	"tidbot"
#define DEFAULT_IRC_USERNAME 	"qircbot"
#define DEFAULT_IRC_REALNAME 	"qircbot"
#define DEFAULT_TIDBIT_FILE "tidbits.txt"
#define DEFAULT_IGNORE_FILE "ignore.txt"
#define DEFAULT_TELL_FILE "tell.txt"
#define DEFAULT_MAX_TIDBIT_LENGTH 32

#define MAGIC_IS " is "
#define MAGIC_FORGET "forget"
#define MAGIC_HELP "help"
#define MAGIC_TELL "tell "
#define MAGIC_WHEREIS "whereis"

struct cfg {
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
} irc_cfg = {
	DEFAULT_CFG_FILE,
	DEFAULT_IRC_SERVER,
	DEFAULT_IRC_PORT,
	DEFAULT_IRC_CHANNEL,
	DEFAULT_IRC_NICK,
	DEFAULT_IRC_USERNAME,
	DEFAULT_IRC_REALNAME,
	"",
	"",
	"",
	"",
	"",
	""
};

#define NUM_CFG_OPTS 12

const char *cfg_options[] = {
	"server", "port", "channel", "nick", "username", "realname", 
	"server_connect_msg", "server_connect_nick", "server_connect_delay",
	"channel_connect_msg", "channel_connect_nick", "channel_connect_delay"
};

char *cfg_vars[] = {
	irc_cfg.server, irc_cfg.port, irc_cfg.channel, irc_cfg.nick, irc_cfg.username, irc_cfg.realname,
		irc_cfg.server_connect_msg, irc_cfg.server_connect_nick, irc_cfg.server_connect_delay,
		irc_cfg.channel_connect_msg, irc_cfg.channel_connect_nick, irc_cfg.channel_connect_delay
};

int verbose = 0;
int use_default_cfg = 1;

irc_session_t *session;

extern char location[255];
extern void geoip_find (const char *ipaddr);
