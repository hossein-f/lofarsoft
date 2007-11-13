/*-------------------------------------------------------------------------*
 | $Id::                                                                 $ |
 *-------------------------------------------------------------------------*
 ***************************************************************************
 *   Copyright (C) 2007 by Joseph Masters                                  *
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

#ifndef DALDATASET_H
#define DALDATASET_H

#ifndef DAL_H
#include "dal.h"
#endif

#ifndef DALARRAY_H
#include "dalArray.h"
#endif

#ifndef DALTABLE_H
#include "dalTable.h"
#endif

#ifndef DALGROUP_H
#include "dalGroup.h"
#endif

#ifndef DALATTRIBUTE_H
#include "dalAttribute.h"
#endif

/*!
  \class dalDataset
  
  \ingroup DAL

  \brief Represents the file containing all sub-structures
         (tables, arrays, etc.)

  \author Joseph Masters

  <h3>Synopsis</h3>

  The dalDataset is the highest level container for dalData.  It may
  consist of one or more files on disk, each of which contain multiple tables, 
  images and attributes.  These tables and images can be gruoped.
*/
class dalDataset{

	void * file;  //!< can be HDF5File, FITS, MS
	string type;  //!< "HDF5", "MSCASA" or "FITS"; for example
	string name;  //!< dataset name
	vector<string> files;  //!< list of files
	vector<dalTable> tables; //!< list of tables
	vector<dalGroup> groups; //!< list of groups
	vector<dalAttribute> attrs;  //!< list of attributes
	
	dalFilter * filter;	//!< dataset filter

	hid_t h5fh;   //!< hdf5 file handle

#ifdef WITH_CASA
	casa::MeasurementSet * ms; //!< CASA measurement set pointer
	casa::MSReader * ms_reader; //!< CASA measurement set reader pointer
	casa::Vector<casa::String> ms_tables; //!< vector of CASA MS tables
#endif

  public:
  	/*!
	  \brief The dataset object constructor
	 */
  	dalDataset();

	/*!
	  \brief The dataset object destructor
	 */
	~dalDataset();
	
	/*!
	  \brief Another constructor with two arguments.
      \param name The name of the dataset to open.
	  \param filetype Type of file to open ("HDF5", "MSCASA", etc.).
	*/
  	dalDataset( char * name, string filetype );
	
	/*!
	  \brief Open the dataset.
	  \return Zero if successful.  Non-zero on failure.
     */
	int open( char * datasetname );

	/*!
	  \brief Close the dataset.
	  \return Zero if successful.  Non-zero on failure.
     */
	int close();

	/*!
	  \brief Create a new array in the root group.
	  \param arrayname The name of the array you want to create.
      \param data_object A pointer to a data object containing the intitial
						 data you want to include in the array.
	  \return dalArray * pointer to an array object.
     */
	dalArray * createArray(
				string arrayname,
				dalData * data_object);
				
	/*!
	  \brief Create a new static integer array in the root group.
	  \param arrayname The name of the array you want to create.
      \param dims A vector containing the dimensions of the array.
	  \param data An array of integer values.
	  \return dalArray * pointer to an array object.
     */
	dalArray * createIntArray(
				string arrayname,
				vector<int> dims,
				int data[]);

	/*!
	  \brief Create a new extendible integer array in the root group.
	  \param arrayname The name of the array you want to create.
      \param dims A vector containing the dimensions of the array.
	  \param data An array of integer values.
      \param cdims A vector a chunk dimensions (necessary for extending an
	               hdf5 dataset).
	  \return dalArray * pointer to an array object.
     */
	dalArray * createIntArray(
				string arrayname,
				vector<int> dims,
				int data[],
				vector<int>cdims);

	/*!
	  \brief Create a new extendible floating point array in the root group.
	  \param arrayname The name of the array you want to create.
      \param dims A vector containing the dimensions of the array.
	  \param data An array of floating point values.
      \param cdims A vector a chunk dimensions (necessary for extending an
	               hdf5 dataset).
	  \return dalArray * pointer to an array object.
     */
	dalArray * createFloatArray(
				string arrayname,
				vector<int> dims,
				float data[],
				vector<int>cdims);

