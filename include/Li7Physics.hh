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

#ifndef TARGET_LI7PHYSICS_HH
#define TARGET_LI7PHYSICS_HH

#include "G4VPhysicsConstructor.hh"

class Li7Physics : public G4VPhysicsConstructor {
public:
    Li7Physics(const G4String& name = "Li7Physics");
    ~Li7Physics() override;

    static void SetDataDirectory(const G4String& directory);

    void ConstructParticle() override;
    void ConstructProcess() override;

private:
    static G4String dataDirectory;
};






#endif //TARGET_LI7PHYSICS_HH
