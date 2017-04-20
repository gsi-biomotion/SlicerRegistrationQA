cmake_minimum_required(VERSION 2.8.9)

project(SlicerDIRQA)

#-----------------------------------------------------------------------------
set(EXTENSION_NAME "RegistrationQualityExtension")
set(EXTENSION_HOMEPAGE "http://www.slicer.org/slicerWiki/index.php/Documentation/4.3/Extensions/LoadableExtensionTemplate")
set(EXTENSION_CATEGORY "Registration")
set(EXTENSION_CONTRIBUTORS "Kristjan Anderle (GSI), Tobias Brandt (University Clinic of Erlangen), Daniel Richter (University Clinic of Erlangen), Jens Woelfelschneider (University Clinic of Erlangen)")
set(EXTENSION_DESCRIPTION "Deformable image registration (DIR) quality assessment tool.")
set(EXTENSION_ICONURL "http://viewvc.slicer.org/viewvc.cgi/Slicer4/trunk/Extensions/Testing/LoadableExtensionTemplate/LoadableExtensionTemplate.png?revision=21746&view=co")
set(EXTENSION_SCREENSHOTURLS "http://wiki.slicer.org/slicerWiki/images/4/42/Slicer-r19441-LoadableExtensionTemplate-screenshot.png")

#-----------------------------------------------------------------------------
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

#-----------------------------------------------------------------------------
include_directories(tinyxml)
add_subdirectory(tinyxml)

add_subdirectory(RegistrationQualityModule)
add_subdirectory(AbsoluteDifference)
add_subdirectory(JacobianFilter)
add_subdirectory(InverseConsistency)
add_subdirectory(AddVectorFields)

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_CPACK})
