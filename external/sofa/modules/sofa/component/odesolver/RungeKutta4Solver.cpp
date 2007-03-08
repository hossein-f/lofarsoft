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
#include <sofa/component/odesolver/RungeKutta4Solver.h>
#include <sofa/core/ObjectFactory.h>
#include <math.h>


namespace sofa
{

namespace component
{

namespace odesolver
{

using namespace core::componentmodel::behavior;
using namespace sofa::defaulttype;

void RungeKutta4Solver::solve(double dt)
{
	//std::cout << "RK4 Init\n";
    //objectmodel::BaseContext* group = getContext();
    OdeSolver* group = this;
    MultiVector pos(group, VecId::position());
    MultiVector vel(group, VecId::velocity());
    MultiVector k1a(group, VecId::V_DERIV);
    MultiVector k2a(group, VecId::V_DERIV);
    MultiVector k3a(group, VecId::V_DERIV);
    MultiVector k4a(group, VecId::V_DERIV);
    MultiVector k1v(group, VecId::V_DERIV);
    MultiVector k2v(group, VecId::V_DERIV);
    MultiVector k3v(group, VecId::V_DERIV);
    MultiVector k4v(group, VecId::V_DERIV);
    MultiVector newX(group, VecId::V_COORD);
    MultiVector newV(group, VecId::V_DERIV);

    double stepBy2 = double(dt / 2.0);
    double stepBy3 = double(dt / 3.0);
    double stepBy6 = double(dt / 6.0);

    double startTime = group->getTime();

    //First step
	//std::cout << "RK4 Step 1\n";
    k1v = vel;
    group->computeAcc (startTime, k1a, pos, vel);

    //Step 2
	//std::cout << "RK4 Step 2\n";
    newX = pos;
    newV = vel;

    newX.peq(k1v, stepBy2);
    newV.peq(k1a, stepBy2);

    k2v = newV;
    group->computeAcc ( startTime+stepBy2, k2a, newX, newV);

    // step 3
	//std::cout << "RK4 Step 3\n";
    newX = pos;
    newV = vel;

    newX.peq(k2v, stepBy2);
    newV.peq(k2a, stepBy2);

    k3v = newV;
    group->computeAcc ( startTime+stepBy2, k3a, newX, newV);

    // step 4
	//std::cout << "RK4 Step 4\n";
    newX = pos;
    newV = vel;
    newX.peq(k3v, dt);
    newV.peq(k3a, dt);

    k4v = newV;
    group->computeAcc( startTime+dt, k4a, newX, newV);

	//std::cout << "RK4 Final\n";
    pos.peq(k1v,stepBy6);
    vel.peq(k1a,stepBy6);
    pos.peq(k2v,stepBy3);
    vel.peq(k2a,stepBy3);
    pos.peq(k3v,stepBy3);
    vel.peq(k3a,stepBy3);
    pos.peq(k4v,stepBy6);
    vel.peq(k4a,stepBy6);
}

int RungeKutta4SolverClass = core::RegisterObject("A popular explicit time integrator")
.add< RungeKutta4Solver >()
.addAlias("RungeKutta4")
;

SOFA_DECL_CLASS(RungeKutta4)


} // namespace odesolver

} // namespace component

} // namespace sofa

