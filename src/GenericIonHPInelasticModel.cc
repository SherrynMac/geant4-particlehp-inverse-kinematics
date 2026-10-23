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

#include "GenericIonHPInelasticModel.hh"
#include "G4ParticleHPChannelList.hh"
#include "G4ParticleHPChannel.hh"
#include "G4NucleiProperties.hh"
#include "G4IonTable.hh"
#include "G4ParticleHPInelasticCompFS.hh"
#include "G4HadFinalState.hh"
#include "G4ParticleHPNInelasticFS.hh"
#include "G4ParticleHPDInelasticFS.hh"
#include "G4ParticleHPAInelasticFS.hh"
#include "G4Li7Ion.hh"
#include "G4ParticleHPThreadLocalManager.hh"

GenericIonHPInelasticModel::GenericIonHPInelasticModel(const G4String& dataDir)
    : G4HadronicInteraction("GenericIonHPInelasticModel"), fDataDir(dataDir)
{
}

void GenericIonHPInelasticModel::Initialise(const G4Element* elem)
{
    if (fChannelList) return;

    // ---- projectile definition (Li‑7) -----------------------------
    auto* li7 = G4Li7Ion::Definition();

    // ---- create HP channel list ----------------------------------
    fChannelList = new G4ParticleHPChannelList(3,li7);

    // Custom directory:
    // MyHPData/GenericIon/Inelastic
    G4String inelDir = fDataDir + "/Inelastic";

    fChannelList->Init(const_cast<G4Element*>(elem), inelDir, li7);

    // Register at least one final‑state handler (F01 mandatory)
    auto* fs = new G4ParticleHPNInelasticFS;
    fChannelList->Register(fs, "F01");
    auto* fs2 = new G4ParticleHPDInelasticFS;
    fChannelList->Register(fs2, "F24");
    auto* fs3 = new G4ParticleHPAInelasticFS;
    fChannelList->Register(fs3, "F27");

}

G4HadFinalState* GenericIonHPInelasticModel::ApplyYourself(
        const G4HadProjectile& projectile,
        G4Nucleus& targetNucleus)
{
    // Skip anything that isn’t hydrogen
    if (targetNucleus.GetZ_asInt() != 1) {
        G4cout << "GenericIonHPInelasticModel: skipping target Z="
               << targetNucleus.GetZ_asInt() << G4endl;
        return nullptr;   // let BIC handle it
    }

    G4ParticleHPThreadLocalManager::GetInstance()->OpenReactionWhiteBoard();

    // Find the element in the material whose Z matches the selected nucleus
    const G4Material* mat = projectile.GetMaterial();
    const G4Element* elem = nullptr;
    for (size_t i = 0; i < mat->GetNumberOfElements(); ++i) {
        const G4Element* e = mat->GetElement(i);
        if (e->GetZ() == targetNucleus.GetZ_asInt()) { elem = e; break; }
    }
    if (!elem) return nullptr;             // should never happen

    Initialise(elem);

	G4HadFinalState* result = fChannelList->ApplyYourself(elem, projectile);

	G4ParticleHPThreadLocalManager::GetInstance()->CloseReactionWhiteBoard();

    return result;
}

G4bool GenericIonHPInelasticModel::IsApplicable(const G4HadProjectile& /*proj*/,
                                              G4Nucleus& target)
{
    return true; //(target.GetZ_asInt() == 1);   // only hydrogen targets
}

//--------------------------------------------------------------------
void GenericIonHPInelasticModel::ModelDescription(std::ostream& out) const
{
    //out << "GenericIonHPInelasticModel using data in " << fDataDir << "\n";
}