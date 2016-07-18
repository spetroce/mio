#ifndef __MIO_STR_H__
#define __MIO_STR_H__

#include <string>
#include <sstream>
#include <vector>
#include <cstring>


template <typename T>
inline std::string numToStr(T num){
  std::stringstream ss;  //create a stringstream
  ss << num;  //add number to the stream
  return ss.str();  //return a string with the contents of the stream
}


inline char* c_str_to_ascii_hex_str(const char *str_src, char *str_out, const size_t str_out_size){
  const size_t str_len = strlen(str_src);
  if(str_out_size >= (str_len*3 + 1) && str_len > 0){
    char *str_out_tmp = str_out;
    size_t i = 0, str_len_less_one = str_len-1;
    while(i < str_len_less_one){
      sprintf(str_out_tmp, "%x ", str_src[i++]);
      str_out_tmp += 3;
    }
    sprintf(str_out_tmp, "%x", str_src[i], 0);
    str_out_tmp[2] = 0;
    return str_out;
  }
  return 0;
}


//src_str is modified by this function
inline char** str_split(char *src_str, const char deliminator, size_t &num_sub_str){
  //replace deliminator's with NULL's and count how many sub_str's with length >= 1 exist
  num_sub_str = 0;
  char *src_str_tmp = src_str;
  bool found_delim = true;
  while(*src_str_tmp){
    if(*src_str_tmp == deliminator){
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


inline int str_split(char *src_str, const char deliminator, std::vector<char*> &sub_str_vec){
  char *src_str_tmp = src_str;
  bool found_delim = true;
  sub_str_vec.clear();
  sub_str_vec.reserve(strlen(src_str)/2);

  //replace deliminator's with NULL's and count how many sub_str's with length >= 1 exist
  while(*src_str_tmp){
    if(*src_str_tmp == deliminator){
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


/*
template <class ContainerT>
inline void str_split(const std::string& str, ContainerT& tokens, const std::string& delimiters = " ", 
                      bool trimEmpty = false){
  std::string::size_type pos, lastPos = 0;
  for(;;){
    pos = str.find_first_of(delimiters, lastPos);
    if(pos == std::string::npos){
      pos = str.length();
      if(pos != lastPos || !trimEmpty)
        tokens.push_back( ContainerT::value_type(str.data()+lastPos, 
                                                 (ContainerT::value_type::size_type)pos-lastPos) );
      break;
    }
    else
      if(pos != lastPos || !trimEmpty)
        tokens.push_back( ContainerT::value_type(str.data()+lastPos, 
                                                 (ContainerT::value_type::size_type)pos-lastPos) );

    lastPos = pos + 1;
  }
}*/


inline void str_split(const std::string &str, std::vector<std::string> &tokens, const std::string &delimiters = " ", 
                      bool trimEmpty = false){
  std::string::size_type pos, lastPos = 0;
  for(;;){
    pos = str.find_first_of(delimiters, lastPos);
    if(pos == std::string::npos){
      pos = str.length();
      if(pos != lastPos || !trimEmpty)
        tokens.push_back( std::string(str.data()+lastPos, pos-lastPos) );
      break;
    }
    else
      if(pos != lastPos || !trimEmpty)
        tokens.push_back( std::string(str.data()+lastPos, pos-lastPos) );

    lastPos = pos + 1;
  }
}


//split string by a deliminator, then place each piece in a vector
inline void SplitStrDelim(const std::string &s, const char delim, std::vector<std::string> &elems, bool bClear = false){
  std::stringstream ss(s);
  std::string item;

  if(bClear)
    elems.clear();

  while( std::getline(ss, item, delim) )
    elems.push_back(item);
}

#endif //__MIO_STR_H__

