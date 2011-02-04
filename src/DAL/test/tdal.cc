/***************************************************************************
 *   Copyright (C) 2006 by Joseph Masters                                  *
 *   jmasters@science.uva.nl                                               *
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

/*!
  \file tdal.cc
  \ingroup DAL
  \brief Test program for basic DAL functionality.
  \author Joseph Masters, Lars B&auml;hren
*/

#include <dal.h>
#include <cassert>
#include <string>

using std::cout;
using std::endl;
using std::complex;
using namespace DAL;

//_______________________________________________________________________________
//                                                             test_hdf5_datafile

/*
  \param filename  -- Name of the HDF5 file used for testing
  \param groupname -- Name of the group created within the file.
  \return status   -- Error status of the function; returns positive value in
          case an error was encountered.
*/

uint test_hdf5_datafile (std::string const &filename,
			 std::string const &groupname)
{
  cout << "\n[tdal::test_hdf5_datafile]\n" << endl;
  cout << "-- Filename  = " << filename    << endl;
  cout << "-- Groupname = " << groupname   << endl;

  uint status = 0;
  
  /*__________________________________________________________________
    Test 1: Testing default constructor dalDataset()
  */
  cout << "[1] Testing dalDataset() ..." << endl;
  try {
    dalDataset ds;
    ds.summary();
  } catch (std::string message) {
    std::cerr << message << endl;
    ++status;
  }

  /*__________________________________________________________________
    Test 2: Open and close HDF5 dataset
  */
  cout << "[2] Testing dalDataset(string,string) ..." << endl;
  try {
    dalDataset ds (filename.c_str(), "HDF5");
    ds.summary();
  } catch (std::string message) {
    std::cerr << message << endl;
    ++status;
  }

  /*__________________________________________________________________
    Test 3: Set attributes attached to HDF5 dataset
  */
  cout << "[3] Set attributes attached to HDF5 dataset ..." << endl;
  try {
    dalDataset ds (filename);
    ds.summary();
    
    std::string sval = "string test value";
    if ( DAL::FAIL == ds.setAttribute( "STRING_ATTR", sval ) )
      status = false;
    
    std::vector<std::string> svals;
    svals.push_back("string");
    svals.push_back("vector");
    svals.push_back("test");
    if ( DAL::FAIL == ds.setAttribute_string( "STRING_ATTRS", svals ) )
      status++;
    
    int ival = 1;
    if ( DAL::FAIL == ds.setAttribute( "ATTRIBUTE_INT", &ival ) )
      status++;
    
    int ivals[] = { 1, 2, 3 };
    if ( DAL::FAIL == ds.setAttribute( "INT_ATTRS", ivals, 3 ) )
      status++;
    
    uint uival = 2;
    if ( DAL::FAIL == ds.setAttribute( "UINT_ATTR", &uival ) )
      status++;
    
    uint uivals[] = { 1, 2, 3};
    if ( DAL::FAIL == ds.setAttribute( "UINT_ATTRS", uivals, 3 ) )
      status++;
    
    float fval = 3.0;
    if ( DAL::FAIL == ds.setAttribute( "ATTRIBUTE_FLOAT", &fval ) )
      status++;
    
    float fvals[] = { 1.0, 2.0, 3.0 };
    if ( DAL::FAIL == ds.setAttribute( "FLOAT_ATTRS", fvals, 3 ) )
      status++;
    
    double dval = 3.0;
    if ( DAL::FAIL == ds.setAttribute( "DOUBLE_ATTR", &dval ) )
      status++;
    
    double dvals[] = { 1.0, 2.0, 3.0 };
    if ( DAL::FAIL == ds.setAttribute( "DOUBLE_ATTRS", dvals, 3 ) )
      status++;

    if ( DAL::FAIL == ds.close() ) {
      status++;
    }

  } catch (std::string message) {
    std::cerr << message << endl;
    ++status;
  }

  /*__________________________________________________________________
    Test 4: Create HDF5 group attached to the root level of the file
  */
  cout << "[4] Create HDF5 group attached to the root level of the file ..." << endl;
  try {
    dalDataset ds (filename.c_str(), "HDF5" );
    dalGroup * group = ds.createGroup ("group");
    
    if ( NULL == group ) {
      status++;
    } else {
      if ( DAL::FAIL == group->close() ) {
	status++;
      }
    }
    
    delete group;
  } catch (std::string message) {
    std::cerr << message << endl;
    ++status;
  }
  
  return status;
}

