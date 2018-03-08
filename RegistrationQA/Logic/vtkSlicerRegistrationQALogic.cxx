#include "vtkSlicerRegistrationQALogic.h"

// RegistrationQA includes
#include "vtkMRMLRegistrationQANode.h"

// SlicerQt Includes
#include "qSlicerApplication.h"
#include "qSlicerCLILoadableModuleFactory.h"
#include "qSlicerCLIModule.h"
#include "qSlicerCLIModuleWidget.h"
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerModuleManager.h"
#include <vtkSlicerCLIModuleLogic.h>
#include "qSlicerIO.h"
#include "qSlicerCoreIOManager.h"
#include "qMRMLLayoutManager.h"
#include "qSlicerLayoutManager.h"


// MRML includes
#include <vtkMRMLVectorVolumeNode.h>
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLVectorVolumeDisplayNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLAnnotationROINode.h>
#include <vtkMRMLSegmentationNode.h>
#include <vtkSlicerSegmentationsModuleLogic.h>
#include <vtkMRMLSegmentationDisplayNode.h>
#include <vtkMRMLSegmentationStorageNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLGridTransformNode.h>
#include <vtkMRMLTransformDisplayNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include "vtkSlicerVolumesLogic.h"
#include "vtkSlicerTransformLogic.h"
#include "vtkSlicerAnnotationModuleLogic.h"
#include "vtkMRMLViewNode.h"
#include "vtkSlicerCLIModuleLogic.h"
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLSliceLogic.h>
#include "vtkMRMLAnnotationSnapshotNode.h"
#include "vtkMRMLAnnotationSnapshotStorageNode.h"
#include "qMRMLUtils.h"
#include "qMRMLScreenShotDialog.h"
#include <vtkMRMLTransformDisplayNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLMarkupsFiducialStorageNode.h>
#include <vtkMRMLMarkupsNode.h>
#include <vtkMRMLMarkupsDisplayNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLSubjectHierarchyConstants.h>
#include <vtkMRMLTableNode.h>
#include <vtkMRMLTableStorageNode.h>

// SlicerRT
#include <vtkSlicerSegmentComparisonModuleLogic.h>
#include <vtkMRMLSegmentComparisonNode.h>

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
#include <vtkSegment.h>
#include <vtkOrientedImageData.h>
#include <vtkTable.h>
#include <vtkStringArray.h>
#include <vtkAbstractArray.h>

// CTK includes
#include <ctkVTKWidgetsUtils.h>

// STD includes
#include <iostream>
#include <cassert>
#include <math.h>
#include <exception>

#include <QStandardItemModel>

// constants
const std::string vtkSlicerRegistrationQALogic::ITEMID = "_itemID";
const std::string vtkSlicerRegistrationQALogic::INVERSE = "Inverse";
const std::string vtkSlicerRegistrationQALogic::IMAGE = "Image";
const std::string vtkSlicerRegistrationQALogic::WARPED_IMAGE = "WarpedImage";
const std::string vtkSlicerRegistrationQALogic::VECTOR_FIELD = "VectorField";
const std::string vtkSlicerRegistrationQALogic::TRANSFORM = "Transform";
const std::string vtkSlicerRegistrationQALogic::ABSOLUTEDIFFERENCE = "AbsDiff";
const std::string vtkSlicerRegistrationQALogic::JACOBIAN = "Jacobian";
const std::string vtkSlicerRegistrationQALogic::INVERSECONSISTENCY = "InvConsist";
const std::string vtkSlicerRegistrationQALogic::REFIMAGEID = "ReferenceImageItemID";
const std::string vtkSlicerRegistrationQALogic::REGISTRATION_TYPE = "RegistrationType";
const std::string vtkSlicerRegistrationQALogic::PHASENUMBER = "PhaseNumber";
const std::string vtkSlicerRegistrationQALogic::PHASETYPE= "Phase";
const std::string vtkSlicerRegistrationQALogic::CT= "CT";
const std::string vtkSlicerRegistrationQALogic::SEGMENTATION= "Segmentation";
const std::string vtkSlicerRegistrationQALogic::SEGMENTATID = "SegmentID";
const std::string vtkSlicerRegistrationQALogic::ROI= "ROI";
const std::string vtkSlicerRegistrationQALogic::ROIITEMID= "ROI" + ITEMID;
const std::string vtkSlicerRegistrationQALogic::DIR = "DIR";
const std::string vtkSlicerRegistrationQALogic::FILEPATH = "FilePath";
const std::string vtkSlicerRegistrationQALogic::FIXEDIMAGEID = "FixedImage" + ITEMID;
const std::string vtkSlicerRegistrationQALogic::MOVINGIMAGEID = "MovingImage" + ITEMID;
const std::string vtkSlicerRegistrationQALogic::NODEITEMID = "Node" + ITEMID;
const std::string vtkSlicerRegistrationQALogic::VECTORITEMID = VECTOR_FIELD + ITEMID;
const std::string vtkSlicerRegistrationQALogic::TRANSFORMITEMID = TRANSFORM + ITEMID;
const std::string vtkSlicerRegistrationQALogic::ABSDIFFNODEITEMID = ABSOLUTEDIFFERENCE + ITEMID;
const std::string vtkSlicerRegistrationQALogic::JACOBIANITEMID = JACOBIAN + ITEMID;
const std::string vtkSlicerRegistrationQALogic::INVERSECONSISTENCYITEMID = INVERSECONSISTENCY + ITEMID;
const std::string vtkSlicerRegistrationQALogic::TABLE = "_table";
const std::string vtkSlicerRegistrationQALogic::ABSDIFFTABLE = ABSOLUTEDIFFERENCE + TABLE;
const std::string vtkSlicerRegistrationQALogic::JACOBIANTABLE = JACOBIAN + TABLE;
const std::string vtkSlicerRegistrationQALogic::INVCONSISTTABLE = INVERSECONSISTENCY + TABLE;
const std::string vtkSlicerRegistrationQALogic::FIDUCIAL = "Fiducial";
const std::string vtkSlicerRegistrationQALogic::FIDUCIALTABLE = FIDUCIAL + TABLE;
const std::string vtkSlicerRegistrationQALogic::REFERENCENUMBER = "ReferenceNumber";
const std::string vtkSlicerRegistrationQALogic::TRIPVF = "TRiP_vf";
const std::string vtkSlicerRegistrationQALogic::BACKWARD = "BackwardReg";
const std::string vtkSlicerRegistrationQALogic::RegistrationQANODEID = "RegQualityNodeID";
//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerRegistrationQALogic);

//----------------------------------------------------------------------------
vtkSlicerRegistrationQALogic::vtkSlicerRegistrationQALogic() {
	this->RegistrationQANode = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerRegistrationQALogic::~vtkSlicerRegistrationQALogic() {
	vtkSetAndObserveMRMLNodeMacro(this->RegistrationQANode, NULL);
}
//----------------------------------------------------------------------------
void vtkSlicerRegistrationQALogic::PrintSelf(ostream& os, vtkIndent indent) {
	this->Superclass::PrintSelf(os, indent);
}
//----------------------------------------------------------------------------
void vtkSlicerRegistrationQALogic
::SetAndObserveRegistrationQANode(vtkMRMLRegistrationQANode *node) {
	vtkSetAndObserveMRMLNodeMacro(this->RegistrationQANode, node);
}

//---------------------------------------------------------------------------
void vtkSlicerRegistrationQALogic::SetMRMLSceneInternal(vtkMRMLScene * newScene) {
	vtkNew<vtkIntArray> events;
	events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
	events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
	events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
	this->SetAndObserveMRMLSceneEvents(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerRegistrationQALogic::RegisterNodes() {
	vtkMRMLScene* scene = this->GetMRMLScene();
	assert(scene != 0);

	scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLRegistrationQANode>::New());
}

//---------------------------------------------------------------------------
void vtkSlicerRegistrationQALogic::UpdateFromMRMLScene() {
	assert(this->GetMRMLScene() != 0);
	this->Modified();
}

//---------------------------------------------------------------------------
void vtkSlicerRegistrationQALogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node) {
	if (!node || !this->GetMRMLScene()) {
		return;
	}

	if( node->IsA("vtkMRMLVectorVolumeNode") ||
		node->IsA("vtkMRMLLinearTransformNode") ||
		node->IsA("vtkMRMLGridTransformNode") ||
		node->IsA("vtkMRMLBSplineTransformNode") ||
		node->IsA("vtkMRMLRegistrationQANode")) {
		this->Modified();
	}
}

//---------------------------------------------------------------------------
void vtkSlicerRegistrationQALogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* node) {
	if (!node || !this->GetMRMLScene()) {
		return;
	}

	if (node->IsA("vtkMRMLVectorVolumeNode") ||
		node->IsA("vtkMRMLLinearTransformNode") ||
		node->IsA("vtkMRMLGridTransformNode") ||
		node->IsA("vtkMRMLBSplineTransformNode") ||
		node->IsA("vtkMRMLRegistrationQANode")) {
		this->Modified();
	}
}

//---------------------------------------------------------------------------
void vtkSlicerRegistrationQALogic::OnMRMLSceneEndImport() {
	//Select parameter node if it exists
	vtkSmartPointer<vtkMRMLRegistrationQANode> paramNode = NULL;
	vtkSmartPointer<vtkMRMLNode> node = this->GetMRMLScene()->GetNthNodeByClass(
			0, "vtkMRMLRegistrationQANode");

	if (node) {
		paramNode = vtkMRMLRegistrationQANode::SafeDownCast(node);
		vtkSetAndObserveMRMLNodeMacro(this->RegistrationQANode, paramNode);
	}
}

