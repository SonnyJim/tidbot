#include "tidbot.h"
#include "cfg.h"
#include "tell.h"
#include "manual.h"

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
    memset (reply, 0, strlen(reply));

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
        irc_cmd_msg (session, target, "'!manual foo' will show the link for the foo manual (if I have it) ");
        irc_cmd_msg (session, target, "'!add_manual foo url' will add the manual link for foo");
        return;
    }
    
    if (strncasecmp (params[1], MAGIC_MANUAL, strlen(MAGIC_MANUAL)) == 0)
    {
       check_manual (params[1]); 
       return;
    }

    if (strncasecmp (params[1], MAGIC_MANUAL_ADD, strlen(MAGIC_MANUAL_ADD)) == 0)
    {
       add_manual (params[1]); 
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