//_______________________________________________________________________________
//                                                      create_hdf5_integer_array

/*
  \param filename -- Name of the HDF5 file used for testing
  \return status  -- Error status of the function; returns positive value in
          case an error was encountered.
*/
uint create_hdf5_integer_array (std::string const &filename)
{
  uint ret (0);
  dalDataset ds (filename.c_str(), "HDF5" );

  // define dimensions of array
  vector<int> dims;
  dims.push_back(4);
  dims.push_back(5);
  dims.push_back(6);
  vector<int> cdims;

  unsigned int nelem (1);
  for (unsigned int n=0; n<dims.size(); ++n) {
    nelem *= dims[n];
  } 

  int data[nelem];
  for (unsigned int gg=0; gg<nelem; gg++)
    data[gg] = gg;

  dalArray * array = ds.createIntArray( "int_array", dims, data, cdims );
  if ( NULL == array )
    ret++;

  if ( DAL::FAIL == array->close() )
    ret++;

  delete array;

  return ret;
}

// ----------------------------------------------- read_hdf5_integer_array

uint read_hdf5_integer_array (std::string const &filename)
{
  uint ret = 0;

  dalDataset ds;

  ds = dalDataset( filename.c_str(), "HDF5" );

  dalArray * array = ds.openArray( "int_array" );
  if ( NULL == array )
    ret++;

  std::vector<int> dims =  array->dims();
  if ( dims.size() > 0 )
    {
      std::cerr << "array dimensions = ( ";
      for ( uint idx = 0; idx < dims.size(); idx++ )
        std::cerr << dims[idx] << ',';
      std::cerr << " )\n";
    }

  if ( DAL::FAIL == array->close() )
    ret++;

  delete array;

  return ret;
}

//_______________________________________________________________________________
//                                                        create_hdf5_float_array

/*
  \param filename -- Name of the HDF5 file used for testing
  \return status  -- Error status of the function; returns positive value in
          case an error was encountered.
*/
uint create_hdf5_float_array (std::string const &filename)
{
  uint ret = 0;

  dalDataset ds;

  ds = dalDataset( filename.c_str(), "HDF5" );

  // define dimensions of array
  vector<int> dims;
  dims.push_back(4);
  dims.push_back(5);
  dims.push_back(6);
  vector<int> cdims;

  float data[4*5*6];
  for (int gg=0; gg<(4*5*6); gg++)
    data[gg] = rand();

  dalArray * array = ds.createFloatArray( "float_array", dims, data, cdims );
  if ( NULL == array )
    ret++;

  if ( DAL::FAIL == array->close() )
    ret++;

  delete array;

  return ret;
}

//_______________________________________________________________________________
//                                                create_hdf5_complex_float_array

/*
  \param filename -- Name of the HDF5 file used for testing
  \return status  -- Error status of the function; returns positive value in
          case an error was encountered.
*/
uint create_hdf5_complex_float_array (std::string const &filename)
{
  uint ret = 0;

  dalDataset ds;

  ds = dalDataset( filename.c_str(), "HDF5" );

  // define dimensions of array
  vector<int> dims;
  dims.push_back(4);
  dims.push_back(5);
  dims.push_back(6);
  vector<int> cdims;

  complex<float> * data = new complex<float>[ 4*5*6 ];
  for (int gg=0; gg<(4*5*6); gg++)
    data[ gg ] = gg;

  dalArray * array = ds.createComplexFloatArray( "complex_float_array",
                     dims, data, cdims );

  if ( NULL == array )
    ret++;

  if ( DAL::FAIL == array->close() )
    ret++;

  delete [] data;
  data = NULL;
  delete array;

  return ret;
}

