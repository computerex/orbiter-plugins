#include "Parser.h"
#include <fstream>

void PARSER::tokenize(const std::string& str,
			  std::vector<std::string>& tokens,
			  const std::string& delimiters)
{
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos)
    {
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        lastPos = str.find_first_not_of(delimiters, pos);
        pos = str.find_first_of(delimiters, lastPos);
    }
}

void PARSER::read(void)
{
    std::string line;
    std::ifstream myfile (file);
    if (myfile.is_open())
    {
        int num = 0;
        while (! myfile.eof() )
        {
            getline (myfile,line);
            size_t CMT_Check = line.find(";");
            if (CMT_Check!=std::string::npos)
            {
                line.erase(CMT_Check);
                if (CMT_Check == 0)
                    continue;
            }
			if (line.length() < 1)
				continue;
            std::vector<std::string> tokens;
            tokenize(line,tokens,"=");
            Convert(tokens);
            lines.push_back(LINE(tokens, line, num));
            num++;
        }
        myfile.close();
    }
}
void PARSER::read(char * fname)
{
     lines.clear();
     strcpy(file,fname);
     read();
}
void PARSER::Convert(std::vector<std::string> &tokens)
{
	if (tokens.size() < 1)
		return;
	std::string arg = tokens[0];
    KillSpaces(arg);
    std::string params = tokensToString(tokens,1);
    tokens.clear();
    tokens.push_back(arg);
    std::vector<std::string> pms;
    tokenize(params,pms);
    for (size_t i = 0; i < pms.size(); i++)
        tokens.push_back(pms[i]);
}
void PARSER::reset(void)
{
     lines.clear();
}
bool PARSER::find(std::string str, LINE &l)
{
     for (size_t i = 0; i < lines.size(); i++)
     {
         for (size_t j = 0; j < lines[i].tok.size(); i++)
         {
             if (!strcasecmp(str.c_str(), lines[i].tok[j].c_str()))
             {
                 l = lines[i];
                 return true;
             }
         }
     }
             
     return false;
}        
bool PARSER::findParam(std::string str, LINE & l)
{
     for (size_t i = 0; i < lines.size(); i++)
     {
         if (!strcasecmp(str.c_str(),lines[i].tok[0].c_str()))
         {
                 l = lines[i];
                 return true;
         }
     }
     return false;
}
bool PARSER::hasArg(LINE & l)
{
     if (l.tok.size() < 1)
         return false;
     return true;
}
void PARSER::KillSpaces(std::string  &str)
{
     for (size_t i=0;i<str.size();i++)
     {
		 if (str[i] == ' ')
			 str.erase(i,1);
     }
}
std::string PARSER::tokensToString(std::vector<std::string> tokens, int inx)
{
    char cbuf[255];
    
    for (size_t i = inx; i < tokens.size(); i++)
    {
        if (i == inx)
        {
              sprintf(cbuf,"%s",tokens[i].c_str());
              continue;
        }
        sprintf(cbuf,"%s %s",cbuf,tokens[i].c_str());
    }
    return std::string(cbuf);
}
std::string PARSER::GetArg(std::string str)
{
     LINE l;
     if (!findParam(str, l))
         return "Not Found";
     
     return tokensToString(l.tok,1);
}
double PARSER::ToDouble(std::string str)
{
       char cbuf[255];
       sprintf(cbuf,"%s",str.c_str());
       return atof(cbuf);
}
int    PARSER::ToInt(std::string str)
{
       char cbuf[255];
       sprintf(cbuf,"%s",str.c_str());
       return atoi(cbuf);
}
bool   PARSER::ToBool(std::string str)
{
       if (!strcasecmp(str.c_str(), "true"))
           return true;
       if (!strcasecmp(str.c_str(), "false"))
           return false;
       return false;
}
std::string PARSER::ToString(double d)
{
     char cbuf[255];
     sprintf(cbuf,"%f",d);
     return std::string(cbuf);
}
std::string PARSER::ToString(VECTOR3 v)
{
	char cbuf[255];
	sprintf(cbuf,"%f %f %f",v.x,v.y,v.z);
	return std::string(cbuf);
}
std::string PARSER::ToString(int    i)
{ 
     char cbuf[255];
     sprintf(cbuf,"%d",i);
     return std::string(cbuf);
}
std::string PARSER::ToString(bool   b)
{
     if (b)
        return "true";
     else
        return "false";
}
double PARSER::Double(std::string str)
{
       std::string rVal = GetArg(str);
       if (rVal == "Not Found")
          return 0;
       return ToDouble(rVal);
}
int    PARSER::Int(std::string str)
{
       std::string rVal = GetArg(str);
       if (rVal == "Not Found")
          return 0;
       return ToInt(rVal);
}
bool   PARSER::Bool(std::string str)
{
       std::string rVal = GetArg(str);
       if (rVal == "Not Found")
          return 0;
       return ToBool(rVal);
}
char * PARSER::String(std::string str, char * cbuf)
{
     std::string rVal = GetArg(str);
     if (rVal == "Not Found")
         strcpy(cbuf,"");
     else
         strcpy(cbuf,rVal.c_str());
     return cbuf;         
}
std::vector<double> PARSER::Vector(std::string str, int a)
{
	std::string value;
	char cbuf[255];
	std::vector<std::string> tokens;
	String(str,cbuf);
	value=cbuf;
	tokenize(value,tokens);
	std::vector<double> r;
	for (int i = 0; i < a; i++)
		r.push_back(ToDouble(tokens[i]));
	return r;
}
VECTOR3 PARSER::Vector3(std::string str)
{
	std::vector<double> dbl = Vector(str,3);
	return _V(dbl[0],dbl[1],dbl[2]);
}