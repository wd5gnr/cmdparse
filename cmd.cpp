// Command processor parsing class using C++ strings
// Public domain -- Williams

// For demo:
// g++ -DDEMO=1 -o cmd cmd.cpp

/*

Create a table of commands with the following format:

CmdParam commands[] = {
   { 1, "help", "Get help", help, NULL },
   { 2, "exit", "Exit program", commands, (void *)1 },
   { 3, "blahblah", "Blah blah blah", setget, &somevar },
   . . .
   { 0, "", "", NULL, NULL }
};

The number is an ID and can be anything but by convention 0 is the end
(the code doesn't look at that, though -- it uses the command name NULL or "\0")

The first string is what the user has to type to trigger the command 
(help, exit, etc.)

The second string is what the built in help processor prints for help.

The 4th argument is a function name and the 5th is a void pointer that will be
sent to the function. Functions look like:

void help(unsigned int id, void *arg, const char *p);

Here, id will be 1, arg will be NULL, and p will be whatever is left
on the command line after eating the help. 

A few ideas:
You can pass anything that will fit in a void pointer and cast it.
So you can set arbitrary data to go to a function (for example
in the demo, look at cmd_val which can set one of several variables).

You can also swtich based on the command id if you want one function
to handle several things.

You can subclass to change error handling, help messages, etc.



 */

#include <cstdio>
#include <stdlib.h>

#include "cmd.h"

// Test for empty entry
#define ISEMPTY(x) (x.length()==0)

// Static variables here
std::string CmdParam::current;
unsigned int CmdParam::index;
std::string CmdParam::sep=" \t\r\n" ;
void (*CmdParam::notfoundfunc)(const char *,const char *)=CmdParam::notfound;
void (*CmdParam::printfunc)(const char *)=CmdParam::print;


// Get float from current command line
// *valid==false if not present and safe to set valid to NULL (default)
float CmdParam::getfloat(bool *valid)
{
  float f=0.0;
  bool tvalid;
  std::string token=gettoken(&tvalid);
  if (valid) *valid=tvalid;
  if (tvalid) f=std::stof(token);
  return f;
}

// Get int from current command line
// *valid==false if not present and safe to set valid to NULL (default)
int CmdParam::getint(bool *valid)
{
  int f=0;
  bool tvalid;
  std::string token=gettoken(&tvalid);
  if (valid) *valid=tvalid;
  if (tvalid) f=std::stoi(token,NULL,0);
  return f;
}
// Get uint from current command line
// *valid==false if not present and safe to set valid to NULL (default)
unsigned int CmdParam::getuint(bool *valid)
{
  unsigned int f=0;
  bool tvalid;
  std::string token=gettoken(&tvalid);
  if (valid) *valid=tvalid;
  if (tvalid) f=std::stoul(token,NULL,0);
  return f;
}

// Get token from current command line
// *valid==false if not present and safe to set valid to NULL (default)
std::string CmdParam::gettoken(bool *valid)
{
  std::string token;
  size_t n1;
  if (valid) *valid=false;
  n1=current.find_first_not_of(sep,index);
  if (n1==std::string::npos) return token;  // all separators or end of string
  if (valid) *valid=true;
  index=current.find_first_of(sep,n1);  // find end of token
  token=current.substr(n1,(index!=std::string::npos)?index-n1:std::string::npos);
  return token;
}

// Take a table and a command line and make it happen
// Note you could have a command that sets a mode that makes
// a different table active, for example
void CmdParam::process(CmdParam *table, const char *cmdline)
{
  std::string ccmd;
    bool valid;
    current=cmdline;   // current line
    index=0;
    ccmd=gettoken(&valid);  // get the command
    if (!valid)
      {
	printfunc("Unknown error:");
	printfunc(cmdline);
	return;
      }
    // search table
    for (int i=0;table[i].cmdname.length()!=0;i++)
    {
      if (ccmd==table[i].cmdname)
        {
            // found
	    current=cmdline;
            table[i].fp(i,table[i].arg,current.substr(index).c_str());
            return;
        }
    }
    // not found
    notfoundfunc(cmdline, ccmd.c_str());
}


#if DEMO==1

void help(unsigned int n, void *arg, const char *p); // forward refs
void list(unsigned int n, void *arg, const char *p); // 
void cmd_exit(unsigned int, void *arg, const char *p) { exit(0); }
void cmd_val(unsigned int id, void *arg, const char *p);

float valueA, valueB;



/* Example */

/*

How to override notfound and print routines!
Because these are static, you have to subclass and use your new 
class. However, static calls don't go virtual so you have to 
set the corresponding base class pointer to your function.

For example, here is a dumb subclass that prints >>> at the start
of each output line for things like help and error messages.

class CmdParam1 : public CmdParam
{
protected:
  static void print(const char *msg)
  {
    static bool newline=true;
    static const char *prefix=">>> ";
    const char *p=msg;
    printf("%s%s",newline?prefix:"",msg);
    while (*p) p++;  // find end of string
    newline=*(p-1)=='\n';  // mark newline for next time
  };
public:
  CmdParam1(unsigned int iid, const char *name,const char *doc,void (*func)(unsigned int, void *,const char *),void *farg) :  CmdParam(iid,name,doc,func,farg)
  {
    printfunc=CmdParam1::print;  // use our function
// could do the same thing for notfoundfunc
  }
};

You would also need to change all the CmdParam in the code below 
to CmdParam1

*/ 
CmdParam commands[] = {
		       { 1, "help", "Get help", help, NULL },
		       { 2, "list", "Dummy list", list, NULL },
		       { 3, "exit", "Quit the program", cmd_exit, NULL },
		       { 4, "A","View/set valueA", cmd_val, &valueA},
		       { 5, "B", "View/set valueB", cmd_val, &valueB},
		       { 6, "testhelp", "Test direct help function", help, commands},

		       { 0, "", "", NULL, NULL }
};

// Set a value
void cmd_val(unsigned int id, void *arg, const char *p)
{
  bool valid;
  float v=CmdParam::getfloat(&valid), *vp=(float *)arg;
  if (valid) *vp=v;
  printf("%f\n",*vp);
}


// Our own help (delegates; see testhelp for direct method)
void help(unsigned int n, void *arg, const char *p)
{
  char token[32];
  bool valid;
  std::string tkn=CmdParam::gettoken(&valid);
  if (valid) printf("No help for %s\n",tkn.c_str());
  CmdParam::help(commands);
}

// Silly command. list shows both float values
// list 1.2 sets the first value to 1.2 and does nothing to 2nd value
// list 1.2 77.5 sets both values
void list(unsigned int n, void *arg, const char *p)
{
  static float value=0.0, value2=0.0;
  bool set;
  float f=CmdParam::getfloat(&set);
  if (set) value=f;
  f=CmdParam::getfloat(&set);
  if (set) value2=f;
  printf("%f %f\n",value,value2);
}

// Simple main

  int main(int argc, char *argv[])
  {
   char cmdline[257];
   while (1)
     {
       putchar('?');
       putchar(' ');
       fgets(cmdline,256,stdin);
       CmdParam::process(commands,cmdline);
     }
   return 0;
  }

#endif
