
#ifndef __vtkSlicerRegistrationQualityLogic_h
#define __vtkSlicerRegistrationQualityLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes

// STD includes
#include <cstdlib>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkMatrix4x4.h>

#include "vtkSlicerRegistrationQualityModuleLogicExport.h"

class vtkMRMLRegistrationQualityNode;
class vtkMRMLVectorVolumeNode;

/// \ingroup Slicer_QtModules_RegistrationQuality
class VTK_SLICER_REGISTRATIONQUALITY_MODULE_LOGIC_EXPORT vtkSlicerRegistrationQualityLogic : 
  public vtkSlicerModuleLogic
{
public:
  static vtkSlicerRegistrationQualityLogic *New();
  vtkTypeMacro(vtkSlicerRegistrationQualityLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  /*!
   * TODO: Add description of the function itself and its arguments
   * \param option (it is named option in the source but it tells nothing, please rename)
   */
  void CreateVisualization(int option);
  
  double GetFieldMaxNorm();
  
  vtkSmartPointer<vtkPolyData> GlyphVisualization(vtkSmartPointer<vtkImageData>, int sourceOption);
  vtkSmartPointer<vtkPolyData> GridVisualization(vtkSmartPointer<vtkImageData>);
  vtkSmartPointer<vtkPolyData> ContourVisualization(vtkSmartPointer<vtkImageData>);
  vtkSmartPointer<vtkPolyData> BlockVisualization(vtkSmartPointer<vtkImageData>);
  vtkSmartPointer<vtkPolyData> GlyphSliceVisualization(vtkSmartPointer<vtkImageData>, vtkSmartPointer<vtkMatrix4x4>); 
  vtkSmartPointer<vtkPolyData> GridSliceVisualization(vtkSmartPointer<vtkImageData>, vtkSmartPointer<vtkMatrix4x4>);

  /*!
   * Issue when input data is changed without modifying node
   * Will either remake or account for scenario some other way
   */
  void GenerateTransformField();

public:
  void SetAndObserveRegistrationQualityNode(vtkMRMLRegistrationQualityNode *node);
  vtkGetObjectMacro(RegistrationQualityNode, vtkMRMLRegistrationQualityNode);

protected:
  vtkSlicerRegistrationQualityLogic();
  ~vtkSlicerRegistrationQualityLogic();

  virtual void RegisterNodes();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);

  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);
  virtual void OnMRMLSceneEndImport();
  virtual void OnMRMLSceneEndClose();

protected:
  vtkSmartPointer<vtkImageData> TransformField;
  
private:
  vtkSlicerRegistrationQualityLogic(const vtkSlicerRegistrationQualityLogic&);// Not implemented
  void operator=(const vtkSlicerRegistrationQualityLogic&);// Not implemented
  
protected:
  /// Parameter set MRML node
  vtkMRMLRegistrationQualityNode* RegistrationQualityNode;
};

#endif
