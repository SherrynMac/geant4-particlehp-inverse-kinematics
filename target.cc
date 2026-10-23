
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
//-------------------==================================------------------
// OoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOo
// Geant4 base code        ////
// Created By Sherryn MacLeod           ////
// ANSTO & UOW                              ////
//-------------------==================================------------------

#include "G4Types.hh"

#ifdef G4MULTITHREADED
 #include "G4MTRunManager.hh"
#else
  #include "G4RunManager.hh"
#endif

#include "DetectorConstruction.hh"
#include "ActionInitialization.hh"
#include "PhysicsList.hh"
#include "RunAction.hh"
#include "PrimaryGeneratorAction.hh"

#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4ScoringManager.hh"
#include "G4UIterminal.hh"
#include "G4UItcsh.hh"

#include "Randomize.hh"
#include <time.h>
#include "G4Timer.hh"

#include "G4TScoreNtupleWriter.hh"
#include "G4AnalysisManager.hh"

#include <cstdlib>
#include <TThread.h>





int main(int argc, char** argv)
{
    TThread::Initialize();

    //Instantiate the G4Timer object, to monitor the CPU time spent for
    //the entire execution
    G4Timer* theTimer = new G4Timer();
    theTimer->Start();
	time_t rawtime;
	struct tm * timeinfo;
	
	//Displaying the start time
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	printf ( "\nCurrent Start Time and date\n: %s", asctime (timeinfo) );

    // Set the Random engine
	CLHEP::MTwistEngine defaultEngine( 1234567 ); 
	G4Random::setTheEngine( &defaultEngine ); 
	G4long seed = time( NULL ); 
	G4Random::setTheSeed( seed + getpid());

	//Selecting the correct runmanager if MT is present
	#ifdef G4MULTITHREADED
		G4MTRunManager* pRunManager = new G4MTRunManager;
		pRunManager->SetNumberOfThreads(1); //if you want more do so in the ui
	#else
		G4RunManager* pRunManager = new G4RunManager;
    #endif

    //Get the pointer to the User Interface manager
    G4UImanager* UImanager = G4UImanager::GetUIpointer();

    G4ScoringManager* scoringManager = G4ScoringManager::GetScoringManager();

  	/// Detector Construction ///
    DetectorConstruction* realWorld = new DetectorConstruction();
    pRunManager -> SetUserInitialization(realWorld);

    /// Instantiation of the physics component ///
	G4VUserPhysicsList* physics = new PhysicsList();
	pRunManager -> SetUserInitialization(physics);

    /// User action initialization ///
	pRunManager->SetUserInitialization(new ActionInitialization());

    /// Activate score ntuple writer ///
    G4TScoreNtupleWriter<G4AnalysisManager> scoreNtupleWriter;
    scoreNtupleWriter.SetVerboseLevel(1);
    scoreNtupleWriter.SetNtupleMerging(true); //merging only available with root output

    /// Initialise the Visualisation ///
    G4VisManager* visManager = nullptr;
    if (argc != 2 || (argc > 2 && std::string(argv[1]) == "-ui"))
    {
        // initialise visualisation if not running a single macro in batch
        visManager = new G4VisExecutive;
        visManager->Initialise();
    }

    /// Running, setting the macro ///
	if(argc == 1)  // If only 1 command line argument, we run in interactive mode
    {
		G4UIExecutive* ui = new G4UIExecutive(argc, argv);
		ui -> SessionStart();
        delete ui;
    }
    else if (argc == 2) // Batch mode with a macro file
    {
        G4String command = "/control/execute ";
        G4String fileName = argv[1];
        UImanager -> ApplyCommand(command+fileName);
    }
    else if (argc > 2 && std::string(argv[1]) == "-ui")  // Interactive mode with a macro and visualisation
    {
        G4UIExecutive* ui = new G4UIExecutive(argc, argv);
        G4String command = "/control/execute ";
        G4String fileName = argv[2];
        UImanager -> ApplyCommand(command+fileName);
        ui -> SessionStart();
        delete ui;
    }

	delete visManager;	
    delete pRunManager;

    //Stop the timer here
    theTimer->Stop();
    
    G4cout << "The simulation took: " << theTimer->GetRealElapsed()/60.0 << " m to run (real time)" << G4endl;
	
    return 0;
}
