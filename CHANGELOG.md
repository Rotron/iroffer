# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project **does not** adhere to Semantic Versioning.

## [Unreleased]

### Changed

- New MD5 implementation
- Use stdint types instead of manually "detected" sizes + typedefs
- Refactor code to increase and modernize POSIX compatibility

### Removed

- Platform support for *SunOS/Solaris*, *HP-UX*, *IRIX/IRIX64*, *Digital UNIX*, *Mac OS X Server* and *AIX*.
- *convertxdccfile*, since the *statefile* has existed for nearly 14 years now.

### Fixed

- Trunc '\1' from end of nicks. (Fixed typo in the assignment).
- Adjust printf strings to avoid potential conversion errors.

## [1.4.1] - 2017-01-12

### Changed

- Reworked the changelog and readme.
- Restructured the projects file layout
- Code clean-ups
- Relicense under the GPL-3.0+
- The *dynip* script can now use either *links*, *lynx* or *wget*.

### Deprecated

- Platform support for *SunOS/Solaris*, *HP-UX*, *IRIX/IRIX64*, *Digital UNIX*, *Mac OS X Server*, *AIX* and *Cygwin*.
- *convertxdccfile*, since the *statefile* has existed for nearly 14 years now.

### Fixed

- Use correct "printf format string" length specifier for size_t and ssize_t.
- Merged Debian patches to fix typos and the *Configure* script.

## 1.4.b03 - 2005-12-12

### Added

- Add *logmessages* config item, similar to *lognotices*.
- Add *xdcclistfileraw* config, includes control characters to XDCC file.

### Changed

- Minor performance changes to memory allocator.
- Better detection of valid usermodes, nick prefixes, and channel-modes.
- Track usermodes of users in channels (shown in **CHANL** command).
- *clearrecords* should clear the transfer limit too.
- Mention directories when you do an **ADDDIR** or **ADDNEW** command
- Some cleanups to *Configure* and *Makefile*.

### Fixed

- Fix invalid CTCP ping response.
- Fix username when it cannot be determined automatically.
- Fix infinite loop when debug output causes memory allocation.

[Unreleased]: https://github.com/AnTiZ/iroffer/compare/v1.4.1...HEAD
[1.4.1]: https://github.com/AnTiZ/iroffer/compare/v1.4.b03...v1.4.1

---

## *Legacy Changelog*

## version 1.4

## 1.4.b02 - 2005-01-16

- all fixes in 1.3.b10
- add autoignore_threshold - auto-ignore rate configurable
- add 'addnew' command

## 1.4.b01 - 2004-09-01

- add autoignore_exclude configurable
- wrap transfer id at 999 or more
- silence file has changed warning for xdcc file
- dont allow a flood of xdcc lists to queue up when we are already busy
- use slow notice for flood
- add server password support
- add daily/weekly/monthly transfer limits

## version 1.3

## 1.3.b11 - 2005-12-12

- fix invalid CTCP ping response
- fix username when it cannot be determined automatically
- fix infinite loop when debug output causes memory allocation

## 1.3.b10 - 2005-01-16

- ignore dev/inode changes as file changes
- add sanity check to 'add' command to only allow regular files
- reaping of old listen ports wasn't very good, caused small leaks
- display memstat list using copy of list
- improve dns lookup error reporting
- fix memory leak with regcomp
- allow 5 minute grace period before disconnecting due to restrictsend
- windows users like their \r characters in xdcclistfile

## 1.3.b09 - 2004-08-23

- fix build issues with OSF1
- fix unaligned bus fault for 64bit writes in md5sum state structure
- fix infinate loop when more than 1 transfer in progress
- dont allow plistoffset must be greater than plist time

## 1.3.b08 - 2004-08-09

- dont use in_port_t, it's not in older glibc's
- fix warnings
- make transfer method determined at runtime
- mmap doesn't work on cygwin sometimes
- warn users about stupid usenatip values
- dont use tr_id 0
- fix crash if rmq called with invalid argument
- fix build for macosx

## 1.3.b07 - 2004-04-06

