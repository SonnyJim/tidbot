//TODO:
//Multiple definitions
//ignore list of words (this, why, how, when, which etc)
//Help message
//Change forget_tidbit() to use a memory buffer rather than a temp file
//FIXME:
//"WPC" matches both "WPC" and "WPC repair"
//ignore is buggy, the will match There

#include "libircclient/libircclient.h"
#include "libircclient/libirc_rfcnumeric.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <getopt.h>

#define DEFAULT_CFG_FILE  	"/.tidbot.cfg"
#define DEFAULT_IRC_SERVER  	"irc.choopa.net"
#define DEFAULT_IRC_PORT	"6667"
#define DEFAULT_IRC_CHANNEL  	"#pinball"
#define DEFAULT_IRC_NICK	"tidbot"
#define DEFAULT_IRC_USERNAME 	"qircbot"
#define DEFAULT_IRC_REALNAME 	"qircbot"
#define DEFAULT_TIDBIT_FILE "tidbits.txt"
#define DEFAULT_IGNORE_FILE "ignore.txt"

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

//Sent on successful connection to server, useful for NickServ
static void send_server_connect_msg (void)
{
	//Check to see if we have commands to run
	if (strlen (irc_cfg.server_connect_msg) != 0)
	{
		//Make sure a nick was specified
		if (strlen (irc_cfg.server_connect_nick) == 0)
		{
			fprintf (stderr, "IRC: server_connect_msg specified but not server_connect_nick\n");
			return;
		}
		if (atoi (irc_cfg.server_connect_delay) > 0)
		{
			if (verbose)
				fprintf (stdout, "IRC: Waiting %i seconds before sending command\n", atoi(irc_cfg.server_connect_delay));
			sleep (atoi (irc_cfg.server_connect_delay));
		}
		if (verbose)
			fprintf (stdout, "IRC: Sending server_connect_msg\n");
		irc_cmd_msg (session, irc_cfg.server_connect_nick, irc_cfg.server_connect_msg);
	}
}

//Sent on successful connection to channel, useful for ChanServ
static void send_channel_connect_msg (void)
{
	//Check to see if we have commands to run
	if (strlen (irc_cfg.channel_connect_msg) != 0)
	{
		//Make sure a nick was specified
		if (strlen (irc_cfg.channel_connect_nick) == 0)
		{
			fprintf (stderr, "IRC: channel_connect_msg specified but not channel_connect_nick\n");
			return;
		}
		if (atoi (irc_cfg.channel_connect_delay) > 0)
		{
			if (verbose)
				fprintf (stdout, "IRC: Waiting %i seconds before sending command\n", atoi(irc_cfg.channel_connect_delay));
			sleep (atoi (irc_cfg.channel_connect_delay));
		}
		if (verbose)
			fprintf (stdout, "IRC: Sending channel_connect_msg\n");
		irc_cmd_msg (session, irc_cfg.channel_connect_nick, irc_cfg.channel_connect_msg);
	}

}

//Called when successfully connected to a server
void event_connect (irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count)
{
	fprintf (stdout, "IRC: Successfully connected to server %s\n", irc_cfg.server);

	send_server_connect_msg ();
	
	if (verbose)
		fprintf (stdout, "IRC: Attempting to join %s\n", irc_cfg.channel);

	if (irc_cmd_join (session, irc_cfg.channel, NULL))
	{
		fprintf (stderr, "IRC: Error joining channel %s\n", irc_cfg.channel);
		return;
	}
	
	fprintf (stdout, "IRC: Connected to %s\n", irc_cfg.channel);

	send_channel_connect_msg ();
}


