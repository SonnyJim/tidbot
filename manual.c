#include "tidbot.h"
#include "cfg.h"
void find_manual (const char *manual)
{
    FILE *manual_file;
    char lineread[1024] = ""; 
    char *reply;
    int found_manual = 0;

    manual_file = fopen (DEFAULT_MANUAL_FILE, "r");
    if (manual_file == NULL)
    {
        fprintf (stderr, "Error opening %s for reading\n", DEFAULT_MANUAL_FILE);
        return;
    }

    //Read throught the manual file line by line
    while (fgets (lineread, 1024, manual_file) != NULL)
    {   
        if (strncasecmp (lineread, manual, strlen(manual)) == 0)
        {
            found_manual = 1;
            strtok (lineread, "|");

            reply = strtok (NULL, "|");
            irc_cmd_msg (session, irc_cfg.channel, reply);

        }
    }
    
    fclose (manual_file);
    if (found_manual == 0)
    {
        strcpy (lineread, "Couldn't find a manual for ");
        strcat (lineread, manual);
        irc_cmd_msg (session, irc_cfg.channel, lineread);
    }
}

void add_manual (const char *ircline)
{
    FILE *manual_file; 
    char manual_line[1024] = "";
    char machine[12] = "";
    char url[1024] = "";
    char *token;


    strcpy (manual_line, ircline);
    token = strtok (manual_line, " ");
    token = strtok (NULL, " ");
    if (token != NULL)
        strcpy (machine, token);
    token = strtok (NULL, " ");
    if (token != NULL)
        strcpy (url, token);

    if ((strlen (url) < 1) || (strlen (machine) < 1))
    {
        fprintf (stderr, "Error adding manual, %s %s\n", machine, url);
        irc_cmd_msg (session, irc_cfg.channel, "Whoops, did you forget something?  !add_manual machine url");
        return;
    }

    manual_file = fopen (DEFAULT_MANUAL_FILE, "a");
    if (manual_file == NULL)
    {
        fprintf (stderr, "Error opening %s for appending\n", DEFAULT_MANUAL_FILE);
        return;
    }
    
    strcpy (manual_line, machine);
    strcat (manual_line, "|");
    strcat (manual_line, url);
    if (verbose)
        fprintf (stdout, "Adding manual: %s\n", manual_line);
    fputs (manual_line, manual_file);
    fputc ('\n', manual_file);
    fclose (manual_file);
    strcpy (manual_line, "Adding manual URL for machine ");
    strcat (manual_line, machine);
    irc_cmd_msg (session, irc_cfg.channel, manual_line);
}

void check_manual (const char *ircline)
{
    //Manual to look for
    char manual[12] = "";

    //No machine specified
    if (strlen (ircline) <= strlen (MAGIC_MANUAL))
    {
        irc_cmd_msg (session, irc_cfg.channel, "What manual do you want me to look for?");
        return;
    }

    strcpy (manual, ircline + (strlen (MAGIC_MANUAL) + 1));
    if (verbose)
        fprintf (stdout, "Looking for manual %s\n", manual);
    find_manual (manual);
}


