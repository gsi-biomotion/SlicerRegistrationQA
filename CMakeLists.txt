cmake_minimum_required(VERSION 2.8.9)

set(EXTENSION_NAME RegistrationQA)

#-----------------------------------------------------------------------------
set(EXTENSION_NAME "RegistrationQAExtension")
set(EXTENSION_HOMEPAGE "https://github.com/gsi-biomotion/SlicerRegistrationQA/wiki")
set(EXTENSION_CATEGORY "Registration")
set(EXTENSION_CONTRIBUTORS "Kristjan Anderle (GSI), Tobias Brandt (University Clinic of Erlangen), Daniel Richter (University Clinic of Erlangen), Jens Woelfelschneider (University Clinic of Erlangen)")
set(EXTENSION_DESCRIPTION "Image registration quality assurance tool.")
set(EXTENSION_STATUS "Beta")
set(EXTENSION_ICONURL "https://github.com/gsi-biomotion/SlicerRegistrationQA/blob/master/RegQA/Resources/Icons/RegQAIcon.png")
set(EXTENSION_SCREENSHOTURLS "")
set(EXTENSION_DEPENDS SlicerRT)


#-----------------------------------------------------------------------------
find_package(Slicer COMPONENTS ConfigurePrerequisites REQUIRED)
project(SlicerRegQA)
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

# find_package(SlicerRT REQUIRED)
#-----------------------------------------------------------------------------

# set(DEPENDENCY_BUILD_DIRS "")
# if(CMAKE_CONFIGURATION_TYPES)
#   foreach(config ${CMAKE_CONFIGURATION_TYPES})
# #     list(APPEND DEPENDENCY_BUILD_DIRS "${SlicerRT_DIR}/inner-build/${Slicer_QTLOADABLEMODULES_LIB_DIR}/${config}")
#     list(APPEND DEPENDENCY_BUILD_DIRS "${SlicerRT_DIR}/inner-build/${Slicer_CLIMODULES_LIB_DIR}/${config}")
#     list(APPEND DEPENDENCY_BUILD_DIRS "${SlicerRT_DIR}/${SLICERRT_PLASTIMATCH_DIR}/${config}")
#   endforeach()
# else()
# #   set(DEPENDENCY_BUILD_DIRS "${SlicerRT_DIR}/inner-build/${Slicer_QTLOADABLEMODULES_LIB_DIR}")
#   set(DEPENDENCY_BUILD_DIRS "${SlicerRT_DIR}/inner-build/${Slicer_CLIMODULES_LIB_DIR}")
#   set(DEPENDENCY_BUILD_DIRS "${SlicerRT_DIR}/${SLICERRT_PLASTIMATCH_DIR}")
# endif()
# # Plastimatch_DIR
# 
set(DEPENDENCIES_ADDITIONAL_MODULE_PATHS
  ${DEPENDENCY_BUILD_DIRS}
  "${SlicerRT_DIR}/inner-build/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}"
  "${SlicerRT_DIR}/Plastimatch-build"
)

#SlicerRT does not add Plastimatch library path to additional link directories
#or target properties, so we need to add it here.
LINK_DIRECTORIES("${SlicerRT_DIR}/../Plastimatch-build")

add_subdirectory(RegistrationQA)
add_subdirectory(AbsoluteDifference)
add_subdirectory(JacobianFilter)
add_subdirectory(InverseConsistency)
add_subdirectory(CreateRegistrationHierarchy)

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_GENERATE_CONFIG})
include(${Slicer_EXTENSION_CPACK})