//_______________________________________________________________________________
//                                                      set_attributes_hdf5_array

/*
  \param filename -- Name of the HDF5 file used for testing
  \return status  -- Error status of the function; returns positive value in
          case an error was encountered.
*/
uint set_attributes_hdf5_array (std::string const &filename)
{
  uint ret = 0;

  dalDataset ds;

  ds = dalDataset( filename.c_str(), "HDF5" );

  dalArray * array = NULL;
  array = ds.openArray( "int_array" );
  if ( NULL == array )
    ret++;

  std::string sval = "string test value";
  if ( DAL::FAIL == array->setAttribute( "STRING_ATTR", sval ) )
    ret = false;

  std::string svals[]= {"string","vector","test"};
  if ( DAL::FAIL == array->setAttribute( "STRING_ATTRS", svals, 3 ) )
    ret++;

  int ival = 1;
  if ( DAL::FAIL == array->setAttribute( "ATTRIBUTE_INT", &ival ) )
    ret++;

  int ivals[] = { 1, 2, 3 };
  if ( DAL::FAIL == array->setAttribute( "INT_ATTRS", ivals, 3 ) )
    ret++;

  uint uival = 2;
  if ( DAL::FAIL == array->setAttribute( "UINT_ATTR", &uival ) )
    ret++;

  uint uivals[] = { 1, 2, 3};
  if ( DAL::FAIL == array->setAttribute( "UINT_ATTRS", uivals, 3 ) )
    ret++;

  float fval = 3.0;
  if ( DAL::FAIL == array->setAttribute( "ATTRIBUTE_FLOAT", &fval ) )
    ret++;

  float fvals[] = { 1.0, 2.0, 3.0 };
  if ( DAL::FAIL == array->setAttribute( "FLOAT_ATTRS", fvals, 3 ) )
    ret++;

  double dval = 3.0;
  if ( DAL::FAIL == array->setAttribute( "DOUBLE_ATTR", &dval ) )
    ret++;

  double dvals[] = { 1.0, 2.0, 3.0 };
  if ( DAL::FAIL == array->setAttribute( "DOUBLE_ATTRS", dvals, 3 ) )
    ret++;

  if ( DAL::FAIL == array->close() )
    ret++;

  delete array;

  return ret;
}

//_______________________________________________________________________________
//                                                      open_and_close_hdf5_group

/*
  \param filename -- Name of the HDF5 file used for testing
  \return status  -- Error status of the function; returns positive value in
          case an error was encountered.
*/
uint open_and_close_hdf5_group (std::string const &filename)
{
  uint ret = 0;

  dalDataset ds;

  ds = dalDataset( filename.c_str(), "HDF5" );

  dalGroup * group = NULL;
  group = ds.openGroup( "group" );
  if ( NULL == group )
    ret++;

  if ( DAL::FAIL == group->close() )
    ret++;

  delete group;

  return ret;
}

//_______________________________________________________________________________
//                                                       set_attribute_hdf5_group

