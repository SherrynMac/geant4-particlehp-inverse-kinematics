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


#include "PhysicsList.hh"
#include "PhysicsListMessenger.hh"

#include "G4VPhysicsConstructor.hh"
#include "G4ParticleDefinition.hh"
#include "G4ProductionCutsTable.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessVector.hh"
#include "G4ParticleTypes.hh"
#include "G4UnitsTable.hh"
#include "G4ios.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

#include "G4PhysListFactory.hh"
#include "G4VModularPhysicsList.hh"

// physics
#include "G4EmStandardPhysics_option4.hh"
#include "G4EmExtraPhysics.hh"
#include "G4HadronElasticPhysicsHP.hh"
#include "G4StoppingPhysics.hh"
#include "G4IonBinaryCascadePhysics.hh"
#include "G4DecayPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4IonElasticPhysics.hh"
#include "G4HadronInelasticQBBC.hh"
#include "G4EmStandardPhysics.hh"
#include "G4HadronElasticPhysicsXS.hh"
#include "G4IonPhysics.hh"
#include "G4NeutronTrackingCut.hh"

//for cuts per region
#include "G4Region.hh"
#include "G4RegionStore.hh"

#include "G4LossTableManager.hh"
#include "G4RunManager.hh"

//for thermal scattering
#include "G4HadronElasticProcess.hh"
#include "G4NeutronHPElasticData.hh"
#include "G4NeutronHPThermalScatteringData.hh"
#include "G4NeutronHPElastic.hh"
#include "G4NeutronHPThermalScattering.hh"

// step limiting
#include "G4StepLimiterPhysics.hh"
#include "G4StepLimiter.hh"

// Processes & particleHP
#include "G4ParticleHPElasticData.hh"
#include "G4ParticleHPThermalScatteringData.hh"
#include "G4ParticleHPElastic.hh"
#include "G4ParticleHPThermalScattering.hh"

#include "G4HadronInelasticProcess.hh"
#include "G4ParticleHPInelasticData.hh"
#include "G4ParticleHPInelastic.hh"

#include "G4NeutronCaptureProcess.hh"
#include "G4ParticleHPCaptureData.hh"
#include "G4ParticleHPCapture.hh"

#include "G4NeutronFissionProcess.hh"
#include "G4ParticleHPFissionData.hh"
#include "G4ParticleHPFission.hh"

#include "G4Deuteron.hh"

#include "G4IonPhysicsPHP.hh"

//manual defining chips model for high energy neutrons
#include "G4ChipsElasticModel.hh"
#include "G4NeutronElasticXS.hh"

// QGSP_BIC_AllHP model
#include "G4HadronPhysicsQGSP_BIC_AllHP.hh"
#include "G4HadronPhysicsQGSP_BIC_HP.hh"

// INCLXX model
#include "G4HadronPhysicsINCLXX.hh"
#include "QGSP_INCLXX_HP.hh"
#include "G4IonINCLXXPhysics.hh"

// QMD model
#include "G4IonQMDPhysics.hh"

// Custom process
#include "G4Li7Ion.hh"
#include "GenericIonHPInelasticModel.hh"
#include "GenericIonHPXS.hh"
#include "G4HadronInelasticProcess.hh"
#include "G4BinaryLightIonReaction.hh"
#include "G4ComponentGGNuclNuclXsc.hh"
#include "G4HadronicInteractionRegistry.hh"
#include "Li7Physics.hh"



PhysicsList::PhysicsList():  G4VModularPhysicsList()
{
    G4LossTableManager::Instance();
	SetVerboseLevel(1); //Always display output
    listChoice = "BIC_AllHP";
    pMessenger = new PhysicsListMessenger(this);
    decayPhysicsList = new G4DecayPhysics(); //decay physics
    Li7PhysicsList = new Li7Physics("Li7Physics");
    SetPhysicsList();
}

PhysicsList::~PhysicsList()
{  	
	delete decayPhysicsList;
	delete emPhysicsList;
    delete Li7PhysicsList;
	for(size_t i=0; i<hadronPhys.size(); i++) {delete hadronPhys[i];}
}

void PhysicsList::ConstructParticle()
{
    Li7PhysicsList -> ConstructParticle();
    decayPhysicsList -> ConstructParticle();
}

