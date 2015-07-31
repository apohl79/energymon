#!/usr/bin/python -u
import serial, thread, threading, time, MySQLdb
from socket import socket, AF_INET, SOCK_DGRAM

DEBUG = 0

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

def read_data():
    ser = serial.Serial('/dev/ttyUSB0')
    while (1):
        try:
            data = ser.readline().rstrip()
            if data.startswith('DBG:'):
                if DEBUG:
                    print data
            else:
                if data.count("=") == 1:
                    (key, value) = data.split('=', 1)
                    if DEBUG:
                        print "DATA: key = " + key + ", value = " + value 
                    if key in allowed_keys:
                        temperatures_lock.acquire()
                        temperatures[key].append(float(value))
                        temperatures_lock.release()
                    else:
                        print "Ignoring: " + data
        except SerialException as e:
            print "Error reading from serial device: " + e.strerror
            time.sleep(5)

def check_table(tbl):
    db = MySQLdb.connect(db_host, db_user, db_pass, db_db)
    cursor = db.cursor()
    cursor.execute("create table if not exists " + tbl + " (time DATETIME NOT NULL PRIMARY KEY, temp DOUBLE NOT NULL)")
    db.commit()
    db.close()

def send_statsd(tbl, val):
    try:
        sock = socket(AF_INET, SOCK_DGRAM)
        if val < 0:
            sock.sendto("stats.home." + tbl + ":0|g\n", ("localhost", 8125))        
        sock.sendto("stats.home." + tbl + ":" + str(val) + "|g\n", ("localhost", 8125))
    except:
        print "error sending record to statsd\n"

for key in allowed_keys:
    if not key == 'null':
        temperatures[key] = list()
        check_table(key)

try:
    thread.start_new_thread(read_data, ())
except:
    print "Error: unable to start thread"

while 1:
    time.sleep(300)
    try:
        db = MySQLdb.connect(db_host, db_user, db_pass, db_db)
        cursor = db.cursor()
        temperatures_lock.acquire()
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
        temperatures_lock.release()
        db.close()
    except:
        temperatures_lock.release()
        print "Error: db error"
