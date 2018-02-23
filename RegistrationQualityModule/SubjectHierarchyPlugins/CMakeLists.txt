project(qSlicer${MODULE_NAME}SubjectHierarchyPlugins)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_${MODULE_NAME_UPPER}_SUBJECT_HIERARCHY_PLUGINS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
   /u/kanderle/Software/Slicer/SlicerRT-build/inner-build/DoseVolumeHistogram
  )


set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicerSubjectHierarchyModuleLogic_INCLUDE_DIRS}
  ${qSlicerSubjectHierarchyModuleWidgets_INCLUDE_DIRS}
  ${vtkSlicerRegistrationQualityModuleLogic_INCLUDE_DIRS}
  ${MRMLCore_INCLUDE_DIRS}
  ${MRMLLogic_INCLUDE_DIRS}
  ${qMRMLWidgets_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  ${vtkSlicerVolumesModuleLogic_INCLUDE_DIRS}
  ${vtkSlicerAnnotationsModuleLogic_INCLUDE_DIRS}
  ${vtkSlicerAnnotationsModuleMRML_INCLUDE_DIRS}
  ${vtkSlicerMarkupsModuleLogic_INCLUDE_DIRS}
  ${vtkSlicerMarkupsModuleMRML_INCLUDE_DIRS}
#   ${vtkSlicerDoseVolumeHistogramModuleLogic_INCLUDE_DIRS}
#   ${vtkSlicerDoseVolumeHistogramModuleMRML_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  qSlicerSubjectHierarchyRegistrationQualityPlugin.cxx
  qSlicerSubjectHierarchyRegistrationQualityPlugin.h
  )

set(${KIT}_MOC_SRCS
  qSlicerSubjectHierarchyRegistrationQualityPlugin.h
  )


set(${KIT}_UI_SRCS
  )

set(${KIT}_RESOURCES
  Resources/${KIT}.qrc
  )

SET (${KIT}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "" FORCE)

#-----------------------------------------------------------------------------
set(${KIT}_TARGET_LIBRARIES
  vtkSlicerRegistrationQualityModuleLogic
  vtkSlicerSubjectHierarchyModuleLogic
  qSlicerSubjectHierarchyModuleWidgets
  vtkSlicerVolumesModuleLogic
  vtkSlicerAnnotationsModuleLogic
  vtkSlicerAnnotationsModuleMRML
  vtkSlicerMarkupsModuleLogic
  vtkSlicerMarkupsModuleMRML
#   vtkSlicerDoseVolumeHistogramModuleLogic
#   vtkSlicerDoseVolumeHistogramModuleMRML
  MRMLCore
  MRMLLogic
  MRMLCLI
  qMRMLWidgets
  ${QT_LIBRARIES}
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleQtLibrary(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  UI_SRCS ${${KIT}_UI_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  RESOURCES ${${KIT}_RESOURCES}
  WRAP_PYTHONQT
  )