void event_numeric (irc_session_t *session, unsigned int event, const char *origin, const char **params, unsigned int count)
{
	if (!verbose)
		return;

	switch (event)
	{
		case LIBIRC_RFC_RPL_NAMREPLY:
		      fprintf (stdout, "IRC: User list: %s\n", params[3]);
		      break;
		case LIBIRC_RFC_RPL_WELCOME:
		      fprintf (stdout, "IRC: %s\n", params[1]);
		      break;
		case LIBIRC_RFC_RPL_YOURHOST:
		      fprintf (stdout, "IRC: %s\n", params[1]);
		case LIBIRC_RFC_RPL_ENDOFNAMES:
		      fprintf (stdout, "IRC: End of user list\n");
		      break;
		case LIBIRC_RFC_RPL_MOTD:
		      fprintf (stdout, "%s\n", params[1]);
		      break;
		default:
		      fprintf (stdout, "IRC: event_numeric %u\n", event);
		      break;
	}
}

void recall_tidbit (const char *tidbit)
{
    FILE *tidbit_file;
    char lineread[2048], reply[2048], msg[2048];
    int i, tidbit_count = 0;

    tidbit_file = fopen (DEFAULT_TIDBIT_FILE, "r");
    if (tidbit_file == NULL)
    {
        fprintf (stderr, "Error opening %s for reading\n", DEFAULT_TIDBIT_FILE);
        return;
    }

    memset (msg, 0, strlen(msg));

    //Scan file for tidbit
    while (fgets (lineread, 2048, tidbit_file) != NULL)
    {
        //Found the tidbit
        if (strncasecmp (lineread, tidbit, strlen(tidbit)) == 0)
        {
            //Find the | separator in the line
            for (i = 0; i < strlen (lineread); i++)
            {
                if (lineread[i] == '|')
                {
                    tidbit_count++;
                    //Strip out and form reply ( -1 to remove \n)
                    strncpy (reply, lineread + i + 1, strlen (lineread) - i);
                    if (tidbit_count == 1)
                        strncpy (msg, reply, strlen(reply) - 1);
                    else
                    {
                        strcat (msg, " or ");
                        strncat (msg, reply, strlen(reply) - 1);
                    }
                    memset (reply, 0, strlen(msg));
                }
            }
        }
    }
    irc_cmd_msg (session, irc_cfg.channel, msg);
    fclose (tidbit_file);
}

void store_tidbit (const char *tidbit, const char *bittid)
{
    char tidbit_store[2048], lineread[2048];
    FILE *tidbit_file, *ignore_file;

    //Check for tidbit in ignore file
    ignore_file = fopen (DEFAULT_IGNORE_FILE, "r");
    while (fgets (lineread, 2048, ignore_file) != NULL)
    {
        if (strncasecmp (lineread, tidbit, strlen(tidbit)) == 0)
        {
            fprintf (stdout, "Found %s in ignore file, ignoring\n", tidbit);
            fclose (ignore_file);
            return;
        }
    }
    fclose (ignore_file);
   
    tidbit_file = fopen (DEFAULT_TIDBIT_FILE, "a");

    if (tidbit_file == NULL)
    {
        fprintf (stderr, "Error opening %s for appending\n", DEFAULT_TIDBIT_FILE);
        return;
    }

    //Construct the tidbit
    strcpy (tidbit_store, tidbit);
    strcat (tidbit_store, "|");
    strcat (tidbit_store, bittid);
    strcat (tidbit_store, "\n");

    //Store it to file
    fputs (tidbit_store, tidbit_file);
    fclose (tidbit_file);
}

void forget_tidbit (const char *tidbit)
{
    FILE *tidbit_file;
    FILE *temp_file;
    char lineread[2048], reply[2048];
    int found_tidbit = 0;

    tidbit_file = fopen (DEFAULT_TIDBIT_FILE, "r");
    temp_file = fopen ("tmp.txt", "w+");
    if (tidbit_file == NULL)
    {
        fprintf (stderr, "Error opening %s for reading\n", DEFAULT_TIDBIT_FILE);
        return;
    }

    //Copy all lines except tidbit line to temp file
    while (fgets (lineread, 2048, tidbit_file) != NULL)
    {
        if (strncmp (lineread, tidbit, strlen(tidbit)) != 0)
            fputs (lineread, temp_file);
        else
            found_tidbit = 1;

    }
    
    //Close and reopen files r/w
    fclose (temp_file);
    fclose (tidbit_file);
    tidbit_file = fopen (DEFAULT_TIDBIT_FILE, "w+");
    temp_file = fopen ("tmp.txt", "r");
    
    //Copy temp_file to tidbit_file
    while (fgets (lineread, 2048, temp_file) != NULL)
    {
        fputs (lineread, tidbit_file);
    }
    fclose (temp_file);
    fclose (tidbit_file);
    
    if (found_tidbit == 1)
    {
        memset (reply, 0, sizeof(reply));
        strcpy (reply, "Ok, forgetting ");
        strcat (reply, tidbit);
        irc_cmd_msg (session, irc_cfg.channel, reply);
    }
}

