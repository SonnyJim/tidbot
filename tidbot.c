//TODO:
//Change forget_tidbit() to use a memory buffer rather than a temp file
//FIXME:
//"WPC" matches both "WPC" and "WPC repair"
//ignore is buggy, the will match There
//ignore tell you, as "tell you what" matches
//cfg file selection is broken
#include "tidbot.h"
#include "cfg.h"

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

void recall_tidbit (const char *tidbit, const char *target)
{
    FILE *tidbit_file;
    char lineread[2048], reply[2048], msg[2048], tidbit_tmp[255];
    int i, tidbit_count = 0;

    tidbit_file = fopen (DEFAULT_TIDBIT_FILE, "r");
    if (tidbit_file == NULL)
    {
        fprintf (stderr, "Error opening %s for reading\n", DEFAULT_TIDBIT_FILE);
        return;
    }

    memset (msg, 0, strlen(msg));

    //Copy tidbit to temp var
    strcpy (tidbit_tmp, tidbit);
    //Add on the terminator, otherwise we match things we don't want
    strcat (tidbit_tmp, "|");
    
    //Scan file for tidbit
    while (fgets (lineread, 2048, tidbit_file) != NULL)
    {
        //Found the tidbit
        if (strncasecmp (lineread, tidbit_tmp, strlen(tidbit_tmp)) == 0)
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
                        strcat (msg, " |*or*| ");
                        strncat (msg, reply, strlen(reply) - 1);
                        msg[strlen(msg)] = '\0';
                    }
                    memset (reply, 0, strlen(msg));
                }
            }
        }
    }
    irc_cmd_msg (session, target, msg);
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

void forget_tidbit (const char *tidbit, const char *target)
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
        if (strncasecmp (lineread, tidbit, strlen(tidbit)) != 0)
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
        irc_cmd_msg (session, target, reply);
        memset (reply, 0, sizeof(reply));
    }
}

void store_tell (const char *origin, const char *target, const char *msg)
{
    FILE *tell_file;
    char tell_line[2048];

    tell_file = fopen (DEFAULT_TELL_FILE, "a+");
    if (tell_file == NULL)
    {
        fprintf (stderr, "Error opening %s for reading\n", DEFAULT_TELL_FILE);
        return;
    }

    strcpy (tell_line, target);
    strcat (tell_line, "|");
    strcat (tell_line, origin);
    strcat (tell_line, "|");
    strcat (tell_line, msg);
    strcat (tell_line, "\n");

   // printf ("tell_line: %s", tell_line);
    fputs (tell_line, tell_file);
    fclose (tell_file);
}

void tell_user (const char *tidbit, const char *origin, const char *channel)
{
    int i, j;
    char target[32];
    char msg[1024];
  
    memset (target, 0, 32);
    memset (msg, 0, 1024);

    //Find the target nick
    j = 0;
    for (i = strlen (MAGIC_TELL); i < strlen (tidbit); i++)
    {
        if (tidbit[i] == ' ')
            break;
        target[j] = tidbit[i];
        j++;
    }
    
    //Copy the message
    strcpy (msg, tidbit + (strlen (MAGIC_TELL) + strlen (target) + 1));

    //Store the message for later
    store_tell (origin, target, msg);
    if (channel == NULL)
        irc_cmd_msg (session, origin, "Ok, I'll tell them the next time I see them");
    else
        irc_cmd_msg (session, irc_cfg.channel, "Ok, I'll tell them the next time I see them");
}

void check_tell_file (const char *target)
{
    FILE *tell_file, *temp_file;
    char lineread[2048], reply[2048];
    int found_tell = 0;
    const char delimiter[] = "|";

    memset (lineread, 0, 2048);
    memset (reply, 0, 2048);
    //Open file read-only, check for username
    //If found, re-read the file to temporary file, give the user the message and remove the line
    //We do this so we don't hammer the temp file on every message
    tell_file = fopen (DEFAULT_TELL_FILE, "r");
    if (tell_file == NULL)
    {
        fprintf (stderr, "Error opening %s for reading\n", DEFAULT_TELL_FILE);
        return;
    }

    while (fgets (lineread, 2048, tell_file) != NULL)
    {
        if (strncasecmp (lineread, target, strlen(target)) == 0)
            found_tell = 1;
    }

    fclose (tell_file);
    if (!found_tell)
        return;

    tell_file = fopen (DEFAULT_TELL_FILE, "r");
    temp_file = fopen ("tmp.txt", "w+");
    if (tell_file == NULL)
    {
        fprintf (stderr, "Error opening %s for reading\n", DEFAULT_TELL_FILE);
        return;
    }

    //Copy all lines except tidbit line to temp file
    while (fgets (lineread, 2048, tell_file) != NULL)
    {
        if (strncasecmp (lineread, target, strlen(target)) != 0)
            fputs (lineread, temp_file);
        else
        {
            //Build reply
            strcpy (reply, strtok (lineread, delimiter));
            strcat (reply, ": ");
            strcat (reply, strtok (NULL, delimiter));
            strcat (reply, " wants you to know; ");
            strcat (reply, strtok (NULL, delimiter));

            irc_cmd_msg (session, irc_cfg.channel, reply);
        }

    }
    
    //Close and reopen files r/w
    fclose (temp_file);
    fclose (tell_file);
    tell_file = fopen (DEFAULT_TELL_FILE, "w+");
    temp_file = fopen ("tmp.txt", "r");
    
    //Copy temp_file to tidbit_file
    while (fgets (lineread, 2048, temp_file) != NULL)
    {
        fputs (lineread, tell_file);
    }
    fclose (temp_file);
    fclose (tell_file);
}

