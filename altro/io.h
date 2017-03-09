#ifndef __MIO_IO_H__
#define __MIO_IO_H__

#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <vector>
#include <algorithm>
#include <string>
#include <sys/stat.h>
#include <unistd.h> // readlkink()
#include <sys/stat.h> // lstat()
#include <time.h>
#include <iomanip>
#include "mio/altro/error.h"
#include "mio/altro/macros.h"


namespace mio{

inline time_t GetEpochTime(const uint32_t year, const uint8_t month, const uint8_t day,
                    const uint8_t hour, const uint8_t min, const uint8_t sec){
  EXP_CHK(year >= 1900, return(0))
  struct tm t = {0};
  t.tm_year = year-1900;
  t.tm_mon = month;
  t.tm_mday = day;
  t.tm_hour = hour;
  t.tm_min = min;
  t.tm_sec = sec;
  return mktime(&t);
}

struct TimeStamp{
  uint32_t year;
  uint8_t month, day, hour, min, sec;
};

inline time_t GetEpochTime(const TimeStamp ts){
  return GetEpochTime(ts.year, ts.month, ts.day, ts.hour, ts.min, ts.sec);
}


//d - day, range [01,31])
//m - month, range [01,12]
//Y - year as a 4 digit decimal number
//H - hour, range [00-23]
//M - minute, range [00,59]
//S - second, range [00,60]
inline std::string GetDateString(){
  std::time_t t = std::time(nullptr);
  std::tm *tm = std::localtime(&t);
  std::ostringstream oss;
#if __GNUC__ >= 5
  oss << std::put_time(tm, "%d-%m-%Y_%H-%M-%S");
#endif
  return oss.str();
}


class FileDescript;
inline void FileNameExpand(FileDescript &fd);


typedef class FileDescript{
  public:
    std::string file_full, ext_sep, file_path, file_name, file_name_no_ext, file_ext;
    long long int file_size;
    int num;
    time_t epoch_time_;

    FileDescript() : ext_sep("."){};

    FileDescript(std::string file_full_, std::string ext_sep_ = ".") :
                  file_full(file_full_), ext_sep(ext_sep_) {};

    void ExpandFileName(){
      if( !(file_full.empty() || ext_sep.empty()) )
        FileNameExpand(*this);
    }

    void Print(){
      printf( "file_full: %s\next_sep: %s\nfile_path: %s\nfileName: %s\nfileNameNoExt: %s\nfileExt: %s\n",
              file_full.c_str(), ext_sep.c_str(), file_path.c_str(),
              file_name.c_str(), file_name_no_ext.c_str(), file_ext.c_str() );
    }

    static void GetFileNameVec(const std::vector<mio::FileDescript> &fd_vec, std::vector<std::string> &fn_vec){
      fn_vec.clear();
      fn_vec.reserve(fd_vec.size());
      for(const mio::FileDescript &fd : fd_vec)
        fn_vec.push_back(fd.file_name);
    }

    COMPARE_VAL_LESS_THAN(FileDescript, file_size) // defines FileDescript::file_size_less_than
    COMPARE_VAL_GREATER_THAN(FileDescript, file_size) // defines FileDescript::file_size_greater_than

    COMPARE_VAL_LESS_THAN(FileDescript, num)
    COMPARE_VAL_GREATER_THAN(FileDescript, num)

    COMPARE_VAL_LESS_THAN_(FileDescript, epoch_time_)
    COMPARE_VAL_GREATER_THAN_(FileDescript, epoch_time_)

    COMPARE_VAL_LESS_THAN(FileDescript, file_name)
    COMPARE_VAL_GREATER_THAN(FileDescript, file_name)
} fileDescript_t;


inline int PathType(const char *file_full){
  struct stat stbuf;

  if(stat(file_full, &stbuf) == 0){
    if(stbuf.st_mode & S_IFDIR) //it's a directory
      return 0;
    else if(stbuf.st_mode & S_IFREG) //it's a file
      return 1;
    else
      return 2;
  }
  else{
    perror("PathType() - stat()");
    return -1;
  }
}

inline int PathType(const std::string &file_full){
  return PathType( file_full.c_str() ) ;
}


inline bool FileExists(const char *file_full){
  if(strlen(file_full) > 0)
    if( FILE *fd = fopen(file_full, "r") ){
      fclose(fd);
      return true;
    }
  return false;
}

inline bool FileExists(const std::string &file_full){
  if( FILE *file = fopen(file_full.c_str(), "r") ){
    fclose(file);
    return true;
  }
  return false;
}


inline bool DirExists(const std::string &dir_path, const bool create = false, const mode_t mkdir_mode = 0777){
  const bool exists = (PathType(dir_path) == 0);
  if(!exists && create){
    printf("%s - creating directory: %s\n", CURRENT_FUNC, dir_path.c_str());
    mkdir(dir_path.c_str(), mkdir_mode);
  }
  return exists;
}

inline bool DirExists(const char *dir_path, const bool create = false, const mode_t mkdir_mode = 0777){
  return DirExists(std::string(dir_path), create, mkdir_mode);
}


inline long long int GetFileSize(const std::string file_full){
  struct stat stbuf;
  long long int llnSize = 0;
  EXP_CHK_ERRNO(stat(file_full.c_str(), &stbuf) == 0, return(-1))
  //get the size of all files in a directory
//  if( (stbuf.st_mode & S_IFMT) == S_IFDIR )
//    lintsize = dirwalk(name, fsize);

  llnSize = llnSize + stbuf.st_size;
  return(llnSize);
}


//formating a file path forces the constraint that a file path must NOT end with a '/'
inline void FormatFilePath(std::string &file_path){
  if( !( file_path.empty() ) )
    if(file_path[file_path.size() - 1] == '/')
      file_path.pop_back();
}


//if the extension contains a '.' as the first character, than it is removed
inline void FormatFileExt(std::string &file_ext){
  if( !( file_ext.empty() ) )
    if(file_ext[0] == '.')
      file_ext.erase(0, 1);
}


//Directory constants are the '.' and '..' files always present in a directory
inline int GetDirList(const std::string dir_path, std::vector<std::string> &dir_list_vec,
                       const bool Remove_dir_constants, const bool print_flag = false){
  struct dirent *entry;
  DIR *dp;
  unsigned int dir_list_size = 0, idx;
  std::string name;

  EXP_CHK_ERRNO((dp = opendir(dir_path.c_str())) != NULL, return(-1))

  while( ( entry = readdir(dp) ) != 0 )
    dir_list_size++;
  rewinddir(dp);
  dir_list_vec.clear();
  dir_list_vec.resize(dir_list_size);

  idx = 0;
  while( ( entry = readdir(dp) ) != 0 ){
    name = std::string(entry->d_name);
    dir_list_vec[idx] = name;
    //printf("inode number: %d\n", entry->d_ino);
    idx++;
  }

  EXP_CHK_ERRNO(closedir(dp) == 0, return(-1));

  std::sort(dir_list_vec.begin(), dir_list_vec.end());
  if(print_flag)
    for(size_t i = 0; i < dir_list_vec.size(); i++)
      printf( "%s\n", dir_list_vec[i].c_str() );

  if(Remove_dir_constants)
    for(size_t i = 0; i < dir_list_vec.size();){
      if( (dir_list_vec[i] == ".") || (dir_list_vec[i] == "..") )
        dir_list_vec.erase(dir_list_vec.begin() + i);
      else
        i++;
    }

  return 0;
}


inline size_t GetDirList(std::string file_path, const std::vector<std::string> &file_prefix_vec,
                         std::string file_ext, std::vector<std::string> &filt_dir_list_vec){
  mio::FormatFilePath(file_path);
  mio::FormatFileExt(file_ext);
  EXP_CHK(mio::FileExists(file_path), return(0))
  // must be filtering by extension and/or prefix
  EXP_CHK(file_prefix_vec.size() > 0 || file_ext.size() > 0, return(0))
  const bool check_ext = file_ext.size() > 0,
             check_prefix = file_prefix_vec.size() > 0;
  if(check_prefix)
    for(size_t i = 0; i < file_prefix_vec.size(); ++i){
      EXP_CHK_M(file_prefix_vec[i].size() > 0, return(0), "occured at i=" + std::to_string(i))
    }

  const size_t file_ext_size = file_ext.size();
  std::vector<std::string> dir_list_vec;
  mio::GetDirList(file_path, dir_list_vec, true);
  filt_dir_list_vec.clear();
  filt_dir_list_vec.reserve(dir_list_vec.size());

  for(std::string &file_name : dir_list_vec){
    bool match = false;
    if(check_prefix)
      for(const std::string &file_prefix : file_prefix_vec)
        if(file_name.compare(0, file_prefix.size(), file_prefix) == 0){
          match = true;
          break;
        }
    if(check_ext)
      match = (file_name.compare(file_name.size() - file_ext_size, file_ext_size, file_ext) == 0);
    if(match)
      filt_dir_list_vec.push_back(file_name);
  }

  EXP_CHK(filt_dir_list_vec.size() > 0, return(0))
  return filt_dir_list_vec.size();
}


/*
  example:
    file_full: /home/sam/Desktop/Ottawa/car/000106car16.upc
    ext_sep: .

  returns:
    file_path: /home/sam/Desktop/Ottawa/car
    file_name: 000106car16.upc
    file_name_no_ext: 000106car16
    file_ext: upc
*/
inline void FileNameExpand(const std::string file_full, const std::string ext_sep, std::string &file_path,
                           std::string &file_name, std::string &file_name_no_ext, std::string &file_ext){
  if( file_full.empty() )
    return;

  size_t path_sep_pos = file_full.find_last_of("/\\"),
         ext_sep_pos = file_full.rfind(ext_sep);

  file_path = (path_sep_pos == std::string::npos) ? "" : file_full.substr(0, path_sep_pos);
  file_name = file_full.substr(path_sep_pos + 1);

  if(ext_sep_pos == std::string::npos)
    file_name_no_ext = file_ext = "";
  else{
    file_name_no_ext = file_full.substr(path_sep_pos + 1, ext_sep_pos - path_sep_pos - 1);
    file_ext = file_full.substr( ext_sep_pos + ext_sep.size() );
  }
}


inline void FileNameExpand(const std::string file_full, const std::string ext_sep, std::string *file_path,
                           std::string *file_name, std::string *file_name_no_ext, std::string *file_ext){
  std::string file_path_temp, file_name_temp, file_name_no_ext_temp, file_ext_temp;
  FileNameExpand(file_full, ext_sep,
                  file_path ? *file_path : file_path_temp,
                  file_name ? *file_name : file_name_temp,
                  file_name_no_ext ? *file_name_no_ext : file_name_no_ext_temp,
                  file_ext ? *file_ext : file_ext_temp);
}

inline void FileNameExpand(const char *file_full, const char *ext_sep, char *file_path,
                           char *file_name, char *file_name_no_ext, char *file_ext){
  if( (file_full != NULL) && (ext_sep != NULL) ){
    std::string file_path_, file_name_, file_name_no_ext_, file_ext_;

    FileNameExpand(std::string(file_full), std::string(ext_sep),
                   file_path_, file_name_, file_name_no_ext_, file_ext_);
    if(file_path != NULL)
      strcpy( file_path, file_path_.c_str() );
    if(file_name != NULL)
      strcpy( file_name, file_name_.c_str() );
    if(file_name_no_ext != NULL)
      strcpy( file_name_no_ext, file_name_no_ext_.c_str() );
    if(file_ext != NULL)
      strcpy( file_ext, file_ext_.c_str() );
  }
}


inline void FileNameExpand(FileDescript &fd){
  FileNameExpand(fd.file_full, fd.ext_sep, fd.file_path, fd.file_name, fd.file_name_no_ext, fd.file_ext);
}


//if old_sub_str is empty, possibly an infinite loop
inline void ReplaceSubStr(std::string &str, const std::string old_sub_str, const std::string new_sub_str){
  size_t pos = 0;
  if( str.empty() || old_sub_str.empty() )
    return;

  while( ( pos = str.find(old_sub_str, pos) ) != std::string::npos ){
     str.replace(pos, old_sub_str.length(), new_sub_str);
     pos += new_sub_str.length();
  }
}


inline void ReplaceSubStr(char *str, const char *old_sub_str, const char *new_sub_str){
  if(str != NULL){
    std::string str_ = std::string(str);
    ReplaceSubStr( str_, std::string(old_sub_str), std::string(new_sub_str) );

    strcpy( str, str_.c_str() );
  }
}


inline bool ForceFileExtension(std::string &file_full, const std::string file_ext_wanted){
  std::string file_path, file_name_no_ext, file_ext_current;
  FileNameExpand(file_full, ".", &file_path, nullptr, &file_name_no_ext, &file_ext_current);
  if(file_ext_current != file_ext_wanted){
    file_full = file_path + "/" + file_name_no_ext + "." + file_ext_wanted;
    return true;
  }
  return false;
}


//on success, sets resolved_symlink and returns the length of resolve_symlink
//TODO - doesn't call free() if error occurs
inline int ResolveSymlink(const char *symlink, char *&resolved_symlink){
  struct stat stat_buf;

  if(lstat(symlink, &stat_buf) != 0){
    perror("lstat() error");
    return -1;
  }

  if((stat_buf.st_mode & S_IFMT) == S_IFLNK){
    //for symlinks, st_size is the length of the target path without a terminating null byte
    resolved_symlink = (char *)malloc(stat_buf.st_size + 1);
    const int rv = readlink(symlink, resolved_symlink, stat_buf.st_size + 1);
    if(rv == -1){
      perror("readlink() error");
      return -1;
    }
    if(rv != stat_buf.st_size){
      printf("resolve_symlink() error : symlink changed in size between lstat() and readlink()\n");
      return -1;
    }

    resolved_symlink[stat_buf.st_size] = '\0';
    return stat_buf.st_size;
  }
  else
    printf("resolve_symlink() : not a symbolic link\n");

  return -1;
}


inline void ResolveSymlink(const std::string symlink, std::string &resolved_symlink){
  resolved_symlink.clear();
  char *resolved_symlink_c;
  EXP_CHK(ResolveSymlink(symlink.c_str(), resolved_symlink_c) != -1, return)
  resolved_symlink = std::string(resolved_symlink_c);
  free(resolved_symlink_c);
}


//udev rules will create a sybolic link whose target is not an abosolute path. This function
//first resolves the symbolic link path and then appends it to a path (which is most likely "/dev/")
inline void ResolveUdevSymlink(const std::string symlink, std::string &resolved_symlink){
  std::string resolved_symlink_temp, file_path;
  ResolveSymlink(symlink, resolved_symlink_temp);
  FileNameExpand(symlink, ".", &file_path, nullptr, nullptr, nullptr);
  resolved_symlink = file_path + "/" + resolved_symlink_temp;
}

} // namespace mio

#endif //__MIO_IO_H__

