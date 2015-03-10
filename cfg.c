#include "cfg.h"

const char *cfg_options[] = {
	"server", "port", "channel", "nick", "username", "realname", 
	"server_connect_msg", "server_connect_nick", "server_connect_delay",
	"channel_connect_msg", "channel_connect_nick", "channel_connect_delay"
};

cfg irc_cfg  = {
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

char *cfg_vars[] = {
	irc_cfg.server, irc_cfg.port, irc_cfg.channel, irc_cfg.nick, irc_cfg.username, irc_cfg.realname,
		irc_cfg.server_connect_msg, irc_cfg.server_connect_nick, irc_cfg.server_connect_delay,
		irc_cfg.channel_connect_msg, irc_cfg.channel_connect_nick, irc_cfg.channel_connect_delay
};

int cfg_load (void)
{
	FILE *in_file;
	char cfg_buff[128];
	int i;
    
    verbose = 0;
    use_default_cfg = 1;
	

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


