/******************************************************************************\
 *  client.c
 *    by Chris Dunlap <cdunlap@llnl.gov>
 *
 *  $Id: client.c,v 1.3 2001/05/21 22:52:39 dun Exp $
\******************************************************************************/


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "conman.h"
#include "client.h"
#include "errors.h"


int main(int argc, char *argv[])
{
    client_conf_t *conf;

    conf = create_client_conf();
    process_client_cmd_line(argc, argv, conf);
    open_client_log(conf);

    connect_to_server(conf);

    if (send_greeting(conf) < 0)
        display_error(conf);
    else if (send_req(conf) < 0)
        display_error(conf);
    else if (recv_rsp(conf) < 0)
        display_error(conf);
    else if (conf->command == QUERY)
        display_consoles(conf, STDOUT_FILENO);
    else if (conf->command == EXECUTE)
        display_data(conf, STDOUT_FILENO);
    else if ((conf->command == CONNECT) || (conf->command == MONITOR))
        connect_console(conf);
    else
        err_msg(0, "Invalid command (%d) at %s:%d",
            conf->command, __FILE__, __LINE__);

    close_client_log(conf);
    destroy_client_conf(conf);
    return(0);
}
