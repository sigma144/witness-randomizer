#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <iterator>

class StringSplitter
{
public:
   template <typename Out>
   static void split(const std::string& s, char delim, Out result) {
      std::istringstream iss(s);
      std::string item;
      while (std::getline(iss, item, delim)) {
         *result++ = item;
      }
   }

   static std::vector<std::string> split(const std::string& s, char delim) {
      std::vector<std::string> elems;
      split(s, delim, std::back_inserter(elems));
      return elems;
   }
};
