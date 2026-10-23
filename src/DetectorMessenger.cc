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

#include "DetectorMessenger.hh"

#include "DetectorConstruction.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWithABool.hh"



DetectorMessenger::DetectorMessenger(DetectorConstruction* Det):Detector(Det)
{
    Dir = new G4UIdirectory("/target/");
    Dir->SetGuidance("Target parameters");

    targetMaterialCmd = new G4UIcmdWithAString("/target/targetMaterial",this);
    targetMaterialCmd->SetGuidance("Select Phantom material");
    targetMaterialCmd->SetParameterName("choice",false);
    targetMaterialCmd->AvailableForStates(G4State_PreInit);

    targetThicknessCmd = new G4UIcmdWithADoubleAndUnit("/target/targetThickness",this);
    targetThicknessCmd->SetGuidance("Set the thickness of the target");
    targetThicknessCmd->SetParameterName("Size",false);
    targetThicknessCmd->AvailableForStates(G4State_PreInit);

    useStepLimitCmd = new G4UIcmdWithABool("/target/useStepLimit",this);
    useStepLimitCmd->SetGuidance("Use step limit for phantom");
    useStepLimitCmd->SetParameterName("useStepLimit",false);
    useStepLimitCmd->AvailableForStates(G4State_PreInit);

    changeMaxStepCmd = new G4UIcmdWithADoubleAndUnit("/target/setMaxStep",this);
    changeMaxStepCmd->SetGuidance("Change the maximum step size for phantom");
    changeMaxStepCmd->SetParameterName("choice",false);
    useStepLimitCmd->AvailableForStates(G4State_PreInit);

    useAuBackingCmd = new G4UIcmdWithABool("/target/useAuBacking",this);
    useAuBackingCmd->SetGuidance("Use gold backing for the target");
    useAuBackingCmd->SetParameterName("useAuBacking",false);
    useAuBackingCmd->AvailableForStates(G4State_PreInit);

    Dir = new G4UIdirectory("/detector/");
    Dir->SetGuidance("Detector parameters");

    useDetectorCmd = new G4UIcmdWithABool("/detector/useDetector",this);
    useDetectorCmd->SetGuidance("Use EDEN detector at end of target");
    useDetectorCmd->SetParameterName("useDetector",false);
    useDetectorCmd->AvailableForStates(G4State_PreInit);

    detectorDistanceCmd = new G4UIcmdWithADoubleAndUnit("/detector/detectorDistance",this);
    detectorDistanceCmd->SetGuidance("Set the distance of the detector from the target");
    detectorDistanceCmd->SetParameterName("distance",false);
    detectorDistanceCmd->AvailableForStates(G4State_PreInit);

    Dir = new G4UIdirectory("/worldBox/");
    Dir->SetGuidance("World parameters");
    worldMaterialCmd = new G4UIcmdWithAString("/worldBox/worldMaterial",this);
    worldMaterialCmd->SetGuidance("Select World material");
    worldMaterialCmd->SetParameterName("worldMaterial",false);
    worldMaterialCmd->AvailableForStates(G4State_PreInit);
  
}

DetectorMessenger::~DetectorMessenger()
{

	delete targetMaterialCmd;
    delete targetThicknessCmd;
    delete useStepLimitCmd;
    delete changeMaxStepCmd;
    delete useDetectorCmd;
    delete detectorDistanceCmd;
    delete worldMaterialCmd;
    delete useAuBackingCmd;
}


void DetectorMessenger::SetNewValue(G4UIcommand* command,G4String newValue)
{ 

    if( command == targetMaterialCmd )
        { Detector->setMaterial(newValue);}
    if( command == targetThicknessCmd )
        { Detector->setSize(targetThicknessCmd->GetNewDoubleValue(newValue));}
    if( command == useStepLimitCmd )
        { Detector->SetUseStepLimiter(useStepLimitCmd->GetNewBoolValue(newValue));}
    if( command == changeMaxStepCmd )
        { Detector->SetMaxStep(changeMaxStepCmd->GetNewDoubleValue(newValue));}
    if( command == useDetectorCmd )
        { Detector->SetUseDetector(useDetectorCmd->GetNewBoolValue(newValue));}
    if( command == detectorDistanceCmd )
        { Detector->SetDetectorDistance(detectorDistanceCmd->GetNewDoubleValue(newValue));}
    if( command == worldMaterialCmd )
        { Detector->SetWorldMaterial(newValue);}
    if( command == useAuBackingCmd )
        { Detector->setUseAuBacking(useAuBackingCmd->GetNewBoolValue(newValue));}
}

