import socket

class Connection:
  sock = None
  response = ''
  
  def __init__(self, addr, serv_port):
    self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    self.sock.connect((addr, serv_port))

  def sendData(self, data):
    self.sock.sendall(bytes(data + '\n', "ascii"))
    if(data.startswith("compress") or data.startswith("decompress")):
      resp = self.sock.recv(1024)
      self.response = resp.decode("ascii")
    
  def getResponse(self):
      resp = self.sock.recv(1024)
      self.response += resp.decode("ascii")
      self.response = self.response[:-1]
      print(self.response)
      self.response = ''