//---------------------------------------------------------------------------
void vtkSlicerRegistrationQALogic::OnMRMLSceneEndClose() {
	this->Modified();
}
//---------------------------------------------------------------------------
void vtkSlicerRegistrationQALogic::CreateBackwardParameters(vtkMRMLRegistrationQANode* node){
   if (!node){
      vtkErrorMacro("CreateBackwardParameters: Invalid registration quality node!");
      return;
   }
   
   vtkSmartPointer<vtkMRMLRegistrationQANode> backNode = 
         vtkSmartPointer<vtkMRMLRegistrationQANode>::New();
   std::string outSS;
   std::string addName("_backward");
   outSS = node->GetName() + addName;
   std::string nameNode = this->GetMRMLScene()->GenerateUniqueName(outSS);
   backNode->SetName(nameNode.c_str());
   this->GetMRMLScene()->AddNode(backNode);
   backNode->SetAndObserveBackwardRegistrationQAParameters(node);
   node->SetAndObserveBackwardRegistrationQAParameters(backNode);
   
   backNode->Modified();
   
   //Change direction to set all necessary nodes
   if (!backNode->ChangeFromBackwardToFoward()){
      vtkErrorMacro("Can't change from forward to backward.");
      return;
   }
   backNode->BackwardRegistrationOn();
}
//---------------------------------------------------------------------------
void vtkSlicerRegistrationQALogic::SwitchRegistrationDirection(){
   if (!this->GetMRMLScene() || !this->RegistrationQANode) {
      vtkErrorMacro("SwitchRegistrationDirection: Invalid scene or parameter set node!");
      return;
   }
   
   vtkSmartPointer<vtkMRMLRegistrationQANode> pNode = this->RegistrationQANode;
   
   // Look at the node and change directions
   if ( !pNode->GetBackwardRegistration() ) {
      // Create backward parameters, if not there yet
      vtkSmartPointer<vtkMRMLRegistrationQANode> backNode = pNode->GetBackwardRegistrationQAParameters();
      if ( backNode == NULL ){
         this->CreateBackwardParameters(pNode);
      }
      if (!backNode->ChangeFromBackwardToFoward()){
         vtkErrorMacro("Can't change from forward to backward.");
         return;
      }
      backNode->BackwardRegistrationOn();
      this->SetAndObserveRegistrationQANode(backNode);
   }
   else{
      //Change only from backward registration
      // Backward Parameters must already exist, since they were the first ones
      vtkSmartPointer<vtkMRMLRegistrationQANode> forwardNode = pNode->GetBackwardRegistrationQAParameters();
      if ( forwardNode == NULL ){
         vtkErrorMacro("Forward parameter doesn't exist");
         return;
      }
      if (!forwardNode->ChangeFromBackwardToFoward()){
         vtkErrorMacro("Can't change from forward to backward.");
         return;
      }
      forwardNode->BackwardRegistrationOff();
      this->SetAndObserveRegistrationQANode(forwardNode);
   }
}
//---------------------------------------------------------------------------
void vtkSlicerRegistrationQALogic::SaveScreenshot(const char* description) {
	if (!this->GetMRMLScene() || !this->RegistrationQANode) {
	    vtkErrorMacro("SaveScreenshot: Invalid scene or parameter set node!");
	    return;
	}
	
	if (!this->RegistrationQANode->GetOutputDirectory()) {
		vtkErrorMacro("SaveScreenshot: No output Directory!");
		return;
	}
	
	int screenShotNumber = this->RegistrationQANode->GetNumberOfScreenshots();
	std::ostringstream convert;
	convert << screenShotNumber;
	std::string outSS;
	std::string Name("Screenshot_");
	outSS = Name + convert.str();
	
	std::string directory = this->RegistrationQANode->GetOutputDirectory();

	
	//Add snapshot node
	vtkMRMLAnnotationSnapshotNode* screenShotNode = NULL;
	vtkNew<vtkMRMLAnnotationSnapshotNode> screenShotNodeNew;
		
	vtkNew<vtkMRMLAnnotationSnapshotStorageNode> screenShotStorageNode;
	this->GetMRMLScene()->AddNode(screenShotStorageNode.GetPointer());
	screenShotNodeNew->SetAndObserveStorageNodeID(screenShotStorageNode->GetID());
	
	this->GetMRMLScene()->AddNode(screenShotNodeNew.GetPointer());
	screenShotNode = screenShotNodeNew.GetPointer();
	
	screenShotNode->SetName(outSS.c_str());
	std::cerr << "Description: "<< description << std::endl;
	if (description) screenShotNode->SetSnapshotDescription(description);
	screenShotNode->SetScreenShotType(4);
	
	//Get Image Data - copied from qMRMLScreenShotDialog
	qSlicerApplication* app = qSlicerApplication::application();
	qSlicerLayoutManager* layoutManager = app->layoutManager();
        
	QImage screenShot = ctk::grabVTKWidget(layoutManager->viewport());
	
	// Rescale the image which gets saved
// 	QImage rescaledScreenShot = screenShot.scaled(screenShot.size().width()
//       * d->scaleFactorSpinBox->value(), screenShot.size().height()
//       * d->scaleFactorSpinBox->value());
//	 convert the screenshot from QPixmap to vtkImageData and store it with this class
	vtkNew<vtkImageData> newImageData;
	qMRMLUtils::qImageToVtkImageData(screenShot,
                                         newImageData.GetPointer());
					 
	screenShotNode->SetScreenShot(newImageData.GetPointer());
	
	//Save screenshot	
	qSlicerCoreIOManager* coreIOManager = qSlicerCoreApplication::application()->coreIOManager();
	qSlicerIO::IOProperties fileParameters;
	char fileName[512];
	sprintf(fileName, "%s/%s.png", directory.c_str(), outSS.c_str());
	
	fileParameters["nodeID"] = screenShotNode->GetID();
	fileParameters["fileName"] = fileName;
	
	
	
	if (coreIOManager->saveNodes("AnnotationFile", fileParameters)) {
		std::cerr << "Saved Screenshot to: " << fileName << "" << std::endl;	
	} else{
		vtkErrorMacro("SaveScreenshot: Cannot save screenshot!");
	}
	
	//Increase screen shot number
	screenShotNumber += 1;
	this->RegistrationQANode->DisableModifiedEventOn();
	this->RegistrationQANode->SetNumberOfScreenshots(screenShotNumber);
	this->RegistrationQANode->DisableModifiedEventOff();
}
//---------------------------------------------------------------------------
void vtkSlicerRegistrationQALogic::CalculateregQAFrom(int number){
  // 1. AbsoluteDifference, 2. Jacobian, 3. InverseConsistency, 4. Fiducial Distance
  if (!this->GetMRMLScene() || !this->RegistrationQANode) {
	    vtkErrorMacro("CalculateregQAFrom: Invalid scene or parameter set node!");
	    return;   
  }
	
  if (number > 4 || number < 1){
    vtkErrorMacro("CalculateregQAFrom: Invalid number must be between 1 and 5!");
    return;
  }
  
  vtkMRMLRegistrationQANode* pNode = this->RegistrationQANode;
  vtkMRMLScene* scene = this->GetMRMLScene();
  
  // Color table node id:
  char colorTableNodeID[64];
  double statisticValues[4];
  // All logic need reference Volume and ROI (if exists)
  vtkMRMLScalarVolumeNode *referenceVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			scene->GetNodeByID(
				pNode->GetVolumeNodeID()));

  if (number == 1){
    vtkMRMLScalarVolumeNode *absoluteDiffVolume;
    if (! this->AbsoluteDifference(pNode,statisticValues) ){
       vtkErrorMacro("CalculateregQAFrom: Can't calculate absolute difference!");
       return;
    }
    sprintf(colorTableNodeID, "vtkMRMLColorTableNodeFileColdToHotRainbow.txt");
    absoluteDiffVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			scene->GetNodeByID(
				pNode->GetAbsoluteDiffVolumeNodeID()));
    absoluteDiffVolume->GetScalarVolumeDisplayNode()->AutoWindowLevelOff();
    int window=300;
    int level=200;
    absoluteDiffVolume->GetScalarVolumeDisplayNode()->SetThreshold(0,3e3);
    absoluteDiffVolume->GetScalarVolumeDisplayNode()->SetLevel(level);
    absoluteDiffVolume->GetScalarVolumeDisplayNode()->SetWindow(window);
    absoluteDiffVolume->GetDisplayNode()->SetAndObserveColorNodeID(colorTableNodeID);
    this->SetForegroundImage(referenceVolume,absoluteDiffVolume,0.5);
  }
  else if(number == 2){
    if (!this->Jacobian(pNode,statisticValues)){
       vtkErrorMacro("CalculateregQAFrom: Can't calculate Jacobian determinant!");
       return;
    }
    vtkMRMLScalarVolumeNode *jacobianVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
                scene->GetNodeByID(
                        pNode->GetJacobianVolumeNodeID()));;
    sprintf(colorTableNodeID, "vtkMRMLColorTableNodeFileColdToHotRainbow.txt");
    
    double window=2;
    int level=1;
    
    jacobianVolume->GetScalarVolumeDisplayNode()->AutoWindowLevelOff();   
    jacobianVolume->GetScalarVolumeDisplayNode()->SetThreshold(0,3);
    jacobianVolume->GetScalarVolumeDisplayNode()->SetLevel(level);
    jacobianVolume->GetScalarVolumeDisplayNode()->SetWindow(window);
    jacobianVolume->GetDisplayNode()->SetAndObserveColorNodeID(colorTableNodeID);
    
    this->SetForegroundImage(referenceVolume,jacobianVolume,0.5);   
  }
  else if(number == 3){
    if (!this->InverseConsist(pNode, statisticValues)){
       vtkErrorMacro("CalculateregQAFrom: Can't calculate inverse consistency!");
       return;
    }
    vtkMRMLScalarVolumeNode *inverseConsistVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
                scene->GetNodeByID(
                        pNode->GetInverseConsistVolumeNodeID()));;;
    sprintf(colorTableNodeID, "vtkMRMLColorTableNodeFileColdToHotRainbow.txt");
    
    
    double* spacing;
    double maxSpacing = -1.0;
    spacing = inverseConsistVolume->GetSpacing();
    for(int i=0;i<3;i++){
       if ( spacing[i] > maxSpacing ) maxSpacing = spacing[i];
    }
    
    double window=maxSpacing/2;
    int level=maxSpacing;
    inverseConsistVolume->GetDisplayNode()->SetAndObserveColorNodeID(colorTableNodeID);
    inverseConsistVolume->GetScalarVolumeDisplayNode()->AutoWindowLevelOff();
      
    inverseConsistVolume->GetScalarVolumeDisplayNode()->SetThreshold(0.1,10);
    inverseConsistVolume->GetScalarVolumeDisplayNode()->SetLevel(level);
    inverseConsistVolume->GetScalarVolumeDisplayNode()->SetWindow(window);
    this->SetForegroundImage(referenceVolume,inverseConsistVolume,0.5);
  }
  else if( number == 4  ){
     vtkMRMLMarkupsFiducialNode *referenceFiducals = vtkMRMLMarkupsFiducialNode::SafeDownCast(
                     scene->GetNodeByID(
                             pNode->GetFiducialNodeID()));
     vtkMRMLMarkupsFiducialNode *movingFiducials = vtkMRMLMarkupsFiducialNode::SafeDownCast(
                     scene->GetNodeByID(
                             pNode->GetBackwardRegistrationQAParameters()->GetFiducialNodeID()));
     vtkMRMLTransformNode *transform;
     if (number == 4){
             transform = vtkMRMLTransformNode::SafeDownCast(scene->GetNodeByID(
                             pNode->GetTransformNodeID()));
             //Check for vector field, if there's no transform
             if ( transform) {
                if (! this->CalculateFiducialsDistance(referenceFiducals, movingFiducials, transform, statisticValues) ){
                     vtkErrorMacro("CalculateregQAFrom: Cannot calculate Fiducal distance!");
                     return;
                }
             }
             else{
                vtkSmartPointer<vtkMRMLVectorVolumeNode> vectorNode;
                vectorNode = vtkMRMLVectorVolumeNode::SafeDownCast(scene->GetNodeByID(
                   pNode->GetVectorVolumeNodeID()));
                if (! this->CalculateFiducialsDistance(referenceFiducals, movingFiducials, vectorNode, statisticValues) ){
                   vtkErrorMacro("CalculateregQAFrom: Cannot calculate Fiducal distance!");
                   return;
                }
                this->UpdateTableWithFiducialValues(referenceFiducals, statisticValues);
             }
     }
     return;
  }
}
//---------------------------------------------------------------------------
void vtkSlicerRegistrationQALogic::CalculateContourStatistic() {

   if (!this->GetMRMLScene() ) {
      vtkErrorMacro("CalculateContourStatistic: Invalid scene or parameter set node!");
      vtkErrorMacro("Internal Error, see command line!");
   }
   vtkSmartPointer<vtkMRMLTransformNode> transform;
   vtkSmartPointer<vtkMRMLSegmentationNode> fixedSegmentationNode;
   vtkSmartPointer<vtkMRMLSegmentationNode> movingSegmentationNode;
   const char* fixedSegmentID;
   const char* movingSegmentID;
   double statisticValues[4];
   
   transform = vtkMRMLTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(
                       this->RegistrationQANode->GetTransformNodeID()));
   // Check if we have vector fields that can be changed in transform
   if ( transform == NULL ) {
      vtkSmartPointer<vtkMRMLVectorVolumeNode> vectorNode = vtkMRMLVectorVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(
                       this->RegistrationQANode->GetVectorVolumeNodeID()));
      if (vectorNode == NULL ) {
         vtkErrorMacro("CalculateContourStatistic: No transform or vector!");
         return;
      }
      transform = this->CreateTransformFromVector(vectorNode);
   }
   
   fixedSegmentationNode = this->RegistrationQANode->GetSegmentationNode();
   fixedSegmentID = this->RegistrationQANode->GetSegmentID();
  
   movingSegmentationNode = this->RegistrationQANode->GetBackwardRegistrationQAParameters()->GetSegmentationNode();
   movingSegmentID = this->RegistrationQANode->GetBackwardRegistrationQAParameters()->GetSegmentID();

   
   if (!fixedSegmentationNode || !fixedSegmentID || 
      !movingSegmentationNode || !movingSegmentID || !transform) {
      vtkErrorMacro("CalculateContourStatistic: Input parameters missing!");
      return;
   }

   // Calculate default statistic
   
   vtkSmartPointer<vtkMRMLSegmentComparisonNode> segmentComparisonNode = vtkMRMLSegmentComparisonNode::New();
   this->GetMRMLScene()->AddNode(segmentComparisonNode);

   segmentComparisonNode->SetAndObserveReferenceSegmentationNode(fixedSegmentationNode);
   segmentComparisonNode->SetReferenceSegmentID(fixedSegmentID);
   segmentComparisonNode->SetAndObserveCompareSegmentationNode(movingSegmentationNode);
   segmentComparisonNode->SetCompareSegmentID(movingSegmentID);

   vtkSmartPointer<vtkMRMLSegmentationNode> testSegmentationNode;
   testSegmentationNode = segmentComparisonNode->GetReferenceSegmentationNode();
   if ( ! testSegmentationNode ) {
      vtkErrorMacro("CalculateContourStatistic: No segmentation node!");
      return;
   }

   vtkNew<vtkSlicerSegmentComparisonModuleLogic> segmentComparisonlogic;
   segmentComparisonlogic->SetMRMLScene(this->GetMRMLScene());
   std::string errorMessage = segmentComparisonlogic->ComputeDiceStatistics(segmentComparisonNode);
   
   if ( ! segmentComparisonNode->GetDiceResultsValid() ){
      vtkErrorMacro("CalculateContourStatistic: No valid results.");
      return;
   }
   errorMessage = segmentComparisonlogic->ComputeHausdorffDistances(segmentComparisonNode);
   if ( ! segmentComparisonNode->GetHausdorffResultsValid() ){
      vtkErrorMacro("CalculateContourStatistic: No valid results.");
      return;
   }
   
   statisticValues[0] = segmentComparisonNode->GetAverageHausdorffDistanceForBoundaryMm();
   statisticValues[2] = segmentComparisonNode->GetDiceCoefficient();

   // Calculate warped statistic
   vtkSmartPointer<vtkMRMLSegmentationNode> warpedSegmentationNode = vtkSmartPointer<vtkMRMLSegmentationNode>::New();
   warpedSegmentationNode->Copy(movingSegmentationNode);
   
   //Rename copied fiducials
   std::string outSS;
   std::string Name("_warped");

   outSS = (movingSegmentationNode->GetName() + Name);
   outSS = this->GetMRMLScene()->GenerateUniqueName(outSS);
   warpedSegmentationNode->SetName( outSS.c_str() );
   
   vtkNew<vtkMRMLSegmentationDisplayNode> wSDisplayNode;
   vtkNew<vtkMRMLSegmentationStorageNode> wSStorageNode;
   this->GetMRMLScene()->AddNode(wSDisplayNode.GetPointer());
   this->GetMRMLScene()->AddNode(wSStorageNode.GetPointer());
   warpedSegmentationNode->SetAndObserveDisplayNodeID(wSDisplayNode->GetID());
   warpedSegmentationNode->SetAndObserveStorageNodeID(wSStorageNode->GetID());
   
   this->GetMRMLScene()->AddNode( warpedSegmentationNode );
   
   warpedSegmentationNode->SetAndObserveTransformNodeID(transform->GetID());
   if ( ! warpedSegmentationNode->HardenTransform() ){
      vtkErrorMacro("CalculateContourStatistic: Can't harden the transform!");
      return;
   }
   
   segmentComparisonNode->SetAndObserveCompareSegmentationNode(warpedSegmentationNode);
   errorMessage = segmentComparisonlogic->ComputeDiceStatistics(segmentComparisonNode);
   
   if ( ! segmentComparisonNode->GetDiceResultsValid() ){
      vtkErrorMacro("CalculateContourStatistic: No valid results.");
      return;
   }
   errorMessage = segmentComparisonlogic->ComputeHausdorffDistances(segmentComparisonNode);
   if ( ! segmentComparisonNode->GetHausdorffResultsValid() ){
      vtkErrorMacro("CalculateContourStatistic: No valid results.");
      return;
   }
   
   statisticValues[1] = segmentComparisonNode->GetAverageHausdorffDistanceForBoundaryMm();
   statisticValues[3] = segmentComparisonNode->GetDiceCoefficient();
   
   this->GetMRMLScene()->RemoveNode(segmentComparisonNode);
   this->GetMRMLScene()->RemoveNode(warpedSegmentationNode);
   if ( this->RegistrationQANode->GetBackwardRegistration() ) {
      this->UpdateTableWithStatisticalValues(statisticValues,19);
   }
   else{
      this->UpdateTableWithStatisticalValues(statisticValues,18);
   }
}
//---------------------------------------------------------------------------
bool vtkSlicerRegistrationQALogic::CalculateFiducialsDistance(vtkMRMLMarkupsFiducialNode* referenceFiducals, vtkMRMLMarkupsFiducialNode* movingFiducials,vtkMRMLVectorVolumeNode *vectorNode, double *statisticValues) {
   if (!this->GetMRMLScene() ) {
            vtkErrorMacro("CalculateFiducialsDistance: Invalid scene or parameter set node!");
            return false;
   }
   vtkSmartPointer<vtkMRMLTransformNode> transform;
   transform = this->CreateTransformFromVector(vectorNode);
   
   if ( transform && this->RegistrationQANode ){
      this->RegistrationQANode->SetAndObserveTransformNodeID(transform->GetID());
   }
   
   if ( this->CalculateFiducialsDistance(referenceFiducals,movingFiducials,transform,statisticValues)){
      return true;
   }
   else{
      return false;
   }
}
//---------------------------------------------------------------------------
bool vtkSlicerRegistrationQALogic::CalculateFiducialsDistance(vtkMRMLMarkupsFiducialNode* referenceFiducals, vtkMRMLMarkupsFiducialNode* movingFiducials,vtkMRMLTransformNode *transform, double *statisticValues) {

	if (!this->GetMRMLScene() ) {
	    vtkErrorMacro("CalculateFiducialsDistance: Invalid scene or parameter set node!");
	    return false;
	}
	if (!referenceFiducals || !movingFiducials || !transform || !statisticValues ) {
		vtkErrorMacro("CalculateFiducialsDistance: Invalid input parameters!");
		return false;
	}
	
	
	int fiducialNumber = referenceFiducals->GetNumberOfFiducials();
	double sumDistance = 0;
	double maxDistance = 0;
	double minDistance = 0;
	double distanceDiff;
        double distanceBefore = 0;
        double distanceAfter = 0;
	
	if (fiducialNumber < 1 ||  movingFiducials->GetNumberOfFiducials() < 1 ){
		vtkErrorMacro("CalculateFiducialsDistance: Need more then 1 fiducial!");
		return false;		
	}
	
	if (fiducialNumber != movingFiducials->GetNumberOfFiducials() ){
		vtkErrorMacro("CalculateFiducialsDistance: Wrong fiducial numbers!");
		return false;		
	}
	
	// Copy Moving fiducials so we can apply transformation
	vtkSmartPointer<vtkMRMLMarkupsFiducialNode> warpedFiducials = vtkSmartPointer<vtkMRMLMarkupsFiducialNode>::New();
	vtkMRMLScene *scene = this->GetMRMLScene();	  
	vtkNew<vtkMRMLMarkupsDisplayNode> wFDisplayNode;
	vtkNew<vtkMRMLMarkupsFiducialStorageNode> wFStorageNode;
	scene->AddNode(wFDisplayNode.GetPointer());
	scene->AddNode(wFStorageNode.GetPointer());
	warpedFiducials->SetAndObserveDisplayNodeID(wFDisplayNode->GetID());
	warpedFiducials->SetAndObserveStorageNodeID(wFStorageNode->GetID());
	scene->AddNode(warpedFiducials);
	
	//Apply Transform to movingFiducials
	warpedFiducials->Copy(movingFiducials);
	warpedFiducials->SetAndObserveTransformNodeID( transform->GetID() );
        
        //Harden Transform
        warpedFiducials->HardenTransform();

	//Rename copied fiducials
	std::string outSS;
	std::string Name("_warped");

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
                distanceBefore = 0;
                distanceAfter = 0;
		distanceDiff = 0;
		for (int j=0; j<3; j++) {
			//Calculate absolute difference in mm
			distanceBefore += (referencePosition[j] - movingPosition[j])*(referencePosition[j] - movingPosition[j]);
                        distanceAfter += (referencePosition[j] - warpedPosition[j])*(referencePosition[j] - warpedPosition[j]);
                }	
		
		if ( i == 0  ){
				maxDistance = distanceDiff;
				minDistance = distanceDiff;
		}
		else{
			if ( distanceDiff > maxDistance) maxDistance = distanceDiff;
			if ( distanceDiff < minDistance) minDistance = distanceDiff;
		}

		sumDistance += distanceDiff;
// 		std::cerr << "Fiducial: " << i <<"/" << fiducialNumber << "," << " with distance: " << distanceDiff <<   std::endl;
                statisticValues[2*i] = sqrt(distanceBefore);
                statisticValues[2*i+1] = sqrt(distanceAfter);
                std::cerr << "Fiducial: " << i <<"/" << fiducialNumber << "" << " before: " << statisticValues[2*i] << " after: " << statisticValues[2*i+1] <<  std::endl;
       
	}
