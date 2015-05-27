#include "tidbot.h"
#include "cfg.h"
#include <time.h>

#define FLIP_FILE   "flip.txt"
void flip (const char *channel)
{
    FILE *flip_file;
    char lineread[1024] = ""; 
    int flip_count = 0;
    int random_pick = 0;
    
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

void flip_add (const char *params)
{
    FILE *flip_file;
    char fliptext[1024] = "";

    strcpy (fliptext, params + strlen (MAGIC_FLIP_ADD) + 1);

    flip_file = fopen (FLIP_FILE, "a");
    
    if (flip_file == NULL)
    {
        fprintf (stderr, "Error opening %s for writing\n", FLIP_FILE);
        return;
    }
       
    fputs (fliptext, flip_file);
    fputc ('\n', flip_file);
    fclose (flip_file);
}

