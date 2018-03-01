project(vtkSlicer${MODULE_NAME}ModuleLogic)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_LOGIC_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${SlicerRT_ModuleLogic_INCLUDE_DIRS}
  ${vtkSlicerMarkupsModuleMRML_INCLUDE_DIRS}
  ${vtkSlicerAnnotationsMRML_INCLUDE_DIRS}
  ${vtkSlicerSegmentationsModuleMRML_INCLUDE_DIRS}
  ${vtkSlicerVolumesModuleLogic_INCLUDE_DIRS}
  ${vtkSlicerTransformsModuleLogic_INCLUDE_DIRS}
  ${vtkSlicerSegmentComparisonModuleLogic_INCLUDE_DIRS}
)

set(${KIT}_SRCS
  vtkSlicer${MODULE_NAME}Logic.cxx
  vtkSlicer${MODULE_NAME}Logic.h
  vtkMRMLRegQANode.cxx
  vtkMRMLRegQANode.h
)
# message(STATUS "Logic: " "${SlicerRT_ModuleLogic_INCLUDE_DIRS}" )
set(${KIT}_TARGET_LIBRARIES
  ${ITK_LIBRARIES}
  vtkSlicerMarkupsModuleMRML
  vtkSlicerVolumesModuleLogic
  vtkSlicerTransformsModuleLogic
  vtkSlicerAnnotationsModuleMRML
  vtkSlicerSegmentationsModuleMRML
  vtkSlicerSegmentComparisonModuleLogic
  )


#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
