# Geant4 ParticleHP-based Inverse-Kinematics Implementation

This repository contains a custom Geant4 implementation for simulating the inverse-kinematics p(7Li,n)7Be reaction through the Geant4 ParticleHP framework. The implementation was developed to reproduce the characteristic forward-focused neutron energy–angle distributions of the 7Li + p reaction at low incident energies.

Geometry replicated from Lebois et al. (2014) is included in this simulation code for neutron time-of-flight benchmarking studies. The macro called **run_nTOF.mac** can be used to run this configuration.

**Code last updated:** 2026-09-24

## Requirements

The implementation was developed and tested with **Geant4 11.3.0**

Other Geant4 versions may work but have not tested or validated.

## Repository Structure

```text
.
├── src/                  # Simulation application source files
├── include/              # Header files
├── data/                 # Custom data files transformed for 7Li + 1H
├── analysis/             # Analysis scripts used for validation/benchmarking
├── CMakeLists.txt
├── LICENSE
└── README.md
```

## 7Li + 1H Reaction Channels - Custom Data Directory

The \data directory currently includes the following evaluated-data channels:

| Channel | Reaction | ENDF MT |
|---|---|---:|
| F01 | p(7Li,n)7Be ground state | MT=50 |
| F01 | p(7Li,n)7Be* first excited state | MT=51 |
| F24 | p(7Li,d)6Li | MT=650 |
| F27 | p(7Li,alpha)4He | MT=800 |

These channels have been transformed from direct-kinematics configurations (7Li(p,x) reactions found in G4TENDL1.4) to inverse-kinematics configurations. 
Two nuclear-data configurations for the F01 were generated:

- **VIII.0-derived** — based on ENDF/B-VIII.0 evaluations;
- **VII.1-MT50 hybrid** — MT=50 taken from ENDF/B-VII.1 while the remaining relevant channels retain the ENDF/B-VIII.0-derived treatment.

Rename the file intended for use during the simulation to **'1_1_Hydrogen'**



## Running the Simulation

The simulation can be executed in either batch mode or using the Geant4 graphical user interface.

#### Batch Mode

```bash
./target run.mac
```

#### Visualisation Mode

```bash
./target -ui run.mac
```

For visualisation, uncomment the following line in `run.mac`:

```text
/control/execute visu.mac
```

The simulation macros are contained in the build directory:

- `run.mac` — primary simulation macro;
- `visu.mac` — visualisation macro.

If you would like to test this implementation with forced interactions, uncomment ``return 1e10 * barn;`` in:
```
G4double GenericIonHPXS::GetElementCrossSection(const G4DynamicParticle *dp, int Z, const G4Material *)
```

