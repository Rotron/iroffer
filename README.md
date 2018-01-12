# iroffer

iroffer is a software program that acts as a fileserver for IRC. It is similar to a FTP server or WEB server, but users can download files using the DCC protocol of IRC instead of a web browser.

## Why use iroffer?

- extremely fast dcc transfers
- extremely low cpu usage, and reasonable ram usage
- its a program not a script that is slowed by a bulky irc program
- only completed transfers are counted
- supports dcc resume
- set max amount of transfers per hostname
- user friendly error messages for users (no "clamp timeout")
- allows a pack to be designated as a "high demand" pack which can have special limitations and its own separate queue
- supports virtual hosts
- auto-send feature, send a pack to someone when they say something
- auto-saves xdcc information
- remote administration via /msg or DCC CHAT
- bandwidth monitoring, shows last 2 minutes bandwidth average
- Allow sending of queued packs when using low amounts of bandwidth, comes in handy when all slots are filled with people transferring 1k/sec, will keep sending out queued items while bandwidth usage is under a specified amount
- Background or Foreground mode. background mode does not require screen and is cronable
- Chroot support (run iroffer from inside a chroot'ed environment)
- overall and pack minspeed
- maximum bandwidth limiting, when set, iroffer will not use more than the set amount of bandwidth (keeps your sysadmin happy)
- can set different maximum bandwidth limits depending on time of day and day of week (keeps your sysadmin very happy)
- logging
- auto-ignores flooders
- support for direct, bnc, wingate, and custom proxy irc server connections
- ignore list

## Getting Started

### Supported Platforms

- Linux
- FreeBSD/OpenBSD/NetBSD
- macOS
- Windows (through Cygwin)

### Prerequisites

- crypt
- nss_files
- nss_dns

### Building

Run the *Configure* script followed by the generated *Makefile*.

No errors or warnings should appear when compiling.

A sample config file is provided.

### How to let colors work while using screen

create a file in your home directory named ".screenrc", and put the
following lines in it:

```sh
termcap  vt100 'AF=\E[3%dm:AB=\E[4%dm'
terminfo vt100 'AF=\E[3%p1%dm:AB=\E[4%p1%dm'
```

### How To Use Cron

Edit the *iroffer.cron* file's *iroffer_dir*, *iroffer_exec*, and
*iroffer_pid* variables

then crontab -e and place the following line in the editor

\*/5 \* \* \* \* /full/path/to/iroffer/iroffer.cron

### Signal Handling

*iroffer* will handle the following signals:

- SIGUSR1 (kill -USR1 xxxx)  jumps to another server (same as admin command "jump")
- SIGUSR2 (kill -USR2 xxxx)  re-reads config file    (same as admin command "rehash")
- SIGTERM (kill xxxx)        shuts down iroffer      (same as admin command "shutdown")

## Acknowledgments

*iroffer* was originally written by PMG and the projects former home was iroffer.org
