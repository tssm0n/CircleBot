from circlebotclient import *
import curses

class GuiClient(CirclebotClient):
    def start(self, com = None):
        self.screen = curses.initscr()

        curses.noecho()
        curses.curs_set(0)
        self.screen.keypad(1)

	self.line_title = 0
	self.line_message = 1
	self.line_status = 2
	self.line_info = 3

	self.screen.addstr(self.line_title, 0, "%s "%(self.name))

        while True:
           event = self.screen.getch()
           self.screen.addstr(self.line_info, 0, "%s    "%(event))
           if event == ord("q"):
                curses.endwin()
		if com is not None:
		    com.finish()
                break
	   else:
		self._parse_input(event)

    def _parse_input(self, event):
	if event == ord("p"):
	    self.ping()
	elif event == 259:
	    self.drive(120,1)
	elif event == 258:
	    self.drive(120,0)
	elif event == 260:
	    self.turn(120,0)
	elif event == 261:
	    self.turn(120,0)
	elif event == ord(" "):
	    self.stop()

    def _handleCommand(self, command, data):
        if command == commands.PING:
            self._send([commands.ACK])
        elif command == commands.ACK:
	    self.screen.addstr(self.line_message, 0, "Received ACK    ")
        elif command == commands.NACK:
	    self.screen.addstr(self.line_message, 0, "Received NACK    ")
        elif command == commands.STATUS:
            self.status = data[1:]
	    self._display_status()
        else:
	    self.screen.addstr(self.line_message, 0, "Invalid Command %s   "%(command))


    def _display_status(self):
	str_status = ""
	for stat in self.status:
	    if len(str_status) > 0:
	        str_status += ", "
	    str_status += "%s"%(stat)
        self.screen.addstr(self.line_status, 0, str_status + "     ")

def init(id = 1, port = '/dev/ttyUSB0'):
    com = Communicator(port)
    c = GuiClient(id)
    com.attach(c)
    c.start(com)
    return (c, com)