// 	statisticValues[0] = sumDistance / (  fiducialNumber);
// 	statisticValues[1] = 0;
// 	statisticValues[2] = maxDistance;
// 	statisticValues[3] = minDistance;
	return true;
}
//--- Image Checks -----------------------------------------------------------
void vtkSlicerRegistrationQALogic::FalseColor(bool invertColor, bool movingImage, bool matchLevels) {
	if (!this->GetMRMLScene() || !this->RegistrationQANode) {
		vtkErrorMacro("FalseColor: Invalid scene or parameter set node!");
		return;
	}
	
	vtkMRMLRegistrationQANode *pNode = this->RegistrationQANode;

	vtkMRMLScalarVolumeNode *referenceVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				pNode->GetVolumeNodeID()));

	vtkMRMLScalarVolumeNode *foregroundVolume;
        vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode> referenceVolumeDisplayNode; 
        vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode> foregroundVolumeDisplayNode;
        
        referenceVolumeDisplayNode = referenceVolume->GetScalarVolumeDisplayNode();
        
        // Create display node, if it doesn't exist
        if ( !referenceVolumeDisplayNode ){
           cout << "Here0" << endl;
           referenceVolumeDisplayNode = vtkMRMLScalarVolumeDisplayNode::New();
           referenceVolumeDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey");
           this->GetMRMLScene()->AddNode(referenceVolumeDisplayNode);
           referenceVolume->SetAndObserveDisplayNodeID(referenceVolumeDisplayNode->GetID());
        }

        if ( movingImage ){
           foregroundVolume= vtkMRMLScalarVolumeNode::SafeDownCast(
                        this->GetMRMLScene()->GetNodeByID(
                                pNode->GetBackwardRegistrationQAParameters()->GetVolumeNodeID()));
        }
        else{
           foregroundVolume= vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				pNode->GetWarpedVolumeNodeID()));
        }

	if (!referenceVolume || !foregroundVolume) {
		vtkErrorMacro("Reference or warped volume not set!");
                return;
	}
	
	foregroundVolumeDisplayNode = foregroundVolume->GetScalarVolumeDisplayNode();
	// Create display node, if it doesn't exist
        if ( !foregroundVolumeDisplayNode ){
           cout << "Here1" << endl;
           foregroundVolumeDisplayNode = vtkMRMLScalarVolumeDisplayNode::New();
           foregroundVolumeDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey");
           this->GetMRMLScene()->AddNode(foregroundVolumeDisplayNode);
           foregroundVolume->SetAndObserveDisplayNodeID(foregroundVolumeDisplayNode->GetID());
        }
        
        
        // Save color ID if they haven't been yet
        
        if (! pNode->GetVolumeColorNodeID() ){
           pNode->SetVolumeColorNodeID(referenceVolumeDisplayNode->GetColorNodeID());
        }
        
        if ( movingImage && !pNode->GetBackwardRegistrationQAParameters()->GetVolumeColorNodeID() ){
              pNode->GetBackwardRegistrationQAParameters()->SetVolumeColorNodeID(
                 foregroundVolumeDisplayNode->GetColorNodeID());
        }
        
        if ( !movingImage && !pNode->GetWarpedColorNodeID() ){
           pNode->SetWarpedColorNodeID(foregroundVolumeDisplayNode->GetColorNodeID());
        }
        
	
	//TODO: Volumes go back to gray value - perhaps we should rembemer previous color settings?
        if (invertColor) {
           referenceVolumeDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeWarmTint1");
           foregroundVolumeDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeCoolTint1");
        }
        else{
           referenceVolumeDisplayNode->SetAndObserveColorNodeID(pNode->GetVolumeColorNodeID());
           if ( movingImage) {
              foregroundVolumeDisplayNode->SetAndObserveColorNodeID(
                 pNode->GetBackwardRegistrationQAParameters()->GetVolumeColorNodeID());
           }
           else{
              foregroundVolumeDisplayNode->SetAndObserveColorNodeID(pNode->GetWarpedColorNodeID());
           }
        }

	// Set window and level the same for warped and reference volume.
        if ( matchLevels ){
           foregroundVolumeDisplayNode->AutoWindowLevelOff();
           foregroundVolumeDisplayNode->SetWindow( referenceVolumeDisplayNode->GetWindow() );
           foregroundVolumeDisplayNode->SetLevel( referenceVolumeDisplayNode->GetLevel() );
        }

	this->SetForegroundImage(referenceVolume,foregroundVolume,0.5);

	return;
}

