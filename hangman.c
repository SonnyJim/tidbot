#include "tidbot.h"
#include "cfg.h"
#include <ctype.h> //tolower()

#define HANGMAN_GUESSES 15;

int hangman_running = 0;
int hangman_guesses = 0;
//26 letters, 10 numbers
char hangman_guessed[26 + 10] = "";
char hangman_phrase[1024] = "";
char hangman_hint[1024] = "";

static void hangman_print_hint (void)
{
    irc_cmd_msg (session, irc_cfg.channel, hangman_hint);
}

static int hangman_build_hint (void)
{
    int i, j, present, complete;
   
    complete = 1;
    present = 0;

    for (i = 0; i < strlen (hangman_phrase); i++)
    {
        present = 0;
        for (j = 0; j < strlen (hangman_guessed); j++)
        {
            if (hangman_phrase[i] == hangman_guessed[j])
            {
                present += 1;
                hangman_hint[i] = hangman_guessed[j];
                break;
            }
        }
        if (!present)
        {
            if (hangman_phrase[i] == ' ')
                hangman_hint[i] = ' ';
            else
            {
                complete = 0;
                hangman_hint[i] = '_';
            }
        }
    }
    return complete;
}


void hangman_start (const char *params, const char *target)
{
    int start, end, i;
    char reply[1024] = "";


    if (hangman_running)
    {
        irc_cmd_msg (session, target, "Hangman game already started");
        return;
    }
   
    //Clear out previous guesses
    memset (hangman_guessed, 0, sizeof (hangman_guessed));
    memset (hangman_hint, 0, sizeof (hangman_hint));
    memset (hangman_phrase, 0, sizeof (hangman_phrase));
    hangman_guesses = 0;
    
    //Copy hangman phrase and strip off leading whitespace
    start = strlen (MAGIC_HANGMAN) + 1;
    while (params[start] == ' ')
        start++;

    //Strip off trailing whitespace
    end = strlen (params) - 1;
    while (params[end] == ' ')
    {
        end--;
    }

    strncpy (hangman_phrase, params + start, end - start + 1);
    hangman_phrase[strlen(hangman_phrase)] = '\0';

    //Convert to all lower case
    for (i = 0; i < strlen (hangman_phrase); i++)
    {
        if (!isalnum (hangman_phrase[i]) && hangman_phrase[i] != ' ')
        {
            irc_cmd_msg (session, target, "Phrase must only be numbers or letters");
            return;
        }
        hangman_phrase[i] = tolower(hangman_phrase[i]);
    }
    
    //Limit phrase length to 30 chars
    if (strlen (hangman_phrase) > 30)
    {
        irc_cmd_msg (session, target, "Phrase must be shorter than 30 characters");
        return;
    }
        
    if (verbose)
        fprintf (stdout, "Hangman phrase set to '%s'\n", hangman_phrase);
    hangman_running = 1;
    hangman_guesses = 0;

    sprintf (reply, "Hangman game started by %s", target);
    irc_cmd_msg (session, irc_cfg.channel, reply);
    hangman_build_hint ();
    hangman_print_hint ();
}

void hangman_stop (void)
{
    char reply[1024] = "";
    if (!hangman_running)
        return;
    sprintf (reply, "The phrase was '%s', stopped game early because it was boring everyone", hangman_phrase);
    irc_cmd_msg (session, irc_cfg.channel, reply);
    hangman_running = 0;
}

void hangman_solve (const char *params, const char *origin)
{
    char guess[1024] = "";
    char reply[1024] = "";
    int len, i;

    len = strlen (params);
   
    //Convert guess to lower case
    for (i = 0; i < len; i++)
        guess[i] = tolower(params[i]);

    if (strcmp (guess, hangman_phrase) == 0)
    {
        sprintf (reply, "The phrase was '%s'.  Correctly guessed by %s after %i guesses\n", hangman_phrase, origin, hangman_guesses);
        irc_cmd_msg (session, irc_cfg.channel, reply); 
        irc_cmd_msg (session, irc_cfg.channel, "PM me with !hangman phrase to start a new game");
        hangman_running = 0;
    }
}

void hangman_guess (const char guess, const char *origin)
{
    char guess_letter;
    char reply[1024] = "";
    int len;

    len = strlen(hangman_guessed);

    guess_letter = tolower(guess);
    if (verbose)
        fprintf (stdout, "Hangman guess by %s: %c\n", origin, guess_letter);

    if (strchr (hangman_guessed, guess_letter))
    {
        irc_cmd_msg (session, irc_cfg.channel, "Someone has already guessed that letter");
    }
    else if (!isalnum (guess_letter))
    {
        irc_cmd_msg (session, irc_cfg.channel, "Alphanumerics only!");
    }
    else
    {
        hangman_guessed[len] = guess_letter;
        hangman_guessed[len + 1] = '\0';
        hangman_guesses++;
        //They got the last letter
        if (hangman_build_hint())
            hangman_solve (hangman_phrase, origin);
        else
        {
            hangman_print_hint ();
            sprintf (reply, "Already guessed: %s\n", hangman_guessed);
            irc_cmd_msg (session, irc_cfg.channel, reply);
        }
    }
}