void PhysicsList::ConstructProcess()
{

    G4cout << "\n\n\n\nPhysics model: " << listChoice << "\n\n\n\n" << G4endl;
	AddTransportation();

	//decayPhysicsList -> ConstructParticle();
	emPhysicsList -> ConstructProcess();

    // parallel world physics
    G4String paraWorldName = "ParallelWorld";
    //RegisterPhysics(new G4ParallelWorldPhysics(paraWorldName, true));

   // hadronic physics lists
	for(size_t i=0; i<hadronPhys.size(); i++) 
	{
		hadronPhys[i] -> ConstructProcess();
	}

    G4VProcess* process = nullptr;
    G4ParticleDefinition* neutron = G4Neutron::Neutron();
    G4ProcessManager* pManager = neutron->GetProcessManager();


    process = pManager->GetProcess("hadElastic");
    G4cout << process << G4endl;
    if (process) pManager->RemoveProcess(process);

    //// Thermal neutron scattering physics ////

    G4HadronElasticProcess* theNeutronElasticProcess = new G4HadronElasticProcess;

    // Cross Section Data set
    G4NeutronElasticXS* heHighEnergyCrossSections = new G4NeutronElasticXS;
    theNeutronElasticProcess->AddDataSet(heHighEnergyCrossSections);

    G4NeutronHPElasticData* theHPElasticData = new G4NeutronHPElasticData;
    theNeutronElasticProcess->AddDataSet(theHPElasticData);
    G4NeutronHPThermalScatteringData* theHPThermalScatteringData = new G4NeutronHPThermalScatteringData;
    theNeutronElasticProcess->AddDataSet(theHPThermalScatteringData);


    // Update the energy threshold for the relevant neutron scattering models
    G4NeutronHPElastic* theNeutronElasticModel = new G4NeutronHPElastic;
    theNeutronElasticModel->SetMinEnergy(4.0*eV);
    theNeutronElasticModel->SetMaxEnergy(20*MeV);
    theNeutronElasticProcess->RegisterMe(theNeutronElasticModel);

    G4ChipsElasticModel* elastic_chip = new G4ChipsElasticModel();
    elastic_chip->SetMinEnergy(19.5*MeV);
    theNeutronElasticProcess->RegisterMe(elastic_chip);


    G4NeutronHPThermalScattering* theNeutronThermalElasticModel = new G4NeutronHPThermalScattering;
    theNeutronThermalElasticModel->SetMaxEnergy(4.0*eV);
    theNeutronElasticProcess->RegisterMe(theNeutronThermalElasticModel);

    // Apply Processes to Process Manager of Neutron
    G4ProcessManager* pmanager = G4Neutron::Neutron()->GetProcessManager();
    pmanager->AddDiscreteProcess(theNeutronElasticProcess);


    /// custom inetlastic for Li7 ///

    auto* li7 = G4Li7Ion::Definition();
    auto* pm = li7->GetProcessManager();


    G4cout << "=== Li7 processes ===" << G4endl;
    li7->GetProcessManager()->DumpInfo();

    G4cout << "\n=== Some random heavy ion (C12) ===" << G4endl;
    auto* c12 = G4IonTable::GetIonTable()->GetIon(6,12,0.0);
    c12->GetProcessManager()->DumpInfo();

	return;
}



void PhysicsList::SetPhysicsList()
{

    G4int verboseMode = 1;
    hadronPhys.clear();
    emPhysicsList = new G4EmStandardPhysics_option4(0);

    G4RunManager::GetRunManager() -> PhysicsHasBeenModified();

    hadronPhys.push_back( new G4DecayPhysics());
    hadronPhys.push_back( new G4RadioactiveDecayPhysics());
    hadronPhys.push_back( new G4EmExtraPhysics());
    hadronPhys.push_back( new G4HadronElasticPhysicsHP(verboseMode));
    hadronPhys.push_back( new G4StoppingPhysics());
    hadronPhys.push_back( new G4IonElasticPhysics());

    //Li7 Physics
    hadronPhys.push_back(new Li7Physics());

    G4PhysListFactory factory;

    // Switch between hadron models
    if (listChoice == "BIC")
    {
        hadronPhys.push_back( new G4HadronPhysicsQGSP_BIC_HP(verboseMode));
        hadronPhys.push_back( new G4IonPhysics(verboseMode));
    }
    else if (listChoice == "BIC_AllHP")
    {
        hadronPhys.push_back( new G4HadronPhysicsQGSP_BIC_AllHP(verboseMode));
        hadronPhys.push_back( new G4IonPhysicsPHP(verboseMode));
    }
    else if (listChoice == "QMD")
    {
        hadronPhys.push_back( new G4IonQMDPhysics());
    }
    else if (listChoice == "INCL")
    {
        hadronPhys.push_back( new G4IonINCLXXPhysics(verboseMode));
        //hadronPhys.push_back( new G4HadronInelasticINCLXX(verboseMode));
    }
    else
    {
        G4Exception("PhysicsListConstructorConstruction::SetPhysicsList()", "HiT-Phys-0001", FatalErrorInArgument , "Error: No appropriate physics model has been selected");
    }
	
	
	
	return;
}


void PhysicsList::SetCuts()
{
    defaultCutValue = 1* um;
    SetCutValue(1*m, "e-");
    SetCutValue(1*m, "proton");
    if (verboseLevel>0) DumpCutValuesTable();
}


void PhysicsList::SelectModel(G4String val)
{
	listChoice = val;
	SetPhysicsList();	
}
