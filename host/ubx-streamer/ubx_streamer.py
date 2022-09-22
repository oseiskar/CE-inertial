import argparse
import json
from time import get_clock_info
from serial import Serial
from ubxtranslator import core
from ubxtranslator import predefined
import os
import threading
import socket
import timestamp_pb2

parser = argparse.ArgumentParser(description="Stream iTOW timestamp from UBX-NAV-PVT")
parser.add_argument("device", help="Serial device")
parser.add_argument("-b", help="Baudrate", type=int, default=460800)
parser.add_argument("-v", help="Verbose, prints errors", action="store_true")

localIP     = "192.168.10.124"
localPort   = 5555
bufferSize  = 128


class Server:
    def __init__(self, localIP, localPort, bufferSize):
        UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
        UDPServerSocket.bind((localIP, localPort))
        self.server = UDPServerSocket
        self.client_address = None

    def getClientAddress(self, buffer = 128):
        bytesAddressPair = self.server.recvfrom(buffer)
        self.client_address = bytesAddressPair[1]

    def sendBuffer(self, message):
        self.server.sendto(message, self.client_address)

    def closeSocket(self):
        self.server.close()


def generateBuffer(data):
    try:
        if int(data.get("flags2").get("confirmedTime")) == 1:
            message = timestamp_pb2.timepoint()
            message.iTOW = data.get("iTOW")
            return message.SerializeToString()
    except (ValueError, IOError) as err:
        if args.v:
            print(err)
        return ""


def inputThreadFn(aList):
    input()
    aList.append(True)


def parseUBX(payload):
    raw = {}
    for field in payload._fields:
        value = getattr(payload, field)
        if type(value) != int:
            obj = {}
            for field2 in value._fields:
                obj[field2] = getattr(value, field2)
            raw[field] = obj
        else:
            raw[field] = value
    return raw


def run(args):
    device = Serial(args.device, args.b, timeout=10)
    parser = core.Parser([predefined.NAV_CLS])

    server = Server(localIP, localPort, bufferSize)
    server.getClientAddress()

    print("Start listening for UBX packets")
    print("Press ENTER to stop streaming...")
    
    aList = []
    threading.Thread(target = inputThreadFn, args=(aList,)).start()
    
    try:
        while not aList:
            try:
                msg, msg_name, payload = parser.receive_from(device)
                if msg_name == "PVT":
                    raw = parseUBX(payload)
                    buffer = generateBuffer(raw)
                    if buffer != "":
                        server.sendBuffer(buffer)

            except (ValueError, IOError) as err:
                if args.v:
                    print(err)
    finally:
        device.close()
        server.closeSocket()
        

if __name__ == "__main__":
    args = parser.parse_args()
    run(args)
    print("Done!")
