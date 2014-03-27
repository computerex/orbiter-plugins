#ifndef _H_COMPUTEREX_PARSER_FILE_H
#define _H_COMPUTEREX_PARSER_FILE_H

#define _CRT_SECURE_NO_DEPRECATE

#include <vector>
#include <string>
#include <stdio.h>
#include <orbitersdk.h>

class LINE
{
public:
       LINE()  : n(0) {}
       LINE(std::vector<std::string> t, std::string ln, int l)
       {
            tok  = t;
            line = ln;
            n    = l; 
       }
       std::vector<std::string> tok;
       std::string line;
       int n;
};

class PARSER
{
public:
      PARSER(char * fname)
      {
           strcpy(file,fname);
      }
      PARSER()  {}
      void read();
      void read(char * fname);
      void tokenize(const std::string& str,
			  std::vector<std::string>& tokens,
			  const std::string& delimiters = " ");
	  void reset(void);	  
	  bool find(std::string str, LINE &l);
	  bool findParam(std::string str, LINE & l);
	  bool hasArg(LINE & l);
	  std::string GetArg(std::string param);
	  void KillSpaces(std::string &str);
	  void Convert(std::vector<std::string> &tokens);
	  std::string tokensToString(std::vector<std::string> tokens, int inx = 0);
	  double ToDouble(std::string str);
	  int    ToInt(std::string str);
	  bool   ToBool(std::string str);
	  std::string ToString(double  n);
	  std::string ToString(int     n);
	  std::string ToString(bool    n);
	  std::string ToString(VECTOR3 n);
      double Double(std::string str);
	  int    Int(std::string str);
	  bool   Bool(std::string str);
	  char*  String(std::string str, char * cbuf);
	  std::vector<double> Vector(std::string str, int a);
	  VECTOR3 Vector3(std::string str);
	  std::vector<LINE> lines;
      char file[255];
};

#endif
