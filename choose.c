#include "tidbot.h"
#include <time.h>

#define DELIM " or "

void choose (const char *params, const char *channel)
{
    int num_choices = 1;
    int choice = 0;
    int i, len;
    const char *p = NULL;
    char selected[255] = "";
    int delim_locations[16];
   
    p = strstr (params, DELIM);
    if (p == NULL)
    {
        irc_cmd_msg (session, channel, "Didn't see any choices to make");
        return;
    }
    
    delim_locations[0] = strlen(MAGIC_CHOOSE) + 1;
    delim_locations[num_choices] = p - params;

    //Find how many choices we have
    while (p)
    {
        delim_locations[num_choices] = p - params;
        if (num_choices < 16)
            num_choices++;
        p = strstr (p + 1, DELIM);
    }

    delim_locations[num_choices] = delim_locations[num_choices - 1] + strlen (params + delim_locations[num_choices -1]);
    for (i = 0; i <= num_choices; i++)
    {
        fprintf (stdout, "delim location: %i\n", delim_locations[i]);
    }
    //Pick a choice
    choice = rand() % (num_choices);
    choice++;
    
    len = delim_locations[choice] - delim_locations[choice -1];
    
    //First choice won't have " or " in front of it
    if (choice == 1)
    {
        strncpy (selected, params + delim_locations[choice - 1], len);
    }
    else
        strncpy (selected, params + delim_locations[choice - 1] + strlen (DELIM), len - strlen (DELIM));

    selected[len] = '\0';
    
    irc_cmd_msg (session, channel, selected);
}

