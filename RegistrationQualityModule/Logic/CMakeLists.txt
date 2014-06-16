project(vtkSlicer${MODULE_NAME}ModuleLogic)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_LOGIC_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ../../tinyxml
  /projects/SlicerRT/SlicerRT-src/SubjectHierarchy/MRML
  /projects/SlicerRT/SlicerRT-4.3.1/inner-build/SubjectHierarchy/MRML/
  /projects/SlicerRT/SlicerRT-src/SubjectHierarchy/Plugins/
  /projects/SlicerRT/SlicerRT-4.3.1/inner-build/SubjectHierarchy/Plugins/
  )

set(${KIT}_SRCS
  vtkSlicer${MODULE_NAME}Logic.cxx
  vtkSlicer${MODULE_NAME}Logic.h
  vtkMRMLRegistrationQualityNode.cxx
  vtkMRMLRegistrationQualityNode.h
  vtkMRMLRegistrationQualityInputNode.cxx
  vtkMRMLRegistrationQualityInputNode.h
  DIRQAImage.cxx
  DIRQAImage.h
  )

set(${KIT}_TARGET_LIBRARIES
  ${ITK_LIBRARIES}
  vtkSlicerVolumesModuleLogic
  tinyXML
  /projects/SlicerRT/SlicerRT-4.3.1/inner-build/lib/Slicer-4.3/qt-loadable-modules/libqSlicerSubjectHierarchyModulePlugins.so
  /projects/SlicerRT/SlicerRT-4.3.1/inner-build/lib/Slicer-4.3/qt-loadable-modules/libvtkSlicerSubjectHierarchyModuleMRML.so
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
