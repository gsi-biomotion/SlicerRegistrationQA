cmake_minimum_required(VERSION 2.8.9)

set(EXTENSION_NAME SlicerDIRQA)

#-----------------------------------------------------------------------------
set(EXTENSION_NAME "RegistrationQualityExtension")
set(EXTENSION_HOMEPAGE "https://github.com/gsi-biomotion/slicer-dirqa")
set(EXTENSION_CATEGORY "Registration")
set(EXTENSION_CONTRIBUTORS "Kristjan Anderle (GSI), Tobias Brandt (University Clinic of Erlangen), Daniel Richter (University Clinic of Erlangen), Jens Woelfelschneider (University Clinic of Erlangen)")
set(EXTENSION_DESCRIPTION "Deformable image registration (DIR) quality assessment tool.")
set(EXTENSION_ICONURL "http://viewvc.slicer.org/viewvc.cgi/Slicer4/trunk/Extensions/Testing/LoadableExtensionTemplate/LoadableExtensionTemplate.png?revision=21746&view=co")
set(EXTENSION_SCREENSHOTURLS "http://wiki.slicer.org/slicerWiki/images/4/42/Slicer-r19441-LoadableExtensionTemplate-screenshot.png")
set(EXTENSION_DEPENDS SlicerRT)


#-----------------------------------------------------------------------------
find_package(Slicer COMPONENTS ConfigurePrerequisites REQUIRED)
project(SlicerDIRQA)
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

find_package(SlicerRT REQUIRED)
#-----------------------------------------------------------------------------

set(DEPENDENCY_BUILD_DIRS "")
if(CMAKE_CONFIGURATION_TYPES)
  foreach(config ${CMAKE_CONFIGURATION_TYPES})
    list(APPEND DEPENDENCY_BUILD_DIRS "${SlicerRT_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}/${config}")
    list(APPEND DEPENDENCY_BUILD_DIRS "${SlicerRT_DIR}/${Slicer_CLIMODULES_LIB_DIR}/${config}")
  endforeach()
else()
  set(DEPENDENCY_BUILD_DIRS "${SlicerRT_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}")
  set(DEPENDENCY_BUILD_DIRS "${SlicerRT_DIR}/${Slicer_CLIMODULES_LIB_DIR}")
endif()


set(DEPENDENCIES_ADDITIONAL_MODULE_PATHS
  ${DEPENDENCY_BUILD_DIRS}
  ${SlicerRT_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
)


add_subdirectory(RegistrationQualityModule)
add_subdirectory(AbsoluteDifference)
add_subdirectory(JacobianFilter)
add_subdirectory(InverseConsistency)
add_subdirectory(ReadRegistrationNode)

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_GENERATE_CONFIG})
include(${Slicer_EXTENSION_CPACK})
