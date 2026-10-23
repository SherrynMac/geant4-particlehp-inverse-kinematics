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

#include "GenericIonHPXS.hh"
#include "G4ParticleHPManager.hh"
#include "G4Ions.hh"
#include <fstream>

GenericIonHPXS::GenericIonHPXS(const G4String& xsFile)
{
    SetName("GenericIonHPXS");

    LoadHPVector(xsFile);
    G4cout << "[GenericIonHPXS] Loaded " << fXS.GetVectorLength()
           << " points from " << xsFile << G4endl;
}

G4bool GenericIonHPXS::IsElementApplicable(const G4DynamicParticle *dp, G4int Z, const G4Material *aMaterial) {

    auto def = dp->GetDefinition();

    // Only apply to G4Li7Ion interactions on 1H.
    if (def->GetParticleName() == "G4Li7Ion" && Z == 1)
    {
        return true;
    };

    return false;

}

G4double GenericIonHPXS::GetElementCrossSection(const G4DynamicParticle *dp, int Z, const G4Material *) {

    auto E = dp->GetKineticEnergy();

    if (E < 139.0*MeV) {

        /*return 1e10 * barn;*/  //Uncomment to force interaction for debugging
		return 1e10 * barn;

        G4double crossSection = fXS.GetXsec(E);
        return crossSection;

    }

    return 0.0;

}

void GenericIonHPXS::LoadHPVector(const G4String& xsFile)
{
    // This handles both plain text and .z compressed HP files.
    std::istringstream iss;
    G4ParticleHPManager::GetInstance()->GetDataStream(xsFile, iss);

    if (!iss.good())
    {
        G4Exception("GenericIonHPXS", "GenIonXS00", FatalException,
                    ("Cannot open/read " + xsFile).c_str());
    }

    // ParticleHP CrossSection files typically begin with:
    // dummyInt
    // dummyInt
    // nData
    G4int dummy1 = 0, dummy2 = 0, nData = 0;
    iss >> dummy1 >> dummy2 >> nData;

    if (!iss.good() || nData <= 0)
    {
        G4String msg = "Failed to parse header (dummy1 dummy2 nData) from " + xsFile +
                       " ; got nData=" + std::to_string(nData);
        G4Exception("GenericIonHPXS", "GenIonXS01", FatalException, msg.c_str());
    }

    // Read pairs into the HP vector.
    // energies in eV, cross sections in barn (same as particle_hp).
    fXS.Init(iss, nData, eV, barn);

    if (fXS.GetVectorLength() <= 0)
    {
        G4Exception("GenericIonHPXS", "GenIonXS02", FatalException,
                    ("Parsed nData but resulting vector is empty for " + xsFile).c_str());
    }
}


