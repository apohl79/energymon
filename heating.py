#!/usr/bin/python -u
import sys, serial, thread, threading, time, MySQLdb
from socket import socket, AF_INET, SOCK_DGRAM

DEBUG = 0
INITDB = 0
MAX_ERRORS = 3
MAX_NO_UPDATE = 360

allowed_keys = frozenset([
    'temp_boiler',
    'temp_outdoor',
    'temp_feed',
    'temp_buffer',
    'temp_collector',
    'temp_hottap',
    'null'
])

db_host = 'mneme'
db_db = 'energy'
db_user = 'root'
db_pass = 'password'

temperatures = dict()
temperatures_lock = threading.Lock()

err_counter = 0
err_lock = threading.Lock()
last_update = time.time()

def inc_errors():
    global err_counter, err_lock
    err_lock.acquire()
    err_counter = err_counter + 1
    err_lock.release()

def reset_errors():
    global err_counter, err_lock, last_update
    err_lock.acquire()
    err_counter = 0
    last_update = time.time()
    err_lock.release()

def get_errors():
    global err_counter, err_lock
    err_lock.acquire()
    ret = err_counter
    err_lock.release()
    return ret

def get_last_update_sec():
    global err_lock, last_update
    now = time.time()
    err_lock.acquire()
    ret = now - last_update
    err_lock.release()
    return ret

def read_data():
    ser = serial.Serial('/dev/ttyUSB0')
    while (get_errors() < MAX_ERRORS and get_last_update_sec() < MAX_NO_UPDATE):
        try:
            data = ser.readline().rstrip()
            if data.startswith('DBG:'):
                if DEBUG:
                    print data
            elif data.startswith('ERR:'):
                print data
                inc_errors()
            else:
                if data.count("=") == 1:
                    (key, value) = data.split('=', 1)
                    if DEBUG:
                        print "DATA: key = " + key + ", value = " + value 
                    if key in allowed_keys:
                        temperatures_lock.acquire()
                        temperatures[key].append(float(value))
                        temperatures_lock.release()
                        reset_errors()
                    else:
                        print "Ignoring: " + data
                        inc_errors()
        except serial.SerialException as e:
            print "Error reading from serial device: " + str(e.strerror)
            inc_errors()
            time.sleep(5)
    ser.close()
    print "reader thread terminated: errors {}, last update secs {}".format(get_errors(), get_last_update_sec())

def check_table(tbl):
    try:
        db = MySQLdb.connect(db_host, db_user, db_pass, db_db)
        cursor = db.cursor()
        cursor.execute("create table if not exists " + tbl + " (time DATETIME NOT NULL PRIMARY KEY, temp DOUBLE NOT NULL)")
        db.commit()
        db.close()
    except:
        print "Error: db error"

def send_statsd(tbl, val):
    try:
        sock = socket(AF_INET, SOCK_DGRAM)
        if val < 0:
            sock.sendto("stats.home." + tbl + ":0|g\n", ("localhost", 8125))        
        sock.sendto("stats.home." + tbl + ":" + str(val) + "|g\n", ("localhost", 8125))
    except:
        print "error sending record to statsd"

#
# MAIN
#
i = 1
while i < len(sys.argv):
    if sys.argv[i] == '-h':
        print('usage: {} [-debug] [-initdb] [-h]'.format(sys.argv[0]))
        sys.exit()
    elif sys.argv[i] == '-debug':
        DEBUG = 1
    elif sys.argv[i] == '-initdb':
        INITDB = 1
    else:
        print('unknown parameter')
        sys.exit(1)
    i += 1

for key in allowed_keys:
    if not key == 'null':
        temperatures[key] = list()
        if INITDB:
            check_table(key)

try:
    thread.start_new_thread(read_data, ())
except:
    print "Error: unable to start thread"

count = 0
while get_errors() < MAX_ERRORS and get_last_update_sec() < MAX_NO_UPDATE:
    time.sleep(1)
    count = count + 1
    if count == 300:
        count = 0
        temperatures_lock.acquire()
        try:
            db = MySQLdb.connect(db_host, db_user, db_pass, db_db)
            cursor = db.cursor()
            for key in temperatures:
                if len(temperatures[key]) > 0:
                    avg = sum(temperatures[key]) / len(temperatures[key])
                    num_vals = len(temperatures[key])
                    if DEBUG:
                        print "temperature(" + key + ", #" + str(num_vals) + ") = " + str(avg)
                    cursor.execute("insert into " + key + " (time, temp) values(now(), " + str(avg) + ")")
                    db.commit()
                    del temperatures[key][:]
                    send_statsd(key, avg)
            db.close()
        except:
            print "Error: db error"
        temperatures_lock.release()
print "writer thread terminated: errors {}, last update secs {}".format(get_errors(), get_last_update_sec())