//----------------------------------------------------------------------------
void vtkSlicerRegistrationQALogic::Flicker(int opacity) {
	if (!this->GetMRMLScene() || !this->RegistrationQANode) {
		vtkErrorMacro("Flicker: Invalid scene or parameter set node!");
		return;
	}

	vtkMRMLScalarVolumeNode *referenceVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQANode->GetVolumeNodeID()));

	vtkMRMLScalarVolumeNode *warpedVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQANode->GetWarpedVolumeNodeID()));

	if (!referenceVolume || !warpedVolume) {
		vtkErrorMacro("Flicker: Invalid reference or warped volume!");
		return;
	}

	this->SetForegroundImage(referenceVolume,warpedVolume,opacity);

	return;
}

bool vtkSlicerRegistrationQALogic
::getSliceCompositeNodeRASBounds(vtkMRMLSliceCompositeNode *scn, double* minmax) {
   if ( !scn ){
      vtkErrorMacro("getSliceCompositeNodeRASBounds: No scn node");
      return false;
   }

   vtkMRMLScalarVolumeNode* background = vtkMRMLScalarVolumeNode::SafeDownCast(
                        this->GetMRMLScene()->GetNodeByID(scn->GetBackgroundVolumeID()));
	
     double rasBounds[6] = {INFINITY,-INFINITY,INFINITY,-INFINITY,INFINITY,-INFINITY};

     if(background) background->GetRASBounds(rasBounds);
     else return false;
        
	for(int i=0;i<3; i++) {
		minmax[2*i] = rasBounds[2*i];
		minmax[2*i+1] = rasBounds[2*i+1];
		if(minmax[2*i]>minmax[2*i+1]) {
			cout << "No RAS bounds" << rasBounds[2*i] << "/" << rasBounds[2*i+1] << endl;
			minmax[2*i] = minmax[2*i+1] = 0;
		}
	}
   return true;
}

//----------------------------------------------------------------------------
/**
 * Movie through slices.
 * TODO:	- Calculate slice spacing
 * 			- Changed slice directions
 * 			- Oblique slices
 */
void vtkSlicerRegistrationQALogic::Movie() {
   vtkMRMLRegistrationQANode *pNode = this->RegistrationQANode;
   
   
   if (!this->GetMRMLScene() || !this->RegistrationQANode) {
      vtkErrorMacro("Movie: Invalid scene or parameter set node!");
      pNode->MovieRunOff();
      return;
   }
   
   if ( ! pNode->GetMovieRun() ) {
      return;
   }
              
   vtkMRMLSliceNode* sliceNodeRed = vtkMRMLSliceNode::SafeDownCast(
               this->GetMRMLScene()->GetNodeByID("vtkMRMLSliceNodeRed"));
   vtkMRMLSliceNode* sliceNodeYellow = vtkMRMLSliceNode::SafeDownCast(
               this->GetMRMLScene()->GetNodeByID("vtkMRMLSliceNodeYellow"));
   vtkMRMLSliceNode* sliceNodeGreen = vtkMRMLSliceNode::SafeDownCast(
               this->GetMRMLScene()->GetNodeByID("vtkMRMLSliceNodeGreen"));
                        
   if ( !sliceNodeRed || !sliceNodeYellow || !sliceNodeGreen ){
      vtkErrorMacro("Movie: Can't get slice nodes!");
      pNode->MovieRunOff();
      return;
   }

   double rasBounds[6];
   double redMin, redMax, yellowMin, yellowMax, greenMin, greenMax;
   double redOffset, yellowOffset, greenOffset;
   double* spacing;

        
   bool foundSlice = true;

   vtkMRMLSliceCompositeNode *scn = vtkMRMLSliceCompositeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID("vtkMRMLSliceCompositeNodeRed"));
   if ( getSliceCompositeNodeRASBounds(scn,rasBounds)) {
      redMin = rasBounds[4];
      redMax = rasBounds[5];
   }
   scn = vtkMRMLSliceCompositeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID("vtkMRMLSliceCompositeNodeYellow"));
   if ( getSliceCompositeNodeRASBounds(scn,rasBounds)){
      yellowMin = rasBounds[0];
      yellowMax = rasBounds[1];
   }
   scn = vtkMRMLSliceCompositeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID("vtkMRMLSliceCompositeNodeGreen"));
   if ( getSliceCompositeNodeRASBounds(scn,rasBounds)) {
      greenMin = rasBounds[2];
      greenMax = rasBounds[3];
   }


   vtkMRMLScalarVolumeNode* background = vtkMRMLScalarVolumeNode::SafeDownCast(
         this->GetMRMLScene()->GetNodeByID(scn->GetBackgroundVolumeID()));
          
   if ( !background ){
      vtkErrorMacro("Movie: No volume in background.");
      pNode->MovieRunOff();
      return;
   }
   spacing = background->GetSpacing();

   while(pNode->GetMovieRun() && foundSlice) {
            //cout << " runRed=" << (runState&1?"true":"false")
            //		<< " runYellow=" << (runState&2?"true":"false")
            //		<< " runGreen=" << (runState&4?"true":"false")
            //		<< " MovieRun=" << runState << endl;

      int redState = pNode->GetMovieBoxRedState();
      int yellowState = pNode->GetMovieBoxYellowState();
      int greenState = pNode->GetMovieBoxGreenState();
      
      foundSlice = false;
      if(redState) {
         if ( redMax == 0 && redMin == 0 ){
            vtkErrorMacro("Wrong RAS bounds.");
            pNode->MovieRunOff();
            break;
         }
         redOffset = sliceNodeRed->GetSliceOffset() + spacing[2];
         if (redOffset > redMax) {
            redOffset = redMin;
         }
         sliceNodeRed->SetSliceOffset( redOffset );
         foundSlice = true;
      }
      if(yellowState) {
              if ( yellowMax == 0 && yellowMin == 0 ){
            vtkErrorMacro("Wrong RAS bounds.");
            pNode->MovieRunOff();
            break;
         }
         yellowOffset = sliceNodeYellow->GetSliceOffset() + spacing[0];
         if (yellowOffset > yellowMax) {
            yellowOffset = yellowMin;
         }
         sliceNodeYellow->SetSliceOffset( yellowOffset );
         foundSlice = true;
      }
      if(greenState) {
         if ( greenMax == 0 && greenMin == 0 ){
            vtkErrorMacro("Wrong RAS bounds.");
            pNode->MovieRunOff();
            break;
         }
         greenOffset = sliceNodeGreen->GetSliceOffset() + spacing[1];
         if (greenOffset > greenMax) {
            greenOffset = greenMin;
         }
         sliceNodeGreen->SetSliceOffset( greenOffset );
         foundSlice = true;
      }
      qSlicerApplication::application()->processEvents();
   }
}
//----------------------------------------------------------------------------
void vtkSlicerRegistrationQALogic::Checkerboard() {
	//   Calling checkerboardfilter cli. Logic has been copied and modified from CropVolumeLogic onApply.
	if (!this->GetMRMLScene() || !this->RegistrationQANode) {
		vtkErrorMacro("Invalid scene or parameter set node!");
		vtkErrorMacro("Internal Error, see command line!");
	}
	vtkMRMLScalarVolumeNode *referenceVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQANode->GetVolumeNodeID()));
	vtkMRMLScalarVolumeNode *warpedVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQANode->GetWarpedVolumeNodeID()));


	if (!referenceVolume || !warpedVolume) {
		vtkErrorMacro("Reference or warped volume not set!");
                return;
	}
	
	//Go to default display, so that we have normal colors 
	this->SetDefaultDisplay();

	if (!this->RegistrationQANode->GetCheckerboardVolumeNodeID()) {
		int PatternValue = this->RegistrationQANode->GetCheckerboardPattern();
		std::string outSS;
		std::string Name("-CheckerboardPattern_");
		std::ostringstream strPattern;
		strPattern << PatternValue;
		outSS = referenceVolume->GetName() + Name + strPattern.str();
		outSS = this->GetMRMLScene()->GenerateUniqueName(outSS);
		
		vtkNew<vtkSlicerVolumesLogic> VolumesLogic;
                
                vtkMRMLScalarVolumeNode *outputVolume = VolumesLogic->CloneVolume(
			this->GetMRMLScene(), referenceVolume, outSS.c_str());
		if (!outputVolume) {
			vtkErrorMacro("Could not create Checkerboard volume!");
			return;
		}

		qSlicerCLIModule* checkerboardfilterCLI = dynamic_cast<qSlicerCLIModule*>(
			qSlicerCoreApplication::application()->moduleManager()->module("CheckerBoardFilter"));
		if (!checkerboardfilterCLI) {
			vtkErrorMacro("No Checkerboard Filter module!");
			return;
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
		this->RegistrationQANode->SetCheckerboardVolumeNodeID(outputVolume->GetID());
		return;
	}

	vtkMRMLScalarVolumeNode *checkerboardVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQANode->GetCheckerboardVolumeNodeID()));
	this->SetForegroundImage(checkerboardVolume,referenceVolume,0);
}
//---------------------------------------------------------------------------
bool vtkSlicerRegistrationQALogic::AbsoluteDifference(vtkMRMLRegistrationQANode* regQAnode, double statisticValues[4]) {
   if ( !regQAnode || !this->GetMRMLScene() ){
      vtkErrorMacro("AbsoluteDifference: No scene or registration quality node");
      return false;
   }
   
   /* If it was already calculate, skip */
   if (regQAnode->GetAbsoluteDiffVolumeNodeID()){
      return true;
   }
   
   vtkMRMLScene* scene = this->GetMRMLScene();
   
   vtkMRMLScalarVolumeNode* refImage = vtkMRMLScalarVolumeNode::SafeDownCast(
      scene->GetNodeByID(regQAnode->GetVolumeNodeID()));
   vtkMRMLScalarVolumeNode* warpedImage = vtkMRMLScalarVolumeNode::SafeDownCast(
      scene->GetNodeByID(regQAnode->GetWarpedVolumeNodeID()));
   vtkMRMLAnnotationROINode* inputROI = vtkMRMLAnnotationROINode::SafeDownCast(
      scene->GetNodeByID(regQAnode->GetROINodeID()));
   
   vtkMRMLScalarVolumeNode* absDiffVolume = this->CalculateAbsoluteDifference(refImage,warpedImage,inputROI);
   if ( !absDiffVolume ) {
             vtkErrorMacro("AbsoluteDifference: Can't calculate absolute difference!");
             return false;
   }

   regQAnode->SetAndObserveAbsoluteDiffVolumeNodeID(absDiffVolume->GetID());      
   this->CalculateStatistics(absDiffVolume,statisticValues);
   if ( regQAnode->GetBackwardRegistration() ){
      this->UpdateTableWithStatisticalValues(statisticValues, 13);
   }
   else{
      this->UpdateTableWithStatisticalValues(statisticValues, 12);
   }
   
   return true;
}
//---------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkSlicerRegistrationQALogic::CalculateAbsoluteDifference(vtkMRMLScalarVolumeNode* referenceVolume, vtkMRMLScalarVolumeNode* warpedVolume,vtkMRMLAnnotationROINode *inputROI  ) {

        if (!this->GetMRMLScene() ) {
            vtkErrorMacro("CalculateAbsoluteDifference: Invalid scene or parameter set node!");
            return NULL;
        }
        if (!referenceVolume || !warpedVolume ) {
                vtkErrorMacro("CalculateAbsoluteDifference: Invalid reference or warped volume!");
                return NULL;
        }


        vtkSmartPointer<vtkMRMLScalarVolumeNode> outputVolume = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();
        vtkSmartPointer<vtkMRMLScalarVolumeNode> svnode;
        svnode = vtkMRMLScalarVolumeNode::SafeDownCast(referenceVolume);
        std::string outSS;
        std::string Name("-absoluteDifference");

        outSS = (referenceVolume->GetName() + Name);
        outSS = this->GetMRMLScene()->GenerateUniqueName(outSS);
        
        vtkNew<vtkSlicerVolumesLogic> VolumesLogic;
        if(svnode){
              outputVolume = VolumesLogic->CloneVolume(this->GetMRMLScene(), referenceVolume, outSS.c_str());
          
        }
        else{
              std::cerr << "Reference volume not scalar volume!" << std::endl;
              return NULL;
          
        }

        if ( !outputVolume ) {
                  vtkErrorMacro("CalculateAbsoluteDifference: No output volume set!");
                  return NULL;
        }

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
                vtkErrorMacro("Error in CLI module, see command line!");
        }

        this->GetMRMLScene()->RemoveNode(cmdNode);

        outputVolume->SetAndObserveTransformNodeID(NULL);
        
        return outputVolume;
}
//----------------------------------------------------------------------------
bool vtkSlicerRegistrationQALogic::Jacobian(vtkMRMLRegistrationQANode* regQAnode, double statisticValues[4]){
   if ( !regQAnode || !this->GetMRMLScene() ){
      vtkErrorMacro("Jacobian: No scene or registration quality node");
      return false;
   }
   
   /* If it was already calculate, skip */
   if (regQAnode->GetJacobianVolumeNodeID()){
      return true;
   }
   
   vtkMRMLScene* scene = this->GetMRMLScene();
   
   vtkMRMLVectorVolumeNode* vectorVolume = vtkMRMLVectorVolumeNode::SafeDownCast(
      scene->GetNodeByID(regQAnode->GetVectorVolumeNodeID()));
   vtkMRMLAnnotationROINode* inputROI = vtkMRMLAnnotationROINode::SafeDownCast(
      scene->GetNodeByID(regQAnode->GetROINodeID()));
   
   vtkMRMLScalarVolumeNode* jacobianVolume = this->CalculateJacobian(vectorVolume,inputROI);
   if ( !jacobianVolume ) {
             vtkErrorMacro("Jacobian: Can't calculate Jacobian!");
             return false;
   }

   regQAnode->SetAndObserveJacobianVolumeNodeID(jacobianVolume->GetID());      
   this->CalculateStatistics(jacobianVolume,statisticValues);
   if ( regQAnode->GetBackwardRegistration() ){
      this->UpdateTableWithStatisticalValues(statisticValues, 15);
   }
   else{
      this->UpdateTableWithStatisticalValues(statisticValues, 14);
   }
   
   return true;
}
//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkSlicerRegistrationQALogic::CalculateJacobian(vtkMRMLVectorVolumeNode *vectorVolume,
                                                        vtkMRMLAnnotationROINode *inputROI) {

     if (!this->GetMRMLScene()) {
             vtkErrorMacro("Can't get scene");
             return NULL;
     }


     if (!vectorVolume ) {
         vtkErrorMacro("No vector field set!");
         return NULL;
     }
     vtkSmartPointer<vtkMRMLScalarVolumeNode> outputVolume = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();
     // Create new scalar volume
     vtkNew<vtkMRMLScalarVolumeDisplayNode> sDisplayNode;
     sDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey");
     this->GetMRMLScene()->AddNode(sDisplayNode.GetPointer());
     outputVolume->SetAndObserveDisplayNodeID(sDisplayNode->GetID());
     outputVolume->SetAndObserveStorageNodeID(NULL);
     this->GetMRMLScene()->AddNode(outputVolume);
     
     if(!outputVolume){
       vtkErrorMacro("Can't create output volume!");
       return NULL;
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
             return NULL;
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
       vtkErrorMacro("Can't create Jacobian determinant");
       return NULL;
     }
	
	
}
//----------------------------------------------------------------------------
bool vtkSlicerRegistrationQALogic::InverseConsist(vtkMRMLRegistrationQANode* regQAnode, double statisticValues[4]) {
   if ( !regQAnode || !this->GetMRMLScene() ){
      vtkErrorMacro("InverseConsist: No scene or registration quality node");
      return false;
   }
   
   /* If it was already calculate, skip */
   if (regQAnode->GetInverseConsistVolumeNodeID()){
      return true;
   }
   
   vtkMRMLScene* scene = this->GetMRMLScene();
   
   vtkMRMLVectorVolumeNode *vectorVolume1 = vtkMRMLVectorVolumeNode::SafeDownCast(
                        scene->GetNodeByID(
                                regQAnode->GetVectorVolumeNodeID()));
   vtkMRMLVectorVolumeNode *vectorVolume2 = vtkMRMLVectorVolumeNode::SafeDownCast(
                        scene->GetNodeByID(
                        regQAnode->GetBackwardRegistrationQAParameters()->GetVectorVolumeNodeID()));
   vtkMRMLAnnotationROINode* inputROI = vtkMRMLAnnotationROINode::SafeDownCast(
      scene->GetNodeByID(regQAnode->GetROINodeID()));
   
   vtkMRMLScalarVolumeNode* inverseConsistVolume = this->CalculateInverseConsist(vectorVolume1,vectorVolume2,inputROI);
   if(!inverseConsistVolume){
          vtkErrorMacro("InverseConsist: Can't calculate inverse consistency!");
          return false;
      }
   regQAnode->SetAndObserveInverseConsistVolumeNodeID(inverseConsistVolume->GetID());
      
   this->CalculateStatistics(inverseConsistVolume,statisticValues);
   this->UpdateTableWithStatisticalValues(statisticValues, 16);
   return true;
}
//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkSlicerRegistrationQALogic::CalculateInverseConsist(vtkMRMLVectorVolumeNode *vectorVolume1,vtkMRMLVectorVolumeNode *vectorVolume2,vtkMRMLAnnotationROINode *inputROI) {

	if (!this->GetMRMLScene() ) {
		vtkErrorMacro("Inverse Consistency: Invalid scene!");
                return NULL;
	}

