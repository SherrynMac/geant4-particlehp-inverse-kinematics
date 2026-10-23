//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//---------------------------------------------------------------------
// Modified by:
//   Sherryn MacLeod
//   University of Wollongong / ANSTO
//
// Last updated: 2026-09-24
//---------------------------------------------------------------------

#include "PhysicsListMessenger.hh"

#include "PhysicsList.hh"
#include "Li7Physics.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAString.hh"

/////////////////////////////////////////////////////////////////////////////
PhysicsListMessenger::PhysicsListMessenger(PhysicsList* pPhys)
:pPhysicsList(pPhys)
{
  physDir = new G4UIdirectory("/physics/");
  physDir->SetGuidance("Commands to activate physics models");

  pListCmd = new G4UIcmdWithAString("/physics/selectPhysics",this);  
  pListCmd->SetGuidance("Select physics model (QMD / BIC / INCL).");
  pListCmd->SetParameterName("PList",false);
  pListCmd->AvailableForStates(G4State_PreInit);  

  li7DataDirectoryCmd = new G4UIcmdWithAString("/physics/li7DataDirectory", this);
  li7DataDirectoryCmd->SetGuidance("Set the root directory containing GenericIon HP data.");
  li7DataDirectoryCmd->SetParameterName("directory", false);
  li7DataDirectoryCmd->AvailableForStates(G4State_PreInit);
  
}

/////////////////////////////////////////////////////////////////////////////
PhysicsListMessenger::~PhysicsListMessenger()
{

  delete pListCmd;
  delete li7DataDirectoryCmd;

}

/////////////////////////////////////////////////////////////////////////////
void PhysicsListMessenger::SetNewValue(G4UIcommand* command,
                                          G4String newValue)
{       

 if( command == pListCmd )
   { pPhysicsList->SelectModel(newValue);}
 else if( command == li7DataDirectoryCmd )
   { Li7Physics::SetDataDirectory(newValue); }

}
