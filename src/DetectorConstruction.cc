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



#include "G4SystemOfUnits.hh"
#include "DetectorConstruction.hh"
#include "globals.hh"
#include "G4Material.hh"
#include "G4PVPlacement.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Orb.hh"
#include "G4Sphere.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"
#include "G4NistManager.hh"
#include "G4RunManager.hh"
#include "G4StateManager.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"

#include "G4GeometryManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"
#include "G4ExtrudedSolid.hh"

#include "DetectorMessenger.hh"


#include "CLHEP/Random/RandGauss.h"
#include "CLHEP/Random/JamesRandom.h"
#include "CLHEP/Units/PhysicalConstants.h"

#include "G4SDManager.hh"
#include "G4UserLimits.hh"


DetectorConstruction::DetectorConstruction()
{	

	fDetectorMessenger = new DetectorMessenger(this);

    TargetThickness = 1*cm;
    useAuBacking = false;

    // step limiter
    useStepLimiter = false;
    maxStep = 0.01*mm;

    // scoring sphere
    useScoringSphere = true;
    scoringSphereRadius = 50*cm;

    // flight tube
    useFlightTube = false;
    flightTubeLength = 20*cm;

    // EDEN detector
    useDetector = false;
    detectorDistance = 300*cm; // distance from beginning of target to detector front face

    DefineMaterials();

    targetMaterial = air;
    worldMaterial = mat_Vacuum;

}

DetectorConstruction::~DetectorConstruction()
{
}

void DetectorConstruction::DefineMaterials()
{

	//---------------------================-  Material to be used -==================------------------
	G4NistManager * nistMan = G4NistManager::Instance();

    polypropylene = nistMan->FindOrBuildMaterial("G4_POLYPROPYLENE");
	air = nistMan->FindOrBuildMaterial("G4_AIR");

    // Defining vacuum
    G4double atomicNumber = 1;
    G4double massOfMole = 1.008*g/mole;
    G4double density = 1.e-25*g/cm3;
    G4double temperature = 2.73*kelvin;
    G4double pressure = 3.e-18*pascal;
    mat_Vacuum = new G4Material("interGalactic", atomicNumber, massOfMole, density, kStateGas, temperature, pressure);


}

G4Material* DetectorConstruction::GetMaterial(G4String choiceMaterial)
{
    if (choiceMaterial == "air") {return air;}
    if (choiceMaterial == "mat_Vacuum") {return mat_Vacuum;}
    if (choiceMaterial == "polypropylene") {return polypropylene;}
    if ( strstr(choiceMaterial.c_str(),"G4_") )
    {
        if (G4Material* pMat = G4NistManager::Instance()->FindOrBuildMaterial(choiceMaterial, false) )
        {
            return pMat;
        }
        else
        {
            G4String errorMessage = "Error: unable to find the request material "  +  choiceMaterial + "  in the NIST database";
            G4Exception("DetectorConstructorConstruction::giveMaterial()", "HiT-Det-0004", FatalErrorInArgument , errorMessage);
        }
    }
    else
    {
        //if Inappropriate choice
        G4Exception("DetectorConstructorConstruction::GetMaterial()", "HiT-Det-0006", FatalErrorInArgument , "Error: The material is not found");
    }
    return air; //This return gets rid of a compiler warning


}


