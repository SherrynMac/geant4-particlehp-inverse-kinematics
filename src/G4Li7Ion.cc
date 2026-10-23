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

#include "G4Li7Ion.hh"

#include "G4ParticleTable.hh"
#include "G4PhysicalConstants.hh"
#include "G4String.hh"
#include "G4SystemOfUnits.hh"
#include "G4Types.hh"

#include "G4NucleiProperties.hh"

G4Li7Ion* G4Li7Ion::theInstance = nullptr;

G4Li7Ion* G4Li7Ion::Definition()
{
    if (theInstance != nullptr) return theInstance;

    const G4String name = "G4Li7Ion";
    // search in particle table]
    G4ParticleTable* pTable = G4ParticleTable::GetParticleTable();
    auto anInstance = static_cast<G4Ions*>(pTable->FindParticle(name));
    if (anInstance == nullptr) {
        // create particle
        //
        //    Arguments for constructor are as follows
        //               name             mass          width         charge
        //             2*spin           parity  C-conjugation
        //          2*Isospin       2*Isospin3       G-parity
        //               type    lepton number  baryon number   PDG encoding
        //             stable         lifetime    decay table
        //             shortlived      subType    anti_encoding
        //             excitation

        const G4int    Z     = 3;                       // proton number
        const G4int    A     = 7;                       // mass number
        const G4double mass  = G4NucleiProperties::GetNuclearMass(A, Z);
        const G4double charge   = +3. * eplus;
        const G4double width    = 0.0;                  // stable
        const G4double lifetime = -1.0;                 // -1 ⇒ stable
        const G4int    twoSpin  = 3;                    // 2 × spin  (ground state = 3/2⁻)
        const G4int    parity   = -1;
        const G4int    PDGCode  = 1000030070;           // official PDG for ⁷Li

        // clang-format off
        anInstance = new G4Li7Ion(
                name,    mass,       width*MeV,  charge,
                twoSpin,              parity,             0,
                0,               0,             0,
                "nucleus",               0,            +7, PDGCode,
                true,            -1.0,          nullptr,
                false,        "static",   -PDGCode,
                0.0,                0
        );
        // clang-format on

        // Magnetic Moment
        G4double mN = eplus * hbar_Planck / 2. / (proton_mass_c2 / c_squared);
        anInstance->SetPDGMagneticMoment(+3.2564268 * mN);
    }

    theInstance = static_cast<G4Li7Ion*>(anInstance);
    return theInstance;
}

G4Li7Ion* G4Li7Ion::Li7Definition()
{
    return Definition();
}

G4Li7Ion* G4Li7Ion::Li7()
{
    return Definition();
}
