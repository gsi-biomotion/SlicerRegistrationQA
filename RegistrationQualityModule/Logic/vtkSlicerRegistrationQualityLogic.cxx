#include "vtkSlicerRegistrationQualityLogic.h"

// RegistrationQuality includes
#include "vtkMRMLRegistrationQualityNode.h"

// SlicerQt Includes
#include "qSlicerApplication.h"
#include "qSlicerCLILoadableModuleFactory.h"
#include "qSlicerCLIModule.h"
#include "qSlicerCLIModuleWidget.h"
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerModuleManager.h"
#include <vtkSlicerCLIModuleLogic.h>

// MRML includes
#include <vtkMRMLVectorVolumeNode.h>
#include <vtkMRMLVectorVolumeDisplayNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLAnnotationROINode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLGridTransformNode.h>
#include <vtkMRMLTransformDisplayNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include "vtkSlicerVolumesLogic.h"
#include "vtkMRMLViewNode.h"
#include "vtkSlicerCLIModuleLogic.h"
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLTransformDisplayNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLMarkupsFiducialStorageNode.h>
#include <vtkMRMLMarkupsNode.h>
#include <vtkMRMLMarkupsDisplayNode.h>
// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkVectorNorm.h>
#include <vtkTransform.h>
#include <vtkGridTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkGeneralTransform.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkImageAccumulate.h>
#include <vtkOrientedGridTransform.h>

// STD includes
#include <iostream>
#include <cassert>
#include <math.h>

class vtkSlicerRegistrationQualityLogic::vtkInternal {
public:
	vtkInternal();
	vtkSlicerVolumesLogic* VolumesLogic;
};

//----------------------------------------------------------------------------
vtkSlicerRegistrationQualityLogic::vtkInternal::vtkInternal() {
	this->VolumesLogic = 0;
}
//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerRegistrationQualityLogic);

//----------------------------------------------------------------------------
vtkSlicerRegistrationQualityLogic::vtkSlicerRegistrationQualityLogic() {
	this->RegistrationQualityNode = NULL;
	this->TransformField = vtkSmartPointer<vtkImageData>::New();
	this->Internal = new vtkInternal;
}

//----------------------------------------------------------------------------
vtkSlicerRegistrationQualityLogic::~vtkSlicerRegistrationQualityLogic() {
	vtkSetAndObserveMRMLNodeMacro(this->RegistrationQualityNode, NULL);
	delete this->Internal;
}
//----------------------------------------------------------------------------
void vtkSlicerRegistrationQualityLogic::SetVolumesLogic(vtkSlicerVolumesLogic* logic) {
	this->Internal->VolumesLogic = logic;
}

//----------------------------------------------------------------------------
vtkSlicerVolumesLogic* vtkSlicerRegistrationQualityLogic::GetVolumesLogic() {
	return this->Internal->VolumesLogic;
}
//----------------------------------------------------------------------------
void vtkSlicerRegistrationQualityLogic::PrintSelf(ostream& os, vtkIndent indent) {
	this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkSlicerRegistrationQualityLogic
::SetAndObserveRegistrationQualityNode(vtkMRMLRegistrationQualityNode *node) {
	vtkSetAndObserveMRMLNodeMacro(this->RegistrationQualityNode, node);
}

//---------------------------------------------------------------------------
void vtkSlicerRegistrationQualityLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene) {
	vtkNew<vtkIntArray> events;
	events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
	events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
	events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
	this->SetAndObserveMRMLSceneEvents(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerRegistrationQualityLogic::RegisterNodes() {
	vtkMRMLScene* scene = this->GetMRMLScene();
	assert(scene != 0);

	scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLRegistrationQualityNode>::New());
}

//---------------------------------------------------------------------------
void vtkSlicerRegistrationQualityLogic::UpdateFromMRMLScene() {
	assert(this->GetMRMLScene() != 0);
	this->Modified();
}

//---------------------------------------------------------------------------
void vtkSlicerRegistrationQualityLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node) {
	if (!node || !this->GetMRMLScene()) {
		return;
	}

	if( node->IsA("vtkMRMLVectorVolumeNode") ||
		node->IsA("vtkMRMLLinearTransformNode") ||
		node->IsA("vtkMRMLGridTransformNode") ||
		node->IsA("vtkMRMLBSplineTransformNode") ||
		node->IsA("vtkMRMLRegistrationQualityNode")) {
		this->Modified();
	}
}

//---------------------------------------------------------------------------
void vtkSlicerRegistrationQualityLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* node) {
	if (!node || !this->GetMRMLScene()) {
		return;
	}

	if (node->IsA("vtkMRMLVectorVolumeNode") ||
		node->IsA("vtkMRMLLinearTransformNode") ||
		node->IsA("vtkMRMLGridTransformNode") ||
		node->IsA("vtkMRMLBSplineTransformNode") ||
		node->IsA("vtkMRMLRegistrationQualityNode")) {
		this->Modified();
	}
}

//---------------------------------------------------------------------------
void vtkSlicerRegistrationQualityLogic::OnMRMLSceneEndImport() {
	//Select parameter node if it exists
	vtkSmartPointer<vtkMRMLRegistrationQualityNode> paramNode = NULL;
	vtkSmartPointer<vtkMRMLNode> node = this->GetMRMLScene()->GetNthNodeByClass(
			0, "vtkMRMLRegistrationQualityNode");

	if (node) {
		paramNode = vtkMRMLRegistrationQualityNode::SafeDownCast(node);
		vtkSetAndObserveMRMLNodeMacro(this->RegistrationQualityNode, paramNode);
	}
}