/* Construct the geometry in terms of materials and geometrical components*/
G4VPhysicalVolume* DetectorConstruction::Construct()
{

//-----------------------================- World -==================------------------

    // Sizes of the World volume
    G4double worldSizeX = 400 * cm;
    G4double worldSizeY = 400 * cm;
    G4double worldSizeZ = 650 * cm;

    G4Box* world = new G4Box("world_box", worldSizeX/2., worldSizeY/2., worldSizeZ/2.);
    G4LogicalVolume* logical_world = new G4LogicalVolume(world, worldMaterial, "logical_world", 0,0,0);
    G4VPhysicalVolume* physical_world = new G4PVPlacement(0,G4ThreeVector(),logical_world, "physical_world", 0, false, 0, true);

//-----------------------================- Scoring sphere -==================------------------

    G4LogicalVolume* logical_scoringSphere = nullptr;
    if (useScoringSphere) {
        G4Orb* scoringSphere = new G4Orb("scoringSphere", scoringSphereRadius);
        logical_scoringSphere = new G4LogicalVolume(scoringSphere, worldMaterial, "logical_scoringSphere", 0,0,0);
        G4VPhysicalVolume* physical_scoringSphere = new G4PVPlacement(0, G4ThreeVector(), logical_scoringSphere, "physical_scoringSphere", logical_world, false, 0, true);
    }

//-----------------------================- Start of Detectors (geometry) -==================------------------

    // Solid target
    target_Thickness = TargetThickness; // size of the phantom box, set by the user in the macro

    G4Tubs* phantom = new G4Tubs("target", 0., 0.5*cm, target_Thickness/2, 0.*deg, 360.*deg);
    G4LogicalVolume* logical_target = new G4LogicalVolume(phantom, targetMaterial, "logical_target", 0,0,0);

    auto motherVol = useScoringSphere ? logical_scoringSphere : logical_world;

    G4VPhysicalVolume* physical_target = new G4PVPlacement(0, G4ThreeVector(), logical_target, "physical_target", motherVol, false, 0, true);

    //Au backing
    G4LogicalVolume* logical_auBacking;
    if (useAuBacking) {
        G4Tubs* auBacking = new G4Tubs("auBacking", 0., 0.5*cm, 50*um/2, 0.*deg, 360.*deg);
        logical_auBacking = new G4LogicalVolume(auBacking, GetMaterial("G4_Au"), "logical_auBacking", 0,0,0);
        G4VPhysicalVolume* physical_auBacking = new G4PVPlacement(0, G4ThreeVector(0,0,(target_Thickness/2 + 50*um/2)), logical_auBacking, "physical_auBacking", motherVol, false, 0, true);
    }

    // Limit the step inside the neutron production target
    if (useStepLimiter)
    {
        G4UserLimits* fStepLimit = new G4UserLimits(maxStep);
        logical_target->SetUserLimits(fStepLimit);
    }

    // Flight tube
    auto targetLength = target_Thickness;
    G4LogicalVolume* logical_flightTube;
    if (useFlightTube) {
        G4Tubs* flightTube = new G4Tubs("flightTube", 0., 0.5*cm, flightTubeLength/2, 0.*deg, 360.*deg);
        logical_flightTube = new G4LogicalVolume(flightTube, mat_Vacuum, "logical_flightTube", 0,0,0);
        G4VPhysicalVolume* physical_flightTube = new G4PVPlacement(0, G4ThreeVector(0,0,-(targetLength/2)-(flightTubeLength/2)), logical_flightTube, "physical_flightTube", motherVol, false, 0, true);
    }


    // EDEN detector (5cm thickness)
    G4double angle = 0*deg;
    G4RotationMatrix* rotationMatrix = new G4RotationMatrix();
    rotationMatrix->rotateY(angle);
    G4LogicalVolume* logical_edenDetector;
    if (useDetector) {
        G4Tubs* edenDetector = new G4Tubs("edenDetector", 0., 10*cm, 5*cm/2, 0.*deg, 360.*deg);
        logical_edenDetector = new G4LogicalVolume(edenDetector, mat_Vacuum, "logical_edenDetector", 0,0,0);
        G4VPhysicalVolume* physical_edenDetector = new G4PVPlacement(rotationMatrix, G4ThreeVector(-(((targetLength/2)+detectorDistance+2.5*cm)*sin(angle)),0,(((targetLength/2)+detectorDistance+2.5*cm)*cos(angle))), logical_edenDetector, "physical_edenDetector", logical_world, false, 0, true);
    }

//-------------------------------------================- Visualisations -==================------------------

   	if (useScoringSphere) {
   	    G4VisAttributes* worldAttributes = new G4VisAttributes(G4Colour(1.0, 1.0, 1.0)); //white
   	    worldAttributes -> SetVisibility(true);
   	    worldAttributes -> SetForceWireframe(true);
   	    logical_world -> SetVisAttributes(worldAttributes);
   	    logical_scoringSphere -> SetVisAttributes(worldAttributes);
   	}

    G4VisAttributes* targetAttributes = new G4VisAttributes(G4Colour(0.0, 1.0, 0.0)); //green
    targetAttributes->SetVisibility(true);
    targetAttributes->SetForceSolid(true);
    logical_target->SetVisAttributes(targetAttributes);

    if (useDetector) {
        G4VisAttributes* detectorAttributes = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0)); //red
        detectorAttributes->SetVisibility(true);
        detectorAttributes->SetForceSolid(true);
        logical_edenDetector->SetVisAttributes(detectorAttributes);
    }

    if (useFlightTube) {
        G4VisAttributes* flightTubeAttributes = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5)); //grey
        flightTubeAttributes->SetVisibility(true);
        flightTubeAttributes->SetForceSolid(true);
        logical_flightTube->SetVisAttributes(flightTubeAttributes);
    }

    if (useAuBacking) {
        G4VisAttributes* auBackingAttributes = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0)); //blue
        auBackingAttributes->SetVisibility(true);
        auBackingAttributes->SetForceSolid(true);
        logical_auBacking->SetVisAttributes(auBackingAttributes);
    }



   return physical_world; 
}

void DetectorConstruction::ConstructSDandField() {

    //initialise SD with unique name

    /*SensitiveDetector* SD = new SensitiveDetector("SD");
    G4SDManager::GetSDMpointer()->AddNewDetector(SD);
    SetSensitiveDetector("logical_detector", SD);*/


}





