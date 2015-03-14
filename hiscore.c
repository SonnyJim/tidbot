#include "tidbot.h"
#include "cfg.h"

struct score {
    char *nick;
    long int score;
    struct score *next;
};

struct score *head;
struct score *current;

//Strip @ or + off start of nick
static char * hiscore_strip_nick (const char *nick)
{
    char *nick_strip = malloc(sizeof(char) * 32);
    if (strncmp (nick, "@", 1) == 0 || strncmp (nick, "+", 1) == 0)
        strcpy (nick_strip, nick + 1);
    else
        strcpy (nick_strip, nick);
    return nick_strip;
}

static void hiscore_print (void)
{
    fprintf (stdout, "Hiscore print\n");
    while (current)
    {
        if (current->next != NULL)
            fprintf (stdout, "%s score=%lu\n", current->nick, current->score);
        current = current->next;
    }
}

static void hiscore_insert_nick (char *nick)
{
    current = head;
    
    if (verbose)
        fprintf (stdout, "hiscore_insert_nick %s\n", nick);
    
    // Check to see if see if nick is already in struct
    while (current)
    {
        if (strcmp (current->nick, nick) == 0)
        {
            if (verbose)
                fprintf (stdout, "Nick %s was already in list\n", nick);
            return;
        }
        current = current->next;
    }

    current = malloc (sizeof(struct score));
    if (current == NULL)
    {
        fprintf (stderr, "Error mallocing current for hiscore\n");
        return;
    }
    current->nick = nick;
    current->score = 0;
    current->next = head;
    head = current;
    //hiscore_print ();
}

void hiscore_init (void)
{
    head = malloc (sizeof(struct score));
    if (head == NULL)
    {
        fprintf (stderr, "hiscore_init malloc error\n");
        return;
    }
    
    head->next = NULL;
    head->nick = "";
    head->score = 0;
}

//Called on channel join
void hiscore_initialise_nicks (const char *nicks)
{
    char *token;
    const char delim[] = " ";
    //Split nicks into separate entities
    token = strtok (nicks, delim);
    while (token != NULL)
    {
        //Add nick to score struct
        hiscore_insert_nick (hiscore_strip_nick(token));
        token = strtok (NULL, delim);
    }
}

//Called when a nick enters the channel
void hiscore_add_nick (const char *nick)
{
    hiscore_insert_nick (hiscore_strip_nick(nick));
}

void hiscore_add_score (const char *nick, long score)
{
    current = head;
    while (current)
    {
        if (strcmp (current->nick, hiscore_strip_nick(nick)) == 0)
        {
            if (verbose)
                fprintf (stdout, "%s gained %lu points\n", nick, score);
            current->score += score;
            return;
        }
        current = current->next;
    }
    //If we didn't find the nick in the hiscore list, then add it and add the score again.
    //This might happen due to reloading the hiscore file whilst running.
    fprintf (stderr, "hiscore did not find %s nick\n", nick);
    hiscore_add_nick (nick);
    hiscore_add_score (nick, score);
}

void hiscore_print_scores (const char *target, const char *channel)
{
    char reply[1024] = "";
    current = head;
    while (current)
    {
        if (current->next != NULL && current->score > 0)
        {
            sprintf (reply, "%s %lu", current->nick, current->score);
            if (channel == NULL)
                irc_cmd_msg (session, target, reply);
            else
                irc_cmd_msg (session, channel, reply);
        }
        current = current->next;
    }
}

void hiscore_save (const char *target)
{
    FILE *hiscore_file;

    hiscore_file = fopen ("scores.bin", "wb");
    
    if (hiscore_file == NULL)
    {
        if (target != NULL)
            irc_cmd_msg (session, target, "Error opening scores.bin for writing");
        fprintf (stderr, "Error opening scores.bin for writing\n");
        return;
    }

    current = head;
    while (current != NULL)
    {
        if (current->score > 0)
        {
            fwrite (current->nick, sizeof(current->nick), 1, hiscore_file);
            fwrite (current->score, sizeof(current->score), 1, hiscore_file);
        }
        current = current->next;
    }
    fclose (hiscore_file);
    if (target != NULL)
        irc_cmd_msg (session, target, "Scores saved");
}

void hiscore_load (const char *target)
{
    FILE *hiscore_file;
    int fread_ret;
    char *nick;
    long int score;

    hiscore_file = fopen ("scores,bin", "rb");

    if (hiscore_file == NULL)
    {
        if (target != NULL)
            irc_cmd_msg (session, target, "Error opening scores.bin for reading");
        fprintf (stderr, "Error opening scores.bin for reading\n");
        return;
    }
    
    while (fread_ret > 0)
    {
        fread_ret = fread (nick, sizeof(nick), 1, hiscore_file);
        if (fread_ret > 0)
            fread_ret = fread (score, sizeof(score), 1, hiscore_file);
        if (fread_ret > 0)
            hiscore_add_score (nick, score);
    }
    fclose (hiscore_file);
    irc_cmd_msg (session, target, "Loaded scores");
}