/*
  \param filename -- Name of the HDF5 file used for testing
  \return status  -- Error status of the function; returns positive value in
          case an error was encountered.
*/
uint set_attribute_hdf5_group (std::string const &filename)
{
  uint ret = 0;
  std::string name;

  dalDataset ds;

  ds = dalDataset( filename.c_str(), "HDF5" );

  dalGroup * group = NULL;
  group = ds.openGroup( "group" );
  if ( NULL == group )
    ret++;

  name             = "STRING_ATTR";
  std::vector<std::string> sval (1,"string test value");
  if ( DAL::FAIL == group->setAttribute( name, sval ) )
    ret = false;

  std::string svals[]= {"string","array","test"};
  if ( DAL::FAIL == group->setAttribute( "STRING_ATTRS", svals, 3 ) )
    ret++;

  int ival = 1;
  if ( DAL::FAIL == group->setAttribute( "ATTRIBUTE_INT", &ival ) )
    ret++;

  int ivals[] = { 1, 2, 3 };
  if ( DAL::FAIL == group->setAttribute( "INT_ATTRS", ivals, 3 ) )
    ret++;

  uint uival = 2;
  if ( DAL::FAIL == group->setAttribute( "UINT_ATTR", &uival ) )
    ret++;

  uint uivals[] = { 1, 2, 3};
  if ( DAL::FAIL == group->setAttribute( "UINT_ATTRS", uivals, 3 ) )
    ret++;

  float fval = 3.0;
  if ( DAL::FAIL == group->setAttribute( "ATTRIBUTE_FLOAT", &fval ) )
    ret++;

  float fvals[] = { 1.0, 2.0, 3.0 };
  if ( DAL::FAIL == group->setAttribute( "FLOAT_ATTRS", fvals, 3 ) )
    ret++;

  double dval = 3.0;
  if ( DAL::FAIL == group->setAttribute( "DOUBLE_ATTR", &dval ) )
    ret++;

  double dvals[] = { 1.0, 2.0, 3.0 };
  if ( DAL::FAIL == group->setAttribute( "DOUBLE_ATTRS", dvals, 3 ) )
    ret++;

  if ( DAL::FAIL == group->close() )
    ret++;

  delete group;
  return ret;
}

//_______________________________________________________________________________
//                                                     create_hdf5_group_subgroup

/*
  \param filename -- Name of the HDF5 file used for testing
  \return status  -- Error status of the function; returns positive value in
          case an error was encountered.
*/
uint create_hdf5_group_subgroup (std::string const &filename)
{
  uint ret = 0;

  dalDataset ds;

  ds = dalDataset( filename.c_str(), "HDF5" );

  dalGroup * group = ds.openGroup("group");
  if ( NULL == group )
    ret++;

  dalGroup * subgroup = group->createGroup( "subgroup" );
  if ( NULL == subgroup )
    ret++;

  if ( DAL::FAIL == subgroup->close() )
    ret++;

  if ( DAL::FAIL == group->close() )
    ret++;

  delete group;
  delete subgroup;

  return ret;

}

//_______________________________________________________________________________
//                                             create_hdf5_integer_array_in_group

/*
  \param filename -- Name of the HDF5 file used for testing
  \return status  -- Error status of the function; returns positive value in
          case an error was encountered.
*/
uint create_hdf5_integer_array_in_group (std::string const &filename)
{
  uint ret = 0;

  dalDataset ds;

  ds = dalDataset( filename.c_str(), "HDF5" );

  dalGroup * group = ds.openGroup("group");
  if ( NULL == group )
    ret++;

  // define dimensions of array
  vector<int> dims;
  dims.push_back(4);
  dims.push_back(5);
  dims.push_back(6);
  vector<int> cdims;

  int data[4*5*6];
  for (int gg=0; gg<(4*5*6); gg++)
    data[gg] = gg;

  dalArray * array = group->createIntArray( "int_array", dims, data, cdims );
  if ( NULL == array )
    ret++;

  if ( DAL::FAIL == array->close() )
    ret++;

  delete array;
  delete group;

  return ret;
}

//_______________________________________________________________________________
//                                               create_hdf5_float_array_in_group

/*
  \param filename -- Name of the HDF5 file used for testing
  \return status  -- Error status of the function; returns positive value in
          case an error was encountered.
*/
uint create_hdf5_float_array_in_group (std::string const &filename)
{
  uint ret = 0;

  dalDataset ds;

  ds = dalDataset( filename.c_str(), "HDF5" );

  dalGroup * group = ds.openGroup("group");
  if ( NULL == group )
    ret++;

  // define dimensions of array
  vector<int> dims;
  dims.push_back(4);
  dims.push_back(5);
  dims.push_back(6);
  vector<int> cdims;

  float data[4*5*6];
  for (int gg=0; gg<(4*5*6); gg++)
    data[gg] = rand();

  dalArray * array = group->createFloatArray( "float_array", dims, data, cdims );
  if ( NULL == array )
    ret++;

  if ( DAL::FAIL == array->close() )
    ret++;

  delete array;
  delete group;

  return ret;
}

