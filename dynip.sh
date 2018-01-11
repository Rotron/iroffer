#!/bin/sh

#### WHAT THIS DOES ####
# 
# If you have a dynamic ip address and require setting 'usenatip'
# in the iroffer config file this script can auto-generate a 1 line
# config file containing your current ip address
# run iroffer with multiple config files (your normal config plus
# this auto-generated config)
# 
# If your ip address changes, the file will be updated by this script
# and iroffer rehashed to see the change



URL="http://iroffer.org/myip.cgi"
CONFIGFILE="dynip.conf"
PIDFILE="mybot.pid"

set -e
set -u
#set -x

if type links >/dev/null 2>&1; then
    PROG="links -source"
elif type lynx >/dev/null 2>&1; then
    PROG="lynx -source"
elif type wget >/dev/null 2>&1; then
    PROG="wget --quiet -O -"
else
    echo "ERROR: cannot find links, lynx, or wget"
    exit 1
fi

${PROG} ${URL} |sed -e 's=^=usenatip =' > ${CONFIGFILE}.tmp

test -f ${CONFIGFILE} || touch ${CONFIGFILE}

if diff ${CONFIGFILE} ${CONFIGFILE}.tmp; then
  echo ip unchanged
  rm -f ${CONFIGFILE}.tmp
else
  echo ip changed
  mv -f ${CONFIGFILE}.tmp ${CONFIGFILE}
  if [ -f ${PIDFILE} ]; then
    echo rehashing iroffer
    kill -USR2 `cat ${PIDFILE}`
  else
    echo no iroffer running
  fi
fi

exit 0
