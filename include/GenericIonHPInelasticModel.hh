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

#ifndef TARGET_GENERICIONHPINELASTICMODEL_HH
#define TARGET_GENERICIONHPINELASTICMODEL_HH

#include "G4HadronicInteraction.hh"
#include "G4ParticleHPChannelList.hh"

class GenericIonHPInelasticModel : public G4HadronicInteraction{
public:
    GenericIonHPInelasticModel(const G4String& dataDir);
    ~GenericIonHPInelasticModel() override = default;

    G4HadFinalState* ApplyYourself(const G4HadProjectile& proj, G4Nucleus& targetNucleus) override;

    // optional description hook
    void ModelDescription(std::ostream& outFile) const override;

    G4bool IsApplicable(const G4HadProjectile&, G4Nucleus& target) override;

private:
    void Initialise(const G4Element* elem);

    G4String fDataDir;
    G4ParticleHPChannelList* fChannelList = nullptr;

};


#endif //TARGET_GENERICIONHPINELASTICMODEL_HH
