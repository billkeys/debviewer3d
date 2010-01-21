/*
 * Package.cpp - Package class
 *
 * Overview: Provides useful functions for working with Ubuntu packages
 *
 * Author: William Keys
 * Created: 7 Nov 2009
 * Modified: 3 Dec 2009
*/

#include "Package.h"

Package::Package(string pname, string plicence,string pversion,string psection,string psize) {
      name	= pname;
      licence	= plicence;
      version	= pversion;
      section	= psection;
      size	= psize;
}

Package::Package() { }

// gets useful information about a defined package
Package Package::get_package_info(string pname) {
   FILE *in;
   char cmd[300] = "apt-cache show ";
   regex reSection("Section: (.*)\n");
   regex reVersion("Version: (.*)\n");
   regex reSize("Size: (.*)\n");
   cmatch matches;
   string version = "Unknown";
   string section = "Unknown";
   string size = "Unknown";

   strcat ( cmd, pname.c_str() );
   char buff[512];

   /* popen creates a pipe so we can read the output of the program we are invoking */
   if (!(in = popen(cmd, "r"))) {
      exit(1);
   }
   /* read the output of netstat, one line at a time */
   while (fgets(buff, sizeof(buff), in) != NULL ) {
     if (regex_match(buff, matches, reVersion))
        version = matches[1];
     if (regex_match(buff, matches, reSection))
        section = matches[1];
     if (regex_match(buff, matches, reSize))
        size = matches[1];
   }

   /* close the pipe */
   pclose(in);

   Package packageInfo = Package(pname,"Unknown", version,section,size); 
   return packageInfo;
}

// returns a list of depences of a given pname
list< pair<string,string> > Package::get_package_dep(string pname) {
    list< pair<string,string> > some_rules;

    FILE *in;
    char cmd[300] = "apt-rdepends -d ";
    regex expression(".*\"(.*)\".*\"(.*)\".*");
    cmatch matches;

    strcat ( cmd,  pname.c_str() );
    char buff[512]; 

    /* popen creates a pipe so we can read the output of the program we are invoking */
    if (!(in = popen(cmd, "r"))) {
     exit(1);
    }

    /* read the output of netstat, one line at a time */
    while (fgets(buff, sizeof(buff), in) != NULL ) {
      if (regex_match(buff, matches, expression)) {

          if (matches[1] == pname) 
          {
            some_rules.push_back(make_pair(matches[1],matches[2]) );
         }
      }
    }

    /* close the pipe */
    pclose(in);

    //return package_map;
    return some_rules;
}

// gets a set of all the packages needed to be displayed
set<string> Package::get_packages(string pname)
{
    FILE *in;
    char cmd[300] = "apt-rdepends -v ";
    set<string> package_set;
    regex expression("node:.*\"(.*)\".*\".*\".*");
    cmatch matches;

    strcat ( cmd,  pname.c_str() );
    char buff[512];

    /* popen creates a pipe so we can read the output of the program we are invoking */
    if (!(in = popen(cmd, "r"))) {
       exit(1);
    }
    /* read the output of netstat, one line at a time */
    while (fgets(buff, sizeof(buff), in) != NULL ) {
      if (regex_match(buff, matches, expression)) {
          package_set.insert(matches[1]);
      }
   }
   /* close the pipe */
   pclose(in);

   return package_set;
}

