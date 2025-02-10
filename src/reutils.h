#pragma once

#include <arpa/inet.h>

#include <unordered_map>
#include <string>

void new_socket(int &server);

void SetAndBindServerSocket(sockaddr_in &server, int &file_descriptor_server);

void ConnectClientSocket(sockaddr_in &client, int &file_descriptor_client, int &file_descriptor_server);

void Communicate(int &clientSocket);

void manage_client(int &serverSocket, volatile int &clientno);

inline std::unordered_map<std::string, std::string> data;

#define DATA_FILE_NAME "data.raotf"
#define CLIENT_MAX 5