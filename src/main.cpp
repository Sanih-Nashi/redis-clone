#include <unistd.h>
#include <arpa/inet.h>

#include <cstdlib>

#include "reutils.h"

int clientSocket;
int serverSocket;

void close_socket();

int main()
{
  // Create a socket
  new_socket(serverSocket);

  // setting and binding the socket
  sockaddr_in serverAddress;
  SetAndBindServerSocket(serverAddress, serverSocket);

  // connecting with the client
  sockaddr_in clientAddress;
  atexit(close_socket);

  while (true)
  {
    ConnectClientSocket(clientAddress, clientSocket, serverSocket);

    // Communicate with the client
    Communicate(clientSocket);
  }

  // Close the sockets

  return 0;
}

void close_socket()
{
  close(clientSocket);
  close(serverSocket);
}