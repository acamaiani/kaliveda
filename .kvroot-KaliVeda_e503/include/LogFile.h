#ifndef _LOGFILE_CLASS
#define _LOGFILE_CLASS

#include "Rtypes.h"
#include"Defines.h"
#include "Riostream.h"

#include <string>
#include <sstream>

enum LogLevel{LOG_LOW, LOG_NORMAL, LOG_HIGH};

class LogFile
{
 public:
  LogFile();
  virtual ~LogFile(void);
  
  
  Char_t LogFileName;
  ofstream Log;
  void Open(char *LogFileName); 
  void Close();
  
    void Message(const char *location, const char *msg);
    string GetTime();

  ClassDef(LogFile,0)
};

#endif