void check_tidbit (const char **params, const char *target, const char *channel)
{
   
    char *ptr, tidbit[2048], bittid[2048];
    int pos1, pos2, i;
   
    //Clear vars
    memset (tidbit, 0, 2048);
    memset (bittid, 0, 2048);
 
    if (strncasecmp (params[1], MAGIC_TELL, strlen(MAGIC_TELL)) == 0)
    {
        tell_user (params[1], target, channel);
        return;
    }

    if (strncasecmp (params[1], MAGIC_WHEREIS, strlen(MAGIC_WHEREIS)) == 0)
    {
        if (channel == NULL)
            irc_cmd_msg (session, target, "I only respond to !whereis in channel");
        else
        {
          //Get the WHOIS information for username
            strcpy (bittid, params[1]);
            strtok (bittid, " ");
            strcpy (tidbit, strtok (NULL, " "));
            irc_cmd_whois (session, tidbit);
            return;
        }
    }

    if (strncasecmp (params[1], MAGIC_IPDB, strlen(MAGIC_IPDB)) == 0)
    {
        strcpy (tidbit, MAGIC_IPDB_URL);
        strcpy (bittid, params[1] + strlen(MAGIC_IPDB) + 1);

        //Change spaces to %20
        pos2 = 0;
        for (pos1 = 0; pos1 < strlen (bittid); pos1++)
        {
            if (bittid[pos1] == ' ')
            {
                strcat (tidbit, "%20");
                pos2 = pos2 + 2;
            }
            else
                tidbit[pos2 + strlen(MAGIC_IPDB_URL)] = bittid[pos1];
            pos2++;
        }
    
        if (channel == NULL)
            irc_cmd_msg (session, target, tidbit);
        else
            irc_cmd_msg (session, channel, tidbit);
        return;
    }

    if (strlen (params[1]) == strlen (MAGIC_HELP)
            && strcasecmp (params[1], MAGIC_HELP) == 0
            //Don't respond to help in channel, only privmsg
            && channel == NULL)
    {
        //Print help text
        irc_cmd_msg (session, target, "How to use tidbot:");
        irc_cmd_msg (session, target, "Typing 'foo is bar' will make tidbot respond to foo? with the answer 'bar'");
        irc_cmd_msg (session, target, "(tidbot can remember multiple definitions for foo)");
        irc_cmd_msg (session, target, "'!forget foo' will make tidbot forget *all* definitions for foo");
        irc_cmd_msg (session, target, "'!tell foo message' will make tidbot tell the user foo message next time they are around ");
        irc_cmd_msg (session, target, "'!whereis user' will use my crappy database to see where the user lives ");
        irc_cmd_msg (session, target, "'!ipdb foo' will provide the link for ipdb foo ");
        return;
    }

    //Check to see if we are being asked a question
    if (params[1][strlen(params[1]) - 1] == '?')
    {
        //Copy question
        strncpy (tidbit, params[1], strlen (params[1]) - 1);
        //Ignore tidbits of less than 2 chars or more than 32
        if (strlen(tidbit) < 2 || strlen(tidbit) > MAX_TIDBIT_LENGTH)
             return;

        
        //Recall
        if (channel != NULL)
            recall_tidbit (tidbit, irc_cfg.channel);
        else
            recall_tidbit (tidbit, target);

        return;
    } 
    
    //Check for "is" in string
    ptr = strstr (params[1], MAGIC_IS);
   
    //If found, store
    if (ptr != NULL)
    {
        //Length of tidbit
        pos1 = ptr - params[1];
        if (pos1 > MAX_TIDBIT_LENGTH)
        {
            if (verbose)
                fprintf (stderr, "tidbit %s\n too long, ignoring\n", params[1]);
            return;
        }

        //position of bittid
        pos2 = pos1 + strlen(MAGIC_IS);

        //Copy vars
        strncpy (tidbit, params[1], pos1);
        strcpy (bittid, params[1] + pos2);

        //Ignore if less than 2 chars
        if (strlen(tidbit)  < 2|| strlen (bittid) < 2)
            return;
        //Check to see if the tidbit has "," in it, if so we can most likely ignore it
        for (i = 0; i < strlen (tidbit); i++)
        {
            if (tidbit[i] == ',')
                return;
        }
        //Store
        store_tidbit (tidbit, bittid);
    }

    //Check to see if we are being asked to forget something
    if (strncasecmp (params[1], MAGIC_FORGET, strlen(MAGIC_FORGET)) == 0)
    {
        strcpy (tidbit, params[1] + strlen(MAGIC_FORGET) + 1);
        //Check to see which tidbit we are being asked to forget
        if (channel != NULL)
            target = irc_cfg.channel;
        forget_tidbit (tidbit, target);
    }
}

void event_channel (irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count)
{
    if (strncasecmp (params[1], MAGIC_HTTP, strlen (MAGIC_HTTP)) == 0)
    {
        fprintf (stdout, "Saw URL %s\n", params[1]);
        char *string;
        string = get_title (params[1]);
        if (string != NULL)
            irc_cmd_msg (session, irc_cfg.channel, string);
        else
            fprintf (stderr, "Error fetching title from %s\n", params[1]);
    }
    else
        //if (strlen (params[1]) < MAX_TIDBIT_LENGTH)
    {
        check_tell_file (origin);
        check_tidbit (params, origin, irc_cfg.channel);
    }
}

void event_privmsg (irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count)
{
    if (strlen (origin) < MAX_TIDBIT_LENGTH)
    {
        check_tidbit (params, origin, NULL);
    }
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