//_______________________________________________________________________________
//                                       create_hdf5_complex_float_array_in_group

/*
  \param filename -- Name of the HDF5 file used for testing
  \return status  -- Error status of the function; returns positive value in
          case an error was encountered.
*/
uint create_hdf5_complex_float_array_in_group (std::string const &filename)
{
  uint ret = 0;

  dalDataset ds;

  ds = dalDataset( filename.c_str(), "HDF5" );

  dalGroup * group = ds.openGroup("group");
  if ( NULL == group )
    ret++;

  // define dimensions of array
  vector<int> dims;
  dims.push_back(4);
  dims.push_back(5);
  dims.push_back(6);
  vector<int> cdims;

  complex<float> * data = new complex<float>[ 4*5*6 ];
  for (int gg=0; gg<(4*5*6); gg++)
    data[ gg ] = gg;

  dalArray * array = group->createComplexFloatArray( "complex_float_array",
                     dims, data, cdims );

  if ( NULL == array )
    ret++;

  if ( DAL::FAIL == array->close() )
    ret++;

  delete [] data;
  data = NULL;
  delete array;
  delete group;

  return ret;
}

//_______________________________________________________________________________
//                                               create_hdf5_short_array_in_group

/*
  \param filename -- Name of the HDF5 file used for testing
  \return status  -- Error status of the function; returns positive value in
          case an error was encountered.
*/
uint create_hdf5_short_array_in_group (std::string const &filename)
{
  uint ret = 0;

  dalDataset ds;

  ds = dalDataset( filename.c_str(), "HDF5" );

  dalGroup * group = ds.openGroup("group");
  if ( NULL == group )
    ret++;

  // define dimensions of array
  vector<int> dims;
  dims.push_back(4);
  dims.push_back(5);
  dims.push_back(6);
  vector<int> cdims;

  short data[4*5*6];
  for (int gg=0; gg<(4*5*6); gg++)
    data[gg] = rand();

  dalArray * array = group->createShortArray( "short_array", dims, data,
                     cdims );
  if ( NULL == array )
    ret++;

  if ( DAL::FAIL == array->close() )
    ret++;

  delete array;
  delete group;

  return ret;
}

//_______________________________________________________________________________
//                                                   read_hdf5_dataset_attributes

/*
  \param filename -- Name of the HDF5 file used for testing
  \return status  -- Error status of the function; returns positive value in
          case an error was encountered.
*/
uint read_hdf5_dataset_attributes (std::string const &filename)
{
  uint ret = 0;

  dalDataset ds;

  ds = dalDataset( filename.c_str(), "HDF5" );

  std::string attr_name("ATTRIBUTE_INT");
  int iattr = 0;
  if ( DAL::FAIL == ds.getAttribute( attr_name, iattr ) )
    ret++;
  else
    std::cerr << attr_name << "  = " << iattr << endl;

  attr_name = "ATTRIBUTE_FLOAT";
  float fattr = 0.0;
  if ( DAL::FAIL == ds.getAttribute( attr_name, fattr ) )
    ret++;
  else
    std::cerr << attr_name << "  = " << fattr << endl;

  attr_name = "DOUBLE_ATTR";
  double dattr = 0.0;
  if ( DAL::FAIL == ds.getAttribute( attr_name, dattr ) )
    ret++;
  else
    printf( "%s = %f\n", attr_name.c_str() , dattr );

  attr_name = "STRING_ATTR";
  std::string sattr = "";
  if ( DAL::FAIL == ds.getAttribute( std::string(attr_name), sattr ) )
    ret++;
  else
    printf( "%s = %s\n", attr_name.c_str() , sattr.c_str() );

  if ( DAL::FAIL == ds.close() )
    ret++;

  return ret;
}

//_______________________________________________________________________________
//                                                              create_hdf5_table

