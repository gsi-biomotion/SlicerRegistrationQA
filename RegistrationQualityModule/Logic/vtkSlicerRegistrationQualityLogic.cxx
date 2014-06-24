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
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include "vtkSlicerVolumesLogic.h"
#include "vtkMRMLViewNode.h"
#include "vtkSlicerCLIModuleLogic.h"
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLSliceLogic.h>

#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkSubjectHierarchyConstants.h>
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkVectorNorm.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkGeneralTransform.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkImageAccumulate.h>

// STD includes
#include <iostream>
#include <cassert>
#include <math.h>
#include <exception>

#include <tinyxml.h>
#include <DIRQAImage.h>
#include <QStandardItemModel>

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
// 	delete subjectModel;
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

void vtkSlicerRegistrationQualityLogic::SquaredDifference(int state) {
	if (!this->GetMRMLScene() || !this->RegistrationQualityNode) {
	    vtkErrorMacro("SquaredDifference: Invalid scene or parameter set node!");
	    return;
	}

	vtkMRMLScalarVolumeNode *referenceVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetReferenceVolumeNodeID()));

	vtkMRMLScalarVolumeNode *warpedVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetWarpedVolumeNodeID()));


// 	vtkMRMLScalarVolumeNode *outputVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
// 			this->GetMRMLScene()->GetNodeByID(
// 				this->RegistrationQualityNode->GetSquaredDiffVolumeNodeID()));
	if (!referenceVolume || !warpedVolume ) {
		vtkErrorMacro("SquaredDifference: Invalid reference or warped volume!");
		return;
	}

	if (!state) {
	      	this->SetDefaultDisplay(referenceVolume,warpedVolume);
		return;
	}


	if (!this->RegistrationQualityNode->GetSquaredDiffVolumeNodeID()){
	  if(!this->Internal->VolumesLogic)
	      {
		std::cerr << "SquaredDifference: ERROR: failed to get hold of Volumes logic" << std::endl;
		return;
	      }

	  vtkMRMLScalarVolumeNode *outputVolume = NULL;
	    vtkMRMLScalarVolumeNode *svnode = vtkMRMLScalarVolumeNode::SafeDownCast(referenceVolume);
	    std::ostringstream outSS;

	    outSS << referenceVolume->GetName() << "-squaredDifference";
	    if(svnode)
	    {
	      outputVolume = this->Internal->VolumesLogic->CloneVolume(this->GetMRMLScene(), referenceVolume, outSS.str().c_str());
	    }
	    else
	    {
	      std::cerr << "Reference volume not scalar volume!" << std::endl;
	      return;
	    }

	  if ( !outputVolume ) {
		  vtkErrorMacro("SquaredDifference: No output volume set!");
		  return;
	  }
	  //Check dimensions of both volume, they must be the same.
	  vtkSmartPointer<vtkImageData> imageDataRef = referenceVolume->GetImageData();
	  vtkSmartPointer<vtkImageData> imageDataWarp = warpedVolume->GetImageData();
	    int* dimsRef = imageDataRef->GetDimensions();
	    int* dimsWarp = imageDataWarp->GetDimensions();
	  // int dims[3]; // can't do this
	  if (dimsRef[0] != dimsWarp[0] || dimsRef[1] != dimsWarp[1] || dimsRef[2] != dimsWarp[2] ) {
	    vtkErrorMacro("SquaredDifference: Dimensions of Reference and Warped image don't match'!");
	    return;
	  }

	  qSlicerCLIModule* checkerboardfilterCLI = dynamic_cast<qSlicerCLIModule*>(
			  qSlicerCoreApplication::application()->moduleManager()->module("SquaredDifference"));
	  QString cliModuleName("SquaredDifference");

	  vtkSmartPointer<vtkMRMLCommandLineModuleNode> cmdNode =
			  checkerboardfilterCLI->cliModuleLogic()->CreateNodeInScene();

	  // Set node parameters
	  cmdNode->SetParameterAsString("inputVolume1", referenceVolume->GetID());
	  cmdNode->SetParameterAsString("inputVolume2", warpedVolume->GetID());
	  cmdNode->SetParameterAsString("outputVolume", outputVolume->GetID());

	  // Execute synchronously so that we can check the content of the file after the module execution
	  checkerboardfilterCLI->cliModuleLogic()->ApplyAndWait(cmdNode);

	  this->GetMRMLScene()->RemoveNode(cmdNode);

	  outputVolume->SetAndObserveTransformNodeID(NULL);
	  this->RegistrationQualityNode->SetSquaredDiffVolumeNodeID(outputVolume->GetID());
	}


	vtkMRMLScalarVolumeNode *squaredDiffVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetSquaredDiffVolumeNodeID()));

	if ( !squaredDiffVolume ) {
		vtkErrorMacro("SquaredDifference: No output volume set!");
		return;
	}
	squaredDiffVolume->GetScalarVolumeDisplayNode()->AutoWindowLevelOff();
	int window=300;
	int level=200;

	squaredDiffVolume->GetScalarVolumeDisplayNode()->SetThreshold(0,3e3);
	squaredDiffVolume->GetScalarVolumeDisplayNode()->SetLevel(level);
	squaredDiffVolume->GetScalarVolumeDisplayNode()->SetWindow(window);
	squaredDiffVolume->GetDisplayNode()->SetAndObserveColorNodeID("vtkMRMLColorTableNodeRainbow");

	this->SetForegroundImage(referenceVolume,squaredDiffVolume,0.5);

	  // Get mean and std from squared difference volume
	double statisticValues[4];
	this->CalculateStatistics(squaredDiffVolume,statisticValues);

	this->RegistrationQualityNode->DisableModifiedEventOn();
	this->RegistrationQualityNode->SetSquaredDiffStatistics( statisticValues );
	this->RegistrationQualityNode->DisableModifiedEventOff();

	return;
}

