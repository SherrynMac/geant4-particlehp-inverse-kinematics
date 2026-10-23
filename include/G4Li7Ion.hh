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


#ifndef TARGET_G4LI7ION_HH
#define TARGET_G4LI7ION_HH


#include "G4Ions.hh"

class G4Li7Ion : public G4Ions
{
public:
    static G4Li7Ion* Definition();
    static G4Li7Ion* Li7Definition();
    static G4Li7Ion* Li7();

private:
    G4Li7Ion(
        const G4String& name,
        G4double mass,
        G4double width,
        G4double charge,
        G4int spin,
        G4int parity,
        G4int conjugation,
        G4int isospin,
        G4int isospin3,
        G4int gParity,
        const G4String& type,
        G4int lepton,
        G4int baryon,
        G4int encoding,
        G4bool stable,
        G4double lifetime,
        G4DecayTable* decayTable,
        G4bool shortLived,
        const G4String& subType,
        G4int antiEncoding,
        G4double excitation,
        G4int isomer)
        : G4Ions(
              name, mass, width, charge,
              spin, parity, conjugation,
              isospin, isospin3, gParity,
              type, lepton, baryon, encoding,
              stable, lifetime, decayTable,
              shortLived, subType, antiEncoding,
              excitation, isomer)
    {
        isGeneralIon = false;
    }
    ~G4Li7Ion() override = default;

    static G4Li7Ion* theInstance;
};

#endif //TARGET_G4LI7ION_HH
