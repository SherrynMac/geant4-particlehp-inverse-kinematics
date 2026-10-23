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

#include "EventAction.hh"
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4TrajectoryContainer.hh"
#include "G4Trajectory.hh"
#include "G4VVisManager.hh"

#include "G4SDManager.hh"

#include "G4VHitsCollection.hh"

#include "G4AnalysisManager.hh"
#include "G4ParticleHPManager.hh"

EventAction* EventAction::instance = 0;

EventAction::EventAction() {
    // Constructor implementation
}
EventAction::~EventAction() {
    // Destructor implementation (can be empty)
}

EventAction* EventAction::GetInstance()
{
    if (instance == 0) instance= new EventAction;
    return instance;
}

void EventAction::BeginOfEventAction(const G4Event *) {

    //G4ParticleHPManager::GetInstance()->OpenReactionWhiteBoard();
}

void EventAction::EndOfEventAction(const G4Event * event)
{

    G4AnalysisManager* analysis = G4AnalysisManager::Instance();

    G4HCofThisEvent* hce = event->GetHCofThisEvent();
    if (!hce)
    {
        //G4cout << "hce is nullptr" << G4endl;
        return;
    }




}
