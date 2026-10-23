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

#ifndef DetectorConstruction_H 
#define DetectorConstruction_H 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"


class G4VPhysicalVolume;
class G4Material;
class DetectorMessenger;


// This is the user defined class that inherits behaviour from the virtual base class
// G4VUserDetectorConstruction of the Geant4 kernel
class DetectorConstruction : public G4VUserDetectorConstruction

{
public:
	// Declare the constructor for this class
    DetectorConstruction();

	// Declare the destructor of this class
    ~DetectorConstruction();

	// Construct() method MUST be defined in this class
    G4VPhysicalVolume* Construct();


	//Detector Layers
	G4LogicalVolume* logical_phantom;

	G4String GivePhantomName();



    ///------- Macro commands ------
    void ChangePhantom(G4String choice);
    void DefineMaterials();

    void setSize(G4double setTargetThickness) {TargetThickness = setTargetThickness;};
    void setMaterial(G4String setTargetMaterial) {targetMaterial = GetMaterial(setTargetMaterial);};
	void setUseAuBacking(G4bool setUseAuBacking) {useAuBacking = setUseAuBacking;};

    // step limiter
    void SetUseStepLimiter(G4bool setStepLimiter) {useStepLimiter = setStepLimiter;}
    void SetMaxStep(G4double newMaxStep) {maxStep = newMaxStep;}
	// Detector
	void SetUseDetector(G4bool setDetector) {useDetector = setDetector;}
	void SetDetectorDistance(G4double setDetectorDistance) {detectorDistance = setDetectorDistance;}
	// world
	void SetWorldMaterial(G4String setWorldMaterial) {worldMaterial = GetMaterial(setWorldMaterial);};

	
private:

   DetectorMessenger* fDetectorMessenger;
   G4VPhysicalVolume* physical_phantom;

   
	//materials
	G4Material* air;
    G4Material* mat_Vacuum;
	G4Material* polypropylene;

    G4Material* GetMaterial(G4String choiceMaterial);

    G4double TargetThickness;
    G4double target_Thickness;
	G4bool useAuBacking;

	G4bool useScoringSphere;
    G4double scoringSphereRadius;

    G4Material* targetMaterial;
	G4Material* worldMaterial;

    // step limiter
    G4bool useStepLimiter;
    G4double maxStep;

	//Detector
	G4bool useDetector;
	G4double detectorDistance;

	// flight tube
	G4bool useFlightTube;
	G4double flightTubeLength;


    void ConstructSDandField();
};
#endif
