cmake_minimum_required(VERSION 3.5)

project(RegistrationQA)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "https://github.com/gsi-biomotion/SlicerRegistrationQA/wiki")
set(EXTENSION_CATEGORY "Registration")
set(EXTENSION_CONTRIBUTORS "Kristjan Anderle (GSI), Tobias Brandt (University Clinic of Erlangen), Daniel Richter (University Clinic of Erlangen), Jens Woelfelschneider (University Clinic of Erlangen)")
set(EXTENSION_DESCRIPTION "Image registration quality assurance tool.")
set(EXTENSION_STATUS "Beta")
set(EXTENSION_ICONURL "https://raw.githubusercontent.com/gsi-biomotion/SlicerRegistrationQA/master/RegistrationQA/Resources/Icons/RegistrationQAIcon.png")
set(EXTENSION_SCREENSHOTURLS "https://raw.githubusercontent.com/gsi-biomotion/SlicerRegistrationQA/master/Screenshots/RegQAOverview.png")
set(EXTENSION_DEPENDS SlicerRT)

#-----------------------------------------------------------------------------
# Extension dependencies

find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

find_package(SlicerRT REQUIRED)

# SlicerRT does not add Plastimatch library path to additional link directories
# or target properties, so we need to add it here.
LINK_DIRECTORIES("${SlicerRT_DIR}/../Plastimatch-build")

#-----------------------------------------------------------------------------
add_subdirectory(RegistrationQA)
add_subdirectory(AbsoluteDifference)
add_subdirectory(JacobianFilter)
add_subdirectory(InverseConsistency)
add_subdirectory(CreateRegistrationHierarchy)

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_GENERATE_CONFIG})
include(${Slicer_EXTENSION_CPACK})
