/**
 * Implementation of parsing functions
 * @file
 * @copyright see CONTRIBUTORS
 * @license
 * This file is licensed under the GPLv3+ as found in the LICENSE file.
 */

#include "iroffer_config.h"
#include "iroffer_defines.h"
#include "iroffer_headers.h"
#include "iroffer_globals.h"
#include "conversions.h"
#include "parsing.h"

void privmsgparse(const char* type, char* line) {
    char *nick, *hostname, *hostmask, *wildhost;
    char *msg1, *msg2, *msg3, *msg4, *msg5, *msg6, *dest;
    int i, j, k;
    userinput ui;
    igninfo* ignore = NULL;
    upload* ul;
    transfer* tr;
    pqueue* pq;
    xdcc* xd;
    int line_len;

    updatecontext();

    floodchk();

    line_len = sstrlen(line);

    hostmask = caps(getpart(line, 1));
    for (i = 1; i <= sstrlen(hostmask); i++) {
        hostmask[i - 1] = hostmask[i];
    }

    dest = caps(getpart(line, 3));
    msg1 = getpart(line, 4);
    msg2 = getpart(line, 5);
    msg3 = getpart(line, 6);
    msg4 = getpart(line, 7);
    msg5 = getpart(line, 8);
    msg6 = getpart(line, 9);

    if (msg1) {
        msg1++; /* point past the ":" */
    }

    nick = mycalloc(line_len + 1);
    hostname = mycalloc(line_len + 1);
    wildhost = mycalloc(line_len + 2);


    i = 1;
    j = 0;
    while (line[i] != '!' && i < line_len) {
        nick[i - 1] = line[i];
        i++;
    }
    nick[i - 1] = '\0';


    /* see if it came from a user or server, ignore if from server */
    if (i == line_len) {
        goto privmsgparse_cleanup;
    }

    while (line[i] != '@' && i < line_len) {
        i++;
    }
    i++;

    while (line[i] != ' ' && i < line_len) {
        hostname[j] = line[i];
        i++;
        j++;
    }
    hostname[j] = '\0';

    snprintf(wildhost, line_len + 2, "*!%s", hostmask + strlen(nick) + 1);

    if (isthisforme(dest, msg1)) {
        if (verifyhost(&gdata.autoignore_exclude, hostmask)) {
            /* host matches autoignore_exclude */
            goto noignore;
        }

        /* add/increment ignore list */
        ignore = irlist_get_head(&gdata.ignorelist);

        while (ignore) {
            if ((ignore->regexp != NULL) &&
                !regexec(ignore->regexp, hostmask, 0, NULL, 0)) {
                /* already in list */
                j = 1;
                ignore->bucket++;
                ignore->lastcontact = gdata.curtime;

                if (!(ignore->flags & IGN_IGNORING) &&
                    (ignore->bucket >= IGN_ON)) {
                    int left;
                    left = gdata.autoignore_threshold * (ignore->bucket + 1);

                    ignore->flags |= IGN_IGNORING;

                    ioutput(
                        CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_NO_COLOR,
                        "Auto-ignore activated for %s (%s) lasting %i%c%i%c",
                        nick, wildhost,
                        left < 3600 ? left / 60 : left / 60 / 60,
                        left < 3600 ? 'm' : 'h',
                        left < 3600 ? left % 60 : (left / 60) % 60,
                        left < 3600 ? 's' : 'm');

                    notice(nick,
                           "Auto-ignore activated for %s (%s) lasting "
                           "%i%c%i%c. Further messages will increase duration.",
                           nick, wildhost,
                           left < 3600 ? left / 60 : left / 60 / 60,
                           left < 3600 ? 'm' : 'h',
                           left < 3600 ? left % 60 : (left / 60) % 60,
                           left < 3600 ? 's' : 'm');

                    write_statefile();
                }

                if (ignore->flags & IGN_IGNORING) {
                    goto privmsgparse_cleanup;
                }
                break;
            }
            ignore = irlist_get_next(ignore);
        }

        if (!ignore) {
            char* tempr;

            /* not in list */
            ignore = irlist_add(&gdata.ignorelist, sizeof(igninfo));
            ignore->regexp = mycalloc(sizeof(regex_t));

            ignore->hostmask = mymalloc(strlen(wildhost) + 1);
            strcpy(ignore->hostmask, wildhost);

            tempr = hostmasktoregex(wildhost);
            if (regcomp(ignore->regexp, tempr, REG_ICASE | REG_NOSUB)) {
                ignore->regexp = NULL;
            }

            ignore->bucket = 1;
            ignore->flags &= ~IGN_MANUAL & ~IGN_IGNORING;
            ignore->lastcontact = gdata.curtime;

            mydelete(tempr);
        }
    }
noignore:

    /*----- CLIENTINFO ----- */
    if (!gdata.ignore &&
        (!strcmp(msg1, "\1CLIENTINFO") || !strcmp(msg1, "\1CLIENTINFO\1"))) {
        gdata.inamnt[gdata.curtime % INAMNT_SIZE]++;
        if (!msg2) {
            notice(nick,
                   "\1CLIENTINFO DCC PING VERSION XDCC UPTIME "
                   ":Use CTCP CLIENTINFO <COMMAND> to get more specific "
                   "information\1");
        } else if (strncmp(caps(msg2), "PING", 4) == 0) {
            notice(nick,
                   "\1CLIENTINFO PING returns the arguments it receives\1");
        } else if (strncmp(caps(msg2), "DCC", 3) == 0) {
            notice(nick,
                   "\1CLIENTINFO DCC requests a DCC for chatting or file "
                   "transfer\1");
        } else if (strncmp(caps(msg2), "VERSION", 7) == 0) {
            notice(nick,
                   "\1CLIENTINFO VERSION shows information about this client's "
                   "version\1");
        } else if (strncmp(caps(msg2), "XDCC", 4) == 0) {
            notice(nick,
                   "\1CLIENTINFO XDCC LIST|SEND list and DCC file(s) to you\1");
        } else if (strncmp(caps(msg2), "UPTIME", 6) == 0) {
            notice(nick,
                   "\1CLIENTINFO UPTIME shows how long this client has been "
                   "running\1");
        }

        ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_YELLOW,
                "[CTCP] %s: CLIENTINFO", nick);
    }

    /*----- PING ----- */
    else if (!gdata.ignore &&
             (!strcmp(msg1, "\1PING") || !strcmp(msg1, "\1PING\1"))) {
        gdata.inamnt[gdata.curtime % INAMNT_SIZE]++;
        if (msg2 && (msg2[strlen(msg2) - 1] == '\1')) {
            msg2[strlen(msg2) - 1] = '\0';
        }
        if (msg3 && (msg3[strlen(msg3) - 1] == '\1')) {
            msg3[strlen(msg3) - 1] = '\0';
        }
        notice(nick, "\1PING%s%s%s%s\1", msg2 ? " " : "", msg2 ? msg2 : "",
               msg3 ? " " : "", msg3 ? msg3 : "");
        ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_YELLOW,
                "[CTCP] %s: PING", nick);
    }

    /*----- VERSION ----- */
    else if (!gdata.ignore &&
             (!strcmp(msg1, "\1VERSION") || !strcmp(msg1, "\1VERSION\1"))) {
        gdata.inamnt[gdata.curtime % INAMNT_SIZE]++;
        notice(nick,
               "\1VERSION iroffer v" VERSIONLONG ", http://iroffer.org/%s%s\1",
               gdata.hideos ? "" : " - ", gdata.hideos ? "" : gdata.osstring);
        ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_YELLOW,
                "[CTCP] %s: VERSION", nick);
    }

    /*----- UPTIME ----- */
    else if (!gdata.ignore &&
             (!strcmp(msg1, "\1UPTIME") || !strcmp(msg1, "\1UPTIME\1"))) {
        char* tempstr2 = mycalloc(maxtextlength);
        gdata.inamnt[gdata.curtime % INAMNT_SIZE]++;
        tempstr2 = getuptime(tempstr2, 0, gdata.startuptime, maxtextlength);
        notice(nick, "\1UPTIME %s\1", tempstr2);
        ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_YELLOW,
                "[CTCP] %s: UPTIME", nick);
        mydelete(tempstr2);
    }

    /*----- STATUS ----- */
    else if (!gdata.ignore &&
             (!strcmp(msg1, "\1STATUS") || !strcmp(msg1, "\1STATUS\1"))) {
        char* tempstr2 = mycalloc(maxtextlength);
        gdata.inamnt[gdata.curtime % INAMNT_SIZE]++;
        tempstr2 = getstatuslinenums(tempstr2, maxtextlength);
        notice(nick, "\1%s\1", tempstr2);
        ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_YELLOW,
                "[CTCP] %s: STATUS", nick);
        mydelete(tempstr2);
    }

    /*----- DCC SEND/CHAT/RESUME ----- */
    else if (!gdata.ignore && gdata.caps_nick &&
             !strcmp(gdata.caps_nick, dest) && !strcmp(caps(msg1), "\1DCC") &&
             msg2) {
        if (!gdata.attop) {
            gototop();
        }
        if (!strcmp(caps(msg2), "RESUME") && msg3 && msg4 && msg5) {
            gdata.inamnt[gdata.curtime % INAMNT_SIZE]++;

            caps(nick);

            if (msg5[strlen(msg5) - 1] == '\1') {
                msg5[strlen(msg5) - 1] = '\0';
            }
            tr = irlist_get_head(&gdata.trans);
            while (tr) {
                if ((tr->tr_status == TRANSFER_STATUS_LISTENING) &&
                    !strcmp(tr->caps_nick, nick) &&
                    (strstrnocase(tr->xpack->file, msg3) ||
                     (tr->listenport == atoi(msg4)))) {
                    if (atoull(msg5) >=
                        (unsigned long long)tr->xpack->st_size) {
                        notice(nick,
                               "You can't resume the transfer at a point "
                               "greater than the size of the file");
                        ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D,
                                COLOR_YELLOW,
                                "XDCC [%02i:%s]: Resume attempted beyond end "
                                "of file ( %" PRIu64 "u >= %" PRIu64 "u )",
                                tr->id, tr->nick, (uint64_t)atoull(msg5),
                                (uint64_t)tr->xpack->st_size);
                    } else {
                        t_setresume(tr, msg5);
                        privmsg_fast(nick, "\1DCC ACCEPT %s %s %s\1", msg3,
                                     msg4, msg5);
                        ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D,
                                COLOR_YELLOW,
                                "XDCC [%02i:%s]: Resumed at %" PRId64 "iK",
                                tr->id, tr->nick,
                                (int64_t)(tr->startresume / 1024));
                    }
                    break;
                }
                tr = irlist_get_next(tr);
            }

            if (!tr) {
                outerror(OUTERROR_TYPE_WARN,
                         "Couldn't find transfer that %s tried to resume!",
                         nick);
                tr = irlist_get_head(&gdata.trans);
                while (tr) {
                    if (gdata.debug > 0) {
                        ioutput(
                            CALLTYPE_NORMAL, OUT_S, COLOR_NO_COLOR,
                            "resume trying %i: %s == %s, %s == %s, %i == %i\n",
                            tr->tr_status, tr->caps_nick, nick, tr->xpack->file,
                            msg3, tr->listenport, atoi(msg4));
                    }
                    tr = irlist_get_next(tr);
                }
            }
        } else if (!strcmp(caps(msg2), "CHAT")) {
            if (verifyhost(&gdata.adminhost, hostmask)) {
                ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_MAGENTA,
                        "DCC CHAT attempt authorized from %s", hostmask);
                setupdccchat(nick, line);
            } else {
                notice(nick, "DCC Chat denied from %s", hostmask);
                ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_MAGENTA,
                        "DCC CHAT attempt denied from %s", hostmask);
            }
        }

        else if (!strcmp(caps(msg2), "SEND") && msg3 && msg4 && msg5 && msg6) {
            if (msg6[strlen(msg6) - 1] == '\1') {
                msg6[strlen(msg6) - 1] = '\0';
            }
            if (!verifyhost(&gdata.uploadhost, hostmask)) {
                notice(nick,
                       "DCC Send Denied, I don't accept transfers from %s",
                       hostmask);
                ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_MAGENTA,
                        "DCC Send Denied from %s", hostmask);
            } else if (gdata.uploadmaxsize &&
                       atoull(msg6) > gdata.uploadmaxsize) {
                notice(nick,
                       "DCC Send Denied, I don't accept transfers that big");
                ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_MAGENTA,
                        "DCC Send Denied (Too Big) from %s", hostmask);
            } else if (atoull(msg6) > gdata.max_file_size) {
                notice(nick,
                       "DCC Send Denied, I can't accept transfers that large");
                ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_MAGENTA,
                        "DCC Send Denied (Too Large) from %s", hostmask);
            } else if (irlist_size(&gdata.uploads) >= MAXUPLDS) {
                notice(nick,
                       "DCC Send Denied, I'm already getting too many files");
                ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_MAGENTA,
                        "DCC Send Denied (too many uploads) from %s", hostmask);
            } else {
                ul = irlist_add(&gdata.uploads, sizeof(upload));
                l_initvalues(ul);
                removenonprintablefile(msg3);
                ul->file = mymalloc(strlen(msg3) + 1);
                strcpy(ul->file, msg3);
                ul->remoteip = atoul(msg4);
                ul->remoteport = atoi(msg5);
                ul->totalsize = (off_t)atoull(msg6);
                ul->nick = mymalloc(strlen(nick) + 1);
                strcpy(ul->nick, nick);
                ul->hostname = mymalloc(strlen(hostname) + 1);
                strcpy(ul->hostname, hostname);
                ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_YELLOW,
                        "DCC Send Accepted from %s: %s (%" PRId64 "iKB)", nick,
                        ul->file, (int64_t)(ul->totalsize / 1024));
                l_establishcon(ul);
            }
        }

        else if (!strcmp(caps(msg2), "ACCEPT") && msg3 && msg4 && msg5) {
            if (msg5[strlen(msg5) - 1] == '\1') {
                msg5[strlen(msg5) - 1] = '\0';
            }
            if (!verifyhost(&gdata.uploadhost, hostmask)) {
                notice(nick,
                       "DCC Send Denied, I don't accept transfers from %s",
                       hostmask);
                ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_MAGENTA,
                        "DCC Send Denied from %s", hostmask);
            } else if (gdata.uploadmaxsize &&
                       atoull(msg5) > gdata.uploadmaxsize) {
                notice(nick,
                       "DCC Send Denied, I don't accept transfers that big");
                ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_MAGENTA,
                        "DCC Send denied from %s (too big)", hostmask);
            } else if (atoull(msg5) > gdata.max_file_size) {
                notice(nick,
                       "DCC Send Denied, I can't accept transfers that large");
                ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_MAGENTA,
                        "DCC Send denied from %s (too large)", hostmask);
            }

            ul = irlist_get_head(&gdata.uploads);
            while (ul) {
                if ((ul->remoteport == atoi(msg4)) && !strcmp(ul->nick, nick)) {
                    ul->resumed = 1;
                    ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D,
                            COLOR_YELLOW,
                            "DCC Send Resumed from %s: %s (%" PRId64
                            "i of %" PRId64 "iKB left)",
                            nick, ul->file,
                            (int64_t)((ul->totalsize - ul->resumesize) / 1024),
                            (int64_t)(ul->totalsize / 1024));
                    l_establishcon(ul);
                    break;
                }
                ul = irlist_get_next(ul);
            }

            if (!ul) {
                notice(nick, "DCC Resume Denied, unable to find transfer");
                outerror(OUTERROR_TYPE_WARN,
                         "Couldn't find upload that %s tried to resume!", nick);
            }
        }
    }

    /*----- ADMIN ----- */
    else if (!gdata.ignore && gdata.caps_nick &&
             !strcmp(gdata.caps_nick, dest) && !strcmp(caps(msg1), "ADMIN")) {
        /*      msg2 = getpart(line,5); */
        if (!gdata.attop) {
            gototop();
        }

        if (verifyhost(&gdata.adminhost, hostmask)) {
            if (verifypass(msg2)) {
                if (line[line_len - 1] == '\n') {
                    line[line_len - 1] = '\0';
                    line_len--;
                }
                u_fillwith_msg(&ui, nick, line);
                u_parseit(&ui);

                /* admin commands shouldn't count against ignore */
                if (ignore) {
                    ignore->bucket--;
                }
            } else {
                notice(nick, "ADMIN: Incorrect Password");
                ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_MAGENTA,
                        "Incorrect ADMIN Password (%s)", hostmask);
            }
        } else {
            notice(nick, "ADMIN: %s is not allowed to issue admin commands",
                   hostmask);
            ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_MAGENTA,
                    "Incorrect ADMIN Hostname (%s)", hostmask);
        }
    }

    /*----- XDCC ----- */
    else if (!gdata.ignore && gdata.caps_nick &&
             (!strcmp(gdata.caps_nick, dest) || gdata.respondtochannelxdcc) &&
             (!strcmp(caps(msg1), "XDCC") || !strcmp(msg1, "\1XDCC") ||
              !strcmp(caps(msg1), "CDCC") || !strcmp(msg1, "\1CDCC"))) {
        gdata.inamnt[gdata.curtime % INAMNT_SIZE]++;

        caps(msg2);

        if (msg3 && msg3[strlen(msg3) - 1] == '\1') {
            msg3[strlen(msg3) - 1] = '\0';
        }
        if (msg2 && (!strcmp(msg2, "LIST") || !strcmp(msg2, "LIST\1"))) {
            if (!gdata.attop) {
                gototop();
            }

            if (gdata.restrictprivlist) {
                j = 2; /* deny */
                if (gdata.restrictprivlistmsg) {
                    notice(nick, "XDCC LIST Denied. %s",
                           gdata.restrictprivlistmsg);
                } else {
                    notice(nick,
                           "XDCC LIST Denied. Wait for the public list in the "
                           "channel.");
                }
            } else if (gdata.restrictlist && (!isinmemberlist(nick))) {
                j = 2; /* deny */
                notice(nick,
                       "XDCC LIST Denied. You must be on a known channel to "
                       "request a list");
            } else {
                char* user;
                user = irlist_get_head(&gdata.xlistqueue);

                while (user) {
                    if (!strcmp(user, nick)) {
                        break;
                    }
                    user = irlist_get_next(user);
                }

                if (!user) {
                    if (irlist_size(&gdata.xlistqueue) >= MAXXLQUE) {
                        j = 2; /* deny */
                        notice_slow(nick,
                                    "XDCC LIST Denied. I'm rather busy at the "
                                    "moment, try again later");
                    } else {
                        user = irlist_add(&gdata.xlistqueue, strlen(nick) + 1);
                        strcpy(user, nick);
                    }
                }
            }

            ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_YELLOW,
                    "XDCC LIST %s: (%s)",
                    (j == 1 ? "ignored" : (j == 2 ? "denied" : "queued")),
                    hostmask);

        } else if (gdata.caps_nick && !strcmp(gdata.caps_nick, dest)) {
            if (msg2 && msg3 &&
                (!strcmp(msg2, "SEND") || !strcmp(msg2, "GET"))) {
                if (!gdata.attop) {
                    gototop();
                }
                ioutput(CALLTYPE_MULTI_FIRST, OUT_S | OUT_L | OUT_D,
                        COLOR_YELLOW, "XDCC SEND %s", msg3);
                sendxdccfile(nick, hostname, hostmask, packnumtonum(msg3),
                             NULL);
            } else if (msg2 && msg3 && (!strcmp(msg2, "INFO"))) {
                if (!gdata.attop) {
                    gototop();
                }
                ioutput(CALLTYPE_MULTI_FIRST, OUT_S | OUT_L | OUT_D,
                        COLOR_YELLOW, "XDCC INFO %s", msg3);
                sendxdccinfo(nick, hostname, hostmask, packnumtonum(msg3),
                             NULL);
            } else if (msg2 && !strcmp(msg2, "REMOVE")) {
                if (!gdata.attop) {
                    gototop();
                }
                k = 0;

                pq = irlist_get_head(&gdata.mainqueue);
                while (pq) {
                    if (!strcmp(pq->nick, nick)) {
                        notice(nick,
                               "Removed you from the queue for \"%s\", you "
                               "waited %li minute%s.",
                               pq->xpack->desc,
                               (long)(gdata.curtime - pq->queuedtime) / 60,
                               ((gdata.curtime - pq->queuedtime) / 60) != 1
                                   ? "s"
                                   : "");
                        mydelete(pq->nick);
                        mydelete(pq->hostname);
                        pq = irlist_delete(&gdata.mainqueue, pq);
                        k = 1;
                    } else {
                        pq = irlist_get_next(pq);
                    }
                }
                if (!k) {
                    notice(nick, "You Don't Appear To Be In A Queue");
                }
                ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_YELLOW,
                        "XDCC REMOVE (%s) ", hostmask);

            } else if (msg2 && !strcmp(msg2, "SEARCH") && msg3) {
                if (!gdata.attop) {
                    gototop();
                }

                notice_slow(nick, "Searching for \"%s\"...", msg3);

                i = 1;
                k = 0;
                xd = irlist_get_head(&gdata.xdccs);
                while (xd) {
                    if (strstrnocase(xd->file, msg3) ||
                        strstrnocase(xd->desc, msg3) ||
                        strstrnocase(xd->note, msg3)) {
                        notice_slow(nick, " - Pack #%i matches, \"%s\"", i,
                                    xd->desc);
                        k++;
                    }
                    i++;
                    xd = irlist_get_next(xd);
                }

                if (!k) {
                    notice_slow(nick,
                                "Sorry, nothing was found, try a XDCC LIST");
                }
                ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_YELLOW,
                        "XDCC SEARCH %s (%s)", msg3, hostmask);
            }
        }
    }

    /*----- !LIST ----- */
    else if (!gdata.ignore && gdata.caps_nick && gdata.respondtochannellist &&
             msg1 && !strcasecmp(caps(msg1), "!LIST") &&
             (!msg2 || !strcmp(caps(msg2), gdata.caps_nick))) {
        gdata.inamnt[gdata.curtime % INAMNT_SIZE]++;

        /* generate !list styled message */

        notice_slow(nick,
                    "\2(\2XDCC\2)\2 Packs:\2(\2%d\2)\2 "
                    "Trigger:\2(\2/msg %s xdcc list\2)\2 "
                    "Sends:\2(\2%i/%i\2)\2 "
                    "Queues:\2(\2%i/%i\2)\2 "
                    "Record:\2(\2%1.1fKB/s\2)\2 "
                    "%s%s%s\2=\2iroffer\2=\2",
                    irlist_size(&gdata.xdccs),
                    (gdata.user_nick ? gdata.user_nick : "??"),
                    irlist_size(&gdata.trans), gdata.slotsmax,
                    irlist_size(&gdata.mainqueue), gdata.queuesize,
                    gdata.record, gdata.creditline ? "Note:\2(\2" : "",
                    gdata.creditline ? gdata.creditline : "",
                    gdata.creditline ? "\2)\2 " : "");
    }

    else {
        if (dest && gdata.caps_nick && !strcmp(dest, gdata.caps_nick)) {
            if ((gdata.lognotices && !strcmp(type, "NOTICE")) ||
                (gdata.logmessages && !strcmp(type, "PRIVMSG"))) {
                msglog_t* ml;
                char* begin;

                ioutput(CALLTYPE_NORMAL, OUT_S | OUT_D, COLOR_GREEN,
                        "%s from %s logged, use MSGREAD to display it.", type,
                        nick);

                ml = irlist_add(&gdata.msglog, sizeof(msglog_t));

                begin =
                    line + 5 + strlen(hostmask) + strlen(type) + strlen(dest);

                ml->when = gdata.curtime;
                ml->hostmask = mymalloc(strlen(hostmask) + 1);
                strcpy(ml->hostmask, hostmask);
                ml->message = mymalloc(strlen(begin) + 1);
                strcpy(ml->message, begin);

                write_statefile();
            } else {
                ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_GREEN,
                        "%s: %s", type, line);
            }
        }
    }

privmsgparse_cleanup:

    if (msg1) {
        msg1--;
    }

    mydelete(dest);
    mydelete(nick);
    mydelete(hostname);
    mydelete(hostmask);
    mydelete(wildhost);
    mydelete(msg1);
    mydelete(msg2);
    mydelete(msg3);
    mydelete(msg4);
    mydelete(msg5);
    mydelete(msg6);
}
