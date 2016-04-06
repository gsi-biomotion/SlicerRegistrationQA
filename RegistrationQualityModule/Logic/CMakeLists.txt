project(vtkSlicer${MODULE_NAME}ModuleLogic)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_LOGIC_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ../../tinyxml
  )

set(${KIT}_SRCS
  vtkSlicer${MODULE_NAME}Logic.cxx
  vtkSlicer${MODULE_NAME}Logic.h
  vtkMRMLRegistrationQualityNode.cxx
  vtkMRMLRegistrationQualityNode.h
)

set(${KIT}_TARGET_LIBRARIES
  ${ITK_LIBRARIES}
  vtkSlicerMarkupsModuleMRML
  vtkSlicerVolumesModuleLogic
  vtkSlicerAnnotationsModuleMRML

  )

# SET_SOURCE_FILES_PROPERTIES(
# 	DIRQAImage.cxx
# 	WRAP_EXCLUDE
# )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
