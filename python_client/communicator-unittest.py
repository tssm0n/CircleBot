import unittest
import time
from communicator import *
from xbeecom import xbee

class CommunicatorTest(unittest.TestCase):
    def setUp(self):
        self.com = xbee.DummyCom()
        self.communicator = Communicator('test', com = self.com)

    def testSendReceive(self):
        client = TestClient()
        self.communicator.attach(client)
        packet = (1,1,(1,2,3))
        self.communicator.send_packet(packet)
        self.communicator.serial.packet_buffer.append(packet)
        time.sleep(.2)
        self.communicator.finish()
        self.assertTrue(client.received)

class TestClient(CommunicatorClient):
    def __init__(self):
        CommunicatorClient.__init__(self, id = 1, name = "test")
        self.received = False

    def receive_packet(self, packet):
        self.received = True

if __name__ == '__main__':
    suite = unittest.TestLoader().loadTestsFromTestCase(CommunicatorTest)
    unittest.TextTestRunner(verbosity=2).run(suite) 
