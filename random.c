#include "tidbot.h"
#include "cfg.h"
#include "time.h"

#define MACHINE_FILE "machine_list.txt"

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
           
            sprintf (reply, "%s, by %s, made: %s, players: %s, type: %s, theme: %s\n", machine, manufacturer, year, players, type, theme);
            
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
