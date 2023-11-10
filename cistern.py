#!/usr/bin/python -u
import socket, select
from datetime import datetime

DEBUG = 1

values = list()
last_update = datetime.now()

def log(msg):
    print "[{}] {}".format(datetime.now().strftime('%Y-%m-%d %H:%M:%S'), msg)

def log_debug(msg):
    if DEBUG == 1:
        log(msg)

def send_statsd(tbl, val):
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        if val < 0:
            sock.sendto("stats.home." + tbl + ":0|g\n", ("localhost", 8125))        
        sock.sendto("stats.home." + tbl + ":" + val + "|g\n", ("localhost", 8125))
    except Exception as e:
        log("Error while sending record to statsd: {}".format(e))

def start_server():
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.settimeout(30)
    srv.bind(("", 9191))
    srv.listen(5)
    return srv

def get_val():
    if len(values) == 0:
        return None
    val = (sum(values) / len(values) / 20) * 20
    if len(values) >= 10:
        return val
    else:
        log('{} values, avg={}'.format(len(values), val))
        return None

#send_statsd("cistern", "705")
while True:
    try:
        log_debug("Starting server")
        srv = start_server()
        log_debug("  accept")
        clnt, addr = srv.accept()
        clnt.setblocking(0)
        log_debug("  recv")
        readable, writable, errors = select.select([clnt], [], [], 3)
        if len(readable) > 0:
            val = clnt.recv(8).strip()
        log_debug("  close")
        clnt.close()
        srv.close()
        log("Received {} from {}:{}".format(val, addr[0], addr[1]))
        values.append(int(val))
    except Exception as e:
        log_debug("Error: {}".format(e))
        srv.close()
    
    #time_dif = datetime.now() - last_update
    # we get 100 values and send out the avg
    val = get_val()
    if val != None:
        log("Sending {} to statsd".format(val))
        send_statsd("cistern", str(val))
        values.pop(0) # remove the oldest item
        last_update = datetime.now()
