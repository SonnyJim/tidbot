#include "tidbot.h"
#include "cfg.h"

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
