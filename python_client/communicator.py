from xbeecom import xbee
import threading
import time
import commands


class CommunicatorClient:
    "Base class for communicating with xbee devices"
    def __init__(self, id, name="Unknown"):
        self.id = id
        self.name = name

    def receive_packet(self, packet):
        pass

    def onAttach(self, comm):
        self.comm = comm
        pass

    def onDetach(self):
        pass

    def get_name(self):
        return self.name

    def _send(self, data):
        self.comm.send(self.id, data)

class Communicator:
    def __init__(self, port, localId = 0, com = None):
        if com is None:
            self.serial = xbee.SerialCom(port)
        else:
            self.serial = com
        self.portLock = threading.Lock()
        self.sendLock = threading.Lock()
        self.clients = {}
        self.id = localId

        self.commThread = CommunicationThread(self)
        self.commThread.start()

    def attach(self, client):
        self.clients[client.id] = client
        client.onAttach(self)
        print "Attached client: " + client.get_name()

    def detach(self, id):
        if self.clients.has_key(id):
            client = self.clients.pop(id)
            client.onDetach()
            print "Detached client: " + client.get_name()

    def receive_packet(self, packet):
        source = packet[0]
        if self.clients.has_key(source):
            self.clients[source].receive_packet(packet)
        else:
            print "Unknown id"
            
    def send(self, destination, data):
        packet = (self.id, destination, data)
        self.send_packet(packet)

    def send_packet(self, packet):
        self.sendLock.acquire()
        self.serial.send_packet(packet)
        self.sendLock.release()

    def finish(self):
        self.commThread.done = True
        self.serial.exit()
    
class CommunicationThread(threading.Thread):
    def __init__(self, communicator, pollTime = 0.01):
        threading.Thread.__init__(self)
        self.communicator = communicator
        self.done = False
        self.pollTime = pollTime

    def run(self):
        print "Starting Communication Thread"
        next_packet = None
        while not self.done:
            time.sleep(self.pollTime)
            self.communicator.portLock.acquire()
            if self.communicator.serial.data_available():
                next_packet = self.communicator.serial.next_packet()
            self.communicator.portLock.release()
            if next_packet is not None:
                self.handle_packet(next_packet)
                next_packet = None
        print "Exiting Communication Thread"

    def handle_packet(self, next_packet):
        self.communicator.receive_packet(next_packet)


class SimpleClient(CommunicatorClient):
    def __init__(self, id, name="Unknown"):
        CommunicatorClient.__init__(self, id, name)

    def ping(self):
        self._send([commands.PING])

    def receive_packet(self, packet):
        command = packet[2][0]
        self._handleCommand(command, packet[2])

    def _handleCommand(self, command, data):
        if command == commands.PING:
            self._send([commands.ACK])
        elif command == commands.ACK:
            print "Received ACK"
	elif command == commands.NACK:
	    print "Received NACK"
        elif command == commands.STATUS:
            print "Status: "
            print data[1:]
        else:
            print "Not handling command: " + command