void dump(TiXmlNode* n, int indent=0) {
	if(!n) return;
	int type = n->Type();
	for(int i=0; i<indent; i++) cout << " ";
	cout << "Type=" << type << endl;
	for(int i=0; i<indent; i++) cout << " ";
	cout << "Val=|" << n->Value() << "|" << endl;
// 	if(type==4) {
// 		cout << n->ToText()->Value() << endl;
// 	}
	for(TiXmlNode *child=n->FirstChild(); child!=0;child=child->NextSibling()) {
		dump(child, indent+1);
	}
}

bool vtkSlicerRegistrationQualityLogic::checkRegistrationIndices(
	std::vector<vtkSmartPointer<DIRQAImage> >& images,
	std::vector<vtkSmartPointer<DIRQAImage> >& warped) {

	// Check images for consistent reference-phase-indices
	std::sort(warped.begin(), warped.end(), DIRQAImage::compareOnFixedIndex);
	//images have to be and warped-reference-phases are sorted based on the same index
	// --> easier lookup of corresponding images
	std::vector<vtkSmartPointer<DIRQAImage> >::iterator correspondingImage = images.begin();
	for(std::vector<vtkSmartPointer<DIRQAImage> >::iterator it = warped.begin(); it!=warped.end(); ++it) {

		while(correspondingImage!=images.end()) {
			if((*correspondingImage)->getIndex() != (*it)->getFixedIndex()) {
				++correspondingImage;
			} else {
				break;
			}
		}

		if(correspondingImage==images.end()) {
			cout << "Error: Reference-image for " << **it << " does not exist." << endl;
			return false;
		} else {
			cout << "Info: Reference-image for " << **it << " exists." << endl;
		}
	}
// 	cout << "Info: Reference-images for all warped-images are OK." << endl;

	// Check warped-images for consistent indices (Not two warped versions of the same image/phase)
	// Corresponding image must exist
	std::sort(warped.begin(), warped.end(), DIRQAImage::compareOnIndex);
	int oldIndex=-1;
	//images and warped are sorted based on the same index --> easier lookup of corresponding images
	correspondingImage = images.begin();
	for(std::vector<vtkSmartPointer<DIRQAImage> >::iterator it = warped.begin(); it!=warped.end(); ++it) {
		int newIndex = (*it)->getIndex();
		if(newIndex-oldIndex <= 0) {
			cout << "Error: Warped-image-indices must be unique and >=0." << endl;
			return false;
		}

		while(correspondingImage!=images.end()) {
			if((*correspondingImage)->getIndex() != (*it)->getIndex()) {
				++correspondingImage;
			} else {
				break;
			}
		}

		if(correspondingImage==images.end()) {
			cout << "Error: No corresponding image for " << **it << "." << endl;
			return false;
		} else {
			cout << "Info: Corresponding image for " << **it << " exists." << endl;
		}
		oldIndex = newIndex;
	}
// 	cout << "Info: Warped-image-indices are OK." << endl;
	return true;
}

