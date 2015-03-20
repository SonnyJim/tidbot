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

void seen_init (void)
{
    //Initialise seen memory
    seen_record_count = 0;
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
    seen_mem = realloc (seen_mem, sizeof (struct seen_t) * seen_record_count);
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
    
    strcpy (nick, seen_strip_nick (MAGIC_SEEN, params)); 
        
    if (channel == NULL)
        strcpy (target, targetnick);
    else
        strcpy (target, channel);
    fprintf (stdout, "target = %s\n", target);

    if (nick[0] == ' ' || strlen (nick) == 0)
    {
        fprintf (stderr, "seen: empty nick\n");
        irc_cmd_msg (session, target, "Have I seen who? (hint: !seen username)");
        return;
    }
    
    for (i = 0; i < seen_record_count; i++)
    {
        strcpy (seen_record.nick, seen_mem + (i * CHUNKSIZE));
        if (strcmp (nick, seen_record.nick) == 0)
        {
            memcpy (&seen_record.time, seen_mem + (i * CHUNKSIZE) + (sizeof (char) * 32), sizeof (time_t));
            sprintf (reply, "Saw %s at %s (UTC)", seen_record.nick, ctime(&seen_record.time));
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
    for (i = 0; i < seen_record_count; i++)
    {
        fprintf (stdout, "i=%i\n", i);
        
        strcpy (seen_record.nick, seen_mem + (i * CHUNKSIZE));
        if (verbose)
        {
            memcpy (&seen_record.time, seen_mem + (i * CHUNKSIZE) + (sizeof (char) * 32), sizeof (time_t));
            fprintf (stdout, "seen: Searching nick=%s time=%s\n", seen_record.nick, ctime(&seen_record.time));
        }

        if (strcmp (origin, seen_record.nick) == 0)
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
    //Fill in the data
    if (verbose)
        fprintf (stdout, "seen: Inserting nick: %s\n", origin);
    strcpy (seen_record.nick, origin);
    seen_record.time = current_time;
    //fprintf (stdout, "nick=%s time=%s\n", seen_record.nick, ctime(&seen_record.time));
    memcpy (seen_mem + (seen_record_count * CHUNKSIZE), &seen_record, sizeof (seen_record));
    seen_mem_add ();
}

