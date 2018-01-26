/*
iroffer - An IRC file server using the DCC protocol
Copyright (C) see CONTRIBUTORS

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/* include the headers */
#include "iroffer_config.h"
#include "iroffer_defines.h"
#include "iroffer_headers.h"
#include "iroffer_globals.h"


/*
 *      ******  FILE FORMAT  ******
 *
 * Word size:     4 Bytes
 * Byte order:    Network Order
 *
 * == OVERALL ==
 * Word 0:        Magic
 * Word 1:        Version
 * Word 2..n-4:   TLV Data
 * Word n-3..n:   md5sum
 *
 * == TLV Format ==
 * Word 0:        Tag
 * Word 1:        Length in Bytes (Tag+Length+Value, but not Padding)
 * Word 2..n-1:   Value
 * Word n:        Value + Zero Padding
 *
 * Some tags are compound, some are not.  Compound tags contain TLV
 * data of their own.
 *
 */

typedef enum {
    STATEFILE_TAG_IROFFER_VERSION = 1 << 8,
    STATEFILE_TAG_TIMESTAMP,

    STATEFILE_TAG_XFR_RECORD = 2 << 8,
    STATEFILE_TAG_SENT_RECORD,
    STATEFILE_TAG_TOTAL_SENT,
    STATEFILE_TAG_TOTAL_UPTIME,
    STATEFILE_TAG_LAST_LOGROTATE,

    STATEFILE_TAG_IGNORE = 10 << 8, /* compound */
    STATEFILE_TAG_IGNORE_FLAGS,
    STATEFILE_TAG_IGNORE_BUCKET,
    STATEFILE_TAG_IGNORE_LASTCONTACT,
    STATEFILE_TAG_IGNORE_HOSTMASK,

    STATEFILE_TAG_MSGLOG = 11 << 8, /* compound */
    STATEFILE_TAG_MSGLOG_WHEN,
    STATEFILE_TAG_MSGLOG_HOSTMASK,
    STATEFILE_TAG_MSGLOG_MESSAGE,

    STATEFILE_TAG_XDCCS = 12 << 8, /* compound */
    STATEFILE_TAG_XDCCS_FILE,
    STATEFILE_TAG_XDCCS_DESC,
    STATEFILE_TAG_XDCCS_NOTE,
    STATEFILE_TAG_XDCCS_GETS,
    STATEFILE_TAG_XDCCS_MINSPEED,
    STATEFILE_TAG_XDCCS_MAXSPEED,
    STATEFILE_TAG_XDCCS_MD5SUM_INFO,

    STATEFILE_TAG_TLIMIT_DAILY_USED = 13 << 8,
    STATEFILE_TAG_TLIMIT_DAILY_ENDS,
    STATEFILE_TAG_TLIMIT_WEEKLY_USED,
    STATEFILE_TAG_TLIMIT_WEEKLY_ENDS,
    STATEFILE_TAG_TLIMIT_MONTHLY_USED,
    STATEFILE_TAG_TLIMIT_MONTHLY_ENDS,

} statefile_tag_t;

typedef struct {
    statefile_tag_t tag;
    int32_t length; /* includes header */
} statefile_hdr_t;

#define STATEFILE_MAGIC (('I' << 24) | ('R' << 16) | ('F' << 8) | 'R')
#define STATEFILE_VERSION 1

typedef struct {
    uint32_t upper;
    uint32_t lower;
} statefile_uint64_t;

typedef struct {
    statefile_hdr_t hdr;
    time_t g_time;
} statefile_item_generic_time_t;

typedef struct {
    statefile_hdr_t hdr;
    int32_t g_int;
} statefile_item_generic_int_t;

typedef struct {
    statefile_hdr_t hdr;
    uint32_t g_uint;
} statefile_item_generic_uint_t;

typedef struct {
    statefile_hdr_t hdr;
    statefile_uint64_t g_ullint;
} statefile_item_generic_ullint_t;

typedef struct {
    statefile_hdr_t hdr;
    float g_float;
} statefile_item_generic_float_t;

typedef struct {
    statefile_hdr_t hdr;
    statefile_uint64_t st_size;
    statefile_uint64_t st_dev;
    statefile_uint64_t st_ino;
    time_t mtime;
    MD5Digest md5sum;
} statefile_item_md5sum_info_t;


static int write_statefile_item(ir_boutput_t* bout, void* item) {
    int callval;
    statefile_hdr_t* hdr = (statefile_hdr_t*)item;
    int32_t length = hdr->length;
    unsigned char dummy[4] = {};

    hdr->tag = htonl(hdr->tag);
    hdr->length = htonl(hdr->length);

    callval = ir_boutput_write(bout, item, length);

    if (callval != length) {
        outerror(OUTERROR_TYPE_WARN_LOUD,
                 "Cant Write To State File (%d != %d) %s", callval, length,
                 strerror(errno));
        return -1;
    }

    if (length & 3) {
        length = 4 - (length & 3);
        callval = ir_boutput_write(bout, dummy, length);

        if (callval != length) {
            outerror(OUTERROR_TYPE_WARN_LOUD,
                     "Cant Write To State File (%d != %d) %s", callval, length,
                     strerror(errno));
            return -1;
        }
    }

    return 0;
}


