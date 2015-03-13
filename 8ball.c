#include "tidbot.h"
#include "cfg.h"

static const char *eightball_responses[] = 
{
    "It is certain",
    "It is decidedly so",
    "Without a doubt",
    "Yes definitely",
    "You may rely on it",
    "As I see it, yes",
    "Most likely",
    "Outlook good",
    "Yes",
    "Signs point to yes",
    "Reply hazy try again",
    "Ask again later",
    "Better not tell you now",
    "Cannot predict now",
    "Concentrate and ask again",
    "Don't count on it",
    "My reply is no",
    "My sources say no",
    "Outlook not so good",
    "Very doubtful"
};

void eightball_reply (void)
{
    char reply[1024];
    int r = rand() % 20;
    strcpy (reply, eightball_responses[r]);
    irc_cmd_msg (session, irc_cfg.channel, reply);
}