void check_tidbit (const char **params)
{
    #define MAGIC_IS " is "
    #define MAGIC_FORGET "forget"
    
    char *ptr, tidbit[2048], bittid[2048];
    int pos1, pos2;
   
    //Clear vars
    memset (tidbit, 0, 2048);
    memset (bittid, 0, 2048);
    
    //Check to see if we are being asked a question
    if (params[1][strlen(params[1]) - 1] == '?')
    {
        //Copy question
        strncpy (tidbit, params[1], strlen (params[1]) - 1);
        //Ignore tidbits of less than 2 chars
        if (strlen(tidbit) < 2)
             return;
        //Recall
        recall_tidbit (tidbit);
        return;
    } 
    
    //Check for "is" in string
    ptr = strstr (params[1], MAGIC_IS);
   
    //If found, store
    if (ptr != NULL)
    {
        //Length of tidbit
        pos1 = ptr - params[1];
        //position of bittid
        pos2 = pos1 + strlen(MAGIC_IS);

        //Copy vars
        strncpy (tidbit, params[1], pos1);
        strcpy (bittid, params[1] + pos2);

        //Ignore if less than 2 chars
        if (strlen(tidbit)  < 2|| strlen (bittid) < 2)
            return;
        //Store
        store_tidbit (tidbit, bittid);
    }

    //Check to see if we are being asked to forget something
    if (strncasecmp (params[1], MAGIC_FORGET, strlen(MAGIC_FORGET)) == 0)
    {
        strcpy (tidbit, params[1] + strlen(MAGIC_FORGET) + 1);
        forget_tidbit (tidbit);
    }
}

void event_channel (irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count)
{
    check_tidbit (params);
}

void event_privmsg (irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count)
{
	//printf ("'%s' said to me (%s): %s\n", origin ? origin : "someone", params[0], params[1] );
    check_tidbit (params);
}

void print_usage (void)
{
	fprintf (stdout, "tidbot\n");
	fprintf (stdout, "Options:\n");
	fprintf (stdout, "-c		Specify config file location, default %s\n", DEFAULT_CFG_FILE);
	fprintf (stdout, "-h		This help text\n");
}

static int cfg_load (void)
{
	FILE *in_file;
	char cfg_buff[64];
	int i;
	

	//If no config file was specified on the command line, look in the users home directory for one
	if (use_default_cfg)
	{
		strcpy (cfg_buff, getenv("HOME"));
		strcat (cfg_buff, irc_cfg.cfg_file);
		strcpy (irc_cfg.cfg_file, cfg_buff);
	}
	
	if (verbose)
	{
		fprintf (stdout, "IRC: Attempting to Load config file %s\n", irc_cfg.cfg_file);
	}

	in_file = fopen (irc_cfg.cfg_file, "r");

	if (in_file == NULL && !use_default_cfg)
		return 2;
	else if (in_file == NULL && use_default_cfg)
		return 1;

	//Copy a line from cfg file into cfg_buff
	while (fgets (cfg_buff, 64, in_file) != NULL)
	{
		//Cycle through the different options
		for (i = 0; i < NUM_CFG_OPTS; i++)
		{
			//Look for a match and make sure there's a = after it
			if (strncmp (cfg_buff, cfg_options[i], strlen(cfg_options[i])) == 0 &&
					strncmp (cfg_buff + strlen (cfg_options[i]), "=", 1) == 0)
			{
				//Copy to variable
				strcpy (cfg_vars[i], cfg_buff + strlen (cfg_options[i]) + 1);
				//Strip newline
				strtok (cfg_vars[i], "\n");
				//Check if empty var
				if (strcmp (cfg_vars[i], "\n") == 0)
				{
					fprintf (stderr, "IRC: Empty cfg option %s\n", cfg_options[i]);
					fclose (in_file);
					return 1;
				}
			}
		}
	}
	fclose (in_file);
	return 0;

}

