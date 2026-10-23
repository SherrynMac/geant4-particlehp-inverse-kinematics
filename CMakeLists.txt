#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(target)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
find_package(Geant4 REQUIRED ui_all vis_all)
#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
#
include(${Geant4_USE_FILE})
#
#----------------------------------------------------------------------------
#
#
find_package(ROOT REQUIRED)
#
#----------------------------------------------------------------------------
# Locate sources and headers for this project
#
include_directories(${PROJECT_SOURCE_DIR}/include 
                    ${PROJECT_SOURCE_DIR}/shared/include 
                    ${Geant4_INCLUDE_DIR}
                    ${ROOT_INCLUDE_DIR})
#					
#----------------------------------------------------------------------------
#
	EXECUTE_PROCESS(COMMAND root-config --cflags OUTPUT_VARIABLE ROOT_CXX_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ROOT_CXX_FLAGS}")
#				  					
#					
#					
#					
#----------------------------------------------------------------------------
#					
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc
                  ${PROJECT_SOURCE_DIR}/shared/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh
                  ${PROJECT_SOURCE_DIR}/shared/include/*.hh)
#
#				  
#				  
#				  
#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(target target.cc ${sources} ${headers}
		src/ActionInitialization.cc
		src/DetectorConstruction.cc
		src/PrimaryGeneratorAction.cc
		src/RunAction.cc
		src/EventAction.cc
		src/SteppingAction.cc
		src/PhysicsList.cc
		src/PhysicsListMessenger.cc
		src/DetectorMessenger.cc
		src/G4Li7Ion.cc
		src/GenericIonHPInelasticModel.cc
		src/GenericIonHPXS.cc
		src/Li7Physics.cc


)
#
	EXECUTE_PROCESS(COMMAND root-config --libs OUTPUT_VARIABLE ROOT_LD_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
	set(CMAKE_EXE_LINKER_FLAGS ${ROOT_LD_FLAGS})
#
target_link_libraries(target ${Geant4_LIBRARIES} ${ROOT_LIBRARIES} )
#
#
#
#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS target DESTINATION bin)
#


