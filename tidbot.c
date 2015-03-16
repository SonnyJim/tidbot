//TODO:
//Change forget_tidbit() to use a memory buffer rather than a temp file
//FIXME:
//"WPC" matches both "WPC" and "WPC repair"
//ignore is buggy, the will match There
//ignore tell you, as "tell you what" matches
//cfg file selection is broken
#include "tidbot.h"
#include "cfg.h"
#include "tell.h"
#include "tidbit.h"
#include "hiscore.h"
#include "ctcptime.h"

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

void whereis_user (const char *target, const char *hostname)
{
    int error;
    struct addrinfo *result;
    struct in_addr address;
    char whereis_reply[255];
    
    //Convert to an IP address
    error = getaddrinfo (hostname, NULL, NULL, &result);
    if (error != 0)
    {
        fprintf (stderr, "Error trying to resolve %s\n", hostname);
        return;
    }

    address.s_addr = ((struct sockaddr_in *)(result->ai_addr))->sin_addr.s_addr;
    freeaddrinfo (result);

    geoip_find (inet_ntoa (address));

    strcpy (whereis_reply, target);
    strcat (whereis_reply, " is in ");
    strcat (whereis_reply, location);
    irc_cmd_msg (session, irc_cfg.channel, whereis_reply);
}

void event_join (irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
    hiscore_add_nick (origin);
}

void event_numeric (irc_session_t *session, unsigned int event, const char *origin, const char **params, unsigned int count)
{
    if (event == LIBIRC_RFC_RPL_WHOISUSER)
    {
        //params[1] == nick associated with WHOIS
        //params[3] == hostname returned from WHOIS
        whereis_user (params[1], params[3]);
    }

	if (!verbose)
		return;

	switch (event)
	{
		case LIBIRC_RFC_RPL_NAMREPLY:
		      fprintf (stdout, "IRC: User list: %s\n", params[3]);
              hiscore_initialise_nicks (params[3]);
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

void event_channel (irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count)
{
    hiscore_add_score (origin, strlen (params[1]));
    check_tell_file (origin);
    check_tidbit (params, origin, irc_cfg.channel);
}

void event_privmsg (irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count)
{
    if (strlen (origin) < MAX_TIDBIT_LENGTH)
    {
        check_tidbit (params, origin, NULL);
    }
}

void event_ctcp_rep (irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count)
{
    if (verbose)
        fprintf (stdout, "CTCP REPLY origin %s reply: %s\n", origin, params[0]);
    ctcp_time_rep (params[0]);
}

void print_usage (void)
{
	fprintf (stdout, "tidbot\n");
	fprintf (stdout, "Options:\n");
	fprintf (stdout, "-c		Specify config file location, default %s\n", DEFAULT_CFG_FILE);
	fprintf (stdout, "-h		This help text\n");
}

int main (int argc, char **argv)
{
	irc_callbacks_t callbacks;
	int c;
	int ret;

    verbose = 0;
    use_default_cfg = 1;
    cfg_url_title = 1;

	// Read command line options
	while ((c = getopt (argc, argv, "c:vhu")) != -1)
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
            case 'u':
                cfg_url_title = 0;
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
    else if (ret != 0)
    {
        fprintf (stderr, "cfg_load returned %i\n", ret);
        return 1;
    }

	fprintf (stdout, "IRC: Bot initilising\n");

	memset (&callbacks, 0, sizeof(callbacks));

	callbacks.event_connect = event_connect;
	callbacks.event_numeric = event_numeric;
	callbacks.event_privmsg = event_privmsg;
	callbacks.event_channel = event_channel;
    callbacks.event_join = event_join;
    callbacks.event_ctcp_rep = event_ctcp_rep;
	
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
    
    //Initialise hiscore table
    hiscore_init ();
	//Enter main loop
	if (irc_run (session))
	{
		fprintf (stderr, "IRC: ERROR %s\n", irc_strerror(irc_errno (session)));
		return 1;
	}
	return 0;
}