//---------------------------------------------------------------------------
void vtkSlicerRegistrationQualityLogic::OnMRMLSceneEndClose() {
	this->Modified();
}
//---------------------------------------------------------------------------
void vtkSlicerRegistrationQualityLogic::CalculateDIRQAFrom(int number,int state){
  // 1. AbsoluteDifference, 2. Jacobian, 3. InverseConsistency, 4. Fiducial Distance, 5. Inverse Fiducial distance
  if(!state) {
    this->SetDefaultDisplay();
    return;
  }
  if (!this->GetMRMLScene() || !this->RegistrationQualityNode) {
	    vtkErrorMacro("CalculateDIRQAFrom: Invalid scene or parameter set node!");
	    return;   
  }
	
  if (number > 5 || number < 1){
    vtkErrorMacro("CalculateDIRQAFrom: Invalid number must be between 1 and 4!");
    return;
  }
  
  // All logic need reference Volume and ROI (if exists)
  vtkMRMLScalarVolumeNode *referenceVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetReferenceVolumeNodeID()));
  vtkMRMLAnnotationROINode *inputROI = vtkMRMLAnnotationROINode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetROINodeID()));
  
  
  if (number == 1){
    vtkMRMLScalarVolumeNode *absoluteDiffVolume = NULL;
    //Check, if it already exist
    if (this->RegistrationQualityNode->GetAbsoluteDiffVolumeNodeID()){
     absoluteDiffVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetAbsoluteDiffVolumeNodeID()));
    }
    else{
      vtkMRMLScalarVolumeNode *warpedVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetWarpedVolumeNodeID()));
      absoluteDiffVolume = this->AbsoluteDifference(referenceVolume,warpedVolume,inputROI);
      if ( !absoluteDiffVolume ) {
		vtkErrorMacro("CalculateDIRQAFrom: No absoluteDiffVolume set!");
		return;
      }
      this->RegistrationQualityNode->DisableModifiedEventOn();
      this->RegistrationQualityNode->SetAbsoluteDiffVolumeNodeID(absoluteDiffVolume->GetID());
      // Get mean and std from squared difference volume
      double statisticValues[4];
      this->CalculateStatistics(absoluteDiffVolume,statisticValues);     
      this->RegistrationQualityNode->SetAbsoluteDiffStatistics( statisticValues );
      this->RegistrationQualityNode->DisableModifiedEventOff();     
    }
    absoluteDiffVolume->GetScalarVolumeDisplayNode()->AutoWindowLevelOff();
    int window=300;
    int level=200;
    absoluteDiffVolume->GetScalarVolumeDisplayNode()->SetThreshold(0,3e3);
    absoluteDiffVolume->GetScalarVolumeDisplayNode()->SetLevel(level);
    absoluteDiffVolume->GetScalarVolumeDisplayNode()->SetWindow(window);
    absoluteDiffVolume->GetDisplayNode()->SetAndObserveColorNodeID("vtkMRMLColorTableNodeRainbow");
    this->SetForegroundImage(referenceVolume,absoluteDiffVolume,0.5);
    return;
  }
  else if(number == 2){
    vtkMRMLScalarVolumeNode *jacobianVolume = NULL;
    if (this->RegistrationQualityNode->GetJacobianVolumeNodeID()){
      jacobianVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
		this->GetMRMLScene()->GetNodeByID(
			this->RegistrationQualityNode->GetJacobianVolumeNodeID()));
    }
    else{
      vtkMRMLVectorVolumeNode *vectorVolume = vtkMRMLVectorVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetVectorVolumeNodeID()));
      jacobianVolume = this->Jacobian(vectorVolume,inputROI);
      if ( !jacobianVolume ) {
		vtkErrorMacro("CalculateDIRQAFrom: No jacobianVolume set!");
		return;
      }
      this->RegistrationQualityNode->DisableModifiedEventOn();
      this->RegistrationQualityNode->SetJacobianVolumeNodeID(jacobianVolume->GetID());      
      double statisticValues[4]; // 1. Mean 2. STD 3. Max 4. Min
      this->CalculateStatistics(jacobianVolume,statisticValues);
      this->RegistrationQualityNode->SetJacobianStatistics( statisticValues );
      this->RegistrationQualityNode->DisableModifiedEventOff();
      
    }
    double window=0.8;
    int level=1;
    
    jacobianVolume->GetScalarVolumeDisplayNode()->AutoWindowLevelOff();   
    jacobianVolume->GetScalarVolumeDisplayNode()->SetThreshold(0,3);
    jacobianVolume->GetScalarVolumeDisplayNode()->SetLevel(level);
    jacobianVolume->GetScalarVolumeDisplayNode()->SetWindow(window);
    jacobianVolume->GetDisplayNode()->SetAndObserveColorNodeID("vtkMRMLColorTableNodeRainbow");
    
    this->SetForegroundImage(referenceVolume,jacobianVolume,0.5);
    return;    
  }
  else if(number == 3){
    vtkMRMLScalarVolumeNode *inverseConsistVolume = NULL; 
    if (this->RegistrationQualityNode->GetInverseConsistVolumeNodeID()){
      inverseConsistVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
		this->GetMRMLScene()->GetNodeByID(
			this->RegistrationQualityNode->GetInverseConsistVolumeNodeID()));
    }
    else{
      vtkMRMLVectorVolumeNode *vectorVolume1 = vtkMRMLVectorVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetVectorVolumeNodeID()));
      vtkMRMLVectorVolumeNode *vectorVolume2 = vtkMRMLVectorVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetInvVectorVolumeNodeID()));
      inverseConsistVolume = this->InverseConsist(vectorVolume1,vectorVolume2,inputROI);
      if(!inverseConsistVolume){
	  vtkErrorMacro("CalculateDIRQAFrom: No inverseConsistVolume set!");
	  return;
      }
      this->RegistrationQualityNode->DisableModifiedEventOn();
      this->RegistrationQualityNode->SetInverseConsistVolumeNodeID(inverseConsistVolume->GetID());
      double statisticValues[4]; // 1. Mean 2. STD 3. Max 4. Min
      this->CalculateStatistics(inverseConsistVolume,statisticValues);
      this->RegistrationQualityNode->SetInverseConsistStatistics( statisticValues );
      this->RegistrationQualityNode->DisableModifiedEventOff();     
    }
    double window=10;
    int level=5;
    inverseConsistVolume->GetDisplayNode()->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGreen");
    inverseConsistVolume->GetScalarVolumeDisplayNode()->AutoWindowLevelOff();
      
    inverseConsistVolume->GetScalarVolumeDisplayNode()->SetThreshold(0,10);
    inverseConsistVolume->GetScalarVolumeDisplayNode()->SetLevel(level);
    inverseConsistVolume->GetScalarVolumeDisplayNode()->SetWindow(window);
    this->SetForegroundImage(referenceVolume,inverseConsistVolume,0.5);
    return;
  }
  else if( number == 4 || number ==5 ){
	 double statisticValues[4]; // 1. Mean 2. STD 3. Max 4. Min
        vtkMRMLMarkupsFiducialNode *referenceFiducals = vtkMRMLMarkupsFiducialNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetFiducialNodeID()));
        vtkMRMLMarkupsFiducialNode *movingFiducials = vtkMRMLMarkupsFiducialNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetInvFiducialNodeID()));
        vtkMRMLTransformNode *transform;
        if (number == 4){
		transform = vtkMRMLTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetTransformNodeID()));
		if (!this->CalculateFiducialsDistance(referenceFiducals, movingFiducials, transform, statisticValues) ){
			vtkErrorMacro("CalculateDIRQAFrom: Cannot calculate Fiducal distance!");
			return;
		}
        }
        else if (number == 5){
		transform = vtkMRMLTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetInvTransformNodeID()));
		if (!this->CalculateFiducialsDistance(movingFiducials, referenceFiducals, transform, statisticValues) ){
			vtkErrorMacro("CalculateDIRQAFrom: Cannot calculate inverse Fiducal distance!");
			return;
		}
        }
        this->RegistrationQualityNode->DisableModifiedEventOn();
        if (number == 4) this->RegistrationQualityNode->SetFiducialsStatistics( statisticValues );
        else if (number == 5) this->RegistrationQualityNode->SetInvFiducialsStatistics( statisticValues );
        this->RegistrationQualityNode->DisableModifiedEventOff();     
        return;
  } 
}
//---------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkSlicerRegistrationQualityLogic::AbsoluteDifference(vtkMRMLScalarVolumeNode* referenceVolume, vtkMRMLScalarVolumeNode* warpedVolume,vtkMRMLAnnotationROINode *inputROI  ) {

	if (!this->GetMRMLScene() ) {
	    vtkErrorMacro("AbsoluteDifference: Invalid scene or parameter set node!");
	    return NULL;
	}
	if (!referenceVolume || !warpedVolume ) {
		vtkErrorMacro("AbsoluteDifference: Invalid reference or warped volume!");
		return NULL;
	}


	if(!this->Internal->VolumesLogic){
		std::cerr << "AbsoluteDifference: ERROR: failed to get hold of Volumes logic" << std::endl;
		return NULL;
	}

	vtkMRMLScalarVolumeNode *outputVolume = NULL;
	vtkMRMLScalarVolumeNode *svnode = vtkMRMLScalarVolumeNode::SafeDownCast(referenceVolume);
	std::string outSS;
	std::string Name("-absoluteDifference");

	outSS = (referenceVolume->GetName() + Name);
	outSS = this->GetMRMLScene()->GenerateUniqueName(outSS);
	    
	if(svnode){
	      outputVolume = this->Internal->VolumesLogic->CloneVolume(this->GetMRMLScene(), referenceVolume, outSS.c_str());
	  
	}
	else{
	      std::cerr << "Reference volume not scalar volume!" << std::endl;
	      return NULL;
	  
	}

	if ( !outputVolume ) {
		  vtkErrorMacro("AbsoluteDifference: No output volume set!");
		  return NULL;
	}
