import sys
import socket

from client import Connection
def main():
  if (len(sys.argv) != 3):
    print("This shit must look like exm: main.py 0.0.0.0 8443")
    exit(0)

  try:
    addr = sys.argv[1]
    port = sys.argv[2]

    clnt = Connection(addr, int(port))
    while(True):
      data = input(">> ")
      clnt.sendData(data)
      clnt.getResponse()
      
  except ValueError:
    print("Your ip or port full of shit")
    exit(-1)
  except socket.error as sock_err:
      print("Socket has raised exception: ", sock_err)
      exit(-1)

if __name__ == '__main__':
    sys.exit(main())