void vtkSlicerRegistrationQualityLogic::associateImagesToPhase(
	std::vector<vtkSmartPointer<DIRQAImage> >& images,
	std::vector<vtkSmartPointer<DIRQAImage> >& warped,
	std::vector<vtkMRMLSubjectHierarchyNode*>& phaseNodes,
	std::string shNodeTag) {
	uint32_t imageIndex = 0;

	// Associate warped images to corresponding phase-SH-nodes
	for(std::vector<vtkSmartPointer<DIRQAImage> >::iterator it = warped.begin(); it!=warped.end(); ++it) {

		// Find corresponding image-index
		while(imageIndex < images.size()) {
			if(images.at(imageIndex)->getIndex() != (*it)->getIndex()) {
				imageIndex++;
			} else {
				break;
			}
		}
		if(imageIndex >= images.size()) {
			throw std::logic_error("Error: This cannot happen! I just checked it.");
		}

		// phaseNodes has same order (and size) as images
		vtkMRMLSubjectHierarchyNode* currentWarpedImage = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(
			GetMRMLScene(), phaseNodes.at(imageIndex), NULL, shNodeTag.c_str(), NULL);
// 		(*it)->load(Internal->VolumesLogic); //TODO set path to load later
// 		currentWarpedImage->SetAssociatedNodeID((*it)->getNodeID().c_str());
	}
}