// 	  //Check dimensions of both volume, they must be the same.
// 	  vtkSmartPointer<vtkImageData> imageDataRef = referenceVolume->GetImageData();
// 	  vtkSmartPointer<vtkImageData> imageDataWarp = warpedVolume->GetImageData();
// 	    int* dimsRef = imageDataRef->GetDimensions();
// 	    int* dimsWarp = imageDataWarp->GetDimensions();
// 	  // int dims[3]; // can't do this
// 	  if (dimsRef[0] != dimsWarp[0] || dimsRef[1] != dimsWarp[1] || dimsRef[2] != dimsWarp[2] ) {
// 	    vtkErrorMacro("AbsoluteDifference: Dimensions of Reference and Warped image don't match'!");
// 	    return NULL;
// 	  }

	qSlicerCLIModule* checkerboardfilterCLI = dynamic_cast<qSlicerCLIModule*>(
			  qSlicerCoreApplication::application()->moduleManager()->module("AbsoluteDifference"));
	QString cliModuleName("AbsoluteDifference");

	vtkSmartPointer<vtkMRMLCommandLineModuleNode> cmdNode =
			  checkerboardfilterCLI->cliModuleLogic()->CreateNodeInScene();

	// Set node parameters
	cmdNode->SetParameterAsString("inputVolume1", referenceVolume->GetID());
	cmdNode->SetParameterAsString("inputVolume2", warpedVolume->GetID());
	cmdNode->SetParameterAsString("outputVolume", outputVolume->GetID());
	if (inputROI){
	    cmdNode->SetParameterAsString("fixedImageROI", inputROI->GetID());
	}
	else{
	    cmdNode->SetParameterAsString("fixedImageROI", "");
	}

	// Execute synchronously so that we can check the content of the file after the module execution
	checkerboardfilterCLI->cliModuleLogic()->ApplyAndWait(cmdNode);

	cout << "cmdNodeStatus: " << cmdNode->GetStatus() << endl;
	if(cmdNode->GetStatus() == vtkMRMLCommandLineModuleNode::CompletedWithErrors) {
		cout << "  Error!" << endl;
		throw std::runtime_error("Error in CLI module, see command line!");
	}

	this->GetMRMLScene()->RemoveNode(cmdNode);

	outputVolume->SetAndObserveTransformNodeID(NULL);
	
	return outputVolume;
}
//---------------------------------------------------------------------------
bool vtkSlicerRegistrationQualityLogic::CalculateFiducialsDistance(vtkMRMLMarkupsFiducialNode* referenceFiducals, vtkMRMLMarkupsFiducialNode* movingFiducials,vtkMRMLTransformNode *transform, double statisticValues[4], bool absoluteDifference/*=true*/) {

	if (!this->GetMRMLScene() ) {
	    vtkErrorMacro("CalculateFiducialsDistance: Invalid scene or parameter set node!");
	    throw std::runtime_error("Internal Error, see command line!");
	}
	if (!referenceFiducals || !movingFiducials || !transform || !statisticValues ) {
		vtkErrorMacro("CalculateFiducialsDistance: Invalid input parameters!");
		throw std::runtime_error("Internal Error, see command line!");
	}
	
	int fiducialNumber = referenceFiducals->GetNumberOfFiducials();
	double sumDistance = 0;
	double maxDistance = 0;
	double minDistance = 0;
	double distanceDiff;
	
	if (fiducialNumber < 1 ||  movingFiducials->GetNumberOfFiducials() < 1 ){
		vtkErrorMacro("CalculateFiducialsDistance: Need more then 1 fiducial!");
		throw std::runtime_error("Internal Error, see command line!");		
	}
	
	if (fiducialNumber != movingFiducials->GetNumberOfFiducials() ){
		vtkErrorMacro("CalculateFiducialsDistance: Wrong fiducial numbers!");
		throw std::runtime_error("Internal Error, see command line!");		
	}
	
	// Copy Moving fiducials so we can apply transformation
	vtkMRMLMarkupsFiducialNode* warpedFiducials = NULL;
	vtkMRMLScene *scene = this->GetMRMLScene();	  
	vtkNew<vtkMRMLMarkupsFiducialNode> warpedFiducialsNew;
	vtkNew<vtkMRMLMarkupsDisplayNode> wFDisplayNode;
	vtkNew<vtkMRMLMarkupsFiducialStorageNode> wFStorageNode;
	scene->AddNode(wFDisplayNode.GetPointer());
	scene->AddNode(wFStorageNode.GetPointer());
	warpedFiducialsNew->SetAndObserveDisplayNodeID(wFDisplayNode->GetID());
	warpedFiducialsNew->SetAndObserveStorageNodeID(wFStorageNode->GetID());
	scene->AddNode(warpedFiducialsNew.GetPointer());
	warpedFiducials = warpedFiducialsNew.GetPointer();
	
	//Apply Transform to movingFiducials
	warpedFiducials->Copy(movingFiducials);
	warpedFiducials->SetAndObserveTransformNodeID( transform->GetID() );
	
	//Rename copied fiducials
	std::string outSS;
	std::string Name("-warped");

	outSS = (movingFiducials->GetName() + Name);
	outSS = this->GetMRMLScene()->GenerateUniqueName(outSS);
	warpedFiducials->SetName( outSS.c_str() );
	
	//Loop through fiducials
	for( int i=0; i<fiducialNumber; i++ ){
		//Change name of warped fiducials
		std::string outSS;
		outSS = (movingFiducials->GetNthMarkupLabel(i) + Name);
		outSS = this->GetMRMLScene()->GenerateUniqueName(outSS);
		warpedFiducials->SetNthMarkupLabel( i, outSS.c_str() );
		
		//Get position
		double referencePosition[4], movingPosition[4], warpedPosition[4];
		referenceFiducals->GetNthFiducialWorldCoordinates(i,referencePosition);
		movingFiducials->GetNthFiducialWorldCoordinates(i,movingPosition);
		warpedFiducials->GetNthFiducialWorldCoordinates(i,warpedPosition);
		distanceDiff = 0;
		for (int j=0; j<3; j++) {
			//Calculate absolute difference in mm
			if (absoluteDifference) 	distanceDiff += fabs(warpedPosition[j] - movingPosition[j]);
			else {
				//Calculate relative difference before and after transformation
				if ( referencePosition[j] !=  movingPosition[j] ) {
	// 				distanceDiff += 1 - fabs(( referencePosition[j] - warpedPosition[j] ) / ( referencePosition[j] - movingPosition[j] ));
				}
				else {
					if ( referencePosition[j] ==  warpedPosition[j]) distanceDiff = 0;
					else distanceDiff = -1;
				}
			}
		}

		if ( distanceDiff > maxDistance) maxDistance = distanceDiff;
		if ( distanceDiff < minDistance) minDistance = distanceDiff;

		sumDistance += distanceDiff;
		std::cerr << "Position: " << i << "," << " with distance: " << distanceDiff <<   std::endl;
	}
	statisticValues[0] = sumDistance / (  fiducialNumber);
	statisticValues[2] = maxDistance;
	statisticValues[3] = minDistance;
	return true;
}

