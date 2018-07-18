import configparser
import os
import os.path
import sys
import pathlib
import time
import threading
from ftplib import FTP

def ParseServerConfig():
	pathRadser = pathlib.Path('radser.cfg')
	if pathRadser.is_file():
		print("Reading server info file...")
		config = configparser.ConfigParser()
		config.read('radser.cfg')
		ip = config['RADIATION_SERVER']['IP']
		username = config['RADIATION_SERVER']['Username']
		password = config['RADIATION_SERVER']['Password']
		
		
	else:
		print("Couldn't find server info file. Creating a new one...")
		
		config = configparser.RawConfigParser()
		config['RADIATION_SERVER'] = {}
		config['RADIATION_SERVER']['IP'] = '192.168.1.1'
		config['RADIATION_SERVER']['Username'] = 'Username'
		config['RADIATION_SERVER']['Password'] = 'Password'
		
		with open('radser.cfg', 'w') as configfile:
			config.write(configfile)
		
		sys.exit("Please edit \"radser.cfg\" with correct information. The program will now stop.")
		
	print("Parsed the following data:")
	print("IP: " + ip)
	print("Username: " + username)
	print("Password: *****")
	
	return [ip, username, password]

def ParseUnitConfig():
	pathRadmon = pathlib.Path('radmon.cfg')
	if pathRadmon.is_file():
		print("Reading measurement unit name file...")
		radmon = open('radmon.cfg', 'r') 
		unitName = radmon.read()
		print("Unit name is set to: " + str(unitName) + ".")
	else:
		print("Couldn't find \"radmon.cfg\".")
		sys.exit("Please create the appropriate file. The program will now stop.")

	logFileName = unitName + "_log_" + time.strftime("%Y-%m" + ".txt")
	print("Current log is: " + logFileName)
	
	print("Parsed the following data:")
	print("Unit Name: " + unitName)
	print("Log File Name: " + logFileName)
	
	return [unitName, logFileName]
	
	
def UploadLog(ip, username, password, unitName, logFileName):
	print("Attempting to connect to server...")
	print("IP: " + ip)
	print("Username: " + username)
	print("Password: *****")
	try:
		connect = FTP(ip,username,password) #connect to server
		print("The server said:")
		print( str(connect.getwelcome()))
		print("The server contains:")
		root = connect.dir()
		print(root)
	except:
		print("Could not connect to the server properly. Check network ans server info file.")
	
	
	print("Setting up measurement unit directory...")
	print("Unit Name: " + unitName)
	print("Log File Name: " + logFileName)
	
	#exception will be thrown all time after the directory is created
	try:
		dir = connect.mkd(unitName)
	except:
		print("The measurement unit directory could not be created or already exists.")
	
	#change directory to measurement unit directory and show content
	try:
		connect.sendcmd('CWD '+unitName)	
		print("The measurement unit directory contains:")
		root = connect.dir()
		print(root)
	except:
		print("Could not read the measurement unit directory content.")

	print("Attempting to send current log...")
	try:
		file = open(logFileName, 'rb')
		connect.storbinary('STOR '+logFileName, file)
		print(file)
		file.close()
		connect.quit()
	except:
		print("An error occured and the file couldn't be sent to the server.")
	
	print("Scheduled tasks done. Now sleeping.")

def Main():
	print("Current time is: " + time.ctime())
	print("Server upload program starting...")
	#serverConfig array contains ip, username, password
	serverConfig = ParseServerConfig()
	#unitConfig array contains unitName, logFileName
	unitConfig = ParseUnitConfig()
	UploadLog(serverConfig[0], serverConfig[1], serverConfig[2], unitConfig[0], unitConfig[1])
	#30 minutes * 60 seconds = 1800 seconds sleep
	#10 minutes * 60 seconds = 600 seconds sleep
	threading.Timer(600, Main).start()

Main()
