#include "tidbot.h"
#include "cfg.h"

void ctcp_time_req (const char *nick)
{
    if (verbose)
        fprintf (stdout, "Asking %s for their local time\n", nick);
    
    irc_cmd_ctcp_request (session, nick, "TIME");
}

void ctcp_time_rep (const char *params)
{
   irc_cmd_msg (session, irc_cfg.channel, params + strlen ("TIME ")); 
}
