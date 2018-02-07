#ifndef __MIO_STR_H__
#define __MIO_STR_H__

#include <string>
#include <sstream>
#include <vector>
#include <cstring>


namespace mio{

inline char* c_str_to_ascii_hex_str(const char *str_src, char *str_out, const size_t str_out_size){
  const size_t str_len = strlen(str_src);
  if(str_out_size >= (str_len*3 + 1) && str_len > 0){
    char *str_out_tmp = str_out;
    size_t i = 0, str_len_less_one = str_len-1;
    while(i < str_len_less_one){
      sprintf(str_out_tmp, "%x ", str_src[i++]);
      str_out_tmp += 3;
    }
    sprintf(str_out_tmp, "%x", str_src[i]);
    str_out_tmp[2] = 0;
    return str_out;
  }
  return 0;
}


//src_str is modified by this function
inline char** StrSplit(char *src_str, const char kDeliminator, size_t &num_sub_str){
  //replace deliminator's with NULL's and count how many sub_str's with length >= 1 exist
  num_sub_str = 0;
  char *src_str_tmp = src_str;
  bool found_delim = true;
  while(*src_str_tmp){
    if(*src_str_tmp == kDeliminator){
      *src_str_tmp = 0;
      found_delim = true;
    }
    else if(found_delim){ //found first character of a new string
      num_sub_str++;
      found_delim = false;
    }
    src_str_tmp++;
  }
  if(num_sub_str <= 0){
    printf("str_split() - no substrings were found\n");
    return 0;
  }

  char **sub_strings = (char **)malloc( (sizeof(char*) * num_sub_str) + 1); //add space for terminating NULL
  const char *src_str_terminator = src_str_tmp;
  src_str_tmp = src_str;
  bool found_null = true;
  size_t idx = 0;
  while(src_str_tmp < src_str_terminator){
    if(!*src_str_tmp) //found a NULL
      found_null = true;
    else if(found_null){
      sub_strings[idx++] = src_str_tmp;
      found_null = false;
    }
    src_str_tmp++;
  }
  sub_strings[num_sub_str] = NULL;

  return sub_strings;
}


// src_str is not modified
// fills sub_str_vec with pointers to c-strings that exist within src_str
// the deliminator characters are replaced will NULL characters to form proper c-strings
inline int StrSplit(char *src_str, const char kDeliminator, std::vector<char*> &sub_str_vec){
  char *src_str_tmp = src_str;
  bool found_delim = true;
  sub_str_vec.clear();
  sub_str_vec.reserve(strlen(src_str)/2);

  //replace deliminator's with NULL's and count how many sub_str's with length >= 1 exist
  while(*src_str_tmp){
    if(*src_str_tmp == kDeliminator){
      *src_str_tmp = 0;
      found_delim = true;
    }
    else if(found_delim){ //found first character of a new string
      found_delim = false;
      sub_str_vec.push_back(src_str_tmp);
    }
    src_str_tmp++;
  }

  return sub_str_vec.size();
}


// split string by multiple deliminators and place split strings in elem_str_vec
inline void SplitStr(const std::string &kStr, std::vector<std::string> &elem_str_vec,
                     const std::string &kDelimiters = " ", const bool kTrimEmpty = false){
  std::string::size_type pos, last_pos = 0;
  for(;;){
    pos = kStr.find_first_of(kDelimiters, last_pos);
    if(pos == std::string::npos){
      pos = kStr.length();
      if(pos != last_pos || !kTrimEmpty)
        elem_str_vec.push_back(std::string(kStr.data()+last_pos, pos-last_pos));
      break;
    }
    else
      if(pos != last_pos || !kTrimEmpty)
        elem_str_vec.push_back(std::string(kStr.data()+last_pos, pos-last_pos));

    last_pos = pos + 1;
  }
}


//split string by a deliminator, then place each piece in a vector
inline void SplitStr(const std::string &kStr, const char kDelim, std::vector<std::string> &elem_str_vec,
                     const bool kClear = false){
  std::stringstream ss(kStr);
  std::string item;
  if(kClear)
    elem_str_vec.clear();
  while(std::getline(ss, item, kDelim))
    elem_str_vec.push_back(item);
}


void ReplaceAll(std::string& str, const std::string& search_str, const std::string& replace_str){
  size_t pos = 0;
  while((pos = str.find(search_str, pos)) != std::string::npos){
    str.replace(pos, search_str.length(), replace_str);
    pos += replace_str.length();
  }
}

} //namespace mio

#endif //__MIO_STR_H__