//--- Image Checks -----------------------------------------------------------
void vtkSlicerRegistrationQualityLogic::FalseColor(int state) {
	if (!this->GetMRMLScene() || !this->RegistrationQualityNode) {
		vtkErrorMacro("Invalid scene or parameter set node!");
		throw std::runtime_error("Internal Error, see command line!");
	}
	
	//TODO: Volumes go back to gray value - perhaps we should rembemer previous color settings?
	if (!state) {
		this->SetDefaultDisplay();
		return;
	}

	vtkMRMLScalarVolumeNode *referenceVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetReferenceVolumeNodeID()));

	vtkMRMLScalarVolumeNode *warpedVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetWarpedVolumeNodeID()));

	if (!referenceVolume || !warpedVolume) {
		throw std::runtime_error("Reference or warped volume not set!");
	}
	
	vtkMRMLScalarVolumeDisplayNode *referenceVolumeDisplayNode = referenceVolume->GetScalarVolumeDisplayNode();
	vtkMRMLScalarVolumeDisplayNode *warpedVolumeDisplayNode = warpedVolume->GetScalarVolumeDisplayNode();

	referenceVolumeDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeWarmTint1");
	warpedVolumeDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeCoolTint1");

	// Set window and level the same for warped and reference volume.
	warpedVolumeDisplayNode->AutoWindowLevelOff();
	warpedVolumeDisplayNode->SetWindow( referenceVolumeDisplayNode->GetWindow() );
	warpedVolumeDisplayNode->SetLevel( referenceVolumeDisplayNode->GetLevel() );

	this->SetForegroundImage(referenceVolume,warpedVolume,0.5);

	return;
}

//----------------------------------------------------------------------------
void vtkSlicerRegistrationQualityLogic::Flicker(int opacity) {
	if (!this->GetMRMLScene() || !this->RegistrationQualityNode) {
		vtkErrorMacro("Flicker: Invalid scene or parameter set node!");
		return;
	}

	vtkMRMLScalarVolumeNode *referenceVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetReferenceVolumeNodeID()));

	vtkMRMLScalarVolumeNode *warpedVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetWarpedVolumeNodeID()));

	if (!referenceVolume || !warpedVolume) {
		vtkErrorMacro("Flicker: Invalid reference or warped volume!");
		return;
	}

	this->SetForegroundImage(referenceVolume,warpedVolume,opacity);

	return;
}

void vtkSlicerRegistrationQualityLogic
::getSliceCompositeNodeRASBounds(vtkMRMLSliceCompositeNode *scn, double* minmax) {

	vtkMRMLScalarVolumeNode* foreground = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(scn->GetBackgroundVolumeID()));
	vtkMRMLScalarVolumeNode* background = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(scn->GetForegroundVolumeID()));

	double rasBounds[6] = {INFINITY,-INFINITY,INFINITY,-INFINITY,INFINITY,-INFINITY};
	double rasBoundsBack[6] = {INFINITY,-INFINITY,INFINITY,-INFINITY,INFINITY,-INFINITY};
	if(foreground) foreground->GetRASBounds(rasBounds);
	if(background) background->GetRASBounds(rasBoundsBack);
	for(int i=0;i<3; i++) {
		minmax[2*i] = std::min(rasBounds[2*i],rasBoundsBack[2*i]);
		minmax[2*i+1] = std::max(rasBounds[2*i+1],rasBoundsBack[2*i+1]);
		if(minmax[2*i]>minmax[2*i+1]) {
			cout << "rasBounds infty" << endl;
			minmax[2*i] = minmax[2*i+1] = 0;
		}
	}
}

//----------------------------------------------------------------------------
/**
 * Movie through slices.
 * TODO:	- Calculate slice spacing
 * 			- Changed slice directions
 * 			- Oblique slices
 */