void vtkSlicerRegistrationQualityLogic::ReadRegistrationXML(/*vtkMRMLScene* scene*/) {

// 	TiXmlDocument doc("/home/brandtts/steeringfile.xml");
	TiXmlDocument doc(RegistrationQualityNode->GetXMLFileName().c_str());
	if(doc.LoadFile()) {
		// 		dump(&doc);
	} else {
		cout << "Fehler beim Laden" << endl;
		throw std::runtime_error("Unable to read XML-File");
	}

	vtkMRMLScene* scene = GetMRMLScene();

	// First accumulate all "images" etc. from file
	std::vector<vtkSmartPointer<DIRQAImage> > images;
	std::vector<vtkSmartPointer<DIRQAImage> > warped;
	std::vector<vtkSmartPointer<DIRQAImage> > vector;

	//Create SH-Node
// 	qSlicerSubjectHierarchyDefaultPlugin* shDefaultPlugin = qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin();
// 	vtkMRMLSubjectHierarchyNode* regSHNode = shDefaultPlugin->createChildNode(NULL,"Registration");

	TiXmlNode *child=doc.FirstChild();
	while(child!=0 && child->Type()!=TiXmlNode::TINYXML_ELEMENT) child=child->NextSibling();

	// Maps strings ("image", ...) to enum-values (IMAGE, ...)
	DIRQAImage::initHashMap();

	child=child->FirstChild(/*"image"*/);
	while(child!=0) {
		cout << "--constructor--" << endl;
		vtkSmartPointer<DIRQAImage> di = vtkSmartPointer<DIRQAImage>::New();
		di->readFromXML(*child);
		// 		di->load(Internal->VolumesLogic);
// 		if(di->getImageType()==DIRQAImage::IMAGE) {
// 			vtkMRMLSubjectHierarchyNode* phaseSHNode = shDefaultPlugin->createChildNode(regSHNode,di->getTag().c_str());
// 			vtkMRMLSubjectHierarchyNode* imageSHNode = shDefaultPlugin->createChildNode(phaseSHNode,"Image");
//
// 		}
		switch(di->getImageType()) {
			case DIRQAImage::IMAGE:
				images.push_back(di);
				break;
			case DIRQAImage::WARPED:
				warped.push_back(di);
				break;
			case DIRQAImage::VECTOR:
				vector.push_back(di);
				break;
			default:
				cout << "Currently not supported!" << endl;
				di->Delete();
				break;
		}


		child=child->NextSibling(/*"image"*/);
	}

	// Report all "images" read from file
	for(std::vector<vtkSmartPointer<DIRQAImage> >::iterator it = images.begin(); it!=images.end(); ++it) {
		cout << "|" << **it << "|" << endl;
	}
	for(std::vector<vtkSmartPointer<DIRQAImage> >::iterator it = warped.begin(); it!=warped.end(); ++it) {
		cout << "|" << **it << "|" << endl;
	}
	for(std::vector<vtkSmartPointer<DIRQAImage> >::iterator it = vector.begin(); it!=vector.end(); ++it) {
		cout << "|" << **it << "|" << endl;
	}

	if(images.empty()) {
		cout << "No images found in xml-file. Nothing to do.";
		return;
	}

	{
		// Check images for consistent indices
		std::sort(images.begin(), images.end(), DIRQAImage::compareOnIndex);
		int oldIndex=-1;
		for(std::vector<vtkSmartPointer<DIRQAImage> >::iterator it = images.begin(); it!=images.end(); ++it) {
			int newIndex = (*it)->getIndex();
			if(newIndex-oldIndex <= 0) {
				cout << "Error: Image-indices must be unique and >=0." << endl;
				return;
			}
			oldIndex = newIndex;
		}
		if( (uint32_t) (images.back()->getIndex() - images.front()->getIndex()) >= images.size()) {
			cout << "Warning: Image-indices are not contigous." << endl;
		} else {
			cout << "Info: Image-indices are OK." << endl;
		}
	}

	if(checkRegistrationIndices(images, warped)) {
		cout << "Info: Warped-image-indices are OK." << endl;
	} else {
		return;
	}

	if(checkRegistrationIndices(images, vector)) {
		cout << "Info: Vector-image-indices are OK." << endl;
	} else {
		return;
	}

	// Don't need SmartPointers here (Nodes are created within the scene)
	vtkMRMLSubjectHierarchyNode* registrationSHNode = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(
		scene, NULL, vtkSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_SUBJECT, "Registration", NULL);

	// Each image defines one phase. Phase name like image tag
	std::vector<vtkMRMLSubjectHierarchyNode*> phaseNodes;
	for(std::vector<vtkSmartPointer<DIRQAImage> >::iterator it = images.begin(); it!=images.end(); ++it) {
		vtkMRMLSubjectHierarchyNode* currentPhase = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(
			scene, registrationSHNode, vtkSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_STUDY,
			(*it)->getTag().c_str(), NULL
		);
		phaseNodes.push_back(currentPhase);

		vtkMRMLSubjectHierarchyNode* currentImage = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(
			scene, currentPhase, NULL, "Image", NULL);
// 		(*it)->load(Internal->VolumesLogic); //TODO set path to load later
// 		currentImage->SetAssociatedNodeID((*it)->getNodeID().c_str());
	}

	associateImagesToPhase(images, warped, phaseNodes, "Warped");
	associateImagesToPhase(images, vector, phaseNodes, "Vector");

	cout << "registrationSHNode->GetID() = " << registrationSHNode->GetID() << endl;
	this->RegistrationQualityNode->SetAndObserveSubjectHierarchyNodeID(registrationSHNode->GetID());
}

