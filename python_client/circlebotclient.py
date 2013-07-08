from communicator import *
import commands
import time

class CirclebotClient(SimpleClient):
    def __init__(self, id, name="Circlebot"):
        CommunicatorClient.__init__(self, id, name)

    def _handleCommand(self, command, data):
        if command == commands.PING:
            self._send([commands.ACK])
        elif command == commands.ACK:
            print "Received ACK"
	elif command == commands.NACK:
	    print "Received NACK"
        elif command == commands.STATUS:
            self.status = data[1:]
        else:
            print "Not handling command: "
            print command
    
    def drive(self, speed = 75, forward = 1):
        self._send([commands.DRIVE, speed, speed, forward, forward])

    def turn(self, speed = 75, duration = 0):
        self._send([commands.DRIVE, speed, speed, 0, 1])
        if duration > 0:
            time.sleep(duration)
            self.stop()

    def stop(self):
        self._send([commands.STOP_DRIVE])

def init(id = 1):
    com = Communicator('/dev/ttyUSB0')
    c = CirclebotClient(id)
    com.attach(c)
    return (c, com)