- add convertxdccfile (convert .xdcc -> .state)
- dont ignore nickserv, re-register on succesful nick rename

## 1.3.b06 - 2004-04-03

- make debug a cmdline option not config file
- replace auto* with autosendpack
- change overallminspeed -> transferminspeed
- remove double queue, will add something better in the future
- add xdcclistfile config, save xdl to text file
- update info and xdl commands, add 'xdcc info' message for users
- add nomd5sum config option
- add md5sum calcluation to packs
- fixup logstats
- add removedir, remove all files from within a directory
- share filedescriptors and mmaps between transfers
- fix total sent problem in statefile
- fix integer overflow if transfer rate exceeds 140mbps
- add sendfile support (for linux and freebsd)
- turn off respondtochannellist by default
- improve log rotation
- fix 'shutdown now' crash if issued through dcc chat
- other cleanups

## 1.3.b05 - 2004-03-22

- dcc chat fixes
- stop updating context log after crash starts
- fix nonblocking connect error checking
- change some privmsg to notice
- fix various rehash crashes if items removed/added
- add generic state file (remove xdccfile, messagefile, ignorefile)
- fix user xdcc list to use notice, broke a while back
- fix error handling if can't bind to listen socket
- track nicks and close when no longer on channels (if restrictsend)
- add punishslowusers

## 1.3.b04 - 2004-02-22

- fix crash on shutdown

## 1.3.b03 - 2004-02-21

- less calls to gethostbyname()
- do server dns resolve in a child process
- fix solaris build
- create fast server queue
- dcc chat cleanup, make connection non-blocking
- add 'chatl' and 'closec'
- multiple dcc chat support
- fix VERASE and VWERASE

## 1.3.b02 - 2004-01-22

- fix build for MacOSX and *BSD
- terminal support on console (arrow, ctrl-*, history)
- 'delhist' admin command (delete console history)
- add 'downloadhost' config command (specify how can download)
- add -k to allow setting of corefile rlimit
- add 'server_connected_raw' config command (send raw message after MODE)

## 1.3.b01 - 2004-01-11

- sort contents of adddir directory
- mmap files to save system calls
- replace uploadallowed with uploadhost (similar to adminhost)
- allow any number of server_join_raw and channel_join_raw
- replace virthost and vhost_ip with local_vhost
- replace firewall and dccrangestart with tcprangestart
- include device size in listul
- major code cleanup

## version 1.2

## 1.2b29 - 2004-03-22

- fix various rehash crashes if items removed/added
- fix error handling if can't bind to listen socket

## 1.2b28 - 2004-02-21

- fix fd leak on switching servers via signal
- fix slow server queue
- fix overwritten errno values on transfer disconnect

## 1.2b27 - 2004-01-22

- update to cygwin 1.5.6
- fix 'not a known channel' bug

## 1.2b26 - 2004-01-09

- channel_join_raw and channel_join_raw were not cleared on rehash
- various solaris fixes
- only dump context if debug set

## 1.2b24 - 2003-12-15

- fix crypt() detection

## 1.2b23 - 2003-12-13

- fix queue notification messages
- fix renumber
- add 'restrictprivlistmsg', custom message for restrictprivlist
- dont reuse listen ports for a while if never connected
- do sanity checks on adminhost
- add 'timestampconsole', show timestamps on console
- dont increment get counter on shutdown
- add 'quietmode', dont send informational messages to users
- fix unaligned transfer ack reads: caused end-of-transfer hangs
- more crazy usermodes: ~ and &
- fix bug when both respondtochannelxdcc and restrictlist are set
- detect broken crypt in Configure
- fix crash in psend under Solaris

## 1.2b22 - 2003-10-18

- fix end of transfer disconnect
- fix qul
- allow setting notifytime to 0 to disable
- fix slotsmaxpack range
- fix renumber
- dont allow uploads to overwrite non-regular files

## 1.2b21 - 2003-10-12

