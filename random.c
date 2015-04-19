#include "tidbot.h"
#include "cfg.h"
#include "time.h"

#define MACHINE_FILE "machine_list.txt"
#define FLIP_FILE   "flip.txt"

int balls_locked = 0;
char* random_machine (const char *target, const char *channel)
{
    FILE *machine_file;
    char lineread[1024] = ""; 
    char reply[1024];
    char *machine;
    char *manufacturer;
    char *year;
    char *players;
    char *type;
    char *theme;

    int machine_count = 0;
    int random_pick = 0;
    
    srand(time(NULL));

    machine_file = fopen (MACHINE_FILE, "r");
    if (machine_file == NULL)
    {
        fprintf (stderr, "Error opening %s for reading\n", MACHINE_FILE);
        return NULL;
    }
    
    while (fgets (lineread, 1024, machine_file) != NULL)
        machine_count++;

    rewind (machine_file);
    
    random_pick = rand () % machine_count;
   
    machine_count = 0;
    strcpy (reply, "");
    //Read throught the manual file line by line
    while (fgets (lineread, 1024, machine_file) != NULL)
    {
        if (machine_count == random_pick)
        {
            machine = strtok (lineread, "|");
            manufacturer = strtok (NULL, "|");
            year = strtok (NULL, "|");
            players = strtok (NULL, "|");
            type = strtok (NULL, "|");
            theme = strtok (NULL, "|");
           
            sprintf (reply, "%s by %s.  Made %s.  Players: %s, type: %s.  Theme: %s\n", machine, manufacturer, year, players, type, theme);
            
            if  (strcmp (target, "tidbot") == 0)
                return machine;
            
            if (channel == NULL)
                irc_cmd_msg (session, target, reply);
            else
                irc_cmd_msg (session, irc_cfg.channel, reply);
            break;
        }
        machine_count++;
    }
    fclose (machine_file);
    return NULL;
}

void flip (const char *channel)
{
    FILE *flip_file;
    char lineread[1024] = ""; 
    int flip_count = 0;
    int random_pick = 0;
    
    srand(time(NULL));

    flip_file = fopen (FLIP_FILE, "r");
    if (flip_file == NULL)
    {
        fprintf (stderr, "Error opening %s for reading\n", FLIP_FILE);
        return;
    }
    
    while (fgets (lineread, 1024, flip_file) != NULL)
        flip_count++;

    rewind (flip_file);
    
    random_pick = rand () % flip_count;
   
    flip_count = 0;
    while (fgets (lineread, 1024, flip_file) != NULL)
    {
        if (flip_count == random_pick)
        {
            irc_cmd_msg (session, channel, lineread);
            break;
        }
        flip_count++;
    }
    fclose (flip_file);
    return;
}

void multiball (const char *channel)
{
    if (balls_locked == 3)
    {
        irc_cmd_msg (session, channel, "MULTIBALL!");
        balls_locked = 0;
    }
    else
        irc_cmd_msg (session, channel, "You need to !lock some more balls");

    return;
}

void lock (const char *channel)
{
    if (balls_locked < 3)
        balls_locked++;

    switch (balls_locked)
    {
        case 1:
                irc_cmd_msg (session, channel, "Ball 1 locked");
                break;
        case 2:
                irc_cmd_msg (session, channel, "Ball 2 locked");
                break;
        case 3:
                irc_cmd_msg (session, channel, "Ball 3 locked, shoot !multiball to start");
                break;
        default:
                irc_cmd_msg (session, channel, "Ball locked");
                break;
    }
    return;
}

void skillshot (const char *channel)
{

    srand(time(NULL));
    switch (rand () % 3)
    {
        case 0:
                irc_cmd_msg (session, channel, "Skillshot missed!");
                break;
        case 1:
                irc_cmd_msg (session, channel, "Skillshot hit!");
                break;
        default:
        case 2:
                irc_cmd_msg (session, channel, "Super duper mega-skill shot");
                break;
    }
}
