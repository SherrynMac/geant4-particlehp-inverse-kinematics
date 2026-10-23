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


#include "RunAction.hh"
#include "PrimaryGeneratorAction.hh"
#include "G4AnalysisManager.hh"

#ifdef G4MULTITHREADED
	 #include "G4MTRunManager.hh"
#else
	#include "G4RunManager.hh"
#endif

#include "G4Run.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"

#include "G4UImanager.hh"
#include "G4ios.hh"
#include "globals.hh"

#include "G4AnalysisManager.hh"
#include "G4EmCalculator.hh"
#include "G4IonTable.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4Proton.hh"
#include "G4Neutron.hh"

RunAction::RunAction()
{
    auto analysis = G4AnalysisManager::Instance();
    analysis->SetVerboseLevel(1);
    analysis->SetNtupleMerging(true);

    analysis->CreateNtuple("ScoringSphere", "ScoringSphere");
    analysis->CreateNtupleDColumn(0, "kineticEnergy");
    analysis->CreateNtupleDColumn(0, "positionX");
    analysis->CreateNtupleDColumn(0, "positionY");
    analysis->CreateNtupleDColumn(0, "positionZ");
    analysis->CreateNtupleSColumn(0, "particleName");
    analysis->CreateNtupleDColumn(0, "theta");
    analysis->CreateNtupleSColumn(0, "processName");
    analysis->CreateNtupleSColumn(0, "modelName");
    analysis->FinishNtuple(0);

    analysis->CreateNtuple("detectorScoring", "Detector Scoring");
    analysis->CreateNtupleDColumn(1, "kineticEnergy");
    analysis->CreateNtupleDColumn(1, "positionX");
    analysis->CreateNtupleDColumn(1, "positionY");
    analysis->CreateNtupleDColumn(1, "positionZ");
    analysis->CreateNtupleSColumn(1, "particleName");
    analysis->FinishNtuple(1);

    analysis->CreateNtuple("nTOF", "nTOF");
    analysis->CreateNtupleDColumn(2, "t0");
    analysis->CreateNtupleDColumn(2, "TOF");
    analysis->FinishNtuple(2);

    analysis->CreateNtuple("conservationAudit", "event-level conservation audit");
    analysis->CreateNtupleIColumn(3, "mt");
    analysis->CreateNtupleDColumn(3, "E_Li_pre_MeV");
    analysis->CreateNtupleDColumn(3, "deltaE_MeV");
    analysis->CreateNtupleDColumn(3, "deltaPx_MeV");
    analysis->CreateNtupleDColumn(3, "deltaPy_MeV");
    analysis->CreateNtupleDColumn(3, "deltaPz_MeV");
    analysis->CreateNtupleDColumn(3, "deltaPmag_MeV");
    analysis->CreateNtupleIColumn(3, "deltaA");
    analysis->CreateNtupleIColumn(3, "nGamma");
    analysis->CreateNtupleDColumn(3, "Be7_excitation_keV");
    analysis->CreateNtupleDColumn(3, "QMass_MeV");
    analysis->CreateNtupleDColumn(3, "QSampled_MeV");
    analysis->CreateNtupleDColumn(3, "QDifference_MeV");
    analysis->CreateNtupleDColumn(3, "deltaq");
    analysis->FinishNtuple(3);

}


RunAction::~RunAction()
{
	
}


void RunAction::BeginOfRunAction(const G4Run*)
{
    auto analysis = G4AnalysisManager::Instance();
    analysis->OpenFile("output.root");
}


void RunAction::EndOfRunAction(const G4Run*)
{
    auto analysis = G4AnalysisManager::Instance();

    analysis->Write();
    analysis->CloseFile();
    G4cout << "Output file is closed" << G4endl;
}



