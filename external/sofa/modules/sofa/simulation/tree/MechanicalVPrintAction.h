/*******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, version 1.0 beta 1       *
*                (c) 2006-2007 MGH, INRIA, USTL, UJF, CNRS                     *
*                                                                              *
* This library is free software; you can redistribute it and/or modify it      *
* under the terms of the GNU Lesser General Public License as published by the *
* Free Software Foundation; either version 2.1 of the License, or (at your     *
* option) any later version.                                                   *
*                                                                              *
* This library is distributed in the hope that it will be useful, but WITHOUT  *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License  *
* for more details.                                                            *
*                                                                              *
* You should have received a copy of the GNU Lesser General Public License     *
* along with this library; if not, write to the Free Software Foundation,      *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.           *
*                                                                              *
* Contact information: contact@sofa-framework.org                              *
*                                                                              *
* Authors: J. Allard, P-J. Bensoussan, S. Cotin, C. Duriez, H. Delingette,     *
* F. Faure, S. Fonteneau, L. Heigeas, C. Mendoza, M. Nesme, P. Neumann,        *
* and F. Poyer                                                                 *
*******************************************************************************/
#ifndef SOFA_SIMULATION_TREE_MECHANICALVPRINTACTION_H
#define SOFA_SIMULATION_TREE_MECHANICALVPRINTACTION_H

#include <sofa/simulation/tree/Action.h>
#include <sofa/core/componentmodel/behavior/BaseMechanicalState.h>
#include <iostream>

namespace sofa
{

namespace simulation
{

namespace tree
{

/** Print a vector */
class MechanicalVPrintAction : public Action
{
public:
    typedef sofa::core::componentmodel::behavior::BaseMechanicalState::VecId VecId;

    MechanicalVPrintAction( VecId v, std::ostream& =std::cerr );
    virtual Result processNodeTopDown(GNode* node);
	virtual void fwdConstraint(GNode* /*node*/, core::componentmodel::behavior::BaseConstraint* c);

protected:
    VecId v_;
    std::ostream& out_;
};

} // namespace tree

} // namespace simulation

} // namespace sofa

#endif