int main (int argc, char **argv)
{
	irc_callbacks_t callbacks;
	int c;
	int ret;

	// Read command line options
	while ((c = getopt (argc, argv, "c:vh")) != -1)
	{
		switch (c)
		{
			default:
			case 'h':
				print_usage ();
				return 0;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'c':
				strcpy (irc_cfg.cfg_file, optarg);
				use_default_cfg = 0;
				break;
			case '?':
				if (optopt == 'c')
					fprintf (stderr, "Option -c requires an argument\n");
				else
					fprintf (stderr, "Unrecognised option %c\n", optopt);
				fprintf (stderr, "Error reading commmand line options\n");
				return 1;
				break;
		}
	}
	
	ret = cfg_load ();
	if (ret > 1)
	{
		fprintf (stderr, "Error reading configuration file %s\n", irc_cfg.cfg_file);
		return 1;
	}
	else if (ret == 1)
	{
		fprintf (stdout, "No configuration file found, using defaults\n");
	}

	if (verbose)
	{
		fprintf (stdout, "Configuration options:\n");
		fprintf (stdout, "server = %s\n", irc_cfg.server);
		fprintf (stdout, "port = %s\n", irc_cfg.port);
		fprintf (stdout, "channel = %s\n", irc_cfg.channel);
		fprintf (stdout, "nick = %s\n", irc_cfg.nick);
		fprintf (stdout, "username = %s\n", irc_cfg.username);
		fprintf (stdout, "realname = %s\n", irc_cfg.realname);
		fprintf (stdout, "server_connect_msg = %s\n", irc_cfg.server_connect_msg);
		fprintf (stdout, "server_connect_nick = %s\n", irc_cfg.server_connect_nick);
		fprintf (stdout, "server_connect_delay = %s\n", irc_cfg.server_connect_delay);
		fprintf (stdout, "channel_connect_msg = %s\n", irc_cfg.channel_connect_msg);
		fprintf (stdout, "channel_connect_nick = %s\n", irc_cfg.channel_connect_nick);
		fprintf (stdout, "channel_connect_delay = %s\n\n", irc_cfg.channel_connect_delay);
	}
	fprintf (stdout, "IRC: Bot initilising\n");

	memset (&callbacks, 0, sizeof(callbacks));

	callbacks.event_connect = event_connect;
	callbacks.event_numeric = event_numeric;
	callbacks.event_privmsg = event_privmsg;
	callbacks.event_channel = event_channel;
	
	session = irc_create_session(&callbacks);

	if (!session)
	{
		fprintf (stderr, "IRC: Error setting up session\n");
		return 1;
	}
	
	irc_option_set(session, LIBIRC_OPTION_STRIPNICKS);

	if (verbose)
	{
		fprintf (stdout, "IRC: Attempting to connect to server %s:%i channel %s with nick %s\n", 
				irc_cfg.server, atoi(irc_cfg.port), irc_cfg.channel, irc_cfg.nick);
	}

	if (irc_connect (session, irc_cfg.server, atoi(irc_cfg.port), 0, irc_cfg.nick, irc_cfg.username, irc_cfg.realname ))
	{
		fprintf (stderr, "IRC: ERROR %s\n", irc_strerror(irc_errno (session)));
	}

	//Enter main loop
	if (irc_run (session))
	{
		fprintf (stderr, "IRC: ERROR %s\n", irc_strerror(irc_errno (session)));
		return 1;
	}
	return 0;
}
