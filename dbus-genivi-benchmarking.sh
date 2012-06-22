#!/bin/sh

DBUS_TEST_DATA="struct:int32:1:double:12.6:double:1e40:string:XXXXXXXXXXXXXXXXXXXX"
DBUS_TEST_DATA_MULTIPLY="0 1 10 100 1000"
DBUS_PING_COUNT=70000

LOG_FILE=dbus-genivi-benchmarking.log


#################################################################


log() {
    local LINE="$@"
    local TIME=$(date +'%d %b %Y %T')

    echo "$TIME $LINE"
    echo "$TIME $LINE" >> $LOG_FILE
}

handle_sigint() {
   log "Interrupted by user request!"

   if [ ! -z "${DBUS_SESSION_BUS_PID}" ]; then
     log "Killing dbus-daemon: ${DBUS_SESSION_BUS_PID}"
     kill -9 $DBUS_SESSION_BUS_PID
   fi

   exit 1
}

run_ping_pong() {
    local DBUS_PING_ARGS=$1
    local DBUS_DAEMON_ARGS=$2

    # start our own session bus
    DBUS_DAEMON_ARGS="--session --print-address=1 --print-pid=1 --fork ${DBUS_DAEMON_ARGS}"
    log "Executing: dbus-daemon ${DBUS_DAEMON_ARGS}"
    DBUS_DAEMON_ADDRES_AND_PID_OUTPUT=$(dbus-daemon ${DBUS_DAEMON_ARGS})
    DBUS_DAEMON_RET=$?
    if [ $DBUS_DAEMON_RET -ne 0 ]; then
        log "dbus-daemon failed: ${DBUS_DAEMON_RET}"
        return $DBUS_DAEMON_RET
    fi

    # extract the bus address and pid from the output
    DBUS_SESSION_BUS_ADDRESS=$(echo ${DBUS_DAEMON_ADDRES_AND_PID_OUTPUT} | cut -d' ' -f1)
    log "DBUS_SESSION_BUS_ADDRESS=${DBUS_SESSION_BUS_ADDRESS}"
    DBUS_SESSION_BUS_PID=$(echo ${DBUS_DAEMON_ADDRES_AND_PID_OUTPUT} | cut -d' ' -f2)
    log "DBUS_SESSION_BUS_PID=${DBUS_SESSION_BUS_PID}"

    # start dbus-ping
    DBUS_PING_ARGS="--count $DBUS_PING_COUNT --destination com.bmw.Test --path /com/bmw/Test --bash ${DBUS_PING_ARGS}"
    export DBUS_SESSION_BUS_ADDRESS
    log "Executing: dbus-ping ${DBUS_PING_ARGS}"
    DBUS_PING_OUTPUT=$(dbus-ping ${DBUS_PING_ARGS})
    DBUS_PING_RET=$?

    # TODO remove dbus_shutdown() from dbus-ping so that we have a proper exit code
    eval $DBUS_PING_OUTPUT

    # clean up
    kill -9 $DBUS_SESSION_BUS_PID

#    return $DBUS_PING_RET
    return 0
}


run_benchmark_case() {
    local BENCHMARK_NAME=$1
    local CONTENTS_MULTIPLY_COUNT=$2
    local DBUS_PING_ARGS=$3
    local DBUS_DAEMON_ARGS=$4

    if [ $CONTENTS_MULTIPLY_COUNT > 0 ]; then
        DBUS_PING_ARGS="$DBUS_PING_ARGS --contents-multiply $CONTENTS_MULTIPLY_COUNT ${DBUS_TEST_DATA}"
    fi

    log "Starting ${BENCHMARK_NAME}: contents_multiply_count=${CONTENTS_MULTIPLY_COUNT}"

    run_ping_pong "${DBUS_PING_ARGS}" "${DBUS_DAEMON_ARGS}"
    if [ $? -ne 0 ]; then
        log "Failed ${BENCHMARK_NAME}"
        return 1
    fi
 
    local BENCHMARK_RESULTS="msgs_per_second=${DBUS_PING_MSGS_PER_SEC}"
    BENCHMARK_RESULTS="$BENCHMARK_RESULTS creation_time=${DBUS_PING_DUPLICATE_TIME}"
    BENCHMARK_RESULTS="$BENCHMARK_RESULTS transport_time=${DBUS_PING_SEND_TIME}"
    log "Finished ${BENCHMARK_NAME}: ${BENCHMARK_RESULTS}"

    return 0
}


### main

DBUS_DAEMON_ARGS=
DBUS_PING_ARGS=

# parse command line options
while getopts a:v o; do
    case "$o" in
    a)    DBUS_DAEMON_ARGS="$DBUS_DAEMON_ARGS --address $OPTARG" ;;
    v)    DBUS_PING_ARGS="$DBUS_PING_ARGS -v" ;;
    [?])  echo "Usage: $0 [-v] [-a dbus-daemon-address]" 1>&2
          exit 1 ;;
    esac
done

# setup interrupt handler
trap handle_sigint INT

# do benchmarking
for test_data_multiply_count in $DBUS_TEST_DATA_MULTIPLY; do
    run_benchmark_case "clone" $test_data_multiply_count "$DBUS_PING_ARGS --member getEcho --clone" "$DBUS_DAEMON_ARGS"
    run_benchmark_case "copy" $test_data_multiply_count "$DBUS_PING_ARGS --member getLastReply" "$DBUS_DAEMON_ARGS"
done