void vtkSlicerRegistrationQualityLogic::Movie() {
	if (!this->GetMRMLScene() || !this->RegistrationQualityNode) {
		vtkErrorMacro("Movie: Invalid scene or parameter set node!");
		return;
	}
	vtkMRMLSliceNode* sliceNodeRed = vtkMRMLSliceNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID("vtkMRMLSliceNodeRed"));
	vtkMRMLSliceNode* sliceNodeYellow = vtkMRMLSliceNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID("vtkMRMLSliceNodeYellow"));
	vtkMRMLSliceNode* sliceNodeGreen = vtkMRMLSliceNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID("vtkMRMLSliceNodeGreen"));

	double rasBoundsRed[6], rasBoundsYellow[6], rasBoundsGreen[6];
	int runState = this->RegistrationQualityNode->GetMovieRun();

	if(runState) {
		vtkMRMLSliceCompositeNode *scn = vtkMRMLSliceCompositeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID("vtkMRMLSliceCompositeNodeRed"));
		getSliceCompositeNodeRASBounds(scn,rasBoundsRed);
		scn = vtkMRMLSliceCompositeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID("vtkMRMLSliceCompositeNodeYellow"));
		getSliceCompositeNodeRASBounds(scn,rasBoundsYellow);
		scn = vtkMRMLSliceCompositeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID("vtkMRMLSliceCompositeNodeGreen"));
		getSliceCompositeNodeRASBounds(scn,rasBoundsGreen);

		double redMin = rasBoundsRed[4];
		double redMax = rasBoundsRed[5];
		double redStep = 3;
		double redPos = redMin;
		double yellowMin = rasBoundsYellow[0];
		double yellowMax = rasBoundsYellow[1];
		double yellowStep = 3;
		double yellowPos = yellowMin;
		double greenMin = rasBoundsGreen[2];
		double greenMax = rasBoundsGreen[3];
		double greenStep = 3;
		double greenPos = greenMin;

		cout << "movie:\n"
			<< " red:    " << redMin << " .. " << redMax << "\n"
			<< " yellow: " << yellowMin << " .. " << yellowMax << "\n"
			<< " green:  " << greenMin << " .. " << greenMax << "\n"
			<< endl;

		while(runState) {
			//cout << " runRed=" << (runState&1?"true":"false")
			//		<< " runYellow=" << (runState&2?"true":"false")
			//		<< " runGreen=" << (runState&4?"true":"false")
			//		<< " MovieRun=" << runState << endl;

			if(runState&1) {
				sliceNodeRed->JumpSliceByCentering((yellowMin+yellowMax)/2,(greenMin+greenMax)/2,redPos);
				redPos += redStep;
				if(redPos>redMax)  {
					redPos -= redMax - redMin;
					cout << "red Overflow" << endl;
				}
			}
			if(runState&2) {
				sliceNodeYellow->JumpSliceByCentering(yellowPos,(greenMin+greenMax)/2,(redMin+redMax)/2);
				yellowPos += yellowStep;
				if(yellowPos>yellowMax)  {
					yellowPos -= yellowMax - yellowMin;
					cout << "yellow Overflow" << endl;
				}
			}
			if(runState&4) {
				sliceNodeGreen->JumpSliceByCentering((yellowMin+yellowMax)/2,greenPos,(redMin+redMax)/2);
				greenPos += greenStep;
				if(greenPos>greenMax)  {
					greenPos -= greenMax - greenMin;
					cout << "green Overflow" << endl;
				}
			}
			qSlicerApplication::application()->processEvents();

			runState = RegistrationQualityNode->GetMovieRun();
		}
	}
}

//----------------------------------------------------------------------------
void vtkSlicerRegistrationQualityLogic::Checkerboard(int state) {
	//   Calling checkerboardfilter cli. Logic has been copied and modified from CropVolumeLogic onApply.
	if (!this->GetMRMLScene() || !this->RegistrationQualityNode) {
		vtkErrorMacro("Invalid scene or parameter set node!");
		throw std::runtime_error("Internal Error, see command line!");
	}
	vtkMRMLScalarVolumeNode *referenceVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetReferenceVolumeNodeID()));
	vtkMRMLScalarVolumeNode *warpedVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetWarpedVolumeNodeID()));

	if (!state) {
	      	this->SetDefaultDisplay();
		return;
	}

	if (!referenceVolume || !warpedVolume) {
		throw std::runtime_error("Reference or warped volume not set!");
	}

	if (!this->RegistrationQualityNode->GetCheckerboardVolumeNodeID()) {
		if(!this->Internal->VolumesLogic) {
			vtkErrorMacro("Failed to get hold of Volumes logic!");
			throw std::runtime_error("Internal Error, see command line!");
		}

		int PatternValue = this->RegistrationQualityNode->GetCheckerboardPattern();
		std::string outSS;
		std::string Name("-CheckerboardPattern_");
		std::ostringstream strPattern;
		strPattern << PatternValue;
		outSS = referenceVolume->GetName() + Name + strPattern.str();
		outSS = this->GetMRMLScene()->GenerateUniqueName(outSS);
		
		vtkMRMLScalarVolumeNode *outputVolume = this->Internal->VolumesLogic->CloneVolume(
			this->GetMRMLScene(), referenceVolume, outSS.c_str());
		if (!outputVolume) {
			vtkErrorMacro("Could not create Checkerboard volume!");
			throw std::runtime_error("Internal Error, see command line!");
		}

		qSlicerCLIModule* checkerboardfilterCLI = dynamic_cast<qSlicerCLIModule*>(
			qSlicerCoreApplication::application()->moduleManager()->module("CheckerBoardFilter"));
		if (!checkerboardfilterCLI) {
			vtkErrorMacro("No Checkerboard Filter module!");
			throw std::runtime_error("Internal Error, see command line!");
		}

		vtkSmartPointer<vtkMRMLCommandLineModuleNode> cmdNode =
			checkerboardfilterCLI->cliModuleLogic()->CreateNodeInScene();

		//Convert PatternValue to string
		std::ostringstream outPattern;
		outPattern << PatternValue << "," << PatternValue << "," << PatternValue;
		// Set node parameters
		cmdNode->SetParameterAsString("checkerPattern",outPattern.str().c_str());
		cmdNode->SetParameterAsString("inputVolume1", referenceVolume->GetID());
		cmdNode->SetParameterAsString("inputVolume2", warpedVolume->GetID());
		cmdNode->SetParameterAsString("outputVolume", outputVolume->GetID());

		// Execute synchronously so that we can check the content of the file after the module execution
		checkerboardfilterCLI->cliModuleLogic()->ApplyAndWait(cmdNode);

		this->GetMRMLScene()->RemoveNode(cmdNode);

		outputVolume->SetAndObserveTransformNodeID(NULL);
		this->RegistrationQualityNode->SetCheckerboardVolumeNodeID(outputVolume->GetID());
		return;
	}

	vtkMRMLScalarVolumeNode *checkerboardVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetCheckerboardVolumeNodeID()));
	this->SetForegroundImage(checkerboardVolume,referenceVolume,0);
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkSlicerRegistrationQualityLogic::Jacobian(vtkMRMLVectorVolumeNode *vectorVolume,vtkMRMLAnnotationROINode *inputROI ) {

	if (!this->GetMRMLScene()) {
		throw std::runtime_error("Internal Error, see command line!");
	}


	if (!vectorVolume ) {
		throw std::runtime_error("Vector field not set!");
	}
	vtkMRMLScalarVolumeNode* outputVolume = NULL;
	// Create new scalar volume
	vtkNew<vtkMRMLScalarVolumeNode> outputVolumeNew;
	vtkNew<vtkMRMLScalarVolumeDisplayNode> sDisplayNode;
	sDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey");
	this->GetMRMLScene()->AddNode(sDisplayNode.GetPointer());
	outputVolumeNew->SetAndObserveDisplayNodeID(sDisplayNode->GetID());
	outputVolumeNew->SetAndObserveStorageNodeID(NULL);
	this->GetMRMLScene()->AddNode(outputVolumeNew.GetPointer());
	outputVolume = outputVolumeNew.GetPointer();
	
	if(!outputVolume){
	  throw std::runtime_error("Can't create output volume!");
	}
		
	std::string outSS;
	std::string Name("-jacobian");
	outSS = vectorVolume->GetName() + Name;
	outSS = this->GetMRMLScene()->GenerateUniqueName(outSS);
	outputVolume->SetName(outSS.c_str());

	qSlicerCLIModule* jacobianfilterCLI = dynamic_cast<qSlicerCLIModule*>(
			qSlicerCoreApplication::application()->moduleManager()->module("JacobianFilter"));
	if (!jacobianfilterCLI) {
		vtkErrorMacro("No Jacobian Filter module!");
		throw std::runtime_error("Internal Error, see command line!");
	}

	vtkSmartPointer<vtkMRMLCommandLineModuleNode> cmdNode =
			jacobianfilterCLI->cliModuleLogic()->CreateNodeInScene();

	// Set node parameters
	cmdNode->SetParameterAsString("inputVolume", vectorVolume->GetID());
	cmdNode->SetParameterAsString("outputVolume", outputVolume->GetID());
	if (inputROI){
	  cmdNode->SetParameterAsString("fixedImageROI", inputROI->GetID());
	}
	else{
	  cmdNode->SetParameterAsString("fixedImageROI", "");
	  
	}
	// Execute synchronously so that we can check the content of the file after the module execution
	jacobianfilterCLI->cliModuleLogic()->ApplyAndWait(cmdNode);

	this->GetMRMLScene()->RemoveNode(cmdNode);

	if(outputVolume){
	  outputVolume->SetAndObserveTransformNodeID(NULL);
	  return outputVolume;
	}
	else{
	  return NULL;
	}
	
	
}
//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkSlicerRegistrationQualityLogic::InverseConsist(vtkMRMLVectorVolumeNode *vectorVolume1,vtkMRMLVectorVolumeNode *vectorVolume2,vtkMRMLAnnotationROINode *inputROI) {

	if (!this->GetMRMLScene() ) {
		throw std::runtime_error("Inverse Consistency: Invalid scene!");
	}

