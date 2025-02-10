#include <unistd.h>
#include <arpa/inet.h>

#include <cstdlib>

#include "reutils.h"

int serverSocket;
volatile int clientno = 0;

void close_socket();

int main()
{
  // Create a socket
  new_socket(serverSocket);

  // setting and binding the socket
  sockaddr_in serverAddress;
  SetAndBindServerSocket(serverAddress, serverSocket);
  
  manage_client(clientno);

  // Close the sockets

  return 0;
}

void close_socket()
{
  close(serverSocket);
}