QStandardItemModel* vtkSlicerRegistrationQualityLogic::getTreeViewModel() {
	return NULL/*subjectModel*/;
}


//--- Image Checks -----------------------------------------------------------
void vtkSlicerRegistrationQualityLogic::FalseColor(int state) {
	if (!this->GetMRMLScene() || !this->RegistrationQualityNode) {
		vtkErrorMacro("Invalid scene or parameter set node!");
		throw std::runtime_error("Internal Error, see command line!");
	}

// 	ReadRegistrationXML(/*GetMRMLScene()*/);

	vtkMRMLScalarVolumeNode *referenceVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetReferenceVolumeNodeID()));

	vtkMRMLScalarVolumeNode *warpedVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetWarpedVolumeNodeID()));

	if (!referenceVolume || !warpedVolume) {
		throw std::runtime_error("Reference or warped volume not set!");
	}

	//TODO: Volumes go back to gray value - perhaps we should rembemer previous color settings?
	if (!state) {
		SetDefaultDisplay(referenceVolume,warpedVolume);
		return;
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
	      	this->SetDefaultDisplay(referenceVolume,warpedVolume);
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
		std::ostringstream outSS;
		outSS << referenceVolume->GetName() << "_CheckerboardPattern_"<< PatternValue;
		vtkMRMLScalarVolumeNode *outputVolume = this->Internal->VolumesLogic->CloneVolume(
			this->GetMRMLScene(), referenceVolume, outSS.str().c_str());
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
void vtkSlicerRegistrationQualityLogic::Jacobian(int state) {

	if (!this->GetMRMLScene() || !this->RegistrationQualityNode) {
		vtkErrorMacro("Invalid scene or parameter set node!");
		throw std::runtime_error("Internal Error, see command line!");
	}

	vtkMRMLScalarVolumeNode *referenceVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetReferenceVolumeNodeID()));
	vtkMRMLVectorVolumeNode *vectorVolume = vtkMRMLVectorVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetVectorVolumeNodeID()));

	if (!vectorVolume || !referenceVolume ) {
		throw std::runtime_error("Reference volume or vector field not set!");
	}

	if (!state) {
		vtkMRMLScalarVolumeNode *warpedVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetWarpedVolumeNodeID()));
		this->SetDefaultDisplay(referenceVolume,warpedVolume);
		return;
	}

	if (!this->RegistrationQualityNode->GetJacobianVolumeNodeID()){

		if(!this->Internal->VolumesLogic) {
			vtkErrorMacro("Failed to get hold of Volumes logic!");
			throw std::runtime_error("Internal Error, see command line!");
		}

		std::ostringstream outSS;
		outSS << referenceVolume->GetName() << "_Jacobian";

		vtkMRMLScalarVolumeNode *outputVolume = this->Internal->VolumesLogic->CloneVolume(
			this->GetMRMLScene(), referenceVolume, outSS.str().c_str());

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

		// Execute synchronously so that we can check the content of the file after the module execution
		jacobianfilterCLI->cliModuleLogic()->ApplyAndWait(cmdNode);

		this->GetMRMLScene()->RemoveNode(cmdNode);

		outputVolume->SetAndObserveTransformNodeID(NULL);
		this->RegistrationQualityNode->SetJacobianVolumeNodeID(outputVolume->GetID());

	}

	vtkMRMLScalarVolumeNode *jacobianVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
		this->GetMRMLScene()->GetNodeByID(
			this->RegistrationQualityNode->GetJacobianVolumeNodeID()));

	jacobianVolume->GetDisplayNode()->SetAndObserveColorNodeID("vtkMRMLColorTableNodeRainbow");
	jacobianVolume->GetScalarVolumeDisplayNode()->AutoWindowLevelOff();
	double window=0.8;
	int level=1;

	jacobianVolume->GetScalarVolumeDisplayNode()->SetThreshold(0,3);
	jacobianVolume->GetScalarVolumeDisplayNode()->SetLevel(level);
	jacobianVolume->GetScalarVolumeDisplayNode()->SetWindow(window);

	this->SetForegroundImage(referenceVolume,jacobianVolume,0.5);

	double statisticValues[4]; // 1. Mean 2. STD 3. Max 4. Min
	this->CalculateStatistics(jacobianVolume,statisticValues);

	this->RegistrationQualityNode->DisableModifiedEventOn();
	this->RegistrationQualityNode->SetJacobianStatistics( statisticValues );
	// 	this->RegistrationQualityNode->SetJacobianSTD( statisticValues[1] );
	this->RegistrationQualityNode->DisableModifiedEventOff();

	return;
}
//----------------------------------------------------------------------------
void vtkSlicerRegistrationQualityLogic::InverseConsist(int state) {

	if (!this->GetMRMLScene() || !this->RegistrationQualityNode) {
		vtkErrorMacro("Inverse Consistency: Invalid scene or parameter set node!");
		return;
	}

	vtkMRMLVectorVolumeNode *vectorVolume1 = vtkMRMLVectorVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetVectorVolumeNodeID()));
	vtkMRMLVectorVolumeNode *vectorVolume2 = vtkMRMLVectorVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetInvVectorVolumeNodeID()));
	vtkMRMLScalarVolumeNode *referenceVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetReferenceVolumeNodeID()));

// 	vtkMRMLScalarVolumeNode *warpedVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
// 			this->GetMRMLScene()->GetNodeByID(
// 				this->RegistrationQualityNode->GetWarpedVolumeNodeID()));
// 	vtkMRMLVolumeNode *outputVolume = vtkMRMLVolumeNode::SafeDownCast(
// 			this->GetMRMLScene()->GetNodeByID(
// 				this->RegistrationQualityNode->GetCheckerboardVolumeNodeID()));
//
	if (!vectorVolume1 || !vectorVolume2 || !referenceVolume ) {
	    std::cerr << "Volumes not set!" << std::endl;
	    return;
	}

	if (!state) {

	  vtkMRMLScalarVolumeNode *warpedVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetWarpedVolumeNodeID()));
	      	this->SetDefaultDisplay(referenceVolume,warpedVolume);
		return;
	}

	if (!this->RegistrationQualityNode->GetInverseConsistVolumeNodeID()){
	  if(!this->Internal->VolumesLogic)
	    {
	      std::cerr << "Inverse Consistency: ERROR: failed to get hold of Volumes logic" << std::endl;
	      return;
	    }


	  vtkMRMLScalarVolumeNode *outputVolume = NULL;
	  vtkMRMLScalarVolumeNode *svnode = vtkMRMLScalarVolumeNode::SafeDownCast(referenceVolume);
	  std::ostringstream outSS;

	  outSS << referenceVolume->GetName() << "-inverseConsist";
	  if(svnode)
	  {
	    outputVolume = this->Internal->VolumesLogic->CloneVolume(this->GetMRMLScene(), referenceVolume, outSS.str().c_str());
	  }
	  else
	  {
	    std::cerr << "Reference volume not scalar volume!" << std::endl;
	    return;
	  }


	qSlicerCLIModule* checkerboardfilterCLI = dynamic_cast<qSlicerCLIModule*>(
			qSlicerCoreApplication::application()->moduleManager()->module("InverseConsistency"));
	QString cliModuleName("InverseConsistency");

	vtkSmartPointer<vtkMRMLCommandLineModuleNode> cmdNode =
			checkerboardfilterCLI->cliModuleLogic()->CreateNodeInScene();

	// Set node parameters
	cmdNode->SetParameterAsString("inputVolume1", vectorVolume1->GetID());
	cmdNode->SetParameterAsString("inputVolume2", vectorVolume2->GetID());
	cmdNode->SetParameterAsString("outputVolume", outputVolume->GetID());

	// Execute synchronously so that we can check the content of the file after the module execution
	checkerboardfilterCLI->cliModuleLogic()->ApplyAndWait(cmdNode);

	this->GetMRMLScene()->RemoveNode(cmdNode);

	outputVolume->SetAndObserveTransformNodeID(NULL);
	this->RegistrationQualityNode->SetInverseConsistVolumeNodeID(outputVolume->GetID());
	}

	vtkMRMLScalarVolumeNode *inverseConsistVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
			this->GetMRMLScene()->GetNodeByID(
				this->RegistrationQualityNode->GetInverseConsistVolumeNodeID()));


	inverseConsistVolume->GetDisplayNode()->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGreen");
	inverseConsistVolume->GetScalarVolumeDisplayNode()->AutoWindowLevelOff();
	double window=10;
	int level=5;

	inverseConsistVolume->GetScalarVolumeDisplayNode()->SetThreshold(0,10);
	inverseConsistVolume->GetScalarVolumeDisplayNode()->SetLevel(level);
	inverseConsistVolume->GetScalarVolumeDisplayNode()->SetWindow(window);

	this->SetForegroundImage(referenceVolume,inverseConsistVolume,0.5);

	double statisticValues[4]; // 1. Mean 2. STD 3. Max 4. Min
	this->CalculateStatistics(inverseConsistVolume,statisticValues);

	this->RegistrationQualityNode->DisableModifiedEventOn();
	this->RegistrationQualityNode->SetInverseConsistStatistics( statisticValues );
	this->RegistrationQualityNode->DisableModifiedEventOff();

	return;
}
//--- Default mode when checkbox is unchecked -----------------------------------------------------------
void vtkSlicerRegistrationQualityLogic::SetDefaultDisplay(vtkMRMLScalarVolumeNode *backgroundVolume, vtkMRMLScalarVolumeNode *foregroundVolume) {
	if (!this->GetMRMLScene() || !this->RegistrationQualityNode) {
		vtkErrorMacro("SetDefaultDisplay: Invalid scene or parameter set node!");
		return;
	}


	if (!backgroundVolume || !foregroundVolume) {
		// 		vtkErrorMacro("SetDefaultDisplay: Invalid volumes!");
		this->SetForegroundImage(backgroundVolume,foregroundVolume,0.5);
		return;
	}
	//TODO: Volumes go back to gray value - perhaps we should rembemer previous color settings?
	backgroundVolume->GetDisplayNode()->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey");
	foregroundVolume->GetDisplayNode()->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey");

	// Set window and level the same for warped and reference volume.
	foregroundVolume->GetScalarVolumeDisplayNode()->AutoWindowLevelOff();
	// 	double window, level;
	// 	window = backgroundVolume->GetScalarVolumeDisplayNode()->GetWindow();
	// 	level = backgroundVolume->GetScalarVolumeDisplayNode()->GetLevel();
	foregroundVolume->GetScalarVolumeDisplayNode()->SetWindow(backgroundVolume->GetScalarVolumeDisplayNode()->GetWindow());
	foregroundVolume->GetScalarVolumeDisplayNode()->SetLevel(backgroundVolume->GetScalarVolumeDisplayNode()->GetLevel());
	this->SetForegroundImage(backgroundVolume,foregroundVolume,0.5);

	return;
}

//---Calculate Statistic of image-----------------------------------
void vtkSlicerRegistrationQualityLogic
::CalculateStatistics(vtkMRMLScalarVolumeNode *inputVolume, double statisticValues[4] ) {
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