//
	if (!vectorVolume1 || !vectorVolume2 ) {
	    throw std::runtime_error("Volumes not set!");
	}
	
	vtkMRMLScalarVolumeNode* outputVolume = NULL;
	// Create new scalar volume
	vtkNew<vtkMRMLScalarVolumeNode> outputVolumeNew;
	vtkNew<vtkMRMLScalarVolumeDisplayNode> sDisplayNode;
	sDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey");
	this->GetMRMLScene()->AddNode(sDisplayNode.GetPointer());
	outputVolumeNew->SetAndObserveDisplayNodeID(sDisplayNode->GetID());
	outputVolumeNew->SetAndObserveStorageNodeID(NULL);
	this->GetMRMLScene()->AddNode(outputVolumeNew.GetPointer());
	outputVolume = outputVolumeNew.GetPointer();
	
	if(!outputVolume){
	  throw std::runtime_error("Can't create output volume!");
	}
	  		
	std::string outSS;
	std::string Name("-invConsist");
	outSS = vectorVolume1->GetName() + Name;
	outSS = this->GetMRMLScene()->GenerateUniqueName(outSS);
	outputVolume->SetName(outSS.c_str());

	qSlicerCLIModule* checkerboardfilterCLI = dynamic_cast<qSlicerCLIModule*>(
			qSlicerCoreApplication::application()->moduleManager()->module("InverseConsistency"));
	QString cliModuleName("InverseConsistency");

	vtkSmartPointer<vtkMRMLCommandLineModuleNode> cmdNode =
			checkerboardfilterCLI->cliModuleLogic()->CreateNodeInScene();

	// Set node parameters
	cmdNode->SetParameterAsString("inputVolume1", vectorVolume1->GetID());
	cmdNode->SetParameterAsString("inputVolume2", vectorVolume2->GetID());
	cmdNode->SetParameterAsString("outputVolume", outputVolume->GetID());
	if (inputROI)
	{
	    cmdNode->SetParameterAsString("fixedImageROI", inputROI->GetID());
	}
	else
	{
	    cmdNode->SetParameterAsString("fixedImageROI", "");
	}

	// Execute synchronously so that we can check the content of the file after the module execution
	checkerboardfilterCLI->cliModuleLogic()->ApplyAndWait(cmdNode);

	this->GetMRMLScene()->RemoveNode(cmdNode);

	if (outputVolume){
	  outputVolume->SetAndObserveTransformNodeID(NULL);
	  return outputVolume;
	}
	else{
	  return NULL;
	}
	
}
//---Change Vector node to transform node-------------------------------------------------------------------------
vtkMRMLGridTransformNode* vtkSlicerRegistrationQualityLogic::CreateTransformFromVector(vtkMRMLVectorVolumeNode* vectorVolume)
{
	if (!vectorVolume) {
	  std::cerr << "CreateTransormFromVector: Volumes not set!" << std::endl;
	  return NULL;
	}
		

	// Logic partialy copied from: vtkMRMLTransformStorageNode.cxx
	vtkNew<vtkImageData> gridImage_Ras;	
	vtkSmartPointer<vtkImageData> imageData = vectorVolume->GetImageData();
	if (!imageData) {
	  std::cerr << "CreateTransormFromVector: No image data!" << std::endl;
	  return NULL;
	}
	
	gridImage_Ras->DeepCopy( imageData.GetPointer() );
	
	this->InvertXandY( gridImage_Ras.GetPointer() );

	// Origin
	gridImage_Ras->SetOrigin( - vectorVolume->GetOrigin()[0], - vectorVolume->GetOrigin()[1], vectorVolume->GetOrigin()[2] );

	// Spacing
	gridImage_Ras->SetSpacing( vectorVolume->GetSpacing()[0], vectorVolume->GetSpacing()[1], vectorVolume->GetSpacing()[2] );
	
	// Grid transform
	vtkSmartPointer<vtkOrientedGridTransform> transformFromParent = vtkSmartPointer<vtkOrientedGridTransform>::New();
	
	//Direction
	vtkNew<vtkMatrix4x4> gridDirectionMatrix_RAS;
	vectorVolume->GetIJKToRASDirectionMatrix(gridDirectionMatrix_RAS.GetPointer());
        transformFromParent->SetGridDirectionMatrix(gridDirectionMatrix_RAS.GetPointer());
	
	gridImage_Ras->Update();
// 	this->InvertXandY( gridImage_Ras.GetPointer() );

// 	int dims[3];
// 	gridImage_Ras->GetDimensions(dims);
// // 	double* displacementVectors_Ras = reinterpret_cast<double*>(gridImage_Ras->GetScalarPointer());
// // 	int allPoints = gridImage_Ras->GetNumberOfScalarComponents() * dims[2] * dims[1] * dims[0];
// // 	int n = 0;
// // 
// 	for(int k=0;k<dims[2];k++)
// 	{
// 		for(int j=0;j<dims[1];j++)
// 		{
// 			for(int i=0;i<dims[0];i++)
// 			{
// 			  double ScalarComponent;
// 			  
// 			  ScalarComponent = gridImage_Ras->GetScalarComponentAsDouble(i,j,k,0);
// 			  gridImage_Ras->SetScalarComponentFromDouble(i,j,k,0,-ScalarComponent);
// 			  
// 			  ScalarComponent = gridImage_Ras->GetScalarComponentAsDouble(i,j,k,1);
// 			  gridImage_Ras->SetScalarComponentFromDouble(i,j,k,1,-ScalarComponent);
// 			}
// 		}
// 	}
				
// 	while ( n < allPoints)
// 	{
// 		if ( !displacementVectors_Ras[n]) {
// 			std::cerr << "Point " << n << "/" << allPoints << std::endl;
// 			n = allPoints;
// 			break;
// 		}
// 		displacementVectors_Ras[n] = -displacementVectors_Ras[n];
// 		n++;
// 		if ( !displacementVectors_Ras[n] ) {
// 			std::cerr << "Point " << n << "/" << allPoints << std::endl;
// 			n = allPoints;
// 			break;
// 		}
// 		displacementVectors_Ras[n] = -displacementVectors_Ras[n];	
// 		n++;
// 		n++;
// 		
// 	}
	
	#if (VTK_MAJOR_VERSION <= 5)
	  transformFromParent->SetDisplacementGrid( gridImage_Ras.GetPointer() );
	#else
	  transformFromParent->SetDisplacementGridData( gridImage_Ras.GetPointer() );
	#endif
	// Set the interpolation to cubic to have smooth derivatives
	transformFromParent->SetInterpolationModeToCubic();
	transformFromParent->Update();
	if (!transformFromParent) {
	  std::cerr << "CreateTransormFromVector: No transform from parent!" << std::endl;
	  return NULL;
	}
	// Setting transform
	vtkNew<vtkMRMLGridTransformNode> transform;
	transform->SetAndObserveTransformFromParent( transformFromParent );
	
	// Name
	std::string name;
	std::string additionName("-transform");
	name = vectorVolume-> GetName() + additionName;
	name = this->GetMRMLScene()->GenerateUniqueName( name );
	transform->SetName( name.c_str() );
	
	this->GetMRMLScene()->AddNode(transform.GetPointer());
	
	
	
	return transform.GetPointer();
	  
}
//---Change Vector node to transform node-------------------------------------------------------------------------
vtkMRMLVectorVolumeNode* vtkSlicerRegistrationQualityLogic::CreateVectorFromTransform(vtkMRMLTransformNode* transform)
{
	if (!transform) {
	  std::cerr << "CreateVectorFromTransform: No transform set!" << std::endl;
	  return NULL;
	}
	//Create new Vector Volume from transform. Copied from Crop Volume Logic.
	
	if (!this->GetMRMLScene()) {
		vtkErrorMacro("CreateVectorFromTransform: Invalid scene!");
		return NULL;
	}
			  
	vtkNew<vtkMRMLVectorVolumeNode> vectorVolume;
// 	vtkNew<vtkMRMLVectorVolumeDisplayNode> vvDisplayNode;
// 	scene->AddNode(vvDisplayNode.GetPointer());
// 	vectorVolume->SetAndObserveDisplayNodeID(vvDisplayNode->GetID());
// 	vectorVolume->SetAndObserveStorageNodeID(NULL);
	
	
	//Create Grid transform	
	vtkOrientedGridTransform* transformFromParent = vtkOrientedGridTransform::SafeDownCast(
							transform->GetTransformFromParent());
	
	if (!transformFromParent) {
	  std::cerr << "CreateVectorFromTransform: No transform from parent!" << std::endl;
	  return NULL;
	}
	
	
	vtkNew<vtkImageData> imageData;
	vtkSmartPointer<vtkImageData> gridImage_Ras = transformFromParent->GetDisplacementGrid();
	
	if (!gridImage_Ras) {
	  std::cerr << "CreateVectorFromTransform: No image data!" << std::endl;
	  return NULL;
	}
	  	
	// Name
	std::string name;
	std::string additionName("-vF");
	name = transform-> GetName() + additionName;
	name = this->GetMRMLScene()->GenerateUniqueName( name );
	vectorVolume->SetName( name.c_str() );
	
	//Spacing
	vectorVolume->SetSpacing( gridImage_Ras->GetSpacing()[0], gridImage_Ras->GetSpacing()[1], gridImage_Ras->GetSpacing()[2] );
	//Origin
	vectorVolume->SetOrigin( -gridImage_Ras->GetOrigin()[0], -gridImage_Ras->GetOrigin()[1], gridImage_Ras->GetOrigin()[2] );
	// Dimensions
	int dims[3];
	gridImage_Ras->GetDimensions(dims);
	imageData->SetDimensions(dims);
	
	//Direction
 	vtkSmartPointer<vtkMatrix4x4> gridDirectionMatrix_RAS = transformFromParent->GetGridDirectionMatrix();
	vectorVolume->SetIJKToRASDirectionMatrix(gridDirectionMatrix_RAS.GetPointer());
	
	imageData->DeepCopy( gridImage_Ras.GetPointer() );
	
	imageData->Update();
	
	this->InvertXandY( imageData.GetPointer() );

	// Image Data
	vectorVolume->SetAndObserveImageData( imageData.GetPointer() );
	
	this->GetMRMLScene()->AddNode(vectorVolume.GetPointer());
	
	return vectorVolume.GetPointer();
}
//--- Invert X and Y in image Data -----------------------------------------------------------
void vtkSlicerRegistrationQualityLogic::InvertXandY(vtkImageData* imageData){
	if (!imageData) {
		      throw std::runtime_error("InvertXandY: No imageData.");
		      return;
	      }
	      
	// Find scalar type and cast the right pointer
	// TODO: Add all types.
	
	int dims[3];
	imageData->GetDimensions(dims);
	int allPoints = imageData->GetNumberOfScalarComponents() * dims[2] * dims[1] * dims[0];
	int n = 0;
	
	if ( imageData->GetScalarType() == VTK_DOUBLE ){
	      double* imageDataVectors = reinterpret_cast<double*>(imageData->GetScalarPointer());
	      while ( n < allPoints ){
		      *(imageDataVectors++) *= -1;
		      *(imageDataVectors++) *= -1;
		      imageDataVectors++;
		      n += 3;
		      if ( !imageDataVectors ){
			      std::cerr << "InvertXandY: End of pointer!" << n << std::endl;
			      return;
		      }
	      }
	}
	else if ( imageData->GetScalarType() == VTK_FLOAT ){
	      float* imageDataVectors = reinterpret_cast<float*>(imageData->GetScalarPointer());
	      while ( n < allPoints ){
		      *(imageDataVectors++) *= -1;
		      *(imageDataVectors++) *= -1;
		      imageDataVectors++;
		      n += 3;
		      if ( !imageDataVectors ){
			      std::cerr << "InvertXandY: End of pointer!" << n << std::endl;
			      return;
		      }
	      }
	}
	else if ( imageData->GetScalarType() == VTK_SHORT ){
	      short* imageDataVectors = reinterpret_cast<short*>(imageData->GetScalarPointer());
	       while ( n < allPoints ){
		      *(imageDataVectors++) *= -1;
		      *(imageDataVectors++) *= -1;
		      imageDataVectors++;
		      n += 3;
		      if ( !imageDataVectors ){
			      std::cerr << "InvertXandY: End of pointer!" << n << std::endl;
			      return;
		      }
	      }
	}
	else if ( imageData->GetScalarType() == VTK_INT ){
	      int* imageDataVectors = reinterpret_cast<int*>(imageData->GetScalarPointer());
	       while ( n < allPoints ){
		      *(imageDataVectors++) *= -1;
		      *(imageDataVectors++) *= -1;
		      imageDataVectors++;
		      n += 3;
		      if ( !imageDataVectors ){
			      std::cerr << "InvertXandY: End of pointer!" << n << std::endl;
			      return;
		      }
	      }
	}
	else{
	      throw std::runtime_error("InvertXandY: Unknown image type." );
	      return;
	}
	return;
}
//--- Default mode when checkbox is unchecked -----------------------------------------------------------
void vtkSlicerRegistrationQualityLogic::SetDefaultDisplay() {
	if (!this->GetMRMLScene() || !this->RegistrationQualityNode) {
		throw std::runtime_error("SetDefaultDisplay: Invalid scene or parameter set node!");
		return;
	}
	vtkMRMLScalarVolumeNode *referenceVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetReferenceVolumeNodeID()));
	
	
	vtkMRMLScalarVolumeNode *warpedVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetWarpedVolumeNodeID()));

	if (!warpedVolume || !referenceVolume) {
		throw std::runtime_error("SetDefaultDisplay: Invalid volumes!");
		return;
	}
	//TODO: Volumes go back to gray value - perhaps we should rembemer previous color settings?
	warpedVolume->GetDisplayNode()->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey");
	referenceVolume->GetDisplayNode()->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey");

	// Set window and level the same for warped and reference volume.
	referenceVolume->GetScalarVolumeDisplayNode()->AutoWindowLevelOff();
	// 	double window, level;
	// 	window = warpedVolume->GetScalarVolumeDisplayNode()->GetWindow();
	// 	level = warpedVolume->GetScalarVolumeDisplayNode()->GetLevel();
	referenceVolume->GetScalarVolumeDisplayNode()->SetWindow(warpedVolume->GetScalarVolumeDisplayNode()->GetWindow());
	referenceVolume->GetScalarVolumeDisplayNode()->SetLevel(warpedVolume->GetScalarVolumeDisplayNode()->GetLevel());
	this->SetForegroundImage(warpedVolume,referenceVolume,0.5);

	return;
}