//
	if (!vectorVolume1 || !vectorVolume2 ) {
	    vtkErrorMacro("Volumes not set!");
            return NULL;
	}
	
	// Create new scalar volume
	vtkSmartPointer<vtkMRMLScalarVolumeNode> outputVolume = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();
	vtkNew<vtkMRMLScalarVolumeDisplayNode> sDisplayNode;
	sDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey");
	this->GetMRMLScene()->AddNode(sDisplayNode.GetPointer());
	outputVolume->SetAndObserveDisplayNodeID(sDisplayNode->GetID());
	outputVolume->SetAndObserveStorageNodeID(NULL);
	this->GetMRMLScene()->AddNode(outputVolume);

	
	if(!outputVolume){
	  vtkErrorMacro("Can't create output volume!");
          return NULL;
	}
	  		
	std::string outSS;
	std::string Name("-invConsist");
	outSS = vectorVolume1->GetName() + Name;
	outSS = this->GetMRMLScene()->GenerateUniqueName(outSS);
	outputVolume->SetName(outSS.c_str());

	qSlicerCLIModule* inverseConstCLI = dynamic_cast<qSlicerCLIModule*>(
			qSlicerCoreApplication::application()->moduleManager()->module("InverseConsistency"));
	QString cliModuleName("InverseConsistency");

	vtkSmartPointer<vtkMRMLCommandLineModuleNode> cmdNode =
			inverseConstCLI->cliModuleLogic()->CreateNodeInScene();

	// Set node parameters
	cmdNode->SetParameterAsString("inputVolume1", vectorVolume1->GetID());
	cmdNode->SetParameterAsString("inputVolume2", vectorVolume2->GetID());
	cmdNode->SetParameterAsString("outputVolume", outputVolume->GetID());
        cmdNode->SetParameterAsBool("normalize", true);
	if (inputROI)
	{
	    cmdNode->SetParameterAsString("fixedImageROI", inputROI->GetID());
	}
	else
	{
	    cmdNode->SetParameterAsString("fixedImageROI", "");
	}

	// Execute synchronously so that we can check the content of the file after the module execution
	inverseConstCLI->cliModuleLogic()->ApplyAndWait(cmdNode);

	this->GetMRMLScene()->RemoveNode(cmdNode);

	if (outputVolume){
	  outputVolume->SetAndObserveTransformNodeID(NULL);
	  return outputVolume;
	}
	else{
	  return NULL;
	}
	
}
//----------------------------------------------------------------------------
//------Get warped image by applying transform on moving image
vtkMRMLScalarVolumeNode* vtkSlicerRegistrationQALogic::GetWarpedFromMoving(vtkMRMLScalarVolumeNode *movingVolume, vtkMRMLTransformNode *transform) {

	if (!this->GetMRMLScene()) {
		vtkErrorMacro("Internal Error, see command line!");
                return NULL;
	}


	if (!movingVolume || !transform ) {
		vtkErrorMacro("Check input parameters!");
                return NULL;
	}
	
	// Create new scalar volume
	vtkSmartPointer<vtkMRMLScalarVolumeNode> outputVolume = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();
	vtkNew<vtkMRMLScalarVolumeDisplayNode> sDisplayNode;
	sDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey");
	this->GetMRMLScene()->AddNode(sDisplayNode.GetPointer());
	outputVolume->SetAndObserveDisplayNodeID(sDisplayNode->GetID());
	outputVolume->SetAndObserveStorageNodeID(NULL);
	this->GetMRMLScene()->AddNode(outputVolume);
	
	if(!outputVolume){
	  vtkErrorMacro("Can't create output volume!");
          return NULL;
	}
	
	vtkNew<vtkImageData> imageData_new;	
	vtkSmartPointer<vtkImageData> imageData_old = movingVolume->GetImageData();
	
	imageData_new->DeepCopy(imageData_old.GetPointer());
	outputVolume->SetAndObserveImageData(imageData_new.GetPointer());
	
	//Direction
	vtkNew<vtkMatrix4x4> gridDirectionMatrix_RAS;
	movingVolume->GetIJKToRASDirectionMatrix(gridDirectionMatrix_RAS.GetPointer());
        outputVolume->SetIJKToRASDirectionMatrix(gridDirectionMatrix_RAS.GetPointer());
	
	// Origin
	outputVolume->SetOrigin( movingVolume->GetOrigin()[0], movingVolume->GetOrigin()[1], movingVolume->GetOrigin()[2] );

	// Spacing
	outputVolume->SetSpacing( movingVolume->GetSpacing()[0], movingVolume->GetSpacing()[1], movingVolume->GetSpacing()[2] );
	
	std::string outSS;
        std::string Name("_warped");
        outSS = movingVolume->GetName() + Name;
        outSS = this->GetMRMLScene()->GenerateUniqueName(outSS);
        outputVolume->SetName(outSS.c_str());
	
	outputVolume->SetAndObserveTransformNodeID( transform->GetID() );
        
        //Harden Transform
        outputVolume->HardenTransform();
        
        return outputVolume;
}
//---Change Vector node to transform node-------------------------------------------------------------------------
vtkMRMLGridTransformNode* vtkSlicerRegistrationQALogic::CreateTransformFromVector(vtkMRMLVectorVolumeNode* vectorVolume)
{
	if (!vectorVolume) {
	  vtkErrorMacro("CreateTransormFromVector: Volumes not set!");
	  return NULL;
	}
	
	
		

	// Logic partialy copied from: vtkMRMLTransformStorageNode.cxx
	vtkNew<vtkImageData> gridImage_Ras;	
	vtkSmartPointer<vtkImageData> imageData = vectorVolume->GetImageData();
	if (!imageData) {
	  vtkErrorMacro("CreateTransormFromVector: No image data!");
	  return NULL;
	}
	
	gridImage_Ras->DeepCopy( imageData.GetPointer() );
	
	this->InvertXandY( gridImage_Ras.GetPointer() );

	// Origin
	gridImage_Ras->SetOrigin( vectorVolume->GetOrigin()[0], vectorVolume->GetOrigin()[1], vectorVolume->GetOrigin()[2] );

	// Spacing
	gridImage_Ras->SetSpacing( vectorVolume->GetSpacing()[0], vectorVolume->GetSpacing()[1], vectorVolume->GetSpacing()[2] );
	
	// Grid transform
	vtkSmartPointer<vtkOrientedGridTransform> transformFromParent = vtkSmartPointer<vtkOrientedGridTransform>::New();
	
	//Direction
	vtkNew<vtkMatrix4x4> gridDirectionMatrix_RAS;
	vectorVolume->GetIJKToRASDirectionMatrix(gridDirectionMatrix_RAS.GetPointer());
        transformFromParent->SetGridDirectionMatrix(gridDirectionMatrix_RAS.GetPointer());
		
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
vtkMRMLVectorVolumeNode* vtkSlicerRegistrationQALogic::CreateVectorFromTransform(vtkMRMLTransformNode* transform)
{
	if (!transform) {
	  std::cerr << "CreateVectorFromTransform: No transform set!" << std::endl;
	  return NULL;
	}
	//Create new Vector Volume from transform. Copied from Crop Volume Logic.
	
	vtkMRMLRegistrationQANode* pNode = this->RegistrationQANode;
	
	if (!this->GetMRMLScene() || !pNode ) {
		vtkErrorMacro("CreateVectorFromTransform: Invalid scene or parameter node!");
		return NULL;
	}
	
	vtkNew<vtkSlicerTransformLogic> TransformLogic;
        vtkSmartPointer<vtkMRMLVolumeNode> vectorVolume;
        
        vtkMRMLVolumeNode* refVolume = vtkMRMLVolumeNode::SafeDownCast(
           this->GetMRMLScene()->GetNodeByID( pNode->GetVolumeNodeID() ));
	
	vectorVolume =  TransformLogic->CreateDisplacementVolumeFromTransform(transform, refVolume,
           false, NULL);
        
        if ( ! vectorVolume ){
           vtkErrorMacro("CreateVectorFromTransform: Can't create vector from transform!");
           return NULL;
        }
        
        return vtkMRMLVectorVolumeNode::SafeDownCast( vectorVolume );
			  
	
// 	vtkNew<vtkMRMLVectorVolumeDisplayNode> vvDisplayNode;
// 	scene->AddNode(vvDisplayNode.GetPointer());
// 	vectorVolume->SetAndObserveDisplayNodeID(vvDisplayNode->GetID());
// 	vectorVolume->SetAndObserveStorageNodeID(NULL);
	
/*	
	//Create Grid transform	
	vtkSmartPointer<vtkOrientedGridTransform> transformFromParent;
        transformFromParent = vtkOrientedGridTransform::SafeDownCast(
							transform->GetTransformFromParent());
	
	if (!transformFromParent) {
	  vtkErrorMacro("CreateVectorFromTransform: No transform from parent!");
	  return NULL;
	}
	
	
	vtkNew<vtkImageData> imageData;
	vtkSmartPointer<vtkImageData> gridImage_Ras = transformFromParent->GetDisplacementGrid();
	
	if (!gridImage_Ras) {
	  vtkErrorMacro("CreateVectorFromTransform: No image data!");
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
	vectorVolume->SetOrigin( gridImage_Ras->GetOrigin()[0], gridImage_Ras->GetOrigin()[1], gridImage_Ras->GetOrigin()[2] );
	// Dimensions
	int dims[3];
	gridImage_Ras->GetDimensions(dims);
	imageData->SetDimensions(dims);

	imageData->DeepCopy( gridImage_Ras.GetPointer() );
	
// 	imageData->Update();
	
// 	this->InvertXandY( imageData.GetPointer() );

	// Image Data
	vectorVolume->SetAndObserveImageData( imageData.GetPointer() );
        
        //Direction
        vtkSmartPointer<vtkMatrix4x4> gridDirectionMatrix_RAS = transformFromParent->GetGridDirectionMatrix();
        vectorVolume->SetIJKToRASDirectionMatrix(gridDirectionMatrix_RAS.GetPointer());
	
	this->GetMRMLScene()->AddNode(vectorVolume.GetPointer());
	
	return vectorVolume.GetPointer();*/
}
//--- Create a ROI from segment in node -----------------------------------------------------------
void vtkSlicerRegistrationQALogic::CreateROI(){
   vtkMRMLSegmentationNode* fixedSegmentation = this->RegistrationQANode->GetSegmentationNode();
   const char* fixedSegmentID = this->RegistrationQANode->GetSegmentID();
   
   vtkMRMLSegmentationNode* movingSegmentation = this->RegistrationQANode->GetBackwardRegistrationQAParameters()->GetSegmentationNode();
   const char* movingSegmentID = this->RegistrationQANode->GetBackwardRegistrationQAParameters()->GetSegmentID();
   
   vtkSmartPointer<vtkMRMLAnnotationROINode> ROINode;
   
   ROINode = this->CreateROIAroundSegments(fixedSegmentation, fixedSegmentID, movingSegmentation, movingSegmentID);
   if (ROINode){
      this->RegistrationQANode->SetAndObserveROINodeID(ROINode->GetID());
   }
   else{
      vtkErrorMacro("Can't get ROI Node!");
      return;
   }
}
//--- Create a ROI around one or two segments -----------------------------------------------------------
vtkMRMLAnnotationROINode* vtkSlicerRegistrationQALogic::CreateROIAroundSegments(vtkMRMLSegmentationNode* segmentation1Node,const char* segment1StringID,
   vtkMRMLSegmentationNode* segmentation2Node,const char* segment2StringID)
{
   if (!segmentation1Node || !segment1StringID )
   {
      vtkErrorMacro("Segmentation input missing!");
      return NULL;
   }
   vtkSmartPointer<vtkSegment> segment1;
   segment1 = segmentation1Node->GetSegmentation()->GetSegment(segment1StringID);
   
   double radius1[3];
   double center1[3];
   
   if (! this->GetRadiusAndCenter(segmentation1Node, segment1, radius1, center1)){
      vtkErrorMacro("Can't get radius and center from segment!");
      return NULL;
   }
   // If we have second segmentation as well, ROI is extended to second segmentation
   if (segmentation2Node && segment2StringID){
      vtkSmartPointer<vtkSegment> segment2;
      double radius2[3];
      double center2[3];
      segment2 = segmentation2Node->GetSegmentation()->GetSegment(segment2StringID);
      if (! this->GetRadiusAndCenter(segmentation2Node,segment2, radius2, center2)){
         vtkErrorMacro("Can't get radius and center from segment!");
         return NULL;
      }
      
      for( int i = 0; i<3; i++){
         double min1, min2, max1, max2;
         double min, max;
         min1 = center1[i] - radius1[i];
         min2 = center2[i] - radius2[i];
         max1 = center1[i] + radius1[i];
         max2 = center2[i] + radius2[i];

         if (min1 < min2) min = min1;
         else min = min2;
         if (max1 > max2) max = max1;
         else max = max2;

         radius1[i] = (max - min) / 2;
         center1[i] = min + radius1[i];
      }
   }
   
   /* create ROI and add radius first, center second */
   vtkSmartPointer<vtkMRMLScene> scene; 
   scene = this->GetMRMLScene();
   vtkSmartPointer< vtkMRMLAnnotationROINode > ROINode = vtkSmartPointer< vtkMRMLAnnotationROINode >::New();
   std::string ROIName = scene->GenerateUniqueName( segment1StringID );
   scene->AddNode(ROINode);
   ROINode->SetName(ROIName.c_str());
   
   ROINode->CreateDefaultDisplayNodes();
   
   int retVal;
   
   retVal = ROINode->SetRadiusXYZ(radius1);
   if (!retVal){
      qCritical() << Q_FUNC_INFO << ": Failed to set ROI Radius";
      return NULL;
   }
   
   retVal = ROINode->SetXYZ(center1);
   
   if (!retVal){
      qCritical() << Q_FUNC_INFO << ": Failed to set ROI center";
      return NULL;
   }
   
   return ROINode;
}
//--- Get radius and center from segment -----------------------------------------------------------
bool vtkSlicerRegistrationQALogic::GetRadiusAndCenter(vtkMRMLSegmentationNode* segmentationNode, vtkSegment* segment,double radius[3], double center[3]){
      vtkSmartPointer<vtkMRMLLabelMapVolumeNode> labelmapNode = vtkSmartPointer<vtkMRMLLabelMapVolumeNode>::New();
      this->GetMRMLScene()->AddNode(labelmapNode);
      std::vector<std::string> segmentIDs;
      segmentIDs.push_back(segmentationNode->GetSegmentation()->GetSegmentIdBySegment(segment));
      vtkNew<vtkSlicerSegmentationsModuleLogic> SegmentationLogic;
      
      if (! SegmentationLogic->ExportSegmentsToLabelmapNode(segmentationNode, segmentIDs,
                            labelmapNode)) {
         vtkErrorMacro("GetRadiusAndCenter: ExportSegmentsToLabelmapNode failed");
         this->GetMRMLScene()->RemoveNode(labelmapNode);
         return false;
      }
    
      
      if ( labelmapNode == NULL ){
         vtkErrorMacro("GetRadiusAndCenter: Can't get binary labelmap!");
         this->GetMRMLScene()->RemoveNode(labelmapNode);
         return false;
      }
   
      int *dims = labelmapNode->GetImageData()->GetDimensions();
      double *spacing = labelmapNode->GetSpacing();
      double *origin = labelmapNode->GetOrigin();
     
      double dimsH[4];
      dimsH[0] = dims[0] - 1;
      dimsH[1] = dims[1] - 1;
      dimsH[2] = dims[2] - 1;
      dimsH[3] = 0.;

      vtkNew<vtkMatrix4x4> ijkToRAS;
      labelmapNode->GetIJKToRASDirectionMatrix(ijkToRAS.GetPointer());
      double rasCorner[4];
      ijkToRAS->MultiplyPoint(dimsH, rasCorner);
      for (int i=0;i<3;i++){
        radius[i] = rasCorner[i]*spacing[i]/2;
        center[i] = origin[i] + radius[i];
        radius[i] = fabs(radius[i]);
        radius[i] += 10*spacing[i]; // Add ten voxels as margin
      }
      qCritical() << Q_FUNC_INFO << ": ROI center, origin and radius\n" << 
      center[0] << " " << center[1] << " " << center[2] << "\n"
      << origin[0] << " " << origin[1] << " " << origin[2]  << "\n"
      << radius[0] << " " << radius[1] << " " << radius[2];
      this->GetMRMLScene()->RemoveNode(labelmapNode);
      return true;
}
//--- Invert X and Y in image Data -----------------------------------------------------------
void vtkSlicerRegistrationQALogic::InvertXandY(vtkImageData* imageData){
	if (!imageData) {
		      vtkErrorMacro("InvertXandY: No imageData.");
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
	      vtkErrorMacro("InvertXandY: Unknown image type." );
	      return;
	}
	return;
}
//--- Default mode when checkbox is unchecked -----------------------------------------------------------
void vtkSlicerRegistrationQALogic::SetDefaultDisplay() {
	if (!this->GetMRMLScene() || !this->RegistrationQANode) {
		vtkErrorMacro("SetDefaultDisplay: Invalid scene or parameter set node!");
		return;
	}
	vtkMRMLScalarVolumeNode *referenceVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQANode->GetVolumeNodeID()));
	
	
	vtkMRMLScalarVolumeNode *warpedVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQANode->GetWarpedVolumeNodeID()));

	if (!warpedVolume || !referenceVolume) {
		vtkErrorMacro("SetDefaultDisplay: Invalid volumes!");
		return;
	}
	//TODO: Volumes go back to gray value - perhaps we should rembemer previous color settings?
	warpedVolume->GetDisplayNode()->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey");
	referenceVolume->GetDisplayNode()->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey");

	// Set window and level the same for warped and reference volume.
	warpedVolume->GetScalarVolumeDisplayNode()->AutoWindowLevelOff();
	warpedVolume->GetScalarVolumeDisplayNode()->SetWindow(referenceVolume->GetScalarVolumeDisplayNode()->GetWindow());
	warpedVolume->GetScalarVolumeDisplayNode()->SetLevel(referenceVolume->GetScalarVolumeDisplayNode()->GetLevel());
	this->SetForegroundImage(referenceVolume,warpedVolume,0.5);

	return;
}

