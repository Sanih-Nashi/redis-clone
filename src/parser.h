#pragma once

#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <string_view>

#define toInt(x) (x - '0')

class Parser
{
  public:

    //returns an std::vector which will contain the parsed value
    inline std::vector<std::string> ParseCommand(const std::string &string)
    {
      std::vector<std::string> v;
      
      //stores an integer which will be the no:of commands given by the client
      int size;


      // protocol for no:of arrays(commands)
      if (string[0] == '*')
      {
        size = toInt(string[1]); 
        v.reserve(size);
      }
      else
      {
        std::cerr << "protocol error";
        exit(1);
      }

      // parses by going through each charactor
      for (size_t i = 0; i < string.size(); i++)
      {

        // checks if it is a "bulk-string", else continues
        if (string[i] == '$')
        {

          // stores the no:of charactors the "bulk-string has"
          int num = toInt(string[++i]);
          // stores each "bulk-string"
          std::string buffer;

          // makes i to point to the actual bulk-string
          // and ignores the '\r' and '\n' charactors
          i += 3;

          // pushes it to the string
          for (int j = 0; j < num; i++)
          {
            buffer.push_back(string[i]);
            j++;
          }

          // places the buffer into the std::vector
          v.emplace_back(buffer);
        }


      }
      // convets the first string or the main command all capital letters
      toUpper(v[0]);

      return v;
    }



  inline std::vector<std::string> ParseData(std::string string) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = string.find(' ');

    while (end != std::string::npos) {
        if (start != end) { // Skip empty tokens (multiple spaces)
            tokens.push_back(string.substr(start, end - start));
        }
        start = end + 1;
        end = string.find(' ', start);
    }

    // Add the last token (if any)
    if (start < string.size()) {
        tokens.push_back(string.substr(start));
    }

    return tokens;
  }

  private:
    //converts all the charactors to capital letters
    inline void toUpper(std::string &str)
    {
      for (char &c : str)
        c = std::toupper(c);
    }
};