/*
  \param filename -- Name of the HDF5 file used for testing
  \return status  -- Error status of the function; returns positive value in
          case an error was encountered.
*/
uint create_hdf5_table (std::string const &filename)
{
  uint ret = 0;

  dalDataset ds;

  ds = dalDataset( filename.c_str(), "HDF5" );

  // create two identical tables - one at root level and one in a group
  dalTable * table_in_ds = ds.createTable( "table" );
  if ( NULL == table_in_ds )
    ret++;

  dalTable * table_in_group = ds.createTable( "table", "group" );
  if ( NULL == table_in_group )
    ret++;

  std::string colname = "TIME";

  table_in_ds->addColumn( colname, dal_FLOAT );
  table_in_group->addColumn( colname, dal_FLOAT );

  colname = "UVW";

  table_in_ds->addColumn( colname, dal_FLOAT, 3 );
  table_in_group->addColumn( colname, dal_FLOAT, 3 );

  colname = "ANTENNA1";
  table_in_ds->addColumn( colname, dal_FLOAT );
  table_in_group->addColumn( colname, dal_FLOAT );

  struct rowStruct
    {
      float time;
      float uvw[3];
      float antenna1;
    } rs;

  rs.time = 1;
  rs.uvw[0] = 1;
  rs.uvw[1] = 1;
  rs.uvw[2] = 1;
  rs.antenna1 = 1;

  int count = 5;
  for (int xx=0; xx<count; xx++)
    {
      table_in_ds->appendRow(&rs);
      table_in_group->appendRow(&rs);
    }

  float md = 3;
  float mi = 5;
  count = 3;
  int colnum = 0;
  std::cerr << "Overwriting first " << count << " rows of column " <<
            colnum << " in table.\n";
  for (int xx=0; xx<count; xx++)
    {
      table_in_ds->writeDataByColNum( &md, colnum, xx );
      table_in_group->writeDataByColNum( &md, colnum, xx );
    }

  count = 3;
  colnum = 2;
  std::cerr << "Overwriting first " << count << " rows of column " <<
            colnum << " in table.\n";
  for (int xx=0; xx<count; xx++)
    {
      table_in_ds->writeDataByColNum( &mi, colnum, xx );
      table_in_group->writeDataByColNum( &mi, colnum, xx );
    }

  return ret;
}

//_______________________________________________________________________________
//                                                                    list_groups

/*
  \param filename -- Name of the HDF5 file used for testing
  \return status  -- Error status of the function; returns positive value in
          case an error was encountered.
*/
uint list_groups (std::string const &filename)
{
  uint ret = 0;
  dalDataset ds;

  ds = dalDataset( filename.c_str(), "HDF5" );

  std::cerr << "Getting list of groups in file.\n";

  std::vector<std::string> groupnames = ds.getGroupNames();
  for (unsigned int jj=0; jj<groupnames.size(); jj++)
    std::cerr << groupnames[jj] << endl;

  if ( groupnames.size() > 0 )
    {
      std::cerr << "Opening group " << groupnames[0] << ".\n";
      dalGroup * mygroup = ds.openGroup( groupnames[0] );
      if ( NULL != mygroup )
        {
          std::cerr << "Getting group member names.\n";
          vector<string> memnames = mygroup->getMemberNames();
          for (unsigned int jj=0; jj<memnames.size(); jj++)
            std::cerr << memnames[jj] << endl;
          delete mygroup;
        }
      else
        ret++;
    }

  return ret;
}

//_______________________________________________________________________________
//                                                                           main

