/***************************************************************************
 *   Copyright (C) 2005 by Sven Lafebre                                    *
 *   s.lafebre@astro.ru.nl                                                 *
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

#ifdef HAVE_PGPLOT

#include <iostream>
#include <string>
#include <Data/Data.h>
#include <Display/PGPlotterCR.h>

using namespace std;

namespace CR { // Namespace CR -- begin
  
  PGPlotterCR::PGPlotterCR() {
    xAxisMin_ = -409.6;
    xAxisMax_ =  409.6;
    
    yAxisMin_ = 0;
    yAxisMax_ = 1;
  }
  
  PGPlotterCR::~PGPlotterCR()
  {
    cpgend();
  }
  
  void PGPlotterCR::initPlot()
  {
    if (cpgbeg(0, "/XSERVE", 1, 1) < 0) {
      if (cpgbeg(0, "/PS", 1, 1) < 0) {
	cerr << "  ERROR: Could not initialize PGPlot environment." << endl;
	throw string("PGPlot environment not found");
      } else {
	mode_ = "/PS";
	cerr << "           Using /ps instead." << endl;
      }
    }
    mode_ = "/XSERVE";
    
    cpgenv(xAxisMin_, xAxisMax_, yAxisMin_, yAxisMax_, 0, 1);
    cpglab(xLabel_.c_str(), yLabel_.c_str(), title_.c_str());
    cpgask(0);
    cpgsch(1.2);
  }
  
} // Namespace CR -- end

#endif