//---Calculate Statistic of image-----------------------------------
void vtkSlicerRegistrationQALogic::CalculateStatistics(vtkMRMLScalarVolumeNode *inputVolume, double statisticValues[4] ) {
	vtkNew<vtkImageAccumulate> StatImage;
	StatImage->SetInputData(inputVolume->GetImageData());
	StatImage->Update();
        statisticValues[0]= StatImage->GetMax()[0];
        statisticValues[1]= StatImage->GetMin()[0];
	statisticValues[2]= StatImage->GetMean()[0];
	statisticValues[3]= StatImage->GetStandardDeviation()[0];

}

//---Load volume node from fileName
vtkMRMLVolumeNode* vtkSlicerRegistrationQALogic::LoadVolumeFromFile( std::string filePath, std::string volumeName){
   
   vtkNew<vtkSlicerVolumesLogic> VolumesLogic;
   vtkMRMLVolumeNode* volume =
    VolumesLogic->AddArchetypeVolume(filePath.c_str(), volumeName.c_str(), 0);
    
   if (!volume){
      vtkErrorMacro("LoadVolumeFromFile: Can't load volume!");
      return NULL;
   }
   
   return volume;
   
}

//---Change backroung image and set opacity-----------------------------------
void vtkSlicerRegistrationQALogic
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
//---Create default table-----------------------------------
vtkMRMLTableNode* vtkSlicerRegistrationQALogic::CreateDefaultRegistrationQATable() {
   if (!this->GetMRMLScene() || !this->RegistrationQANode) {
      vtkErrorMacro("CreateDefaultRegistrationQATable: Invalid scene or parameter set node!");
      return NULL;
   }

   vtkSmartPointer<vtkMRMLTableNode> tableNode = vtkSmartPointer<vtkMRMLTableNode>::New();

   std::string tableNodeName = this->GetMRMLScene()->GenerateUniqueName("RegistrationQA");
   tableNode->SetName(tableNodeName.c_str());
   this->GetMRMLScene()->AddNode(tableNode);
   
   vtkSmartPointer<vtkMRMLTableStorageNode> tableStorageNode = vtkMRMLTableStorageNode::SafeDownCast(
      tableNode->CreateDefaultStorageNode());
   this->GetMRMLScene()->AddNode(tableStorageNode);
   tableNode->SetAndObserveStorageNodeID(tableStorageNode->GetID());
   
   tableNode->SetUseColumnNameAsColumnHeader(false);
   
   vtkStringArray* zero = vtkStringArray::SafeDownCast(tableNode->AddColumn());
   // Add input information to the table
   zero->InsertNextValue("Reference Image"); //0
   zero->InsertNextValue("Moving Image"); //1
   zero->InsertNextValue("Forward Warped Image"); //2
   zero->InsertNextValue("Backward Warped Image"); //3
   zero->InsertNextValue("Forward vector field"); //4
   zero->InsertNextValue("Backward vector field"); //5
   zero->InsertNextValue("Reference Contour");//6
   zero->InsertNextValue("Moving Contour");//7
   zero->InsertNextValue("Reference Fiducial");//8
   zero->InsertNextValue("Moving Fiducial");//9
   zero->InsertNextValue("ROI");//10
   zero->InsertNextValue("Measure:");//11
   zero->InsertNextValue("AbsDiff (for)");//12
   zero->InsertNextValue("AbsDiff (back)");//13
   zero->InsertNextValue("Jacobian (for)");//14
   zero->InsertNextValue("Jacobian (back)");//15
   zero->InsertNextValue("InvConsist ");//16
   zero->InsertNextValue("");//17
   zero->InsertNextValue("Contour (for)");//18
   zero->InsertNextValue("Contour (back)");//19
   zero->InsertNextValue("Fiducial");//20
   
   // Add four columns */
   tableNode->AddColumn();
   tableNode->AddColumn();
   tableNode->AddColumn();
   tableNode->AddColumn();
   
   if (! tableNode->SetCellText(11,1,"Max")){
      vtkErrorMacro("CreateDefaultRegistrationQATable: Can't set cell!");
      return NULL;
   }
   if (! tableNode->SetCellText(11,2,"Min")){
      vtkErrorMacro("CreateDefaultRegistrationQATable: Can't set cell!");
      return NULL;
   }
   if (! tableNode->SetCellText(11,3,"Mean")){
      vtkErrorMacro("CreateDefaultRegistrationQATable: Can't set cell!");
      return NULL;
   }
   if (! tableNode->SetCellText(11,4,"STD")){
      vtkErrorMacro("CreateDefaultRegistrationQATable: Can't set cell!");
      return NULL;
   }
   if (! tableNode->SetCellText(20,1,"Distance Before")){
      vtkErrorMacro("CreateDefaultRegistrationQATable: Can't set cell!");
      return NULL;
   }
   if (! tableNode->SetCellText(20,2,"Distance After")){
      vtkErrorMacro("CreateDefaultRegistrationQATable: Can't set cell!");
      return NULL;
   }
   if (! tableNode->SetCellText(17,1,"Mean Hauss. Before")){
      vtkErrorMacro("CreateDefaultRegistrationQATable: Can't set cell!");
      return NULL;
   }
   if (! tableNode->SetCellText(17,2,"Mean Hauss. After")){
      vtkErrorMacro("CreateDefaultRegistrationQATable: Can't set cell!");
      return NULL;
   }
   if (! tableNode->SetCellText(17,3,"Dice Coeff. Before")){
      vtkErrorMacro("CreateDefaultRegistrationQATable: Can't set cell!");
      return NULL;
   }
   if (! tableNode->SetCellText(17,4,"Dice Coeff. After")){
      vtkErrorMacro("CreateDefaultRegistrationQATable: Can't set cell!");
      return NULL;
   }

   return tableNode;
}
//---Create default table-----------------------------------
void vtkSlicerRegistrationQALogic::UpdateRegistrationQATable() {
   if (!this->GetMRMLScene() || !this->RegistrationQANode) {
      vtkErrorMacro("UpdateRegistrationQATable: Invalid scene or parameter set node!");
      return;
   }
   vtkSmartPointer<vtkMRMLRegistrationQANode> pNode = this->RegistrationQANode;
   vtkSmartPointer<vtkMRMLNode> node;
   char* segmentID;

   //Get forward RegistrationQA parameter node
   if ( pNode->GetBackwardRegistration() ){
      pNode = this->RegistrationQANode->GetBackwardRegistrationQAParameters();
   }
   else{
      pNode = this->RegistrationQANode;
   }
   
   if (!pNode) {
      vtkErrorMacro("UpdateRegistrationQATable: Wrong node setting!");
      return;
   }

   vtkSmartPointer<vtkMRMLTableNode> regQATable = pNode->GetRegistrationQATableNode();
   vtkSmartPointer<vtkTable> table;
   table = regQATable->GetTable();

   node = this->GetMRMLScene()->GetNodeByID(pNode->GetVolumeNodeID());
   if ( node ){
      table->SetValue(0, 1, vtkVariant(node->GetName()));
   }
   node = this->GetMRMLScene()->GetNodeByID(pNode->GetWarpedVolumeNodeID());
   if ( node ){
      table->SetValue(2, 1, vtkVariant(node->GetName()));
   }
   node = this->GetMRMLScene()->GetNodeByID(pNode->GetVectorVolumeNodeID());
   if ( node ){
      table->SetValue(4, 1, vtkVariant(node->GetName()));
   }
   segmentID = pNode->GetSegmentID();
   if ( segmentID ){
      table->SetValue(6, 1, vtkVariant(segmentID));
   }
   node = this->GetMRMLScene()->GetNodeByID(pNode->GetFiducialNodeID());
   if ( node ){
      table->SetValue(8, 1, vtkVariant(node->GetName()));
   }
   node = this->GetMRMLScene()->GetNodeByID(pNode->GetROINodeID());
   if ( node ){
      table->SetValue(10, 1, vtkVariant(node->GetName()));
   }
   
   //Repeat all for backward
   if ( !pNode->GetBackwardRegistrationQAParameters()){
      regQATable->Modified();
      return;
   }
   pNode = pNode->GetBackwardRegistrationQAParameters();

   node = this->GetMRMLScene()->GetNodeByID(pNode->GetVolumeNodeID());
   if ( node ){
      table->SetValue(1, 1, vtkVariant(node->GetName()));
   }
   node = this->GetMRMLScene()->GetNodeByID(pNode->GetWarpedVolumeNodeID());
   if ( node ){
      table->SetValue(3, 1, vtkVariant(node->GetName()));
   }
   node = this->GetMRMLScene()->GetNodeByID(pNode->GetVectorVolumeNodeID());
   if ( node ){
      table->SetValue(5, 1, vtkVariant(node->GetName()));
   }
   segmentID = pNode->GetSegmentID();
   if ( segmentID ){
      table->SetValue(7, 1, vtkVariant(segmentID));
   }
   node = this->GetMRMLScene()->GetNodeByID(pNode->GetFiducialNodeID());
   if ( node ){
      table->SetValue(9, 1, vtkVariant(node->GetName()));
   }

   regQATable->Modified();
}
void vtkSlicerRegistrationQALogic::UpdateTableWithStatisticalValues(double statisticValues[4], int row) {
   if (!this->RegistrationQANode) {
      vtkErrorMacro("UpdateRegistrationQATable: Invalid scene or parameter set node!");
      return;
   }
   
   vtkSmartPointer<vtkMRMLRegistrationQANode> pNode = this->RegistrationQANode;
   vtkSmartPointer<vtkMRMLNode> node;
   vtkSmartPointer<vtkMRMLTableNode> regQATable = pNode->GetRegistrationQATableNode();
   vtkSmartPointer<vtkTable> table;
   table = regQATable->GetTable();


   table->SetValue(row, 1, vtkVariant(statisticValues[0])); //Max
   table->SetValue(row, 2, vtkVariant(statisticValues[1])); //Min
   table->SetValue(row, 3, vtkVariant(statisticValues[2])); //Mean
   table->SetValue(row, 4, vtkVariant(statisticValues[3])); //STD
   
   regQATable->Modified();

}
//---------------------------------------------------------------------------
void vtkSlicerRegistrationQALogic::UpdateTableWithFiducialValues(vtkMRMLMarkupsFiducialNode* fiducals, double statisticValues[4]){
   if ( !this->RegistrationQANode ) {
      vtkErrorMacro("UpdateTableWithFiducialValues: Invalid parameter set node!");
      vtkErrorMacro("Internal Error, see command line!");
   }
   if ( !fiducals || !statisticValues){
      vtkErrorMacro("UpdateTableWithFiducialValues: Invalid input parameters!");
      vtkErrorMacro("Internal Error, see command line!");
   }
   
   vtkSmartPointer<vtkMRMLTableNode> tableNode = this->RegistrationQANode->GetRegistrationQATableNode();
   vtkSmartPointer<vtkTable> table = tableNode->GetTable();
   int fiducialNumber = fiducals->GetNumberOfFiducials();
   vtkIdType nRows = table->GetNumberOfRows();
   vtkIdType nAllRows = 21;
   
   //Sanity check
   if ( nRows < nAllRows ){
      vtkErrorMacro("UpdateTableWithFiducialValues: Invalid number of rows in table!");
      vtkErrorMacro("Internal Error, see command line!");
   }
   
   //First cells are for forward registration, followed by backward
   if ( this->RegistrationQANode->GetBackwardRegistration() ){
      nAllRows += fiducialNumber;
   }
   
   
   for (int i=0;i<fiducialNumber;i++){
      // Add rows, if necessary, otherwise rewrite
      if ( nRows == nAllRows ){
         table->InsertNextBlankRow();
      }
      int row = nRows + i;
      table->SetValue(row,0,vtkVariant(fiducals->GetNthMarkupLabel(i)));
      table->SetValue(row,1,vtkVariant(statisticValues[2*i]));
      table->SetValue(row,2,vtkVariant(statisticValues[2*i+1]));
   }
   tableNode->Modified();
}

