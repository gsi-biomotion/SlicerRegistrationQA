project(vtkSlicer${MODULE_NAME}ModuleLogic)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_LOGIC_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicerSegmentComparisonModuleLogic_INCLUDE_DIRS}
  ${vtkSlicerMarkupsModuleMRML_INCLUDE_DIRS}
  ${vtkSlicerAnnotationsMRML_INCLUDE_DIRS}
  ${vtkSlicerSegmentationsModuleMRML_INCLUDE_DIRS}
  ${vtkSlicerVolumesModuleLogic_INCLUDE_DIRS}
  ${PLASTIMATCH_INCLUDE_DIRS}
#   /u/kanderle/Software/Slicer/SlicerRt/SlicerRT/SegmentComparison/Logic
# #   /u/kanderle/Software/Slicer/SlicerRT-build/inner-build/SegmentComparison/Logic
#   /u/motion/Software/Slicer/Modules/SlicerRT-build/inner-build/SegmentComparison/Logic
  
)

# link_directories(/u/motion/Software/Slicer/Modules/SlicerRT-build/inner-build/lib/Slicer-4.8/qt-loadable-modules)
# include_directories(/u/kanderle/Software/Slicer/SlicerRt/src/SegmentComparison/Logic/)
# link_directories(/u/kanderle/Software/Slicer/SlicerRT-build/inner-build/lib/Slicer-4.8/qt-loadable-modules)

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
  vtkSlicerSegmentationsModuleMRML
#   ${PLASTIMATCH_LIBRARIES}
#   vtkPlmCommon
  vtkSlicerSegmentComparisonModuleLogic
#   vtkSlicerSegmentComparisonModuleMRML
#   "/u/motion/Software/Slicer/Modules/SlicerRT-build/inner-build/lib/Slicer-4.8/qt-loadable-modules/libvtkSlicerSegmentComparisonModuleLogic.so"
#   "/u/kanderle/Software/Slicer/SlicerRT-build/inner-build/lib/Slicer-4.8/qt-loadable-modules/libvtkSlicerSegmentComparisonModuleLogic.so"
  )


#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
