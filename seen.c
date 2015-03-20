#include <ctype.h>
#include <time.h>
#include "tidbot.h"
#include "cfg.h"

char nick[32] = "";
void *seen_mem;
int seen_record_count;

struct seen_t {
    char nick[32];
    time_t time;
} seen_record;

#define CHUNKSIZE sizeof (struct seen_t)
#define SEENFILE "seen.bin"

void seen_save (void)
{
    FILE *seen_file;

    seen_file = fopen (SEENFILE, "wb");
    
    if (seen_file == NULL)
    {
        fprintf (stderr, "Error opening %s for writing\n", SEENFILE);
        return;
    }

    if (verbose)
        fprintf (stdout, "Saving seen data\n");
    fwrite (seen_mem, sizeof (struct seen_t), seen_record_count, seen_file);
    fclose (seen_file);
}

int seen_load (void)
{
    FILE *seen_file;
    int filesize = 0;

    if (verbose)
        fprintf (stdout, "Loading seen file from %s\n", SEENFILE);

    seen_file = fopen (SEENFILE, "r");

    if (seen_file == NULL)
    {
        fprintf (stderr, "Error opening %s for reading\n", SEENFILE);
        return 0;
    }
    
    //Get filesize
    fseek (seen_file, 0, SEEK_END);
    filesize = ftell (seen_file);
    fseek (seen_file, 0, SEEK_SET);

    seen_record_count = filesize / CHUNKSIZE;
    //Alloc memory and read file in
    seen_mem = malloc (filesize);
    fread (seen_mem, filesize, 1, seen_file);
    fclose (seen_file);
    if (verbose)
        fprintf (stdout, "seen: Loaded %i records\n", seen_record_count);
    return 1;
}

void seen_init (void)
{
    //Initialise seen memory
    seen_record_count = 1;
    seen_mem = malloc (sizeof (struct seen_t));

    if (seen_mem == NULL)
    {
        fprintf (stderr, "seen:  Unable to alloc seen_mem\n");
    }
}

//Add another record to seen_mem
static void seen_mem_add (void)
{
    seen_record_count++;
    seen_mem = realloc (seen_mem, sizeof (struct seen_t) * (seen_record_count));
}

static char * seen_strip_nick (const char *cmd, const char *params)
{
    int start, end, nicklen;

    memset (nick, 0, sizeof(char) * 32);
    //Strip off leading whitespace
    for (start = strlen (MAGIC_SEEN); start < strlen (params); start++)
    {
        if (!isspace (params[start]))
            break;
    }


    //Strip off trailing whitespace
    
    end = strlen (params) - 1;
    while (isspace (params[end]))
        end--;

    nicklen = end - start + 1;
    strncpy (nick, params + start, nicklen);
    
    return nick;
}

void seen_check (const char *params, const char *targetnick, const char *channel)
{
    int i;
    char nick[32] = "";
    char reply[1024] = "";
    char target[32] = "";
    time_t time_diff;
    struct tm *time_data;
    int d, h, m, s = 0;
    
        
    if (channel == NULL)
        strcpy (target, targetnick);
    else
        strcpy (target, channel);
    
    strcpy (nick, seen_strip_nick (MAGIC_SEEN, params)); 
    if (strlen (nick) == 0)
    {
        fprintf (stderr, "seen: empty nick\n");
        irc_cmd_msg (session, target, "Have I seen who? (hint: !seen username)");
        return;
    }
    
    for (i = 0; i <= seen_record_count; i++)
    {
        strcpy (seen_record.nick, seen_mem + (i * CHUNKSIZE));
        if (strcasecmp (nick, seen_record.nick) == 0)
        {
            memcpy (&seen_record.time, seen_mem + (i * CHUNKSIZE) + (sizeof (char) * 32), sizeof (time_t));
            time_diff = difftime (time(NULL), seen_record.time);
            time_data = gmtime (&time_diff);
            d = time_data->tm_yday;
            h = time_data->tm_hour;
            m = time_data->tm_min;
            s = time_data->tm_sec;
           
            if (d == 1)
                sprintf (reply, "Saw %s %d Day, %d Hours and %d Minutes ago", seen_record.nick, d, h, m);
            else if (d > 1)
                sprintf (reply, "Saw %s %d Days, %d Hours and %d Minutes ago", seen_record.nick, d, h, m);
            else if (h == 1)
                sprintf (reply, "Saw %s %d Hour and %d Minutes ago", seen_record.nick, h, m);
            else if (h > 1)
                sprintf (reply, "Saw %s %d Hours and %d Minutes ago", seen_record.nick, h, m);
            else if (m > 1)
                sprintf (reply, "Saw %s %d Minutes %d Seconds ago", seen_record.nick, m, s);
            else if (s > 1)
                sprintf (reply, "Saw %s %d Seconds ago", seen_record.nick, s);
            else
                sprintf (reply, "Err?  Check yourself before you wreck yourself");
            //sprintf (reply, "Saw %s at %s (UTC)", seen_record.nick, ctime(&seen_record.time));
            irc_cmd_msg (session, target, reply); 
            return;
        }
    }

    irc_cmd_msg (session, target, "I haven't seen them speak");
}


void seen_store (char *origin)
{
    int i;
    //Store current time
    time_t current_time = time (NULL);

    if (verbose)
        fprintf (stdout, "seen: Saw nick %s at time:%s\n", origin, ctime (&current_time));
    
    //Search seen_mem to see if we already have an entry for them
    for (i = 0; i <= seen_record_count; i++)
    {
        //Don't bother searching if we are on the first record, as it'll be empty
        if (seen_record_count == 1)
            break;
        strcpy (seen_record.nick, seen_mem + (i * CHUNKSIZE));
        if (verbose)
        {
            memcpy (&seen_record.time, seen_mem + (i * CHUNKSIZE) + (sizeof (char) * 32), sizeof (time_t));
            fprintf (stdout, "seen: Searching nick=%s time=%s\n", seen_record.nick, ctime(&seen_record.time));
        }

        if (strcasecmp (origin, seen_record.nick) == 0)
        {
            //If so, modify the last seen time
            if (verbose)
            {
                fprintf (stdout, "seen: Found nick=%s, updating with new time=%s\n", seen_record.nick, ctime(&current_time));
            }
            seen_record.time = current_time;
            memcpy (seen_mem + (i * CHUNKSIZE), &seen_record, sizeof (seen_record));
            return;
        }
    }
    //There wasn't already a record for that nick, so copy the data into seen_mem
    if (verbose)
        fprintf (stdout, "seen: Inserting nick: %s\n", origin);
    strcpy (seen_record.nick, origin);
    seen_record.time = current_time;
    if (seen_record_count == 1)
        memcpy (seen_mem, &seen_record, sizeof (seen_record));
    else
        memcpy (seen_mem + ((seen_record_count - 1) * CHUNKSIZE), &seen_record, sizeof (seen_record));

    //Increase seen_mem to fit in the next record.
    seen_mem_add ();
}

