//
// --------------------------------------------------------------------
// Copyright (c) 2026 Sherryn MacLeod
//
// This file is part of the Geant4 inverse-kinematics implementation
//
// Author:
//   Sherryn MacLeod
//   University of Wollongong / ANSTO
//
// Created: 2026
// Last updated: 2026-09-24
//
// This software uses the Geant4 toolkit.
// Geant4 is developed by the Geant4 Collaboration and is distributed
// under the Geant4 Software License:
// https://geant4.web.cern.ch/license/license
//
// This file is distributed under the terms of the LICENSE file
// included with this repository.
// --------------------------------------------------------------------

#include "Li7Physics.hh"
#include "G4Li7Ion.hh"
#include "G4ProcessManager.hh"

// EM processes
#include "G4hMultipleScattering.hh"
#include "G4ionIonisation.hh"
#include "G4NuclearStopping.hh"
#include "G4LindhardSorensenIonModel.hh"

// Hadronic processes
#include "G4HadronInelasticProcess.hh"
#include "GenericIonHPInelasticModel.hh"

#include "G4CrossSectionElastic.hh"
#include "G4HadronElasticProcess.hh"
#include "G4NuclNuclDiffuseElastic.hh"

#include "GenericIonHPXS.hh"
#include "G4BinaryLightIonReaction.hh"
#include "G4ComponentGGNuclNuclXsc.hh"
#include "G4HadronicInteractionRegistry.hh"
#include "G4CrossSectionInelastic.hh"

G4String Li7Physics::dataDirectory = "../data/GenericIon";

Li7Physics::Li7Physics(const G4String& name)
    : G4VPhysicsConstructor(name) {}

Li7Physics::~Li7Physics() {};

void Li7Physics::SetDataDirectory(const G4String& directory)
{
    dataDirectory = directory;
}

void Li7Physics::ConstructParticle() {
    G4Li7Ion::Definition();
}

void Li7Physics::ConstructProcess() {

    // --- Get the Li7 ion definition ---
    auto* li7 = G4Li7Ion::Definition();

    // --- ensure Li7 has a ProcessManager ---
    auto* pm = li7->GetProcessManager();
    if (!pm) {
        pm = new G4ProcessManager(li7);
        li7->SetProcessManager(pm);
    }
    auto* ph  = G4PhysicsListHelper::GetPhysicsListHelper();

    // EM processes
    auto *ionIoni = new G4ionIonisation();
    ionIoni->SetEmModel(new G4LindhardSorensenIonModel());
    G4hMultipleScattering *hmsc = new G4hMultipleScattering("ionmsc");
    G4NuclearStopping *pnuc = new G4NuclearStopping();

    ph->RegisterProcess(hmsc, li7);
    ph->RegisterProcess(ionIoni, li7);
    ph->RegisterProcess(pnuc, li7);

    // ionElastic processes
    G4HadronElasticProcess* ionElasticProcess = new G4HadronElasticProcess("ionElastic");

    G4NuclNuclDiffuseElastic* ionElastic = new G4NuclNuclDiffuseElastic;
    ionElastic->SetMinEnergy(0.0);
    ionElasticProcess->RegisterMe(ionElastic);

    G4ComponentGGNuclNuclXsc* ionElasticXS = new G4ComponentGGNuclNuclXsc;
    G4VCrossSectionDataSet* ionElasticXSDataSet = new G4CrossSectionElastic(ionElasticXS);
    ionElasticXSDataSet->SetMinKinEnergy(0.0);
    ionElasticProcess->AddDataSet(ionElasticXSDataSet);

    pm->AddDiscreteProcess( ionElasticProcess );


    // Custom inelastic processes
    G4HadronInelasticProcess* li7InelasticProcess = new G4HadronInelasticProcess("Li7Inelastic", li7);



    //----- BIC model for Li7 -----
    auto* pre = static_cast<G4PreCompoundModel*>(
            G4HadronicInteractionRegistry::Instance()->FindModel("PRECO"));
    if(!pre) pre = new G4PreCompoundModel;

    auto* bicModel = new G4BinaryLightIonReaction(pre);
    bicModel->SetMinEnergy(0.*MeV);
    bicModel->SetMaxEnergy(6.*GeV);

    li7InelasticProcess->AddDataSet( new G4CrossSectionInelastic(new G4ComponentGGNuclNuclXsc()) );


    //----- HP model for Li7 -----
    const G4String crossSectionFile = dataDirectory + "/Inelastic/CrossSection/1_1_Hydrogen";
    G4cout << "Loading GenericIon HP cross-section data from: " << crossSectionFile << G4endl;
    auto* xs = new GenericIonHPXS(crossSectionFile);
    li7InelasticProcess->AddDataSet(xs);

    auto* model = new GenericIonHPInelasticModel(dataDirectory);

    G4Element* elementH = G4NistManager::Instance()->FindOrBuildElement("H");
    G4Element* elementC = G4NistManager::Instance()->FindOrBuildElement("C");

    //Deactivate the custom HP model for C; Deactivate the BIC model for H (uses custom model)
    model->DeActivateFor(elementC);
    bicModel->DeActivateFor(elementH);

    li7InelasticProcess->RegisterMe(bicModel);
    li7InelasticProcess->RegisterMe(model);


    pm->AddDiscreteProcess(li7InelasticProcess);




}


