#include "reutils.h"
#include "parser.h"

#include <iostream>
#include <thread>
#include <cstring>
#include <fstream>
#include <string_view>

#include <unistd.h>

#define toString(x) (x + '0')

void newSocket(int &server)
{

  server = socket(AF_INET, SOCK_STREAM, 0);
  if (server == -1)
  {
    std::cerr << "Failed to create socket!" << "\n";
    exit(1);
  }
}

void setAndBindServerSocket(sockaddr_in &server, int &file_descriptor_server)
{
  std::cout <<"Port on which it should run on: ";
  int serverNum;
  std::cin >> serverNum;
  server.sin_family = AF_INET;
  server.sin_port = htons(serverNum);       // Port number
  server.sin_addr.s_addr = INADDR_ANY; // Listen on all available interfaces

  if (bind(file_descriptor_server, (sockaddr *)&server, sizeof(server)) == -1)
  {
    std::cerr << "Failed to bind socket!" << "\b";
    close(file_descriptor_server);
    exit(1);
  }

  // Listen for incoming connections
  if (listen(file_descriptor_server, 5) == -1)
  { // Backlog of 5
    std::cerr << "Failed to listen on socket!" << "\n";
    close(file_descriptor_server);
    exit(1);
  }

  std::cout << "Server is listening on port 8080..." << "\n";
}

bool connectClientSocket(sockaddr_in &client, int &file_descriptor_client, const int &file_descriptor_server)
{

  // accepts the request by the client

  socklen_t clientAddressSize = sizeof(client);
  file_descriptor_client = accept(file_descriptor_server, (sockaddr *)&client, &clientAddressSize);
  if (file_descriptor_client == -1)
  {
    std::cerr << "Failed to accept connection!" << "\n";
    close(file_descriptor_server);
    return false;
  }

  std::cout << "Client connected!" << "\n";
  return true;
}

void readDatafile();

void Communicate(int &clientSocket)
{


  // reads the file "data.raotf" aka DATA_FILE_NAME
  readDatafile();

  // creates an ostream obj to append the commands that will be executed by the client in a the "data.raotf" file
  std::ofstream aot(DATA_FILE_NAME, std::ios_base::app);

  char buffer[1024];
  while (true)
  {
    memset(buffer, 0, sizeof(buffer));

    // Receive data from the client
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

    if (bytesReceived == -1)
    {
      std::cerr << "Error receiving data!" << "\n";
      break;
    }
    else if (bytesReceived == 0)
    {
      std::cout << "Client disconnected." << "\n";
      break;
    }

    Parser parser;
    std::vector<std::string> v = parser.ParseCommand(buffer);


#ifdef DEV_AND_DEBUG
    for (std::string a : v)
    {
      std::cout << a << "\n";
    }
#endif

    // processes the commands
    if (v[0] == "GET")
    {
      if (v.size() > 1 && Data.count(v[1]) > 0)
      {
        // makes the element in the redis protocol
        std::string response = "+" + Data[v[1]] + "\r\n";

        send(clientSocket, response.c_str(), response.size(), 0);
      }
      else
      {
        const char *response = "+[nil]\r\n";

        send(clientSocket, response, strlen(response), 0);
      }
    }
    else if (v[0] == "SET")
    {
      // inserts the data

      // checks if the command has valid no:of parameters
      if (v.size() < 3)
      {
        const char *response = "-invalid use of SET\r\n";
        send(clientSocket, response, strlen(response), 0);
        continue;
      }

      // if the key already exists, modifies its contents to the curent
      if (Data.contains(v[1]))
        Data[v[1]] = v[2];

      else
        Data.insert(std::make_pair(v[1], v[2]));

      // prints ok in the redis protocol
      const char *response = "+Ok\r\n";
      send(clientSocket, response, strlen(response), 0);

      // stores it in a file
      for (std::string &str : v)
        aot << str << " ";

      aot <<std::endl;

    }

    else if (v[0] == "DEL")
    {
      std::string response;

      // if it exists then will return 1, else 0
      if (v.size() >= 2)
      {
        Data.erase(v[1]);
        response = "+(integer)1\r\n";
        send(clientSocket, response.c_str(), response.size(), 0);
      }
      else
      {
        response = "+(integer)0\r\n";
        send(clientSocket, response.c_str(), response.size(), 0);
      }

      for (std::string &str : v)
        aot << str << " ";

      aot <<std::endl;
    }
    else if (v[0] == "PING")
    {
      std::string response;
      response = "+" + *(v.end()) + "\r\n";

      send(clientSocket, response.c_str(), response.size(), 0);
    }
    else
    { // will expand it furthur
      const char *response = "-Not a valid operator yet\r\n";
      send(clientSocket, response, strlen(response), 0);
    }
  }
}

void readDatafile()
{
  std::ifstream in(DATA_FILE_NAME);

  if (!in.is_open())
  {
    std::cerr << "file \"" << DATA_FILE_NAME << "\" is not open"
              << "\n exited with code 1";
    exit(1);
  }

  std::string buffer;
  Parser parser;

  while (std::getline(in, buffer))
  {

    std::vector<std::string> v = parser.ParseData(buffer);

    if (v[0] == "SET")
    {
      // inserts the data

      // checks whether it is valid or not
      if (v.size() < 3)
      {
        continue;
      }

      // if the key exists, modify it to the current one
      if (Data.contains(v[1]))
        Data[v[1]] = v[2];

      else
        Data.insert(std::make_pair(v[1], v[2]));
    }
    else if (v[0] == "DEL")
    {
      // if it is valid then deletes
      if (v.size() >= 2)
      {
        Data.erase(v[1]);
      }
    }
  }
}

void closeServer(int &serverSocket)
{
  std::cin.get();
  std::cout << "\nexiting\n";
  for (size_t i = 0; i < CLIENT_MAX; i++)
  {
    shutdown(clientFile[i], SHUT_RD);
    close(clientFile[i]);
  }
  close(serverSocket);
  exit(0);
}

void manageClient(const int &serverSocket, const int clientno)
{
  if (clientno == CLIENT_MAX)
    return;

  sockaddr_in clientAddress;

  if (connectClientSocket(clientAddress, clientFile[clientno], serverSocket));
    std::thread client(manageClient, std::ref(serverSocket), clientno + 1); // for simultaneously connect to other clients
  
  Communicate(clientFile[clientno]); // to create the thread at the first iteration of the loop, it is outside the loop itself

  while (true)
  {

    connectClientSocket(clientAddress, clientFile[clientno], serverSocket);
    Communicate(clientFile[clientno]);
  }

  client.join();
}