	/*!
	  \brief Create a new extendible complex floating point array in the
	         root group.
	  \param arrayname The name of the array you want to create.
      \param dims A vector containing the dimensions of the array.
	  \param data An array of complex floating point values.
      \param cdims A vector a chunk dimensions (necessary for extending an
	               hdf5 dataset).
	  \return dalArray * pointer to an array object.
     */
	dalArray * createComplexArray(
				string arrayname,
				vector<int> dims,
				complex<float> data[],
				vector<int>cdims);

	/*!
	  \brief create a new table in the root group
	  \param tablename
	  \return dalTable
	*/
	dalTable * createTable( string tablename );

	/*!
	  \brief create a new table in a specified group
	  \param tablename 
	  \param groupname 
	  \return dalTable 
	*/
	dalTable * createTable( string tablename, string groupname );

	/*!
	  \brief Create a new group
	  \param groupname 
	  \return dalGroup 
	*/
	dalGroup * createGroup( char* groupname );

    /*!
	  \brief Open a table (that's not in a group) by name.
      \param tablename The name of the table you want to open
	  \return dalTable * A pointer to a table object.
     */
	dalTable * openTable( string tablename );

    /*!
	  \brief Set table filter
	  \param columns A string containing a comma-separated list of columns to
	                 include in a filtered dataset.
	 */
	void setFilter( string columns );

    /*!
	  \brief Set table filter
	  \param columns A string containing a comma-separated list of columns to
	                 include in a filtered dataset.
	  \param conditions A string describing restraints on which rows are
	                 included in an opened dataset.  For example: "time>100".
	 */
	void setFilter( string columns, string conditions ); //!< set table filter
	
	/*!
	  \brief Open a table in a group.
	  \param tablename The name of the table to open.
	  \param groupname The name of the group containing the table.
	  \return dalTable * A pointer to a table object.
	 */
	dalTable * openTable( string tablename, string groupname );

	/*!
	  \brief Open a group in a dataset
	  \param groupname The name of the group to open
	  \return dalGroup * A pointer to a group object.
	 */
	dalGroup * openGroup( string groupname );
	
	/*!
	  \brief List the tables in a dataset.
	 */
	void listTables();

	/*!
	  \brief Retrieve the dataset type ("HDF5", "MSCASA", etc.)
	  \return A string describing the file format ("HDF5", "MSCASA", etc.)
     */
	string getType();

/************************************************************************
 *
 * The following functions are boost wrappers to allow some previously
 *   defined functions to be easily called from a python prompt.
 *
 ************************************************************************/
#ifdef PYTHON

	// create[]Array wrappers
	dalArray * cia_boost1(string arrayname, bpl::list dims, bpl::list data);
	dalArray * cia_boost2(string arrayname, bpl::list dims, bpl::list data,
				 bpl::list cdims );
	dalArray * cia_boost_numarray1( string arrayname, bpl::list dims,
			bpl::numeric::array data );
	dalArray * cia_boost_numarray2( string arrayname, bpl::list dims,
			bpl::numeric::array data, bpl::list cdims );
	dalArray * cfa_boost( string arrayname, bpl::list dims, bpl::list data,
				 bpl::list cdims );
	dalArray * cfa_boost_numarray( string arrayname, bpl::list dims,
			 bpl::numeric::array data, bpl::list cdims );

	bpl::numeric::array ria_boost( string arrayname );
	bpl::numeric::array rfa_boost( string arrayname );

	// createTable wrappers
	dalTable * ct1_boost( string tablename );
	dalTable * ct2_boost( string tablename, string groupname );

	// openTable wrappers
	dalTable * ot1_boost( string tablename );
	dalTable * ot2_boost( string tablename, string groupname );

	void setFilter_boost1(string);
	void setFilter_boost2(string,string);
#endif
};
#endif
