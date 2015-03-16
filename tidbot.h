#define DEFAULT_CFG_FILE  	"/.tidbot.cfg"
#define DEFAULT_IRC_SERVER  	"irc.choopa.net"
#define DEFAULT_IRC_PORT	"6667"
#define DEFAULT_IRC_CHANNEL  	"#pinball"
#define DEFAULT_IRC_NICK	"tidbot"
#define DEFAULT_IRC_USERNAME 	"tidbot"
#define DEFAULT_IRC_REALNAME 	"tidbot"
#define DEFAULT_TIDBIT_FILE "tidbits.txt"
#define DEFAULT_IGNORE_FILE "ignore.txt"
#define DEFAULT_TELL_FILE "tell.txt"
#define DEFAULT_MANUAL_FILE "manuals.txt"
#define DEFAULT_HISCORE_FILE "hiscore.bin"
#define DEFAULT_MAX_TIDBIT_LENGTH 32

#define MAGIC_IS " is "
#define MAGIC_FORGET "!forget"
#define MAGIC_HELP "help"
#define MAGIC_TELL "!tell "
#define MAGIC_WHEREIS "!whereis"
#define MAGIC_IPDB "!ipdb"
#define MAGIC_IPDB_URL "http://www.ipdb.org/search.pl?any="
#define MAGIC_MANUAL "!manual"
#define MAGIC_MANUAL_ADD "!add_manual"

#define MAGIC_HTTP "http"

#define MAGIC_8BALL "!8ball"

#define MAGIC_SCORES "!scores"
#define MAGIC_SCORES_LOAD "!scores_load"
#define MAGIC_SCORES_SAVE "!scores_save"
#define MAGIC_SCORES_INIT "!scores_init"

#define MAGIC_TIME "!time"

#define MAGIC_HANGMAN "!hangman"

#define MAX_TIDBIT_LENGTH 16

#include "libircclient/libircclient.h"
#include "libircclient/libirc_rfcnumeric.h"
#include <stdio.h>
#include <unistd.h>
//for strcasestr
#define _GNU_SOURCE
#include <string.h>
#include <strings.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

irc_session_t *session;

extern char location[255];
extern void geoip_find (const char *ipaddr);

//Curl get title
char* get_title (const char *url);
    
//cfg.c
