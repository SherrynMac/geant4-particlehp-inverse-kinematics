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

#include "G4ios.hh"
#include "G4SteppingManager.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4StepPoint.hh"
#include "G4ParticleDefinition.hh"
#include "G4VPhysicalVolume.hh"
#include "SteppingAction.hh"
#include "G4TrackStatus.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"
#include "TrackingAction.hh"
#include "G4HadronicProcess.hh"

#include "G4LorentzVector.hh"
#include "G4Proton.hh"
#include "G4IonTable.hh"

SteppingAction::SteppingAction(TrackingAction* trackAction)
    : G4UserSteppingAction(), fTrackAction(trackAction)
{
    conservationAuditOn = true;
    G4cout << "\n\n\nSteppingAction::UserSteppingAction: conservationAuditOn = " << conservationAuditOn << "\n\n\n" << G4endl;
}

SteppingAction::~SteppingAction()
{
}

void SteppingAction::UserSteppingAction(const G4Step* aStep) {
    G4AnalysisManager* analysis = G4AnalysisManager::Instance();


    // particle name, kinetic energy, position, time - from current step
    G4String particleName = aStep-> GetTrack()-> GetDefinition() -> GetParticleName();
    auto proc = aStep-> GetPostStepPoint()-> GetProcessDefinedStep();
    G4double kineticEnergy = aStep-> GetTrack()-> GetKineticEnergy();
    G4double positionX = aStep-> GetTrack()-> GetPosition().x();
    G4double positionY = aStep-> GetTrack()-> GetPosition().y();
    G4double positionZ = aStep-> GetTrack()-> GetPosition().z();
    G4ThreeVector dir = aStep->GetTrack()->GetMomentumDirection();
    G4double theta = dir.theta();
    theta = theta / CLHEP::deg;

    const G4VProcess* creator = aStep->GetTrack()->GetCreatorProcess();  // can be nullptr
    G4String processName = creator ? creator->GetProcessName()
                                   : G4String("primary");

    G4String modelName;
    if (auto* hp = dynamic_cast<const G4HadronicProcess*>(creator)) {
        if (const auto* m = hp->GetHadronicInteraction()) {
            modelName = m->GetModelName();
        }
    }

    // checking that the physical volumes exist
    if (aStep->GetPreStepPoint()->GetPhysicalVolume() && aStep->GetPostStepPoint()->GetPhysicalVolume())
    {
        G4String volName = aStep->GetPreStepPoint()->GetPhysicalVolume()->GetName();
        G4String postVolName = aStep->GetPostStepPoint()->GetPhysicalVolume()->GetName();

        if (volName == "physical_scoringSphere" && postVolName != "physical_scoringSphere" && particleName == "neutron") {
            G4double theta = dir.theta()*rad;
            analysis->FillNtupleDColumn(0, 0, kineticEnergy);
            analysis->FillNtupleDColumn(0, 1, positionX);
            analysis->FillNtupleDColumn(0, 2, positionY);
            analysis->FillNtupleDColumn(0, 3, positionZ);
            analysis->FillNtupleSColumn(0, 4, particleName);
            analysis->FillNtupleDColumn(0, 5, theta);
            analysis->FillNtupleSColumn(0,6, processName);
            analysis->FillNtupleSColumn(0, 7, modelName);
            analysis->AddNtupleRow(0);
        }

        G4double t0;
        if (volName == "physical_flightTube" && postVolName == "physical_target") {

            t0 = aStep->GetTrack()->GetGlobalTime()*ns;
            fTrackAction->SetT0(t0);
        }

        if (volName != "physical_edenDetector" && postVolName == "physical_edenDetector")
        {
            analysis->FillNtupleDColumn(1, 0, kineticEnergy);
            analysis->FillNtupleDColumn(1, 1, positionX);
            analysis->FillNtupleDColumn(1, 2, positionY);
            analysis->FillNtupleDColumn(1, 3, positionZ);
            analysis->FillNtupleSColumn(1, 4, particleName);
            analysis->AddNtupleRow(1);

            //dt_electronics
            G4double t_hit = aStep->GetTrack()->GetGlobalTime()*ns;
            G4double sigma_elec = 3.33*ns;
            G4double dt_elec = G4RandGauss::shoot(0, sigma_elec);

            //dt_pulse width
            G4double dt_pulse = (G4UniformRand()-0.5)*2.0*ns; // +/- 1ns pulse width

            //dt_detector depth
            G4double theta = dir.theta()*rad;
            G4double d = 5*cm;
            G4double x = G4UniformRand()*d;
            G4double v = aStep->GetTrack()->GetVelocity();
            G4double dt_depth = x / (v * std::cos(theta));

            G4double t_0 = fTrackAction->GetT0();

            G4double TOF = (t_hit - t_0) + dt_depth + dt_elec + dt_pulse;

            analysis->FillNtupleDColumn(2, 0, t_0);
            analysis->FillNtupleDColumn(2, 1, TOF);
            analysis->AddNtupleRow(2);

            aStep->GetTrack()->SetTrackStatus(fStopAndKill);

        }

        // event-level conservation audit
        if (conservationAuditOn && particleName == "G4Li7Ion" && aStep->GetTrack()->GetParentID() == 0) {

            const auto* stepProc = aStep->GetPostStepPoint()->GetProcessDefinedStep();
            if (stepProc) {
                const G4String stepProcName = stepProc->GetProcessName();

                if (stepProcName == "Li7Inelastic") {

                    const G4Track* track = aStep->GetTrack();

                    //Entrance channel - Li7 before interaction, proton at rest
                    G4double mBeam = track->GetDefinition()->GetPDGMass();
                    G4double EkBeam = aStep->GetPreStepPoint()->GetKineticEnergy();
                    G4ThreeVector dirBeam = aStep->GetPreStepPoint()->GetMomentumDirection();
                    G4double EBeam   = EkBeam + mBeam;
                    G4double pBeam   = std::sqrt(std::max(0., EBeam*EBeam - mBeam*mBeam));
                    G4LorentzVector p4Beam(pBeam*dirBeam, EBeam); //(px, py, pz, E)


                    G4double mTarget = G4Proton::Proton()->GetPDGMass();
                    G4LorentzVector p4Target(0, 0, 0, mTarget);

                    G4LorentzVector p4In = p4Beam + p4Target;
                    G4double qIn = track->GetDefinition()->GetPDGCharge() + G4Proton::Proton()->GetPDGCharge();
                    G4int    aIn = track->GetDefinition()->GetBaryonNumber() + G4Proton::Proton()->GetBaryonNumber();

                    //exit channel - secondaries
                    G4LorentzVector p4Out(0., 0., 0., 0.);
                    G4double qOut = 0.;
                    G4int    zOut = 0;
                    G4int    aOut = 0;
                    G4int    mtTag = 50; // ground-state Be7 residual => MT=50 by default

                    G4int nNeutron = 0;
                    G4int nBe7 = 0;
                    G4int nGamma = 0;
                    G4double be7Excitation = -1.0;
                    G4double mFinalSum = 0.0;
                    G4double kFinalSum = 0.0;

                    const auto* secondaries = aStep->GetSecondaryInCurrentStep();
                    if (secondaries) {
                        for (const auto* secTrack : *secondaries){

                            const auto* dp = secTrack->GetDynamicParticle();
                            const auto* def = dp->GetDefinition();

                            G4double m  = def->GetPDGMass();
                            G4double Ek = secTrack->GetKineticEnergy();
                            G4ThreeVector dir = secTrack->GetMomentumDirection();
                            G4double E = Ek + m;
                            G4double p = std::sqrt(std::max(0., E*E - m*m));

                            mFinalSum += def->GetPDGMass();
                            kFinalSum += secTrack->GetKineticEnergy();

                            p4Out += dp->Get4Momentum();

                            qOut += std::lround(def->GetPDGCharge()/eplus);
                            aOut += def->GetBaryonNumber();

                            if (def == G4Neutron::NeutronDefinition())
                            {
                                nNeutron++;
                            }

                            const auto* ion = dynamic_cast<const G4Ions*>(def);
                            if (ion && ion->GetAtomicNumber() == 4 && ion->GetAtomicMass() == 7)
                            {
                                nBe7++;
                                be7Excitation = ion->GetExcitationEnergy();
                            }

                            if (def->GetParticleName() == "gamma")
                            {
                                nGamma++;
                            }


                        }
                    }


                    if (nNeutron == 1 && nBe7 == 1)
                    {
                        const G4LorentzVector dP = p4In - p4Out;
                        const G4ThreeVector dp3 = dP.vect();

                        const G4double qMass_MeV = (mBeam + mTarget - mFinalSum)/MeV;
                        const G4double qSampled_MeV = (kFinalSum - EkBeam)/MeV;
                        const G4double qDifference_MeV = qMass_MeV - qSampled_MeV;

                        const G4double deltaE = dP.e()/MeV;
                        const G4double deltaPx = dp3.x()/MeV;
                        const G4double deltaPy = dp3.y()/MeV;
                        const G4double deltaPz = dp3.z()/MeV;
                        const G4double deltaPmag = dp3.mag()/MeV;

                        const G4int deltaq = qIn - qOut;
                        const G4int deltaA = aIn - aOut;

                        G4int mtFromQ = -1;

                        if (std::abs(qSampled_MeV - (-1.644)) < 0.05) {
                            mtFromQ = 50;
                        }
                        else if (std::abs(qSampled_MeV - (-2.073)) < 0.05) {
                            mtFromQ = 51;
                        }

                        analysis->FillNtupleIColumn(3, 0, mtFromQ);
                        analysis->FillNtupleDColumn(3, 1, aStep->GetPreStepPoint()->GetKineticEnergy()/MeV);
                        analysis->FillNtupleDColumn(3, 2, deltaE);
                        analysis->FillNtupleDColumn(3, 3, deltaPx);
                        analysis->FillNtupleDColumn(3, 4, deltaPy);
                        analysis->FillNtupleDColumn(3, 5, deltaPz);
                        analysis->FillNtupleDColumn(3, 6, deltaPmag);
                        analysis->FillNtupleIColumn(3, 7, deltaA);
                        analysis->FillNtupleIColumn(3, 8, nGamma);
                        analysis->FillNtupleDColumn(3, 9, be7Excitation/keV);
                        analysis->FillNtupleDColumn(3, 10, qMass_MeV);
                        analysis->FillNtupleDColumn(3, 11, qSampled_MeV);
                        analysis->FillNtupleDColumn(3, 12, qDifference_MeV);
                        analysis->FillNtupleDColumn(3, 13, deltaq);
                        analysis->AddNtupleRow(3);
                    }



                }
            }

        }
    }
}
