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

#ifndef TARGET_GENERICIONHPXS_HH
#define TARGET_GENERICIONHPXS_HH

#include "G4VCrossSectionDataSet.hh"
#include "G4ParticleDefinition.hh"
#include "G4DynamicParticle.hh"
#include "G4Element.hh"
#include "G4ParticleHPVector.hh"
#include "G4SystemOfUnits.hh"


class GenericIonHPXS : public G4VCrossSectionDataSet {
public:
    GenericIonHPXS(const G4String& xsFile);
      ~GenericIonHPXS() override = default;

    G4bool IsElementApplicable(const G4DynamicParticle* aParticle, G4int Z,
                                const G4Material* aMaterial) override;

    G4double GetElementCrossSection(const G4DynamicParticle* dp,
                                    int Z, const G4Material*) override;

    void BuildPhysicsTable(const G4ParticleDefinition&) override {};


private:

    void LoadHPVector(const G4String& xsFile);

    G4ParticleHPVector fXS;  // HP file parser
};


#endif //TARGET_GENERICIONHPXS_HH