void write_statefile(void) {
    char *statefile_tmp, *statefile_bkup;
    int fd;
    int callval;
    statefile_hdr_t* hdr;
    int32_t length;
    ir_boutput_t bout;

    updatecontext();

    if (gdata.statefile == NULL) {
        return;
    }

    statefile_tmp = mymalloc(strlen(gdata.statefile) + 5);
    statefile_bkup = mymalloc(strlen(gdata.statefile) + 2);

    sprintf(statefile_tmp, "%s.tmp", gdata.statefile);
    sprintf(statefile_bkup, "%s~", gdata.statefile);

    if (gdata.debug > 0) {
        ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_NO_COLOR,
                "Saving State File... ");
    }

    fd = open(statefile_tmp, O_WRONLY | O_CREAT | O_TRUNC,
              CREAT_PERMISSIONS);

    if (fd < 0) {
        outerror(OUTERROR_TYPE_WARN_LOUD, "Cant Create State File '%s': %s",
                 statefile_tmp, strerror(errno));
        goto error_out;
    }

    ir_boutput_init(&bout, fd, BOUTPUT_MD5SUM | BOUTPUT_NO_LIMIT);

    /*** write ***/

    {
        uint32_t magic = htonl(STATEFILE_MAGIC);
        callval = ir_boutput_write(&bout, &magic, sizeof(magic));
        if (callval != sizeof(magic)) {
            outerror(OUTERROR_TYPE_WARN_LOUD,
                     "Cant Write To State File (%d != %d) %s", callval,
                     (int)sizeof(magic), strerror(errno));
        }
    }

    {
        uint32_t version = htonl(STATEFILE_VERSION);
        callval = ir_boutput_write(&bout, &version, sizeof(version));
        if (callval != sizeof(version)) {
            outerror(OUTERROR_TYPE_WARN_LOUD,
                     "Cant Write To State File (%d != %d) %s", callval,
                     (int)sizeof(version), strerror(errno));
        }
    }

    {
        unsigned char* data;
        unsigned char* next;

        length = sizeof(statefile_hdr_t) + maxtextlength;

        data = mycalloc(length);

        /* header */
        hdr = (statefile_hdr_t*)data;
        hdr->tag = STATEFILE_TAG_IROFFER_VERSION;
        hdr->length = sizeof(statefile_hdr_t);
        next = (unsigned char*)(&hdr[1]);

        length = snprintf((char*)next, maxtextlength - 1,
                          "iroffer v" VERSIONLONG ", %s", gdata.osstring);

        if ((length < 0) || (length >= maxtextlength)) {
            outerror(OUTERROR_TYPE_WARN_LOUD, "Version too long! %d", length);
        } else {
            hdr->length += ceiling(length + 1, 4);
            write_statefile_item(&bout, data);
        }

        mydelete(data);
    }

    {
        statefile_item_generic_time_t item_timestamp;
        item_timestamp.hdr.tag = STATEFILE_TAG_TIMESTAMP;
        item_timestamp.hdr.length = sizeof(statefile_item_generic_time_t);
        item_timestamp.g_time = htonl(gdata.curtime);
        write_statefile_item(&bout, &item_timestamp);
    }

    {
        statefile_item_generic_float_t item_xfr_record;
        item_xfr_record.hdr.tag = STATEFILE_TAG_XFR_RECORD;
        item_xfr_record.hdr.length = sizeof(statefile_item_generic_float_t);
        item_xfr_record.g_float = gdata.record;
        write_statefile_item(&bout, &item_xfr_record);
    }

    {
        statefile_item_generic_float_t item_sent_record;
        item_sent_record.hdr.tag = STATEFILE_TAG_SENT_RECORD;
        item_sent_record.hdr.length = sizeof(statefile_item_generic_float_t);
        item_sent_record.g_float = gdata.sentrecord;
        write_statefile_item(&bout, &item_sent_record);
    }

    {
        statefile_item_generic_ullint_t item_total_sent;
        item_total_sent.hdr.tag = STATEFILE_TAG_TOTAL_SENT;
        item_total_sent.hdr.length = sizeof(statefile_item_generic_ullint_t);
        item_total_sent.g_ullint.upper = htonl(gdata.totalsent >> 32);
        item_total_sent.g_ullint.lower = htonl(gdata.totalsent & 0xFFFFFFFF);
        write_statefile_item(&bout, &item_total_sent);
    }

    {
        statefile_item_generic_int_t item_total_uptime;
        item_total_uptime.hdr.tag = STATEFILE_TAG_TOTAL_UPTIME;
        item_total_uptime.hdr.length = sizeof(statefile_item_generic_int_t);
        item_total_uptime.g_int = htonl(gdata.totaluptime);
        write_statefile_item(&bout, &item_total_uptime);
    }

    {
        statefile_item_generic_time_t item_last_logrotate;
        item_last_logrotate.hdr.tag = STATEFILE_TAG_LAST_LOGROTATE;
        item_last_logrotate.hdr.length = sizeof(statefile_item_generic_int_t);
        item_last_logrotate.g_time = htonl(gdata.last_logrotate);
        write_statefile_item(&bout, &item_last_logrotate);
    }

    {
        unsigned char* data;
        unsigned char* next;
        igninfo* ignore;
        statefile_item_generic_int_t* g_int;
        statefile_item_generic_uint_t* g_uint;
        statefile_item_generic_time_t* g_time;

        ignore = irlist_get_head(&gdata.ignorelist);

        while (ignore) {
            if (ignore->flags & IGN_IGNORING) {
                /*
                 * need room to write:
                 *  flags         uint
                 *  bucket        int
                 *  lastcontact   time_t
                 *  hostmask      string
                 */
                length = sizeof(statefile_hdr_t) +
                         sizeof(statefile_item_generic_uint_t) +
                         sizeof(statefile_item_generic_int_t) +
                         sizeof(statefile_item_generic_time_t) +
                         sizeof(statefile_hdr_t) +
                         ceiling(strlen(ignore->hostmask) + 1, 4);

                data = mycalloc(length);

                /* outer header */
                hdr = (statefile_hdr_t*)data;
                hdr->tag = STATEFILE_TAG_IGNORE;
                hdr->length = length;
                next = (unsigned char*)(&hdr[1]);

                /* flags */
                g_uint = (statefile_item_generic_uint_t*)next;
                g_uint->hdr.tag = htonl(STATEFILE_TAG_IGNORE_FLAGS);
                g_uint->hdr.length = htonl(sizeof(*g_uint));
                g_uint->g_uint = htonl(ignore->flags);
                next = (unsigned char*)(&g_uint[1]);

                /* bucket */
                g_int = (statefile_item_generic_int_t*)next;
                g_int->hdr.tag = htonl(STATEFILE_TAG_IGNORE_BUCKET);
                g_int->hdr.length = htonl(sizeof(*g_int));
                g_int->g_int = htonl(ignore->bucket);
                next = (unsigned char*)(&g_int[1]);

                /* lastcontact */
                g_time = (statefile_item_generic_time_t*)next;
                g_time->hdr.tag = htonl(STATEFILE_TAG_IGNORE_LASTCONTACT);
                g_time->hdr.length = htonl(sizeof(*g_time));
                g_time->g_time = htonl(ignore->lastcontact);
                next = (unsigned char*)(&g_time[1]);

                /* hostmask */
                hdr = (statefile_hdr_t*)next;
                hdr->tag = htonl(STATEFILE_TAG_IGNORE_HOSTMASK);
                hdr->length = htonl(sizeof(statefile_hdr_t) +
                                    strlen(ignore->hostmask) + 1);
                next = (unsigned char*)(&hdr[1]);
                strcpy((char*)next, ignore->hostmask);
                next += ceiling(strlen(ignore->hostmask) + 1, 4);

                write_statefile_item(&bout, data);

                mydelete(data);
            }
            ignore = irlist_get_next(ignore);
        }
    }

    {
        unsigned char* data;
        unsigned char* next;
        msglog_t* msglog;
        statefile_item_generic_time_t* g_time;

        msglog = irlist_get_head(&gdata.msglog);

        while (msglog) {
            /*
             * need room to write:
             *  when          time_t
             *  hostmask      string
             *  message       string
             */
            length = sizeof(statefile_hdr_t) +
                     sizeof(statefile_item_generic_time_t) +
                     sizeof(statefile_hdr_t) +
                     ceiling(strlen(msglog->hostmask) + 1, 4) +
                     sizeof(statefile_hdr_t) +
                     ceiling(strlen(msglog->message) + 1, 4);

            data = mycalloc(length);

            /* outer header */
            hdr = (statefile_hdr_t*)data;
            hdr->tag = STATEFILE_TAG_MSGLOG;
            hdr->length = length;
            next = (unsigned char*)(&hdr[1]);

            /* when */
            g_time = (statefile_item_generic_time_t*)next;
            g_time->hdr.tag = htonl(STATEFILE_TAG_MSGLOG_WHEN);
            g_time->hdr.length = htonl(sizeof(*g_time));
            g_time->g_time = htonl(msglog->when);
            next = (unsigned char*)(&g_time[1]);

            /* hostmask */
            hdr = (statefile_hdr_t*)next;
            hdr->tag = htonl(STATEFILE_TAG_MSGLOG_HOSTMASK);
            hdr->length =
                htonl(sizeof(statefile_hdr_t) + strlen(msglog->hostmask) + 1);
            next = (unsigned char*)(&hdr[1]);
            strcpy((char*)next, msglog->hostmask);
            next += ceiling(strlen(msglog->hostmask) + 1, 4);

            /* message */
            hdr = (statefile_hdr_t*)next;
            hdr->tag = htonl(STATEFILE_TAG_MSGLOG_MESSAGE);
            hdr->length =
                htonl(sizeof(statefile_hdr_t) + strlen(msglog->message) + 1);
            next = (unsigned char*)(&hdr[1]);
            strcpy((char*)next, msglog->message);
            next += ceiling(strlen(msglog->message) + 1, 4);

            write_statefile_item(&bout, data);

            mydelete(data);
            msglog = irlist_get_next(msglog);
        }
    }

    {
        unsigned char* data;
        unsigned char* next;
        xdcc* xd;
        statefile_item_generic_int_t* g_int;
        statefile_item_generic_float_t* g_float;
        statefile_item_md5sum_info_t* md5sum_info;

        xd = irlist_get_head(&gdata.xdccs);

        while (xd) {
            /*
             * need room to write:
             *  file          string
             *  desc          string
             *  note          string
             *  gets          int
             *  minspeed      float
             *  maxspeed      float
             */
            length =
                sizeof(statefile_hdr_t) + sizeof(statefile_hdr_t) +
                ceiling(strlen(xd->file) + 1, 4) + sizeof(statefile_hdr_t) +
                ceiling(strlen(xd->desc) + 1, 4) + sizeof(statefile_hdr_t) +
                ceiling(strlen(xd->note) + 1, 4) +
                sizeof(statefile_item_generic_int_t) +
                sizeof(statefile_item_generic_float_t) +
                sizeof(statefile_item_generic_float_t);

            if (xd->has_md5sum) {
                length += ceiling(sizeof(statefile_item_md5sum_info_t), 4);
            }

            data = mycalloc(length);

            /* outer header */
            hdr = (statefile_hdr_t*)data;
            hdr->tag = STATEFILE_TAG_XDCCS;
            hdr->length = length;
            next = (unsigned char*)(&hdr[1]);

            /* file */
            hdr = (statefile_hdr_t*)next;
            hdr->tag = htonl(STATEFILE_TAG_XDCCS_FILE);
            hdr->length = htonl(sizeof(statefile_hdr_t) + strlen(xd->file) + 1);
            next = (unsigned char*)(&hdr[1]);
            strcpy((char*)next, xd->file);
            next += ceiling(strlen(xd->file) + 1, 4);

            /* desc */
            hdr = (statefile_hdr_t*)next;
            hdr->tag = htonl(STATEFILE_TAG_XDCCS_DESC);
            hdr->length = htonl(sizeof(statefile_hdr_t) + strlen(xd->desc) + 1);
            next = (unsigned char*)(&hdr[1]);
            strcpy((char*)next, xd->desc);
            next += ceiling(strlen(xd->desc) + 1, 4);

            /* note */
            hdr = (statefile_hdr_t*)next;
            hdr->tag = htonl(STATEFILE_TAG_XDCCS_NOTE);
            hdr->length = htonl(sizeof(statefile_hdr_t) + strlen(xd->note) + 1);
            next = (unsigned char*)(&hdr[1]);
            strcpy((char*)next, xd->note);
            next += ceiling(strlen(xd->note) + 1, 4);

            /* gets */
            g_int = (statefile_item_generic_int_t*)next;
            g_int->hdr.tag = htonl(STATEFILE_TAG_XDCCS_GETS);
            g_int->hdr.length = htonl(sizeof(*g_int));
            g_int->g_int = htonl(xd->gets);
            next = (unsigned char*)(&g_int[1]);

            /* minspeed */
            g_float = (statefile_item_generic_float_t*)next;
            g_float->hdr.tag = htonl(STATEFILE_TAG_XDCCS_MINSPEED);
            g_float->hdr.length = htonl(sizeof(*g_float));
            if (gdata.transferminspeed == xd->minspeed) {
                g_float->g_float = 0;
            } else {
                g_float->g_float = xd->minspeed;
            }
            next = (unsigned char*)(&g_float[1]);

            /* maxspeed */
            g_float = (statefile_item_generic_float_t*)next;
            g_float->hdr.tag = htonl(STATEFILE_TAG_XDCCS_MAXSPEED);
            g_float->hdr.length = htonl(sizeof(*g_float));
            if (gdata.transfermaxspeed == xd->maxspeed) {
                g_float->g_float = 0;
            } else {
                g_float->g_float = xd->maxspeed;
            }
            next = (unsigned char*)(&g_float[1]);

            if (xd->has_md5sum) {
                /* md5sum */
                md5sum_info = (statefile_item_md5sum_info_t*)next;
                md5sum_info->hdr.tag = htonl(STATEFILE_TAG_XDCCS_MD5SUM_INFO);
                md5sum_info->hdr.length = htonl(sizeof(*md5sum_info));
                md5sum_info->st_size.upper =
                    htonl(((uint64_t)xd->st_size) >> 32);
                md5sum_info->st_size.lower = htonl(xd->st_size & 0xFFFFFFFF);
                md5sum_info->st_dev.upper = htonl(((uint64_t)xd->st_dev) >> 32);
                md5sum_info->st_dev.lower = htonl(xd->st_dev & 0xFFFFFFFF);
                md5sum_info->st_ino.upper = htonl(((uint64_t)xd->st_ino) >> 32);
                md5sum_info->st_ino.lower = htonl(xd->st_ino & 0xFFFFFFFF);
                md5sum_info->mtime = htonl(xd->mtime);
                memcpy(md5sum_info->md5sum, xd->md5sum, sizeof(MD5Digest));
                next = (unsigned char*)(&md5sum_info[1]);
            }

            write_statefile_item(&bout, data);

            mydelete(data);
            xd = irlist_get_next(xd);
        }
    }

    {
        statefile_item_generic_ullint_t item_daily_used;
        item_daily_used.hdr.tag = STATEFILE_TAG_TLIMIT_DAILY_USED;
        item_daily_used.hdr.length = sizeof(statefile_item_generic_ullint_t);
        item_daily_used.g_ullint.upper =
            htonl(gdata.transferlimits[TRANSFERLIMIT_DAILY].used >> 32);
        item_daily_used.g_ullint.lower =
            htonl(gdata.transferlimits[TRANSFERLIMIT_DAILY].used & 0xFFFFFFFF);
        write_statefile_item(&bout, &item_daily_used);
    }
    {
        statefile_item_generic_time_t item_daily_ends;
        item_daily_ends.hdr.tag = STATEFILE_TAG_TLIMIT_DAILY_ENDS;
        item_daily_ends.hdr.length = sizeof(statefile_item_generic_time_t);
        item_daily_ends.g_time =
            htonl(gdata.transferlimits[TRANSFERLIMIT_DAILY].ends);
        write_statefile_item(&bout, &item_daily_ends);
    }

    {
        statefile_item_generic_ullint_t item_weekly_used;
        item_weekly_used.hdr.tag = STATEFILE_TAG_TLIMIT_WEEKLY_USED;
        item_weekly_used.hdr.length = sizeof(statefile_item_generic_ullint_t);
        item_weekly_used.g_ullint.upper =
            htonl(gdata.transferlimits[TRANSFERLIMIT_WEEKLY].used >> 32);
        item_weekly_used.g_ullint.lower =
            htonl(gdata.transferlimits[TRANSFERLIMIT_WEEKLY].used & 0xFFFFFFFF);
        write_statefile_item(&bout, &item_weekly_used);
    }
    {
        statefile_item_generic_time_t item_weekly_ends;
        item_weekly_ends.hdr.tag = STATEFILE_TAG_TLIMIT_WEEKLY_ENDS;
        item_weekly_ends.hdr.length = sizeof(statefile_item_generic_time_t);
        item_weekly_ends.g_time =
            htonl(gdata.transferlimits[TRANSFERLIMIT_WEEKLY].ends);
        write_statefile_item(&bout, &item_weekly_ends);
    }

    {
        statefile_item_generic_ullint_t item_monthly_used;
        item_monthly_used.hdr.tag = STATEFILE_TAG_TLIMIT_MONTHLY_USED;
        item_monthly_used.hdr.length = sizeof(statefile_item_generic_ullint_t);
        item_monthly_used.g_ullint.upper =
            htonl(gdata.transferlimits[TRANSFERLIMIT_MONTHLY].used >> 32);
        item_monthly_used.g_ullint.lower = htonl(
            gdata.transferlimits[TRANSFERLIMIT_MONTHLY].used & 0xFFFFFFFF);
        write_statefile_item(&bout, &item_monthly_used);
    }
    {
        statefile_item_generic_time_t item_monthly_ends;
        item_monthly_ends.hdr.tag = STATEFILE_TAG_TLIMIT_MONTHLY_ENDS;
        item_monthly_ends.hdr.length = sizeof(statefile_item_generic_time_t);
        item_monthly_ends.g_time =
            htonl(gdata.transferlimits[TRANSFERLIMIT_MONTHLY].ends);
        write_statefile_item(&bout, &item_monthly_ends);
    }

    {
        MD5Digest digest = {};

        ir_boutput_get_md5sum(&bout, digest);

        callval = ir_boutput_write(&bout, &digest, sizeof(digest));
        if (callval != sizeof(digest)) {
            outerror(OUTERROR_TYPE_WARN_LOUD,
                     "Cant Write md5sum To State File (%d != %d) %s", callval,
                     (int)sizeof(digest), strerror(errno));
        }
    }

    /*** end write ***/

    ir_boutput_set_flags(&bout, 0);

    callval = ir_boutput_attempt_flush(&bout);
    if (callval < 0) {
        outerror(OUTERROR_TYPE_WARN_LOUD, "Cant Write to State File: %s",
                 strerror(errno));
    }

    if (bout.count_dropped || (bout.count_written != bout.count_flushed)) {
        outerror(OUTERROR_TYPE_WARN_LOUD,
                 "Write failed to State File: %d/%d/%d", bout.count_written,
                 bout.count_flushed, bout.count_dropped);
    }

    ir_boutput_delete(&bout);
    close(fd);

    /* remove old bkup */
    callval = unlink(statefile_bkup);
    if ((callval < 0) && (errno != ENOENT)) {
        outerror(OUTERROR_TYPE_WARN_LOUD, "Cant Remove Old State File '%s': %s",
                 statefile_bkup, strerror(errno));
        /* ignore, continue */
    }

    /* backup old -> bkup */
    callval = link(gdata.statefile, statefile_bkup);
    if ((callval < 0) && (errno != ENOENT)) {
        outerror(OUTERROR_TYPE_WARN_LOUD,
                 "Cant Backup Old State File '%s' -> '%s': %s", gdata.statefile,
                 statefile_bkup, strerror(errno));
        /* ignore, continue */
    }

    /* rename new -> current */
    callval = rename(statefile_tmp, gdata.statefile);
    if (callval < 0) {
        outerror(OUTERROR_TYPE_WARN_LOUD, "Cant Save New State File '%s': %s",
                 gdata.statefile, strerror(errno));
        /* ignore, continue */
    }

    if (gdata.debug > 0) {
        ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_NO_COLOR, "Done");
    }

