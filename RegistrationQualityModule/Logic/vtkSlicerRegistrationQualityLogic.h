#ifndef __vtkSlicerRegistrationQualityLogic_h
#define __vtkSlicerRegistrationQualityLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLVectorVolumeNode.h>
#include <vtkMRMLGridTransformNode.h>
#include <vtkMRMLAnnotationROINode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
// STD includes
#include <cstdlib>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkMatrix4x4.h>


#include "vtkSlicerRegistrationQualityModuleLogicExport.h"

class vtkMRMLRegistrationQualityNode;
// class vtkMRMLVectorVolumeNode;
class vtkSlicerVolumesLogic;
// class vtkSlicerTransformLogic;

/// \ingroup Slicer_QtModules_RegistrationQuality
class VTK_SLICER_REGISTRATIONQUALITY_MODULE_LOGIC_EXPORT vtkSlicerRegistrationQualityLogic :
	public vtkSlicerModuleLogic {

public:
	static vtkSlicerRegistrationQualityLogic *New();
	vtkTypeMacro(vtkSlicerRegistrationQualityLogic, vtkSlicerModuleLogic);
	void PrintSelf(ostream& os, vtkIndent indent);

	/**
	 * Issue when input data is changed without modifying node
	 * Will either remake or account for scenario some other way
	 */

	void SaveScreenshot(const char*);
	void SaveOutputFile();
        void CalculateDIRQAFrom(int number, int state);
	vtkMRMLScalarVolumeNode* AbsoluteDifference(vtkMRMLScalarVolumeNode*,vtkMRMLScalarVolumeNode*, vtkMRMLAnnotationROINode* inputROI = NULL);
	bool CalculateFiducialsDistance(vtkMRMLMarkupsFiducialNode* referenceFiducals, vtkMRMLMarkupsFiducialNode* movingFiducials,vtkMRMLTransformNode *transofrm, double statisticValues[4],bool absoluteDifference=true);

	void FalseColor(int state);
	void Flicker(int opacity);
	void getSliceCompositeNodeRASBounds(vtkMRMLSliceCompositeNode *scn, double* minmax);
	void Movie();
	void Checkerboard(int state);
	void SetForegroundImage(vtkMRMLScalarVolumeNode*,vtkMRMLScalarVolumeNode*,double opacity);
	vtkMRMLScalarVolumeNode* Jacobian(vtkMRMLVectorVolumeNode *vectorVolume,vtkMRMLAnnotationROINode *inputROI = NULL);
	vtkMRMLScalarVolumeNode* InverseConsist(vtkMRMLVectorVolumeNode *vectorVolume1,vtkMRMLVectorVolumeNode *vectorVolume2,vtkMRMLAnnotationROINode *inputROI=NULL);
	void SetDefaultDisplay();
	void CalculateStatistics(vtkMRMLScalarVolumeNode*, double statisticValues[4]);
	vtkMRMLGridTransformNode* CreateTransformFromVector(vtkMRMLVectorVolumeNode* vectorVolume);
	vtkMRMLVectorVolumeNode* CreateVectorFromTransform(vtkMRMLTransformNode* transform);
	


public:
	void SetAndObserveRegistrationQualityNode(vtkMRMLRegistrationQualityNode *node);
	vtkGetObjectMacro(RegistrationQualityNode, vtkMRMLRegistrationQualityNode);

	void SetVolumesLogic(vtkSlicerVolumesLogic* logic);
	vtkSlicerVolumesLogic* GetVolumesLogic();

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
	void InvertXandY(vtkImageData* imageData);

protected:
	vtkSmartPointer<vtkImageData> TransformField;
	/// Parameter set MRML node
	vtkMRMLRegistrationQualityNode* RegistrationQualityNode;
	

private:
	vtkSlicerRegistrationQualityLogic(const vtkSlicerRegistrationQualityLogic&);// Not implemented
	void operator=(const vtkSlicerRegistrationQualityLogic&);// Not implemented
	class vtkInternal;
	vtkInternal* Internal;
};

#endif
