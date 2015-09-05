#!/usr/bin/python -u
import socket

DEBUG = 0

temperatures = dict()

def send_statsd(tbl, val):
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        if val < 0:
            sock.sendto("stats.home." + tbl + ":0|g\n", ("localhost", 8125))        
        sock.sendto("stats.home." + tbl + ":" + val + "|g\n", ("localhost", 8125))
    except:
        print "error sending record to statsd\n"

srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
srv.bind(("", 9191))
srv.listen(5)

while 1:
    clnt, addr = srv.accept()
    val = clnt.recv(8).strip()
    clnt.close()
    #print "Received {} from {}:{}".format(val, addr[0], addr[1])
    send_statsd("cistern", val)
