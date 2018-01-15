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

#ifndef IROFFER_GLOBALS_H
#define IROFFER_GLOBALS_H

#if !defined GEX
#define GEX extern
#endif


typedef struct {
    /* config */
    connectionmethod_t connectionmethod;
    char* configfile[MAXCONFIG];
    char* osstring;
    int hideos;
    int lognotices;
    int logmessages;
    int timestampconsole;
    long startuptime;
    unsigned long local_vhost;
    int lowbdwth;
    int logstats;
    char* logfile;
    time_t logrotate;
    char* headline;
    char* creditline;
    int background;
    unsigned long ourip;
    int usenatip;

    struct {
        int pack;
        char* word;
        char* message;
    } autosend;

    struct {
        uint64_t limit;
        uint64_t used;
        time_t ends;
    } transferlimits[NUMBER_TRANSFERLIMITS];
    int transferlimits_over;

    char* pidfile;
    irlist_t proxyinfo;
    int tcprangestart;
    float transferminspeed, transfermaxspeed;
    int overallmaxspeed, overallmaxspeeddayspeed;
    int maxb;
    int overallmaxspeeddaytimestart, overallmaxspeeddaytimeend;
    char overallmaxspeeddaydays;
    int maxtransfersperperson, maxqueueditemsperperson;
    irlist_t downloadhost;
    char* adminpass;
    irlist_t adminhost;
    char* filedir;
    char* statefile;
    char* xdcclistfile;
    int xdcclistfileraw;
    char *periodicmsg_nick, *periodicmsg_msg;
    int periodicmsg_time;
    char* uploaddir;
    off_t uploadmaxsize;
    irlist_t uploadhost;
    char* config_nick;
    char *user_nick, *caps_nick;
    char *user_realname, *user_modes, *loginname;
    int restrictlist, restrictsend, restrictprivlist;
    char* restrictprivlistmsg;
    int punishslowusers;
    int nomd5sum;
    char* nickserv_pass;
    int notifytime;
    int respondtochannelxdcc;
    int respondtochannellist;
    int quietmode;
    int smallfilebypass;
    irlist_t autoignore_exclude;
    int autoignore_threshold;

    /* raw on join */
    irlist_t server_join_raw;
    irlist_t server_connected_raw;
    irlist_t channel_join_raw;

    /* rehash temp variables */
    irlist_t r_channels;
    unsigned long r_local_vhost;
    char* r_pidfile;
    char* r_config_nick;
    float r_transferminspeed, r_transfermaxspeed;

    /* server */
    irlist_t servers;
    server_t curserver;
    char* curserveractualname;
    int nocon;
    int servertime;

    struct {
        char* to_ip;
        unsigned short to_port;
        int sp_fd[2];
        pid_t child_pid;
    } serv_resolv;

    enum {
        SERVERSTATUS_NEED_TO_CONNECT,
        SERVERSTATUS_RESOLVING,
        SERVERSTATUS_TRYING,
        SERVERSTATUS_CONNECTED,
    } serverstatus;
    long lastservercontact;
    irlist_t serverq_fast;
    irlist_t serverq_normal;
    irlist_t serverq_slow;
    int serverbucket;
    int ircserver;
    int serverconnectbackoff;
    prefix_t prefixes[MAX_PREFIX];
    char chanmodes[MAX_CHANMODES];

    irlist_t msglog;

    int adjustcore;

    /* screen */
    int attop, needsclear, termcols, termlines, nocolor, noscreen;
    int curcol;
    irlist_t console_history;
    int console_history_offset;
    char* console_input_line;

    struct termios startup_tio;

    int stdout_buffer_init;
    ir_boutput_t stdout_buffer;

    /* channel */
    irlist_t channels;

    irlist_t dccchats;
    int num_dccchats;

    time_t curtime;
    unsigned long long curtimems;

    fd_set readset, writeset;

    float record;
    float sentrecord;
    unsigned long long totalsent;
    long totaluptime;
    int debug;
    int exiting;
    int crashing;

    unsigned long xdccsent[XDCC_SENT_SIZE];

    int inamnt[INAMNT_SIZE];
    int ignore;

    int slotsmax;
    int recentsent;
    int queuesize;


    int noautosave;
    long nonewcons;
    long nolisting;
    int needsrehash;
    int needsshutdown;
    int needsswitch;
    int needsreap;
    int delayedshutdown;
    int cursendptr;
    int next_tr_id;

    off_t max_file_size;

    unsigned int max_fds_from_rlimit;
    int nick_number;

    int logfd;

    time_t last_logrotate;

    unsigned char* sendbuff;

    context_t context_log[MAXCONTEXTS];
    int context_cur_ptr;

    irlist_t xlistqueue;

    irlist_t ignorelist;

    irlist_t xdccs;
    irlist_t mainqueue;
    irlist_t trans;
    irlist_t uploads;

    meminfo_t* meminfo;
    int meminfo_count;
    int meminfo_depth;

#if !defined(NO_CHROOT)
    char* chrootdir;
#endif
#if !defined(NO_SETUID)
    char* runasuser;
#endif

    irlist_t listen_ports;

    struct {
        xdcc* xpack;
        int file_fd;
        MD5_CTX md5sum;
    } md5build;

    enum {
#ifdef HAVE_FREEBSD_SENDFILE
        TRANSFERMETHOD_FREEBSD_SENDFILE,
#endif
#ifdef HAVE_LINUX_SENDFILE
        TRANSFERMETHOD_LINUX_SENDFILE,
#endif
#ifdef HAVE_MMAP
        TRANSFERMETHOD_MMAP,
#endif
        TRANSFERMETHOD_READ_WRITE,
    } transfermethod;

} gdata_t;


GEX gdata_t gdata;


#endif //IROFFER_GLOBALS_H
