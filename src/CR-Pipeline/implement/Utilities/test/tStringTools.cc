/***************************************************************************
 *   Copyright (C) 2004,2005                                               *
 *   Lars B"ahren (bahren@astron.nl)                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/* $Id: tStringTools.cc,v 1.3 2006/10/31 18:24:08 bahren Exp $ */

#include <lopes/Utilities/StringTools.h>

// ------------------------------------------------------------------ string2char

int string2char ()
{
  int nofFailedTests (0);
  int maxlen (30);
  string teststring ("");

  for (int i=0; i<maxlen; i++) {
    teststring += "a";
    //
    char* tmp = LOPES::string2char(teststring);
    //
    cout << i+1 << "\t[" << teststring << "] -> ["
	 << tmp << "]" << endl;
  }

  return nofFailedTests;
}

// ------------------------------------------------------------------------- main

/*!
  \file tStringTools.cc

  \brief A collection of tests for StringTools.

  \author Lars B&auml;hren

  \date 2005/03/02
*/

int main ()
{
  int nofFailedTests (0);
  String filepath = "/home/user/bin/myfile.h";

  String filename = LOPES::fileFromPath (filepath);
  String dirname = LOPES::dirFromPath (filepath);
  Vector<String> substrings = LOPES::getSubstrings (filepath,"/");
  Int nofSubstrings = LOPES::nofSubstrings (filepath,"/");

  cout << "\n[tStringTools]\n" << endl;
  cout << " - Complete path to file : " << filepath << endl;
  cout << " - Name of file ........ : " << filename << endl;
  cout << " - Directory name ...... : " << dirname << endl;
  cout << " - Substrings .......... : " << substrings << endl;
  cout << " - Number of substrings  : " << nofSubstrings << endl;

  cout << endl;

  {
    nofFailedTests += string2char ();
  }

  return nofFailedTests;

}
