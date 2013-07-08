from communicator import *
import commands
import time

class IRobotClient(SimpleClient):
    def __init__(self, id, name="Circlebot"):
        CommunicatorClient.__init__(self, id, name)
	self.status = [[],[]]

    def _handleCommand(self, command, data):
        if command == commands.PING:
            self._send([commands.ACK])
        elif command == commands.ACK:
            print "Received ACK"
	elif command == commands.NACK:
	    print "Received NACK"
        elif command == commands.STATUS:
	    self.status[data[1]] = data[2:]
        else:
            print "Not handling command: "
            print command
    
    def drive(self, speed = 75):
	msb, lsb = self._split_number(speed)
        self._send([commands.DRIVE, msb, lsb, msb, lsb])

    def turn(self, speed = 75, duration = 0, direction = 1):
	msb1, lsb1 = self._split_number(speed)
	msb2 = 0
	lsb2 = 0
	if direction == 0:
		msb2 = msb1
		lsb2 = lsb1
		msb1 = 0
		lsb1 = 0
        self._send([commands.DRIVE, msb1, lsb1, msb2, lsb2])
        if duration > 0:
            time.sleep(duration)
            self.stop()

    def stop(self):
        self._send([commands.STOP_DRIVE])

    def raw(self, bytes):
	self._send([commands.RAW] + bytes)

    def on(self):
	self._send([commands.POWER_ON])

    def off(self):
	self._send([commands.POWER_OFF])

    def _split_number(self, num):
	msb = (num >> 8) & 0xFF
	lsb = (num % 256) & 0xFF
	return (msb, lsb)

def init(id = 2):
    com = Communicator('/dev/ttyUSB0')
    c = IRobotClient(id)
    com.attach(c)
    return (c, com)