error_out:

    mydelete(statefile_tmp);
    mydelete(statefile_bkup);
}


static statefile_hdr_t* read_statefile_item(uint32_t** buffer,
                                            int* buffer_len) {
    statefile_hdr_t* all;

    if (*buffer_len < sizeof(statefile_hdr_t)) {
        return NULL;
    }

    all = (statefile_hdr_t*)*buffer;

    all->tag = ntohl(all->tag);
    all->length = ntohl(all->length);

    if (*buffer_len < all->length) {
        outerror(OUTERROR_TYPE_WARN_LOUD,
                 "Bad Header From State File (%d < %d)", *buffer_len,
                 all->length);
        return NULL;
    }

    *buffer += ceiling(all->length, 4) / sizeof(uint32_t);
    *buffer_len -= ceiling(all->length, 4);

    return all;
}


void read_statefile(void) {
    int fd;
    uint32_t *buffer, *buffer_begin;
    int buffer_len;
    MD5_CTX md5sum;
    MD5Digest digest;
    struct stat st;
    statefile_hdr_t* hdr;
    int callval;
    time_t timestamp = 0;

    updatecontext();

    if (gdata.statefile == NULL) {
        return;
    }

    ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_NO_COLOR,
            "Loading State File... ");

    fd = open(gdata.statefile, O_RDONLY | O_CREAT,
              CREAT_PERMISSIONS);

    if (fd < 0) {
        outerror(OUTERROR_TYPE_WARN_LOUD, "Cant Access State File '%s': %s",
                 gdata.statefile, strerror(errno));
        return;
    }

    if ((fstat(fd, &st) < 0) ||
        (st.st_size < ((sizeof(uint32_t) * 2) + sizeof(MD5Digest)))) {
        ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_NO_COLOR,
                "State File: Too small, Skipping");
        close(fd);
        return;
    }

    buffer_len = st.st_size;
    buffer_begin = buffer = mycalloc(buffer_len);

    callval = read(fd, buffer, buffer_len);
    close(fd);
    if (callval != buffer_len) {
        outerror(OUTERROR_TYPE_WARN_LOUD, "Cant Read State File (%d != %d) %s",
                 callval, buffer_len, strerror(errno));
        goto error_out;
    }

    /* verify md5sum */
    buffer_len -= sizeof(MD5Digest);

    MD5_Init(&md5sum);
    MD5_Update(&md5sum, (uint8_t*)buffer, buffer_len);
    MD5_Final(digest, &md5sum);

    if (memcmp(digest, buffer + (buffer_len / sizeof(uint32_t)),
               sizeof(MD5Digest))) {
        outerror(OUTERROR_TYPE_CRASH,
                 "\"%s\" Appears corrupt or is not an iroffer state file",
                 gdata.statefile);
        goto error_out;
    }

    /* read */

    if (ntohl(*buffer) != STATEFILE_MAGIC) {
        outerror(OUTERROR_TYPE_CRASH,
                 "\"%s\" Does not appear to be an iroffer state file",
                 gdata.statefile);
        goto error_out;
    }
    buffer++;
    buffer_len -= sizeof(uint32_t);

    if (gdata.debug > 0) {
        ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_NO_COLOR,
                "  [Version %lu State File]", (unsigned long)ntohl(*buffer));
    }
    buffer++;
    buffer_len -= sizeof(uint32_t);

    while ((hdr = read_statefile_item(&buffer, &buffer_len))) {
        switch (hdr->tag) {
        case STATEFILE_TAG_TIMESTAMP:
            if (hdr->length == sizeof(statefile_item_generic_time_t)) {
                char* tempstr;
                statefile_item_generic_time_t* g_time =
                    (statefile_item_generic_time_t*)hdr;
                timestamp = ntohl(g_time->g_time);

                tempstr = mycalloc(maxtextlength);
                getdatestr(tempstr, timestamp, maxtextlength);

                ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_NO_COLOR,
                        "  [Written on %s]", tempstr);
                mydelete(tempstr);
            } else {
                outerror(OUTERROR_TYPE_WARN,
                         "Ignoring Bad Timestamp Tag (len = %d)", hdr->length);
            }
            break;

        case STATEFILE_TAG_XFR_RECORD:
            if (hdr->length == sizeof(statefile_item_generic_float_t)) {
                statefile_item_generic_float_t* g_float =
                    (statefile_item_generic_float_t*)hdr;
                gdata.record = g_float->g_float;

                if (gdata.debug > 0) {
                    ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D,
                            COLOR_NO_COLOR, "  [Record %1.1fKB/s]",
                            gdata.record);
                }
            } else {
                outerror(OUTERROR_TYPE_WARN,
                         "Ignoring Bad xfr Record Tag (len = %d)", hdr->length);
            }
            break;

        case STATEFILE_TAG_SENT_RECORD:
            if (hdr->length == sizeof(statefile_item_generic_float_t)) {
                statefile_item_generic_float_t* g_float =
                    (statefile_item_generic_float_t*)hdr;
                gdata.sentrecord = g_float->g_float;

                if (gdata.debug > 0) {
                    ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D,
                            COLOR_NO_COLOR, "  [Bandwidth Record %1.1fKB/s]",
                            gdata.sentrecord);
                }
            } else {
                outerror(OUTERROR_TYPE_WARN,
                         "Ignoring Bad sent Record Tag (len = %d)",
                         hdr->length);
            }
            break;

        case STATEFILE_TAG_TOTAL_SENT:
            if (hdr->length == sizeof(statefile_item_generic_ullint_t)) {
                statefile_item_generic_ullint_t* g_ullint =
                    (statefile_item_generic_ullint_t*)hdr;
                gdata.totalsent =
                    (((uint64_t)ntohl(g_ullint->g_ullint.upper)) << 32) |
                    ((uint64_t)ntohl(g_ullint->g_ullint.lower));

                if (gdata.debug > 0) {
                    ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D,
                            COLOR_NO_COLOR, "  [Total Transferred %1.2f %cB]",
                            (gdata.totalsent / 1024 / 1024) > 1024
                                ? ((gdata.totalsent / 1024 / 1024 / 1024) > 1024
                                       ? ((float)gdata.totalsent) / 1024 /
                                             1024 / 1024 / 1024
                                       : ((float)gdata.totalsent) / 1024 /
                                             1024 / 1024)
                                : ((float)gdata.totalsent) / 1024 / 1024,
                            (gdata.totalsent / 1024 / 1024) > 1024
                                ? ((gdata.totalsent / 1024 / 1024 / 1024) > 1024
                                       ? 'T'
                                       : 'G')
                                : 'M');
                }
            } else {
                outerror(OUTERROR_TYPE_WARN,
                         "Ignoring Bad sent Record Tag (len = %d)",
                         hdr->length);
            }
            break;

        case STATEFILE_TAG_TOTAL_UPTIME:
            if (hdr->length == sizeof(statefile_item_generic_int_t)) {
                char* tempstr;
                statefile_item_generic_int_t* g_int =
                    (statefile_item_generic_int_t*)hdr;
                gdata.totaluptime = ntohl(g_int->g_int);

                if (gdata.debug > 0) {
                    tempstr = mycalloc(maxtextlength);
                    getuptime(tempstr, 0, gdata.curtime - gdata.totaluptime,
                              maxtextlength);
                    ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D,
                            COLOR_NO_COLOR, "  [Total Runtime %s]", tempstr);
                    mydelete(tempstr);
                }
            } else {
                outerror(OUTERROR_TYPE_WARN,
                         "Ignoring Bad sent Record Tag (len = %d)",
                         hdr->length);
            }
            break;

        case STATEFILE_TAG_LAST_LOGROTATE:
            if (hdr->length == sizeof(statefile_item_generic_time_t)) {
                char* tempstr;
                statefile_item_generic_time_t* g_time =
                    (statefile_item_generic_time_t*)hdr;
                gdata.last_logrotate = ntohl(g_time->g_time);

                if (gdata.debug > 0) {
                    tempstr = mycalloc(maxtextlength);
                    getdatestr(tempstr, gdata.last_logrotate, maxtextlength);

                    ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D,
                            COLOR_NO_COLOR, "  [Last Log Rotate %s]", tempstr);
                    mydelete(tempstr);
                }
            } else {
                outerror(OUTERROR_TYPE_WARN,
                         "Ignoring Bad Last Log Rotate Tag (len = %d)",
                         hdr->length);
            }
            break;

        case STATEFILE_TAG_IROFFER_VERSION:
            if (hdr->length > sizeof(statefile_hdr_t)) {
                char* iroffer_version = (char*)(&hdr[1]);
                iroffer_version[hdr->length - sizeof(statefile_hdr_t) - 1] =
                    '\0';

                if (gdata.debug > 0) {
                    ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D,
                            COLOR_NO_COLOR, "  [Written by %s]",
                            iroffer_version);
                }
            } else {
                outerror(OUTERROR_TYPE_WARN,
                         "Ignoring Bad Iroffer Version Tag (len = %d)",
                         hdr->length);
            }
            break;

        case STATEFILE_TAG_IGNORE: {
            igninfo* ignore;
            char* tempr;
            statefile_hdr_t* ihdr;

            ignore = irlist_add(&gdata.ignorelist, sizeof(igninfo));

            hdr->length -= sizeof(*hdr);
            ihdr = &hdr[1];

            while (hdr->length >= sizeof(*hdr)) {
                ihdr->tag = ntohl(ihdr->tag);
                ihdr->length = ntohl(ihdr->length);
                switch (ihdr->tag) {
                case STATEFILE_TAG_IGNORE_FLAGS:
                    if (ihdr->length == sizeof(statefile_item_generic_uint_t)) {
                        statefile_item_generic_uint_t* g_uint =
                            (statefile_item_generic_uint_t*)ihdr;
                        ignore->flags = ntohl(g_uint->g_uint);
                    } else {
                        outerror(OUTERROR_TYPE_WARN,
                                 "Ignoring Bad Ignore Flags Tag (len = %d)",
                                 ihdr->length);
                    }
                    break;

                case STATEFILE_TAG_IGNORE_BUCKET:
                    if (ihdr->length == sizeof(statefile_item_generic_int_t)) {
                        statefile_item_generic_int_t* g_int =
                            (statefile_item_generic_int_t*)ihdr;
                        ignore->bucket = ntohl(g_int->g_int);
                    } else {
                        outerror(OUTERROR_TYPE_WARN,
                                 "Ignoring Bad Ignore Bucket Tag (len = %d)",
                                 ihdr->length);
                    }
                    break;

                case STATEFILE_TAG_IGNORE_LASTCONTACT:
                    if (ihdr->length == sizeof(statefile_item_generic_time_t)) {
                        statefile_item_generic_time_t* g_time =
                            (statefile_item_generic_time_t*)ihdr;
                        ignore->lastcontact = ntohl(g_time->g_time);
                    } else {
                        outerror(
                            OUTERROR_TYPE_WARN,
                            "Ignoring Bad Ignore Lastcontact Tag (len = %d)",
                            ihdr->length);
                    }
                    break;

                case STATEFILE_TAG_IGNORE_HOSTMASK:
                    if (ihdr->length > sizeof(statefile_hdr_t)) {
                        char* hostmask = (char*)(&ihdr[1]);
                        hostmask[ihdr->length - sizeof(statefile_hdr_t) - 1] =
                            '\0';
                        ignore->hostmask = mymalloc(strlen(hostmask) + 1);
                        strcpy(ignore->hostmask, hostmask);
                    } else {
                        outerror(OUTERROR_TYPE_WARN,
                                 "Ignoring Bad Ignore Hostmask Tag (len = %d)",
                                 ihdr->length);
                    }
                    break;

                default:
                    outerror(OUTERROR_TYPE_WARN,
                             "Ignoring Unknown Ignore Tag 0x%X (len=%d)",
                             ihdr->tag, ihdr->length);
                }
                hdr->length -= ceiling(ihdr->length, 4);
                ihdr = (statefile_hdr_t*)(((char*)ihdr) +
                                          ceiling(ihdr->length, 4));
            }

            if ((!ignore->lastcontact) || (!ignore->hostmask)) {
                outerror(OUTERROR_TYPE_WARN, "Ignoring Incomplete Ignore Tag");

                mydelete(ignore->hostmask);
                irlist_delete(&gdata.ignorelist, ignore);
            } else {
                ignore->bucket -=
                    (gdata.curtime - timestamp) / gdata.autoignore_threshold;

                ignore->regexp = mycalloc(sizeof(regex_t));
                tempr = hostmasktoregex(ignore->hostmask);
                if (regcomp(ignore->regexp, tempr, REG_ICASE | REG_NOSUB)) {
                    ignore->regexp = NULL;
                }
                mydelete(tempr);
            }
        }

        break;

        case STATEFILE_TAG_MSGLOG: {
            msglog_t* msglog;
            statefile_hdr_t* ihdr;

            msglog = irlist_add(&gdata.msglog, sizeof(msglog_t));

            hdr->length -= sizeof(*hdr);
            ihdr = &hdr[1];

            while (hdr->length >= sizeof(*hdr)) {
                ihdr->tag = ntohl(ihdr->tag);
                ihdr->length = ntohl(ihdr->length);
                switch (ihdr->tag) {
                case STATEFILE_TAG_MSGLOG_WHEN:
                    if (ihdr->length == sizeof(statefile_item_generic_time_t)) {
                        statefile_item_generic_time_t* g_time =
                            (statefile_item_generic_time_t*)ihdr;
                        msglog->when = ntohl(g_time->g_time);
                    } else {
                        outerror(OUTERROR_TYPE_WARN,
                                 "Ignoring Bad Msglog When Tag (len = %d)",
                                 ihdr->length);
                    }
                    break;

                case STATEFILE_TAG_MSGLOG_HOSTMASK:
                    if (ihdr->length > sizeof(statefile_hdr_t)) {
                        char* hostmask = (char*)(&ihdr[1]);
                        hostmask[ihdr->length - sizeof(statefile_hdr_t) - 1] =
                            '\0';
                        msglog->hostmask = mymalloc(strlen(hostmask) + 1);
                        strcpy(msglog->hostmask, hostmask);
                    } else {
                        outerror(OUTERROR_TYPE_WARN,
                                 "Ignoring Bad Msglog Hostmask Tag (len = %d)",
                                 ihdr->length);
                    }
                    break;

                case STATEFILE_TAG_MSGLOG_MESSAGE:
                    if (ihdr->length > sizeof(statefile_hdr_t)) {
                        char* message = (char*)(&ihdr[1]);
                        message[ihdr->length - sizeof(statefile_hdr_t) - 1] =
                            '\0';
                        msglog->message = mymalloc(strlen(message) + 1);
                        strcpy(msglog->message, message);
                    } else {
                        outerror(OUTERROR_TYPE_WARN,
                                 "Ignoring Bad Msglog Message Tag (len = %d)",
                                 ihdr->length);
                    }
                    break;

                default:
                    outerror(OUTERROR_TYPE_WARN,
                             "Ignoring Unknown Msglog Tag 0x%X (len=%d)",
                             ihdr->tag, ihdr->length);
                }
                hdr->length -= ceiling(ihdr->length, 4);
                ihdr = (statefile_hdr_t*)(((char*)ihdr) +
                                          ceiling(ihdr->length, 4));
            }

            if ((!msglog->when) || (!msglog->hostmask) || (!msglog->message)) {
                outerror(OUTERROR_TYPE_WARN, "Ignoring Incomplete Msglog Tag");

                mydelete(msglog->hostmask);
                mydelete(msglog->message);
                irlist_delete(&gdata.msglog, msglog);
            }
        }

        break;

        case STATEFILE_TAG_XDCCS: {
            xdcc* xd;
            statefile_hdr_t* ihdr;

            xd = irlist_add(&gdata.xdccs, sizeof(xdcc));

            xd->minspeed = gdata.transferminspeed;
            xd->maxspeed = gdata.transfermaxspeed;

            hdr->length -= sizeof(*hdr);
            ihdr = &hdr[1];

            while (hdr->length >= sizeof(*hdr)) {
                ihdr->tag = ntohl(ihdr->tag);
                ihdr->length = ntohl(ihdr->length);
                switch (ihdr->tag) {
                case STATEFILE_TAG_XDCCS_FILE:
                    if (ihdr->length > sizeof(statefile_hdr_t)) {
                        char* file = (char*)(&ihdr[1]);
                        file[ihdr->length - sizeof(statefile_hdr_t) - 1] = '\0';
                        xd->file = mymalloc(strlen(file) + 1);
                        strcpy(xd->file, file);
                    } else {
                        outerror(OUTERROR_TYPE_WARN,
                                 "Ignoring Bad XDCC File Tag (len = %d)",
                                 ihdr->length);
                    }
                    break;

                case STATEFILE_TAG_XDCCS_DESC:
                    if (ihdr->length > sizeof(statefile_hdr_t)) {
                        char* desc = (char*)(&ihdr[1]);
                        desc[ihdr->length - sizeof(statefile_hdr_t) - 1] = '\0';
                        xd->desc = mymalloc(strlen(desc) + 1);
                        strcpy(xd->desc, desc);
                    } else {
                        outerror(OUTERROR_TYPE_WARN,
                                 "Ignoring Bad XDCC Desc Tag (len = %d)",
                                 ihdr->length);
                    }
                    break;

                case STATEFILE_TAG_XDCCS_NOTE:
                    if (ihdr->length > sizeof(statefile_hdr_t)) {
                        char* note = (char*)(&ihdr[1]);
                        note[ihdr->length - sizeof(statefile_hdr_t) - 1] = '\0';
                        xd->note = mymalloc(strlen(note) + 1);
                        strcpy(xd->note, note);
                    } else {
                        outerror(OUTERROR_TYPE_WARN,
                                 "Ignoring Bad XDCC Note Tag (len = %d)",
                                 ihdr->length);
                    }
                    break;

                case STATEFILE_TAG_XDCCS_GETS:
                    if (ihdr->length == sizeof(statefile_item_generic_int_t)) {
                        statefile_item_generic_int_t* g_int =
                            (statefile_item_generic_int_t*)ihdr;
                        xd->gets = ntohl(g_int->g_int);
                    } else {
                        outerror(OUTERROR_TYPE_WARN,
                                 "Ignoring Bad XDCC Gets Tag (len = %d)",
                                 ihdr->length);
                    }
                    break;

                case STATEFILE_TAG_XDCCS_MINSPEED:
                    if (ihdr->length ==
                        sizeof(statefile_item_generic_float_t)) {
                        statefile_item_generic_float_t* g_float =
                            (statefile_item_generic_float_t*)ihdr;
                        if (g_float->g_float) {
                            xd->minspeed = g_float->g_float;
                        }
                    } else {
                        outerror(OUTERROR_TYPE_WARN,
                                 "Ignoring Bad XDCC Minspeed Tag (len = %d)",
                                 ihdr->length);
                    }
                    break;

                case STATEFILE_TAG_XDCCS_MAXSPEED:
                    if (ihdr->length ==
                        sizeof(statefile_item_generic_float_t)) {
                        statefile_item_generic_float_t* g_float =
                            (statefile_item_generic_float_t*)ihdr;
                        if (g_float->g_float) {
                            xd->maxspeed = g_float->g_float;
                        }
                    } else {
                        outerror(OUTERROR_TYPE_WARN,
                                 "Ignoring Bad XDCC Maxspeed Tag (len = %d)",
                                 ihdr->length);
                    }
                    break;

                case STATEFILE_TAG_XDCCS_MD5SUM_INFO:
                    if (ihdr->length == sizeof(statefile_item_md5sum_info_t)) {
                        statefile_item_md5sum_info_t* md5sum_info =
                            (statefile_item_md5sum_info_t*)ihdr;
                        xd->has_md5sum = 1;

                        xd->st_size = (off_t)(
                            (((uint64_t)ntohl(md5sum_info->st_size.upper))
                             << 32) |
                            ((uint64_t)ntohl(md5sum_info->st_size.lower)));
                        xd->st_dev = (dev_t)(
                            (((uint64_t)ntohl(md5sum_info->st_dev.upper))
                             << 32) |
                            ((uint64_t)ntohl(md5sum_info->st_dev.lower)));
                        xd->st_ino = (ino_t)(
                            (((uint64_t)ntohl(md5sum_info->st_ino.upper))
                             << 32) |
                            ((uint64_t)ntohl(md5sum_info->st_ino.lower)));
                        xd->mtime = ntohl(md5sum_info->mtime);
                        memcpy(xd->md5sum, md5sum_info->md5sum,
                               sizeof(MD5Digest));
                    } else {
                        outerror(OUTERROR_TYPE_WARN,
                                 "Ignoring Bad XDCC md5sum Tag (len = %d)",
                                 ihdr->length);
                    }
                    break;

                default:
                    outerror(OUTERROR_TYPE_WARN,
                             "Ignoring Unknown XDCC Tag 0x%X (len=%d)",
                             ihdr->tag, ihdr->length);
                }
                hdr->length -= ceiling(ihdr->length, 4);
                ihdr = (statefile_hdr_t*)(((char*)ihdr) +
                                          ceiling(ihdr->length, 4));
            }

            if ((!xd->file) || (!xd->desc) || (!xd->note)) {
                outerror(OUTERROR_TYPE_WARN, "Ignoring Incomplete XDCC Tag");

                mydelete(xd->file);
                mydelete(xd->desc);
                mydelete(xd->note);
                irlist_delete(&gdata.xdccs, xd);
            } else {
                int xfd;
                xfd = open(xd->file, O_RDONLY);
                if (xfd < 0) {
                    outerror(OUTERROR_TYPE_WARN,
                             "Cant Access Offered File '%s': %s", xd->file,
                             strerror(errno));
                    memset(&st, 0, sizeof(st));
                } else if (fstat(xfd, &st) < 0) {
                    outerror(OUTERROR_TYPE_WARN,
                             "Cant Access Offered File Details '%s': %s",
                             xd->file, strerror(errno));
                    memset(&st, 0, sizeof(st));
                }

                if (!xd->has_md5sum || (xd->st_dev != st.st_dev) ||
                    (xd->st_ino != st.st_ino) || (xd->mtime != st.st_mtime) ||
                    (xd->st_size != st.st_size)) {
                    xd->st_size = st.st_size;
                    xd->st_dev = st.st_dev;
                    xd->st_ino = st.st_ino;
                    xd->mtime = st.st_mtime;
                    xd->has_md5sum = 0;
                    memset(xd->md5sum, 0, sizeof(MD5Digest));
                }

                if (xd->st_size == 0) {
                    outerror(OUTERROR_TYPE_WARN,
                             "The file \"%s\" has size of 0 bytes!", xd->file);
                }

                if (xd->st_size > gdata.max_file_size) {
                    outerror(OUTERROR_TYPE_CRASH,
                             "The file \"%s\" is too large!", xd->file);
                }

                if (xfd >= 0) {
                    close(xfd);
                }
            }
        }

        break;

        case STATEFILE_TAG_TLIMIT_DAILY_USED:
            if (hdr->length == sizeof(statefile_item_generic_ullint_t)) {
                statefile_item_generic_ullint_t* g_ullint =
                    (statefile_item_generic_ullint_t*)hdr;
                gdata.transferlimits[TRANSFERLIMIT_DAILY].used =
                    (((uint64_t)ntohl(g_ullint->g_ullint.upper)) << 32) |
                    ((uint64_t)ntohl(g_ullint->g_ullint.lower));

                if (gdata.debug > 0) {
                    ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D,
                            COLOR_NO_COLOR,
                            "  [Daily Transfer Limit Used %" PRIu64 "uMB]",
                            gdata.transferlimits[TRANSFERLIMIT_DAILY].used /
                                1024 / 1024);
                }
            } else {
                outerror(
                    OUTERROR_TYPE_WARN,
                    "Ignoring Bad Daily Transfer Limit Used Tag (len = %d)",
                    hdr->length);
            }
            break;

        case STATEFILE_TAG_TLIMIT_DAILY_ENDS:
            if (hdr->length == sizeof(statefile_item_generic_time_t)) {
                char* tempstr;
                statefile_item_generic_time_t* g_time =
                    (statefile_item_generic_time_t*)hdr;
                gdata.transferlimits[TRANSFERLIMIT_DAILY].ends =
                    ntohl(g_time->g_time);

                if (gdata.debug > 0) {
                    tempstr = mycalloc(maxtextlength);
                    getdatestr(tempstr,
                               gdata.transferlimits[TRANSFERLIMIT_DAILY].ends,
                               maxtextlength);

                    ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D,
                            COLOR_NO_COLOR, "  [Daily Transfer Limit Ends %s]",
                            tempstr);
                    mydelete(tempstr);
                }
            } else {
                outerror(
                    OUTERROR_TYPE_WARN,
                    "Ignoring Bad Daily Transfer Limit Ends Tag (len = %d)",
                    hdr->length);
            }
            break;

        case STATEFILE_TAG_TLIMIT_WEEKLY_USED:
            if (hdr->length == sizeof(statefile_item_generic_ullint_t)) {
                statefile_item_generic_ullint_t* g_ullint =
                    (statefile_item_generic_ullint_t*)hdr;
                gdata.transferlimits[TRANSFERLIMIT_WEEKLY].used =
                    (((uint64_t)ntohl(g_ullint->g_ullint.upper)) << 32) |
                    ((uint64_t)ntohl(g_ullint->g_ullint.lower));

                if (gdata.debug > 0) {
                    ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D,
                            COLOR_NO_COLOR,
                            "  [Weekly Transfer Limit Used %" PRIu64 "uMB]",
                            gdata.transferlimits[TRANSFERLIMIT_WEEKLY].used /
                                1024 / 1024);
                }
            } else {
                outerror(
                    OUTERROR_TYPE_WARN,
                    "Ignoring Bad Weekly Transfer Limit Used Tag (len = %d)",
                    hdr->length);
            }
            break;

        case STATEFILE_TAG_TLIMIT_WEEKLY_ENDS:
            if (hdr->length == sizeof(statefile_item_generic_time_t)) {
                char* tempstr;
                statefile_item_generic_time_t* g_time =
                    (statefile_item_generic_time_t*)hdr;
                gdata.transferlimits[TRANSFERLIMIT_WEEKLY].ends =
                    ntohl(g_time->g_time);

                if (gdata.debug > 0) {
                    tempstr = mycalloc(maxtextlength);
                    getdatestr(tempstr,
                               gdata.transferlimits[TRANSFERLIMIT_WEEKLY].ends,
                               maxtextlength);

                    ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D,
                            COLOR_NO_COLOR, "  [Weekly Transfer Limit Ends %s]",
                            tempstr);
                    mydelete(tempstr);
                }
            } else {
                outerror(
                    OUTERROR_TYPE_WARN,
                    "Ignoring Bad Weekly Transfer Limit Ends Tag (len = %d)",
                    hdr->length);
            }
            break;

        case STATEFILE_TAG_TLIMIT_MONTHLY_USED:
            if (hdr->length == sizeof(statefile_item_generic_ullint_t)) {
                statefile_item_generic_ullint_t* g_ullint =
                    (statefile_item_generic_ullint_t*)hdr;
                gdata.transferlimits[TRANSFERLIMIT_MONTHLY].used =
                    (((uint64_t)ntohl(g_ullint->g_ullint.upper)) << 32) |
                    ((uint64_t)ntohl(g_ullint->g_ullint.lower));

                if (gdata.debug > 0) {
                    ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D,
                            COLOR_NO_COLOR,
                            "  [Monthly Transfer Limit Used %" PRIu64 "uMB]",
                            gdata.transferlimits[TRANSFERLIMIT_MONTHLY].used /
                                1024 / 1024);
                }
            } else {
                outerror(
                    OUTERROR_TYPE_WARN,
                    "Ignoring Bad Monthly Transfer Limit Used Tag (len = %d)",
                    hdr->length);
            }
            break;

        case STATEFILE_TAG_TLIMIT_MONTHLY_ENDS:
            if (hdr->length == sizeof(statefile_item_generic_time_t)) {
                char* tempstr;
                statefile_item_generic_time_t* g_time =
                    (statefile_item_generic_time_t*)hdr;
                gdata.transferlimits[TRANSFERLIMIT_MONTHLY].ends =
                    ntohl(g_time->g_time);

                if (gdata.debug > 0) {
                    tempstr = mycalloc(maxtextlength);
                    getdatestr(tempstr,
                               gdata.transferlimits[TRANSFERLIMIT_MONTHLY].ends,
                               maxtextlength);

                    ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D,
                            COLOR_NO_COLOR,
                            "  [Monthly Transfer Limit Ends %s]", tempstr);
                    mydelete(tempstr);
                }
            } else {
                outerror(
                    OUTERROR_TYPE_WARN,
                    "Ignoring Bad Monthly Transfer Limit Ends Tag (len = %d)",
                    hdr->length);
            }
            break;

        default:
            outerror(OUTERROR_TYPE_WARN, "Ignoring Unknown Tag 0x%X (len=%d)",
                     hdr->tag, hdr->length);
            break;
        }
    }

    if (buffer_len) {
        outerror(OUTERROR_TYPE_WARN,
                 "Extra data at end of state file!? %d left", buffer_len);
    }

    /* end read */

    if ((gdata.debug > 0) || irlist_size(&gdata.ignorelist)) {
        ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_NO_COLOR,
                "  [Found %d Ignore%s]", irlist_size(&gdata.ignorelist),
                (irlist_size(&gdata.ignorelist) == 1) ? "" : "s");
    }

    if ((gdata.debug > 0) || irlist_size(&gdata.msglog)) {
        ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_NO_COLOR,
                "  [Found %d Message%s]", irlist_size(&gdata.msglog),
                (irlist_size(&gdata.msglog) == 1) ? "" : "s");
    }

    if ((gdata.debug > 0) || irlist_size(&gdata.xdccs)) {
        ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_NO_COLOR,
                "  [Found %d Pack%s]", irlist_size(&gdata.xdccs),
                (irlist_size(&gdata.xdccs) == 1) ? "" : "s");
    }

    ioutput(CALLTYPE_NORMAL, OUT_S | OUT_L | OUT_D, COLOR_NO_COLOR, "  [Done]");

error_out:

    mydelete(buffer_begin);
}