void vtkSlicerRegistrationQALogic::UpdateNodeFromSHNode(vtkIdType itemID){
   if ( !this->RegistrationQANode || !this->GetMRMLScene() ) {
      vtkErrorMacro("UpdateNodeFromSHNode: Invalid parameter set node!");
      return;
   }
   
   vtkMRMLRegistrationQANode* pNode;
   
   vtkSmartPointer<vtkMRMLSubjectHierarchyNode> shNode;
   shNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(
         this->GetMRMLScene()->GetNodeByID("vtkMRMLSubjectHierarchyNode"));
   
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   
   std::string regType = shNode->GetItemAttribute(itemID,
         vtkSlicerRegistrationQALogic::REGISTRATION_TYPE.c_str());
   std::string inverse = shNode->GetItemAttribute(itemID,
         vtkSlicerRegistrationQALogic::INVERSE.c_str());
   vtkMRMLNode* node = shNode->GetItemDataNode(itemID);
   
   if ( ! node ){
      return;
   }
   
   if ( regType.empty() ){
      return;
   }
   
   if ( inverse.empty() ){
      pNode = this->RegistrationQANode;
   }
   else{
      pNode = this->RegistrationQANode->GetBackwardRegistrationQAParameters();
   }
   
   if ( ! pNode ) {
      return;
   }
   
   if ( regType.compare(vtkSlicerRegistrationQALogic::IMAGE) == 0) pNode->SetAndObserveVolumeNodeID(node->GetID());
   else if ( regType.compare(vtkSlicerRegistrationQALogic::WARPED_IMAGE) == 0) pNode->SetAndObserveWarpedVolumeNodeID(node->GetID());
   else if ( regType.compare(vtkSlicerRegistrationQALogic::VECTOR_FIELD) == 0) pNode->SetAndObserveVectorVolumeNodeID(node->GetID());
   else if ( regType.compare(vtkSlicerRegistrationQALogic::FIDUCIAL) == 0) pNode->SetAndObserveFiducialNodeID(node->GetID());
   else if ( regType.compare(vtkSlicerRegistrationQALogic::ROI) == 0) pNode->SetAndObserveROINodeID(node->GetID());
   else if ( regType.compare(vtkSlicerRegistrationQALogic::ABSOLUTEDIFFERENCE) == 0) pNode->SetAndObserveAbsoluteDiffVolumeNodeID(node->GetID());
   else if ( regType.compare(vtkSlicerRegistrationQALogic::JACOBIAN) == 0) pNode->SetAndObserveJacobianVolumeNodeID(node->GetID());
   else if ( regType.compare(vtkSlicerRegistrationQALogic::INVERSECONSISTENCY) == 0) pNode->SetAndObserveInverseConsistVolumeNodeID(node->GetID());
   //TODO:
//    else if ( regType.compare(vtkSlicerRegistrationQALogic::CONTOUR) == 0) pNode->SetAndObserveAbsoluteDiffVolumeNodeID(node->GetID());
}
//TODO: Integrate save of output file
//---------------------------------------------------------------------------
void vtkSlicerRegistrationQALogic::SaveOutputFile() {
        if (!this->GetMRMLScene() || !this->RegistrationQANode) {
            vtkErrorMacro("saveOutputFile: Invalid scene or parameter set node!");
            return;         
        }
        
//      double statisticValues[4];
//      std::string color;
//      std::string directory = this->RegistrationQANode->GetOutputDirectory();
//      char fileName[512];
//      sprintf(fileName, "%s/OutputFile.html", directory.c_str() );
//      
//      std::ofstream outfile;
//      outfile.open(fileName, std::ios_base::out | std::ios_base::trunc);
//      
//      if ( !outfile ){
//              vtkErrorMacro("SaveOutputFile: Output file '" << fileName << "' cannot be opened!");
//              return;
//      }
//      
//      outfile << "<!DOCTYPE html>" << std::endl << "<html>" << std::endl 
//      << "<head>" << std::endl << "<title>Registration Quality</title>" << std::endl 
//      << "<meta charset=\"UTF-8\">" << std::endl << "</head>" << "<body>" << std::endl;
//      
//      outfile << "<h1>Registration Quality Output File</h1>" << std::endl;
//      d->regQALogic->CalculateregQAFrom(4);
//      // --- Image Checks ---
//      outfile << "<h2>Image Checks</h2>" << std::endl;
//      //Set table
//      outfile << "<table style=\"width:60%\">" << std::endl
//      << "<tr>" << std::endl
//      << "<td> </td> <td> Mean </td> <td> STD </td> <td> Max </td> <td> Min </td>" << std::endl
//      << "</tr>" << std::endl;
//      
//      //Absolute Difference
//      this->RegistrationQANode->GetAbsoluteDiffStatistics(statisticValues);
//      //Check values for color
//      if(statisticValues[0] < 100 && statisticValues[1] < 200) color = "green";
//      else if (statisticValues[0] == 0 && statisticValues[1] == 0 &&  statisticValues[2] == 0 && statisticValues[3] == 0) color = "blue";
//      else color = "red";
//      
//      outfile << "<tr style=\"background-color:" << color <<"; color:white;\">" << std::endl
//      << "<td>" << "Absolute Difference between reference and warped image in HU" << "</td>";
//      for(int i=0;i<4;i++) {  
//              outfile << "<td>" << statisticValues[i] << "</td>";
//      }
//      outfile << std::endl<< "</tr>" << std::endl;
//      
//      //TODO: Inverse Absolute Difference
//      
//      //Fiducials
//      this->RegistrationQANode->GetFiducialsStatistics(statisticValues);
//      //Check values for color
//      if(statisticValues[0] > 0 && statisticValues[1] > 0 && statisticValues[1] > 0) color = "green";
//      else if (statisticValues[0] == 0 && statisticValues[1] == 0 &&  statisticValues[2] == 0 && statisticValues[3] == 0) color = "blue";
//      else color = "red";
//      
//      outfile << "<tr style=\"background-color:" << color <<"; color:white;\">" << std::endl
//      << "<td>" << "Distance difference in fiducials after transformation in mm" << "</td>";
//      for(int i=0;i<4;i++) {  
//              outfile << "<td>" << statisticValues[i] << "</td>";
//      }
//      outfile << std::endl<< "</tr>" << std::endl;
//      
//      //Inverse Fiducials
//      this->RegistrationQANode->GetInvFiducialsStatistics(statisticValues);
//      //Check values for color
//      if(statisticValues[0] > 0 && statisticValues[1] > 0 && statisticValues[1] > 0) color = "green";
//      else if (statisticValues[0] == 0 && statisticValues[1] == 0 &&  statisticValues[2] == 0 && statisticValues[3] == 0) color = "blue";
//      else color = "red";
//      
//      outfile << "<tr style=\"background-color:" << color <<"; color:white;\">" << std::endl
//      << "<td>" << "Distance difference in fiducials after inverse transformation in mm" << "</td>";
//      for(int i=0;i<4;i++) {  
//              outfile << "<td>" << statisticValues[i] << "</td>";
//      }
//      outfile << std::endl<< "</tr>" << std::endl;
//      
//      outfile << "</table>" << std::endl;
//              
//      // --- Vector checks ---        
//      outfile << "<h2>Vector Checks</h2>" << std::endl;       
//      //Set table
//      outfile << "<table style=\"width:60%\">" << std::endl
//      << "<tr>" << std::endl
//      << "<td> </td> <td> Mean </td> <td> STD </td> <td> Max </td> <td> Min </td>" << std::endl
//      << "</tr>" << std::endl;
//      
//      //Jacobian
//      this->RegistrationQANode->GetJacobianStatistics(statisticValues);
//      //Check values for color
//      if( abs(statisticValues[0]-1) < 0.02 && statisticValues[1] < 0.05 && statisticValues[2] < 5 && statisticValues[3] > 0) color = "green";
//      else if (statisticValues[0] == 0 && statisticValues[1] == 0 &&  statisticValues[2] == 0 && statisticValues[3] == 0) color = "blue";
//      else color = "red";
//      
//      outfile << "<tr style=\"background-color:" << color <<"; color:white;\">" << std::endl
//      << "<td>" << "Jacobian of vector field" << "</td>";
//      for(int i=0;i<4;i++) {  
//              outfile << "<td>" << statisticValues[i] << "</td>";
//      }
//      outfile << std::endl<< "</tr>" << std::endl;
//      
//      //TODO: Inverse Jacobian
//      
//      //Inverse Consistency
//      this->RegistrationQANode->GetInverseConsistStatistics(statisticValues);
//      //Check values for color
//      if( statisticValues[0] < 2 && statisticValues[1] < 2) color = "green";
//      else if (statisticValues[0] == 0 && statisticValues[1] == 0 &&  statisticValues[2] == 0 && statisticValues[3] == 0) color = "blue";
//      else color = "red";
//      
//      outfile << "<tr style=\"background-color:" << color <<"; color:white;\">" << std::endl
//      << "<td>" << "Inverse Consistency in mm" << "</td>";
//      for(int i=0;i<4;i++) {  
//              outfile << "<td>" << statisticValues[i] << "</td>";
//      }
//      outfile << std::endl<< "</tr>" << std::endl;
// 
//      outfile << "</table>" << std::endl;
// 
//      // --- Images ---
//      outfile << "<h2>Images</h2>" << std::endl;      
//      int screenShotNumber = this->RegistrationQANode->GetNumberOfScreenshots();
//      if (screenShotNumber > 1){      
//              for(int i = 1; i < screenShotNumber; i++) {
//                      std::ostringstream screenShotName;
//                      screenShotName << "Screenshot_" << i;
//                      
//                              
//                      //Find the associated node for description
//                      vtkSmartPointer<vtkCollection> collection = vtkSmartPointer<vtkCollection>::Take(
//                                              this->GetMRMLScene()->GetNodesByName(screenShotName.str().c_str()));
//                      if (collection->GetNumberOfItems() == 1) {      
//                              vtkMRMLAnnotationSnapshotNode* snapshot = vtkMRMLAnnotationSnapshotNode::SafeDownCast(
//                                              collection->GetItemAsObject(0));
//                              outfile << "<h3>" << snapshot->GetSnapshotDescription() << "</h3>" << std::endl;
//                      }
//                      
//                      outfile << "<img src=\"" << screenShotName.str() << ".png"
//                              <<"\" alt=\"Screenshot "<< i << "\" width=\"80%\"> " << std::endl;
//              }
//      }
//      outfile  << "</body>" << std::endl << "</html>" << std::endl;
//      
//      outfile << std::endl;
//      outfile.close();
        
//      std::cerr << "Output file save to: " << fileName << "" << std::endl;
}