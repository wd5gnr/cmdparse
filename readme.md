This is a simple class to parse command lines from a user input or a file. This version uses C++ style strings

By Al Williams, Hackaday May 2022

You can compile with -DDEMO=1 and get a simple demo program.

Example: g++ -DDEMO=1 -g -o cmd cmd.cpp

Features
-----------
* Commands call functions with an ID number, a user-defined argument, and the rest of the command line
* There are helper functions to parse strings, floats, and ints from the rest of the command line.
* Functions handle simple errors and help messages, but you can override these
* Default separator is space, tab, CR, and LF but you can adjust 

Defining Commands
-------------------------

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

Custom Print and Error Messages
----------------------------------------
The base class calls notfoundfunc and printfunc when it wants to say something. These are defaulted to static notfound and print routines. You can change these but to do so, you have to create new static members in your derived class and set them in the constructor so the base class can find them. Here's an example of a custom print routine:

This dumb subclass prints >>> at the start
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


A Few Ideas:
---------------

 - You can pass anything that will fit in a void pointer and cast it. So
   you can set arbitrary data to go to a function (for example in the
   demo, look at cmd_val which can set one of several variables).
 - You can also swtich based on the command id if you want one function
   to handle several things.
 - You can subclass to change error handling, help messages, etc.

Public API
---
* **CmdParam** - constructor generally used as in the above example with literals forming an array.
* **process** - Takes a table of commands and a command line. Note that you can pass different tables at different times as this is a class member. This allows you to have different modes.
* **notfound** - Called when a command is not found via the notfoundfunc pointer that you can override (see above)
* **print** - Called when help or notfound or anything else needs to say something. This is called via the printfunc pointer that you can override (see above).
* **getdoc** - Returns the document string for this entry.
* **help** - Delegate that prints help messages if you like. There is a version that you can use directly in the command table (you have to supply the command table pointer as an argument).
* **getfloat, getint, getuint** - Returns the next item off the command line. If you pass a bool pointer in, it will be true if the item was there.
* **gettoken, setseperators** - By default the seperator is " \t\r\n" but you can change it with setseperators. Get token returns the next token between those separators. Note, you can't have empty fields. So for example if you set the seperator to a comma and you have 1,,,,2 that's two fields, a 1 and a 2. This could probably be fixed on demand. Also note that all the other "get" functions call gettoken, so the separators are the same for all items and apply to the whole class
* **geteos** - Get the rest of the command line after reading other things you want from it.