- fix crash when running on Digital UNIX
- when pack max speed is set, send data more smoothly (250ms instead of 4 second graunlarity)
- unlimited number of packs
- unlimited number of channels
- unlimited number of servers
- unlimited number of adminhosts
- unlimited number of ignores
- upto 504 simultaneous transfers under Linux/UNIX
- upto 119 simultaneous transfers under Windows
- better messages when xdcc list is not sent
- dont say 'for a listing...' if restrictprivlist is on
- add 'closeu' command, disconnect upload
- split server queue into low and high priorities
- better excess flood protection

## 1.2b20 - 2003-09-14

- more debugging information
- fix 'chfile'
- 'qsend' fixes to send first queued item
- fix pack queue crash
- reuse TCP ports from 'dccrangestart'
- fix infinate loop if all servers don't resolve
- fix 'lowbdwth' bug (debian bug #210349)
- add 'notifytime' configurable, how often to notify queue/bandwidth
- add 'dynip.sh' shell script to update 'usenatip'
- auto-convert dos slash to unix slash
- add 'smallfilebypass' bypass queue for small files
- allow starting up with missing offered files
- add 'chgets' command
- ignore list now uses full hostmasks
- add 'unignore' command
- admin commands via msg dont count towards ignore

## 1.2b19 - 2003-07-06

- fix upload bug introduced in 1.2b18
- add 'lognotices', configure if notice messages should be logged
- add recognition of admin, founder and halfop used by some irc networks

## 1.2b18 - 2003-07-05

- alternate nick support
- respond to '!LIST nick' (debian bug #191027)
- set IP TOS for transfers
- add 'hideos', to not show os in quit/version, etc..
- add 'respondtochannelxdcc', configure respons to 'xdcc sent' on channel
- add 'respondtochannellist', configure respons to 'xdcc list' on channel
- decrease queue/bandwidth notification to every 5 minutes
- allow queueing even if slots available but maxtransfersperperson applies
- smart dequeuing: skip over people that would violate maxtransfersperperson

### Fixed

- fix delete headline on rehash (debian bug #193110)
- fix freebsd, didn't have all sigcodes
- fix super-long retry backoff
- fix for some os's dont have strsignal()
- fix invocation to require at least one config file
- fix to handle CRLF on dcc chat
- work around cygwin bug: over 30 transfers fail
- work around cygwin bug: >2GB files return bad size in stat()
- proper handling of max open files ulimit setting
- fix 'chatme' to use firewall ports too
- fix perodicmsg every second bug
- work around cygwin bug: close() hangs
- fix uninitialized data problem

## 1.2b17 - 2003-06-10

- server retry backoff
- stdout buffering (ctrl-s will no longer block iroffer)
- fix signal hander mess
- fix infinate looping
- largefile support (upto 4GB on OSs that support it)
- xdcc list is saved to text file for external use
- handle files that change on the fly (size, replace, modify, etc...)
- nickserv support
- pick up actual server name for pinging and display
- log notice messages with privmsgs
- admin command 'msg' (send privmsg to someone)
- admin command 'servqc' (clear server queue)
- dont notify queued people if server queue is big
- log all chat attempts

## 1.2b16 - 2003-05-25

- increased number of packs and transfers
- background and color modes on cygwin
- proper file handling for xdcc and ignl files (no possibility of corruption now)
- fix disconnect transfer bug (would previously have to timeout transfer)
- fix channel user list 'unknown channel' bug on some servers
- handle bogus reading after signal (ctrl-c stuck bug)
- better error reasons in logs/messages
- allow color/nocolor screen/noscreen from command line
- fix cygwin 'not running cygwin' bug

## 1.2b15 - 2003-04-21

- performance improvement for high speed connections
- big code cleanups
- patch: add restrictprivlist: dont allow private lists (only public)
- patch: add -plistoffset: offset plist time in channel config
- patch: allow spaces in filenames
- patch: upload resume support!
- patch: respond to '!list' in channels with summary
- patch: chroot/setuid support: iroffer can chroot/setuid itself
- patch: psend type option: psend can now send full/minimal/summary
- patch: onjoin: send something on server connect or channel join
- patch: headline: add line of text to all xdcc list types

### Fixed

- fix Configure for systems that don't have -Wall
- improve memory allocator
- fix select() usage to work correctly

## 1.2b13 - 2001-11-10

### Fixed

- add -lcrypto checking in Configure, cleanup
- add SCCS tags
- wouldn't queue correctly if extra slots were in use
- add checks for backwards windows paths
- fix uninited dcc chat socket
- increase ignore count limit
- fix gcc-3.0 #endif warnings
- add profiling option
- usenatip was lost of server reconnect, requried rehash
- fix more dcc chat / server text buffer corruption
- don't send notifications if not connected to server
- logging recursive loop problem fixed
- fix compile error for systems missing sprintf()

## 1.2b12 - 2001-01-30

- bug fixes

## 1.2b11 - 2001-01-21

- bug fixes

## 1.2b10 - 2000-12-05

- bug fixes

## 1.2b9 - 2000-11-26

- Windows 95/98/NT/2000 support via cygwin (experimental)
- bug fixes

## 1.2b8 - 2000-10-17

- bug fixes

## 1.2b7 - 2000-10-17

- bug fixes

## 1.2b6 - 2000-10-01

- added restricted list, user must be on channel to get list 
- added restricted send, user must be on channel to get files
- chanl admin command, list users on channels
- code cleanups
- better crash reporting
- MacOS X Support (Darwin)

## 1.2b5 - 2000-07-04

- can now switch to specific server, (jump and servers admin commands)
- queue'd users now get an estimated remaining time in reminder
- qul now shows estimated remaining time
- maxqueueditemsperperson in config file
- AIX support (untested)
- code cleanup
- bug fixes

## 1.2b4 - 2000-05-19

- bug fixes

## 1.2b3 - 2000-05-16

- bug fixes

## 1.2b2 - 2000-04-22

- can now use vhosts in a bnc
- delete a file from upload dir
- bug fixes

## 1.2b1 - 2000-03-04

- ignore file added to config file, will now store ignore list across a shutdown
- search xdcc list for a keyword, "/msg \<bot\> xdcc search \<word\>"
- support for Digial UNIX
- support for MacOS X Server
- clear records admin command
- can now compile with either gcc or cc
- Configure wont stop when it encounters an error

## version 1.1

## 1.1.1 - 2000-05-14

### Fixed

- minor error messages for some admin commands were fixed
- dcc chat command processor could mangle or just not process commands under some situations
- fixed crash if user_realname was long
- fixed crash if certain incorrectly formatted commands were received
- "%" now show properly in admin command output

## 1.1 - 2000-02-12

- now supports NetBSD

### Fixed

- will once again compile on SunOS/Solaris

## 1.1b8 - 2000-01-23

- code cleanup
- now supports IRIX and IRIX64
- now supports os's that dont have snprintf()

### Fixed

- if queuesize was set to 0, plists would never go out
- if an admin send was used the user may not have been able to resume it
- xdcc file would revert to 0's on some os's

## 1.1b7 - 2000-01-08

- code cleanup

### Fixed

- finishing transfers now works correctly for *BSD/Solaris/HP-UX
- nicks with high ASCII characters (used in non-english character sets) now work
- password encryption can be disabled in defines.h

## 1.1b6 - 2000-01-02

- help is now broken up into sections
- when packs are added, desctiption now contains filename
- ADDDIR admin command, adds every file in a directory
- LISTUL admin command, lists contents of upload directory
- can use either XDCC SEND or XDCC GET
- bug fixes

## 1.1b5 - 1999-12-19

- TRINFO admin command, lists information about a transfer
- combigned DSHUTDOWN and SHUTDOWN into one command
- support for BNC, wingate, and custom proxies (connectionmethod in config file)
- bandwidth limit is now reported only to users who are actually effected by the limit
- bug fixes

## 1.1b4 - 1999-12-05

- removed architecture specific code under linux, should work on any linux now
- bandwidth sharing algorithm vastly improved, every user should now get a fair share
- CDCC can be used in addition to XDCC
- bug fixes

## 1.1b3 - 1999-11-24

- overallminspeed and transfermaxspeed can now be specified in 1/10th K increments.
- pack min/max speed can now be specified in 1/10th K increments.
- now supports joining +k channels
- bug fixes

## 1.1b2 - 1999-10-31

- accept dcc transfers (upload)
- Configure fixes
- bug fixes

## 1.1b1 - 1999-10-23

**IMPORTANT**: v1.1b1 changes the xdcc file format, use the included perl
script "convertxdccfile" to convert your v1.0 xdcc file

- contains all 1.0 fixes upto 1.0b37
- adminpass is now stored encrypted in the config file
- removed total snagged and added total transferred.  total transferred is amount actually sent by iroffer
- can now read multiple config files.  if more than one config file is specified on the command line, iroffer will read them all in sequence. This can be used to have common commands for multiple bots in one file and seperate smaller files for each individual bot, or for a system administrator that wants to enforce some commands for your bots.
- manual send first queued person (qsend)
- maximum bandwidth per transfer, overall and per-pack
- transfermaxspeed in config file, overall max speed for transfer
- chmaxs admin command, change max speed for individual pack
- nomax admin command, override max speed for individual transfer
- HP-UX support, still experimental

## version 1.0

## 1.0.1 - 1999-12-05

### Fixed

- if iroffer was started up between 12:00 AM and 1:00 AM it could rotate the log even if it had already done so
- bandwidth sharing algorithm vastly improved, every user should now get a fair share.
- some servers (undernet) would return a JOIN in lowercase even though it was requested in uppercase causing iroffer to think it hadn't joined the channel.

## 1.0 - 1999-11-07

### Fixed

- a crash could occur if number of channels was decreased with a rehash
- after rehash join status on channels could be wrong
- some times xdcc file wouldn't be written on shutdown

## 1.0b37 - 1999-10-21

### Fixed

- if maxslots is increased with rehash, people would stay in queue even though there were free slots
- unable to parse dcc chat password/commands in some situations
- connection established now shows ips/ports for connection
- if changing vhost with rehash, existing transfers would cause /proc/net/tcp errors under linux
- /proc/net/tcp errors in linux are now only printed once per transfer

## 1.0b36 - 1999-10-14

- bug fixes

## 1.0b35 - 1999-10-02

- bug fixes

## 1.0b34 - 1999-09-27

- new channel config, each channel gets plist status, minimal setting and time
- dont plist when queue is full
- periodic message
- bug fixes

## 0.1b33 - 1999-09-10

- bug fixes

## 0.1b32 - 1999-09-09

- alpha linux support
- manual ignore
- delayed shutdown (dshutdown)
- etailed dcl (dcld)
- safe memory allocating/freeing/reporting
- bug fixes

## 0.1b31 - 1999-08-29

- log rotation, daily, weekly, or monthly
- send message to queued people
- ported to C, now compiles with gcc
- xdcc and server connections are now non-blocking
- better minspeed calculation and enforcing
- smoother slow speed transfers
- better overall speed fareness
- better server switching and responsiveness when switching
- better server connection timeout support
- less likely to have problems when both slow and fast transfers happen at the same time
- larger transfer buffers on some OS's
- bug fixes

## 0.1b30 - 1999-08-04

- renumber pack numbers
- transfer rate in dcl
- default dir for files
- message log
- transfer rates/records are all shown in 1/10th a K now
- more consistant xdcc messages
- bug fixes

## 0.1b29 - 1999-07-10

- nosave, nosend, nolist can now specify number of minutes (0 to end now)
- max bandwidth limiting can now apply to any day(s) of the week
- bug fixes

## 0.1b28 - 1999-06-18

- bug fixes

## 0.1b27 - 1999-06-01

- auto-ignores individual flooding hosts (including IGNL admin command)
- users can now remove themselves from a queue
- bug fixes

## 0.1b26 - 1999-05-08

- xdcc lists are now queued and sent together to the server with low priority
- Configure does more tests, Makefile should now work everywhere
- NAT support for devices that do not intercept/translate dcc commands
- BOTINFO and NOLIST admin commands
- can now define different bandwidth limits depending on time of day!
- bug fixes

## 0.1b25 - 1999-04-18

- minimal public plist format option
- no more extra spaces in xdcc list
- redid signal handling: USR1 = switch server, USR2 = rehash
- added "#" wild card to adminhost for any number
- joining channels now is logged
- bug fixes

## 0.1b24 - 1999-03-29

- rehash function - re-reads config file
- Sparc Linux support
- Solaris support (still may have bugs)
- more extensive Configure
- bug fixes

## 0.1b23 - 1999-03-20

- new Makefile and source structure
- lots of new #define's
- adminhost now does normal hostmask checking (no more regular expressions)
- config file range checking on many values
- will send pack if "#" is missing
- bug fixes

## 0.1b22 - 1999-02-28

- now extra space friendly in config file
- regular expression admin hostname matching
- can now start with no xdcc file
- ctrl-c now shuts down correctly
- can now send special info to a proxy/gateway when connecting
- bug fixes

## 0.1b21 - 1999-02-21

- upto 50 channels
- allow any number of transfers per person
- added PSEND
- better xdcc/config file error messages
- allow override of loginname
- can now start with no files offered
- bug fixes

## 0.1b20 - 1999-02-01

- upto 50 servers
- consolidated remote admin functions (saved over 20K in size of binary)
- xdl and info in remote admin
- remote status (/ctcp nick status)
- resume in mirc now works
- shorter server connect timeout (5 seconds)
- support for manual local address assignment for dcc's
- bug fixes

## 0.1b19 - 1998-12-30

- agressive server retrying (if not connected, will try to get connected)
- maximum bandwidth limiting with fair share to all users
- bug fixes

## 0.1b18 - 1998-12-11

- bug fixes

## 0.1b17 - 1998-10-22

- LinuxPPC Support
- uptime (/ctcp nick uptime)
- NOSEND (disables new connections for next 15min)
- bug fixes

## 0.1b16 - 1998-10-02

- iroffer can now initiate the dcc chat (through an admin /msg)
- can now disable the minspeed requirement for individual transfers
- bug fixes

## 0.1b15 - 1998-09-05

- add packs on the fly!
- added CHFILE, CHDESC, CHNOTE, CHMINS
- disable xdcc autosave for next 15 min
- remove packs on the fly!
- allowed limiting of number of packs shown in channel plists
- bug fixes

## 0.1b14 - 1998-08-02

- new xdcc file format:
- notes now allowed
- individual pack min speeds
- bug fixes

## 0.1b13 - 1998-07-26

- switch servers when SIGHUP received ( kill -HUP xxxx ) 
- fixed bug where some servers wouldn't work
- all sprintf's changed to snprintf (less seg faults)
- overall min speed !

## 0.1b12 - 1998-07-19

- 6 channel support
- dcc firewall support (you can specify a range of dcc ports to use)
- plugin support !! (added interface for user plugins) see plugins.cpp for more info
- long filename support (full path can be upto 511 chars)
- bug fixes

## 0.1b11 - 1998-07-11

- Background mode now correctly detaches itself from the controlling terminal
- added slowlink feature

## 0.1b10 - 1998-07-02

- Background Mode / Foreground Mode (no screen required in background mode)
- status line at bottom of xdcc list
- xdcc file is now specified in config file
- fixed bug where "unable to open log file" would repeat indefinitely
- other bug fixes

## 0.1b9 - 1998-06-28

- fixed serious memory leak introduced in b8
- some optimizations
- fixed seg fault problem when finishing transfers on linux

## 0.1b8 - 1998-06-24

- allow manual changing of servers
- reminds users who don't accept a dcc that one is pending
- better file I/O (less seg faults)
- allow multiple hosts for admin (upto 4)
- allow stats not to be written to the log (useful for mostly idle bots)
- send a pack to a user manually (avoiding all slot restrictions)
- full remote administration through /msg and DCC CHAT!!
- bug fixes

## 0.1b7 - 1998-06-14

- log files
- keep trying to rejoin channel if not able to join until successful
- bug fixes

## 0.1b6 - 1998-06-09

- original release