/*!
  \brief Main routine of the test program

  \return nofFailedTests -- The number of failed tests encountered within and
          identified by this test program.
*/
int main()
{
  uint ret (0);
  uint nofFailedTests (0);
  std::string filename ("tdal.h5");
  std::string groupname ("GROUP");

  /*__________________________________________________________________
    Create HDF5 data file an add attributes.
  */
  nofFailedTests += test_hdf5_datafile (filename,
					groupname);
  
  /*__________________________________________________________________
    Create integer array within HDF5 data file and read back the data
  */
  nofFailedTests += create_hdf5_integer_array(filename);
  nofFailedTests += read_hdf5_integer_array(filename);
  
  /*__________________________________________________________________
    Create float array within HDF5 data file and read back the data
  */
  if ( 0 != ( ret = create_hdf5_float_array(filename) ) )
    {
      std::cerr << "FAIL\n";
      nofFailedTests += ret;
    }

  /*__________________________________________________________________
    Create complex array within HDF5 data file and read back the data
  */
  if ( 0 != ( ret = create_hdf5_complex_float_array(filename) ) )
    {
      std::cerr << "FAIL\n";
      nofFailedTests += ret;
    }

  /*__________________________________________________________________
    Set attributes attached to a data array
  */
  if ( 0 != ( ret = set_attributes_hdf5_array(filename) ) )
    {
      std::cerr << "FAIL\n";
      nofFailedTests += ret;
    }
  
  /*__________________________________________________________________
    Open and close HDF5 group
  */
  if ( 0 != ( ret = open_and_close_hdf5_group(filename) ) )
    {
      std::cerr << "FAIL\n";
      nofFailedTests += ret;
    }

  std::cerr << "\n[ set_attribute_hdf5_group ]\n";
  std::cerr << "-----------------------------------------------------\n";
  if ( 0 != ( ret = set_attribute_hdf5_group(filename) ) )
    {
      std::cerr << "FAIL\n";
      nofFailedTests += ret;
    }

  std::cerr << "\n[ create_hdf5_group_subgroup ]\n";
  std::cerr << "-----------------------------------------------------\n";
  if ( 0 != ( ret = create_hdf5_group_subgroup(filename) ) )
    {
      std::cerr << "FAIL\n";
      nofFailedTests += ret;
    }

  std::cerr << "\n[ create_hdf5_short_array_in_group ]\n";
  std::cerr << "-----------------------------------------------------\n";
  if ( 0 != ( ret = create_hdf5_short_array_in_group(filename) ) )
    {
      std::cerr << "FAIL\n";
      nofFailedTests += ret;
    }

  std::cerr << "\n[ create_hdf5_integer_array_in_group ]\n";
  std::cerr << "-----------------------------------------------------\n";
  if ( 0 != ( ret = create_hdf5_integer_array_in_group(filename) ) )
    {
      std::cerr << "FAIL\n";
      nofFailedTests += ret;
    }

  std::cerr << "\n[ create_hdf5_float_array_in_group ]\n";
  std::cerr << "-----------------------------------------------------\n";
  if ( 0 != ( ret = create_hdf5_float_array_in_group(filename) ) )
    {
      std::cerr << "FAIL\n";
      nofFailedTests += ret;
    }

  std::cerr << "\n[ create_hdf5_complex_float_array_in_group ]\n";
  std::cerr << "-----------------------------------------------------\n";
  if ( 0 != ( ret = create_hdf5_complex_float_array_in_group(filename) ) )
    {
      std::cerr << "FAIL\n";
      nofFailedTests += ret;
    }

  std::cerr << "\n[  read_hdf5_dataset_attributes ]\n";
  std::cerr << "-----------------------------------------------------\n";
  if ( 0 != ( ret = read_hdf5_dataset_attributes(filename) ) )
    {
      std::cerr << "FAIL\n";
      nofFailedTests += ret;
    }

  std::cerr << "\n[ create_hdf5_table ]\n";
  std::cerr << "-----------------------------------------------------\n";
  if ( 0 != ( ret = create_hdf5_table(filename) ) )
    {
      std::cerr << "FAIL\n";
      nofFailedTests += ret;
    }

  std::cerr << "\n[ list_groups ]\n";
  std::cerr << "-----------------------------------------------------\n";
  if ( 0 != ( ret = list_groups(filename) ) )
    {
      std::cerr << "FAIL\n";
      nofFailedTests += ret;
    }

  std::cerr << "\nFailed tests:  " << nofFailedTests << endl;
  return 0;
}
