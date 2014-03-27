#ifndef _H_COMPUTEREX_LOGGER_H
#define _H_COMPUTEREX_LOGGER_H

#include <iostream>
#include <fstream>

class LOGGER
{
public:
       LOGGER(char * f, char * mode)
       {
		   if (strcmp(mode,"log"))
			   myfile.open(f);
		   else
			   myfile.open(f, std::ios::out | std::ios::app);
       }
	   ~LOGGER()
	   {
		   myfile.close();
	   }
       void write(const char * msg)
       {
           myfile << msg << "\n";
       }
	   void write()
	   {
		   myfile << l << "\n";
	   }
	   char * line()
	   {
		   return l;
	   }
private:
	   std::ofstream myfile;
	   char l[255];
};
#endif
