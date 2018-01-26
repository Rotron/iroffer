/**
 * Implementation of autosend function
 * @file
 * @copyright see CONTRIBUTORS
 * @license
 * This file is licensed under the GPLv3+ as found in the LICENSE file.
 */

#include "iroffer_config.h"
#include "iroffer_defines.h"
#include "iroffer_headers.h"
#include "iroffer_globals.h"

#include "autosend.h"

void autosendf(char* line) {
    char *nick, *hostname, *hostmask;
    int i, j;

    updatecontext();

    floodchk();

    nick = mycalloc(maxtextlengthshort);
    hostname = mycalloc(maxtextlength);

    hostmask = caps(getpart(line, 1));
    for (i = 1; i <= sstrlen(hostmask); i++) {
        hostmask[i - 1] = hostmask[i];
    }

    i = 1;
    j = 0;
    while (line[i] != '!' && i < sstrlen(line) && i < maxtextlengthshort - 1) {
        nick[i - 1] = line[i];
        i++;
    }
    nick[i - 1] = '\0';

    while (line[i] != '@' && i < sstrlen(line)) {
        i++;
    }
    i++;

    while (line[i] != ' ' && i < sstrlen(line) && j < maxtextlength - 1) {
        hostname[j] = line[i];
        i++;
        j++;
    }
    hostname[j] = '\0';

    if (!gdata.ignore) {
        char* tempstr;
#define SENDING_FORMAT_STR " :** Sending You %s by DCC"

        gdata.inamnt[gdata.curtime % INAMNT_SIZE]++;

        if (!gdata.attop) {
            gototop();
        }

        ioutput(CALLTYPE_MULTI_FIRST, OUT_S | OUT_L | OUT_D, COLOR_YELLOW,
                "AutoSend ");

        tempstr = mycalloc(strlen(gdata.autosend.message) +
                           strlen(SENDING_FORMAT_STR) - 1);
        snprintf(tempstr,
                 strlen(gdata.autosend.message) + strlen(SENDING_FORMAT_STR) -
                     1,
                 SENDING_FORMAT_STR, gdata.autosend.message);

        sendxdccfile(nick, hostname, hostmask, gdata.autosend.pack, tempstr);

        mydelete(tempstr);
    }

    mydelete(nick);
    mydelete(hostname);
}
