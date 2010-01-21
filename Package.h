/*
 * Package.h - Package header
 *
 * Overview: Defines functions implemented in Package.cpp
 *
 * Author:  William Keys
 * Created: 7 Nov 2009
 * Modified: 1 Dec 2009
*/

#include <stdlib.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <cstring>
#include <boost/regex.hpp>
#include <set>
#include <list>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <cstring>
#include <set>
#include <list>

using namespace std;
using namespace boost;

class Package
{
  public:
     string name;
     string licence;
     string version;
     string section;
     string size;
  public:
    // Constructor
    Package(string pname, string plicence,string pversion,string psection,string psize);
    Package();

    // gets information about the defined package
    Package get_package_info(string pname);

    // returns a list of depences of a given pname
    list< pair<string,string> > get_package_dep(string pname);

    // returns a set of all the packages needed to be displayed
    set<string> get_packages(string pname);
};
