/* This file implements a singly linked list using the specification from 
   http://www.sgi.com/Technology/STL/Slist.html
   It uses an internal doubly linked list and acts as a wrapper for slist 
   member functions, invoking the corresponding list member functions. It
   is intended to be used for portability purposes and not for efficiency. 
   The standard does not contain definition for slist, and consequently
   KAI and other standard conforming compilers may not support slist. However,
   code written for STL prior to the ratification of the standard may need this.
   Credits: Program Database Toolkit team at University of Oregon.
   see http://www.acl.lanl.gov/pdtoolkit for more information. 
   Contact: pdtoolkit@acl.lanl.gov
*/
#ifndef _PDT_SLIST_H
#define _PDT_SLIST_H

#include <slist>
using namespace std;

#endif /* _PDT_SLIST_H */