//---Calculate Statistic of image-----------------------------------
void vtkSlicerRegistrationQualityLogic::CalculateStatistics(vtkMRMLScalarVolumeNode *inputVolume, double statisticValues[4] ) {
	vtkNew<vtkImageAccumulate> StatImage;
	StatImage->SetInput(inputVolume->GetImageData());
	StatImage->Update();
	statisticValues[0]= StatImage->GetMean()[0];
	statisticValues[1]= StatImage->GetStandardDeviation()[0];
	statisticValues[2]= StatImage->GetMax()[0];
	statisticValues[3]= StatImage->GetMin()[0];
}


//---Change backroung image and set opacity-----------------------------------
void vtkSlicerRegistrationQualityLogic
::SetForegroundImage(vtkMRMLScalarVolumeNode *backgroundVolume,
					 vtkMRMLScalarVolumeNode *foregroundVolume,
					 double opacity) {

	qSlicerApplication * app = qSlicerApplication::application();
	qSlicerLayoutManager * layoutManager = app->layoutManager();

	if (!layoutManager) {
		return;
	}

	vtkMRMLSliceCompositeNode *rcn = vtkMRMLSliceCompositeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID("vtkMRMLSliceCompositeNodeRed"));
	vtkMRMLSliceCompositeNode *ycn = vtkMRMLSliceCompositeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID("vtkMRMLSliceCompositeNodeYellow"));
	vtkMRMLSliceCompositeNode *gcn = vtkMRMLSliceCompositeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID("vtkMRMLSliceCompositeNodeGreen"));

	vtkMRMLSliceLogic* redSliceLogic = rcn != NULL ?
			GetMRMLApplicationLogic()->GetSliceLogicByLayoutName(rcn->GetLayoutName()) : NULL;
	if (redSliceLogic == NULL) {
		vtkErrorMacro("SetForegroundImage: Invalid SliceLogic!");
		return;
	}
	// Link Slice Controls
	rcn->SetLinkedControl(1);
	ycn->SetLinkedControl(1);
	gcn->SetLinkedControl(1);

	// Set volumes and opacity for all three layers.
	if (backgroundVolume) {
		rcn->SetBackgroundVolumeID(backgroundVolume->GetID());
	} else {
		rcn->SetBackgroundVolumeID(NULL);
	}

	if (foregroundVolume) {
		rcn->SetForegroundVolumeID(foregroundVolume->GetID());
	} else {
		rcn->SetForegroundVolumeID(NULL);
	}

	rcn->SetForegroundOpacity(opacity);
	redSliceLogic->StartSliceCompositeNodeInteraction(
		vtkMRMLSliceCompositeNode::ForegroundVolumeFlag
		| vtkMRMLSliceCompositeNode::BackgroundVolumeFlag);
	redSliceLogic->EndSliceCompositeNodeInteraction();
	return;
}
//----------------------------------------------------------------------------
