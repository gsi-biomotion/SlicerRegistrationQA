// SlicerQt includes
#include "qSlicerRegistrationQualityModuleWidget.h"
#include "ui_qSlicerRegistrationQualityModule.h"
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"


// Qt includes
#include <QProgressDialog>
#include <QMainWindow>
#include <QDebug>
#include <QFileDialog>
#include <QCheckBox>
#include <QTimer>
#include <QInputDialog>
#include <QLineEdit>
#include <QStandardItemModel>
#include <QMenu>


// SlicerQt includes
#include <qSlicerAbstractCoreModule.h>

// DeformationVisualizer includes
#include "vtkSlicerRegistrationQualityLogic.h"
#include "vtkMRMLRegistrationQualityNode.h"

// SlicerRT Isodose includes for legend
#include "vtkSlicerRTScalarBarActor.h"

// MMRL includes
#include <vtkMRMLScene.h>
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLVectorVolumeNode.h>
#include <vtkMRMLVectorVolumeDisplayNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLBSplineTransformNode.h>
#include <vtkMRMLGridTransformNode.h>
#include <vtkMRMLTransformDisplayNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLColorTableNode.h>

// qMRMLWidget includes
#include "qMRMLSliceWidget.h"
#include "qMRMLSliceView.h"


// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkNew.h>
#include <vtkDataArray.h>
#include <vtkGeneralTransform.h>
#include <vtkSmartPointer.h>
#include <vtkScalarBarWidget.h>
#include <vtkLookupTable.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkColorTransferFunction.h>

//C include
#include <time.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_RegistrationQuality
// TODO: Keeping private for now until after fixes and enhancements
class qSlicerRegistrationQualityModuleWidgetPrivate: public Ui_qSlicerRegistrationQualityModule {
	Q_DECLARE_PUBLIC(qSlicerRegistrationQualityModuleWidget);
protected:
	qSlicerRegistrationQualityModuleWidget* const q_ptr;
public:
	qSlicerRegistrationQualityModuleWidgetPrivate(qSlicerRegistrationQualityModuleWidget& object);
	~qSlicerRegistrationQualityModuleWidgetPrivate();
	vtkSlicerRegistrationQualityLogic* logic() const;
	void updateScalarBarsFromSelectedColorTable();
	
	vtkScalarBarWidget* ScalarBarWidget2DRed;
	vtkScalarBarWidget* ScalarBarWidget2DYellow;
	vtkScalarBarWidget* ScalarBarWidget2DGreen;
	
	vtkSlicerRTScalarBarActor* ScalarBarActor2DRed;
	vtkSlicerRTScalarBarActor* ScalarBarActor2DYellow;
	vtkSlicerRTScalarBarActor* ScalarBarActor2DGreen;
};

//-----------------------------------------------------------------------------
// qSlicerRegistrationQualityModuleWidgetPrivate methods
//-----------------------------------------------------------------------------
qSlicerRegistrationQualityModuleWidgetPrivate::qSlicerRegistrationQualityModuleWidgetPrivate(
	qSlicerRegistrationQualityModuleWidget& object) : q_ptr(&object) {
	
	this->ScalarBarWidget2DRed = vtkScalarBarWidget::New();
	this->ScalarBarActor2DRed = vtkSlicerRTScalarBarActor::New();
	this->ScalarBarWidget2DRed->SetScalarBarActor(this->ScalarBarActor2DRed);
	this->ScalarBarWidget2DRed->GetScalarBarActor()->SetOrientationToVertical();
	this->ScalarBarWidget2DRed->GetScalarBarActor()->SetNumberOfLabels(6);
	this->ScalarBarWidget2DRed->GetScalarBarActor()->SetMaximumNumberOfColors(256);
	this->ScalarBarWidget2DRed->GetScalarBarActor()->SetTitle("Legend:");
	this->ScalarBarWidget2DRed->GetScalarBarActor()->SetLabelFormat(" %s");
  
	// it's a 2d actor, position it in screen space by percentages
	this->ScalarBarWidget2DRed->GetScalarBarActor()->SetPosition(0.1, 0.1);
	this->ScalarBarWidget2DRed->GetScalarBarActor()->SetWidth(0.1);
	this->ScalarBarWidget2DRed->GetScalarBarActor()->SetHeight(0.8);

	this->ScalarBarWidget2DYellow = vtkScalarBarWidget::New();
	this->ScalarBarActor2DYellow = vtkSlicerRTScalarBarActor::New();
	this->ScalarBarWidget2DYellow->SetScalarBarActor(this->ScalarBarActor2DYellow);
	this->ScalarBarWidget2DYellow->GetScalarBarActor()->SetOrientationToVertical();
	this->ScalarBarWidget2DYellow->GetScalarBarActor()->SetNumberOfLabels(6);
	this->ScalarBarWidget2DYellow->GetScalarBarActor()->SetMaximumNumberOfColors(256);
	this->ScalarBarWidget2DYellow->GetScalarBarActor()->SetTitle("Legend:");
	this->ScalarBarWidget2DYellow->GetScalarBarActor()->SetLabelFormat(" %s");

	// it's a 2d actor, position it in screen space by percentages
	this->ScalarBarWidget2DYellow->GetScalarBarActor()->SetPosition(0.1, 0.1);
	this->ScalarBarWidget2DYellow->GetScalarBarActor()->SetWidth(0.1);
	this->ScalarBarWidget2DYellow->GetScalarBarActor()->SetHeight(0.8);

	this->ScalarBarWidget2DGreen = vtkScalarBarWidget::New();
	this->ScalarBarActor2DGreen = vtkSlicerRTScalarBarActor::New();
	this->ScalarBarWidget2DGreen->SetScalarBarActor(this->ScalarBarActor2DGreen);
	this->ScalarBarWidget2DGreen->GetScalarBarActor()->SetOrientationToVertical();
	this->ScalarBarWidget2DGreen->GetScalarBarActor()->SetNumberOfLabels(6);
	this->ScalarBarWidget2DGreen->GetScalarBarActor()->SetMaximumNumberOfColors(256);
	this->ScalarBarWidget2DGreen->GetScalarBarActor()->SetTitle("Legend:");
	this->ScalarBarWidget2DGreen->GetScalarBarActor()->SetLabelFormat(" %s");

	// it's a 2d actor, position it in screen space by percentages
	this->ScalarBarWidget2DGreen->GetScalarBarActor()->SetPosition(0.1, 0.1);
	this->ScalarBarWidget2DGreen->GetScalarBarActor()->SetWidth(0.1);
	this->ScalarBarWidget2DGreen->GetScalarBarActor()->SetHeight(0.8);
}
//-----------------------------------------------------------------------------
qSlicerRegistrationQualityModuleWidgetPrivate::~qSlicerRegistrationQualityModuleWidgetPrivate()
{
  if (this->ScalarBarWidget2DRed)
  {
    this->ScalarBarWidget2DRed->Delete();
    this->ScalarBarWidget2DRed = 0;
  }
  if (this->ScalarBarActor2DRed)
  {
    this->ScalarBarActor2DRed->Delete();
    this->ScalarBarActor2DRed = 0;
  }
  if (this->ScalarBarWidget2DYellow)
  {
    this->ScalarBarWidget2DYellow->Delete();
    this->ScalarBarWidget2DYellow = 0;
  }
  if (this->ScalarBarActor2DYellow)
  {
    this->ScalarBarActor2DYellow->Delete();
    this->ScalarBarActor2DYellow = 0;
  }
  if (this->ScalarBarWidget2DGreen)
  {
    this->ScalarBarWidget2DGreen->Delete();
    this->ScalarBarWidget2DGreen = 0;
  }
  if (this->ScalarBarActor2DGreen)
  {
    this->ScalarBarActor2DGreen->Delete();
    this->ScalarBarActor2DGreen = 0;
  }
}
//-----------------------------------------------------------------------------
vtkSlicerRegistrationQualityLogic* qSlicerRegistrationQualityModuleWidgetPrivate::logic() const {
	Q_Q( const qSlicerRegistrationQualityModuleWidget );
	return vtkSlicerRegistrationQualityLogic::SafeDownCast( q->logic() );
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidgetPrivate::updateScalarBarsFromSelectedColorTable(){
	Q_Q(qSlicerRegistrationQualityModuleWidget);
	
	vtkMRMLRegistrationQualityNode* paramNode = this->logic()->GetRegistrationQualityNode();
	if (!q->mrmlScene() || !paramNode){
		return;
	}
	vtkMRMLColorTableNode* selectedColorNode = paramNode->GetColorTableNode();
	if (!selectedColorNode){
		qDebug() << "qSlicerRegistrationQualityModuleWidgetPrivate::updateScalarBarsFromSelectedColorTable: No color table node is selected";
		return;
	}

	int numberOfColors = selectedColorNode->GetNumberOfColors();
	// 2D scalar bar
	this->ScalarBarActor2DRed->SetLookupTable(selectedColorNode->GetLookupTable());
	this->ScalarBarActor2DYellow->SetLookupTable(selectedColorNode->GetLookupTable());
	this->ScalarBarActor2DGreen->SetLookupTable(selectedColorNode->GetLookupTable());

// 	for (int i=0; i<7; ++i){
// 		int colorIndex = i*255/6;
	for (int colorIndex=0; colorIndex<numberOfColors; ++colorIndex){
		#if (VTK_MAJOR_VERSION <= 5)
		this->ScalarBarActor2DRed->SetColorName(colorIndex, selectedColorNode->GetColorName(colorIndex));
		this->ScalarBarActor2DYellow->SetColorName(colorIndex, selectedColorNode->GetColorName(colorIndex));
		this->ScalarBarActor2DGreen->SetColorName(colorIndex, selectedColorNode->GetColorName(colorIndex));
		#else
		this->ScalarBarActor2DRed->GetLookupTable()->SetAnnotation(colorIndex, vtkStdString(selectedColorNode->GetColorName(colorIndex)));
		this->ScalarBarActor2DYellow->GetLookupTable()->SetAnnotation(colorIndex, vtkStdString(selectedColorNode->GetColorName(colorIndex)));
		this->ScalarBarActor2DGreen->GetLookupTable()->SetAnnotation(colorIndex, vtkStdString(selectedColorNode->GetColorName(colorIndex)));
		#endif
	}
}
//-----------------------------------------------------------------------------
// qSlicerRegistrationQualityModuleWidget methods
//-----------------------------------------------------------------------------
qSlicerRegistrationQualityModuleWidget::qSlicerRegistrationQualityModuleWidget(QWidget* _parent)
	: Superclass( _parent )
	, d_ptr(new qSlicerRegistrationQualityModuleWidgetPrivate(*this))
	, flickerTimer(new QTimer(this)) {
}

//-----------------------------------------------------------------------------
qSlicerRegistrationQualityModuleWidget::~qSlicerRegistrationQualityModuleWidget() {
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setMRMLScene(vtkMRMLScene* scene) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	this->Superclass::setMRMLScene(scene);

	// Find parameters node or create it if there is no one in the scene
	if (scene &&  d->logic()->GetRegistrationQualityNode() == 0) {
		vtkMRMLNode* node = scene->GetNthNodeByClass(0, "vtkMRMLRegistrationQualityNode");
		if (node) {
			this->setRegistrationQualityParametersNode(
				vtkMRMLRegistrationQualityNode::SafeDownCast(node));
		}
	}
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::onSceneImportedEvent() {
	this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::enter() {
	this->onEnter();
	this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::onEnter() {
	if (!this->mrmlScene()) {
		return;
	}

	Q_D(qSlicerRegistrationQualityModuleWidget);

	if (d->logic() == NULL) {
		return;
	}

	//Check for existing parameter node
	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();

	if (pNode == NULL) {
		vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass(0,"vtkMRMLRegistrationQualityNode");
		if (node) {
			pNode = vtkMRMLRegistrationQualityNode::SafeDownCast(node);
			d->logic()->SetAndObserveRegistrationQualityNode(pNode);
			return;
		} else {
			vtkSmartPointer<vtkMRMLRegistrationQualityNode> newNode = vtkSmartPointer<vtkMRMLRegistrationQualityNode>::New();
			this->mrmlScene()->AddNode(newNode);
			d->logic()->SetAndObserveRegistrationQualityNode(newNode);
		}
	}
	this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setRegistrationQualityParametersNode(vtkMRMLNode *node) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = vtkMRMLRegistrationQualityNode::SafeDownCast(node);

	qvtkReconnect( d->logic()->GetRegistrationQualityNode(), pNode,
				   vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));

	d->logic()->SetAndObserveRegistrationQualityNode(pNode);

	
	if (pNode){
		// Set default color table ID if none specified yet
		if (!pNode->GetColorTableNode()){
			vtkMRMLColorTableNode* selectedColorNode = vtkMRMLColorTableNode::SafeDownCast(this->mrmlScene()
					  ->GetNodeByID("vtkMRMLColorTableNodeRainbow"));
			if (selectedColorNode){
				pNode->SetAndObserveColorTableNode(selectedColorNode);
			}
		}

	}
	this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::updateWidgetFromMRML() {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();

	if (pNode && this->mrmlScene()) {
		d->ParameterComboBox->setCurrentNode(pNode);

// 		if (pNode->GetVectorVolumeNodeID()) {
// 			d->InputFieldComboBox->setCurrentNode(pNode->GetVectorVolumeNodeID());
// 		} else {
// 			this->vectorVolumeChanged(d->InputFieldComboBox->currentNode());
// 		}
// 		if (pNode->GetInvVectorVolumeNodeID()) {
// 			d->InputInvFieldComboBox->setCurrentNode(pNode->GetInvVectorVolumeNodeID());
// 		} else {
// 			this->invVectorVolumeChanged(d->InputInvFieldComboBox->currentNode());
// 		}
// 
// 		if (pNode->GetFixedVolumeNodeID()) {
// 			d->InputReferenceComboBox->setCurrentNode(pNode->GetFixedVolumeNodeID());
// 		} else {
// 			this->referenceVolumeChanged(d->InputReferenceComboBox->currentNode());
// 		}
// 
// 		if (pNode->GetWarpedVolumeNodeID()) {
// 			d->InputWarpedComboBox->setCurrentNode(pNode->GetWarpedVolumeNodeID());
// 		} else {
// 			this->warpedVolumeChanged(d->InputWarpedComboBox->currentNode());
// 		}
// 		
// 		if (pNode->GetROINodeID()) {
// 			d->ROIInputComboBox->setCurrentNode(pNode->GetROINodeID());
//                 } else {
//                         this->ROIChanged(d->ROIInputComboBox->currentNode());
//                 }
// 
// 		if (pNode->GetFixedFiducialNodeID()) {
// 			d->FiducialInputComboBox->setCurrentNode(pNode->GetFixedFiducialNodeID());
// 		} else {
// 			this->fiducialChanged(d->FiducialInputComboBox->currentNode());
// 		}
// 		
// 		if (pNode->GetMovingFiducialNodeID()) {
// 			d->InvFiducialInputComboBox->setCurrentNode(pNode->GetMovingFiducialNodeID());
// 		} else {
// 			this->invFiducialChanged(d->InvFiducialInputComboBox->currentNode());
// 		}
// 
// 
// 		d->movieBoxRed->setChecked(pNode->GetMovieBoxRedState());
// 		//Update Visualization Parameters
// 		d->CheckerboardPattern->setValue(pNode->GetCheckerboardPattern());
	}
	
	d->updateScalarBarsFromSelectedColorTable();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::onLogicModified() {
	this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::vectorVolumeChanged(vtkMRMLNode* node) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	// TODO: Move into updatefrommrml?
	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene() || !node) {
		return;
	}

	d->jacobianButton->setEnabled(true);
	d->consistencyButton->setEnabled(true);

	pNode->DisableModifiedEventOn();
	if (node->IsA("vtkMRMLTransformNode")){
             pNode->SetAndObserveTransformNodeID(node->GetID());
             
//              //Convert transform to vector
//              if ( !pNode->GetVectorVolumeNodeID()){
//                 vtkMRMLTransformNode *transform = vtkMRMLTransformNode::SafeDownCast(
//                                           this->mrmlScene()->GetNodeByID(
//                                                 node->GetID()));
//                 if (transform) {
//                   vtkMRMLVectorVolumeNode *vectorVolume = d->logic()->CreateVectorFromTransform(transform);
//                   if (vectorVolume){
//                     pNode->SetAndObserveVectorVolumeNodeID(vectorVolume->GetID());
//                   }
//                 }
//                 else{
//                   std::cerr << "Widget: Transform not set, no creation of vector volume." << pNode->GetTransformNodeID() << std::endl;
//                 }  
//              }
        }
	else if (node->IsA("vtkMRMLVectorVolumeNode")){
	  pNode->SetAndObserveVectorVolumeNodeID(node->GetID());

// 	  //Convert transform to vector
//           if ( !pNode->GetTransformNodeID() ){
//              vtkMRMLVectorVolumeNode *vectorVolume= vtkMRMLVectorVolumeNode::SafeDownCast(
//                                              this->mrmlScene()->GetNodeByID(
//                                                    node->GetID()));
//              if (vectorVolume) {
//                vtkMRMLGridTransformNode *transform = d->logic()->CreateTransformFromVector(vectorVolume);
//                if( transform ){
//                  pNode->SetAndObserveTransformNodeID(transform->GetID());
//                }
//              }
//              else{
//                std::cerr << "Widget: Vector not set, no creation of transform." << pNode->GetTransformNodeID() << std::endl;
//              }
//           }
	}
	pNode->DisableModifiedEventOff();
	
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::invVectorVolumeChanged(vtkMRMLNode* node) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	// TODO: Move into updatefrommrml?
	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene() || !node) {
		return;
	}

	d->consistencyButton->setEnabled(true);


	pNode->DisableModifiedEventOn();
	if (node->IsA("vtkMRMLTransformNode")){
	  pNode->SetAndObserveMovingTransformNodeID(node->GetID());
	  	  
// 	  //Convert transform to vector
//           if ( !pNode->GetVectorVolumeNodeID()){
//              vtkMRMLTransformNode *transform = vtkMRMLTransformNode::SafeDownCast(
//                                        this->mrmlScene()->GetNodeByID(
//                                              node->GetID()));
//              if (transform) {
//                vtkMRMLVectorVolumeNode *vectorVolume = d->logic()->CreateVectorFromTransform(transform);
//                if (vectorVolume){
//                  pNode->SetAndObserveVectorVolumeNodeID(vectorVolume->GetID());
//                }
//              }
//              else{
//                std::cerr << "Widget: Transform not set, no creation of vector volume." << pNode->GetTransformNodeID() << std::endl;
//              }
//           }  
	}
	else if (node->IsA("vtkMRMLVectorVolumeNode")){
	  pNode->SetAndObserveInvVectorVolumeNodeID(node->GetID());
	  
// 	  //Convert transform to vector
//           if ( ! pNode->GetMovingTransformNodeID() ) {
//              vtkMRMLVectorVolumeNode *vectorVolume= vtkMRMLVectorVolumeNode::SafeDownCast(
//                                              this->mrmlScene()->GetNodeByID(
//                                                    node->GetID()));
//              if (vectorVolume) {
//                vtkMRMLGridTransformNode *transform = d->logic()->CreateTransformFromVector(vectorVolume);
//                if( transform ){
//                  pNode->SetAndObserveMovingTransformNodeID(transform->GetID());
//                }
//              }
//              else{
//                std::cerr << "Widget: Vector not set, no creation of transform." << pNode->GetTransformNodeID() << std::endl;
//              }
//           }
	}
	pNode->DisableModifiedEventOff();
	// Calculate fiducial distance if possible

}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::referenceVolumeChanged(vtkMRMLNode* node) {

	Q_D(qSlicerRegistrationQualityModuleWidget);

	//TODO: Move into updatefrommrml?
	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();

// 	//TODO: Check somewhere if referenceVolume or warpedVolume exist
// 	vtkSlicerRegistrationQualityLogic *logic = d->logic();
// 	logic->ImageDifference();

	if (!pNode || !this->mrmlScene() || !node) {
		return;
	}

	pNode->DisableModifiedEventOn();
	pNode->SetAndObserveFixedVolumeNodeID(node->GetID());
	pNode->DisableModifiedEventOff();

	bool currentState;
	vtkSmartPointer<vtkMRMLVolumeNode> warpedVolumeNode = vtkMRMLVolumeNode::SafeDownCast(this->mrmlScene()->GetNodeByID(pNode->GetWarpedVolumeNodeID()));
	if (warpedVolumeNode == NULL) {
	    currentState=false;
	}
	else{
	  currentState=true;
	}

	d->AbsoluteDiffPushButton->setEnabled(currentState);
	d->FalseColorPushButton->setEnabled(currentState);
	d->CheckerboardPushButton->setEnabled(currentState);
	d->MovieToggle->setEnabled(currentState);
	d->FlickerToggle->setEnabled(currentState);
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::warpedVolumeChanged(vtkMRMLNode* node) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	//TODO: Move into updatefrommrml?
	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene() || !node) {
		return;
	}

	pNode->DisableModifiedEventOn();
	pNode->SetAndObserveWarpedVolumeNodeID(node->GetID());
	pNode->DisableModifiedEventOff();

	bool currentState;
	vtkSmartPointer<vtkMRMLVolumeNode> referenceVolumeNode = vtkMRMLVolumeNode::SafeDownCast(this->mrmlScene()->GetNodeByID(pNode->GetFixedVolumeNodeID()));
	if (referenceVolumeNode == NULL) {
	    currentState=false;
	}
	else{
	  currentState=true;
	}

	d->AbsoluteDiffPushButton->setEnabled(currentState);
	d->FalseColorPushButton->setEnabled(currentState);
	d->CheckerboardPushButton->setEnabled(currentState);
	d->MovieToggle->setEnabled(currentState);
	d->FlickerToggle->setEnabled(currentState);
// 	vtkSlicerRegistrationQualityLogic *logic = d->logic();
// 	logic->ImageDifference();
}


//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::outputModelChanged(vtkMRMLNode* node) {
        Q_D(qSlicerRegistrationQualityModuleWidget);

        //TODO: Move into updatefrommrml?
        vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
        if (!pNode || !this->mrmlScene() || !node) {
                return;
        }

        pNode->DisableModifiedEventOn();
        pNode->SetAndObserveOutputModelNodeID(node->GetID());
        pNode->DisableModifiedEventOff();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::outputDirectoyChanged() {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	//TODO: Move into updatefrommrml?
	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	QString fileName = QFileDialog::getExistingDirectory(NULL, QString( tr("Set directory for output file") ));
	
	if (fileName.isNull()) {
		return;
	}

	pNode->DisableModifiedEventOn();
	pNode->SetAndObserveOutputDirectory(fileName.toAscii().data());
	pNode->DisableModifiedEventOff();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::ROIChanged(vtkMRMLNode* node) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	//TODO: Move into updatefrommrml?
	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene() || !node) {
		return;
	}

	pNode->DisableModifiedEventOn();
	pNode->SetAndObserveROINodeID(node->GetID());
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::fiducialChanged(vtkMRMLNode* node) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	//TODO: Move into updatefrommrml?
	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene() || !node) {
		return;
	}

	pNode->DisableModifiedEventOn();
	pNode->SetAndObserveFixedFiducialNodeID(node->GetID());
	pNode->DisableModifiedEventOff();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::invFiducialChanged(vtkMRMLNode* node) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	//TODO: Move into updatefrommrml?
	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene() || !node) {
		return;
	}

	pNode->DisableModifiedEventOn();
	pNode->SetAndObserveMovingFiducialNodeID(node->GetID());
	pNode->DisableModifiedEventOff();
}

void qSlicerRegistrationQualityModuleWidget::setup() {
	Q_D(qSlicerRegistrationQualityModuleWidget);
	d->setupUi(this);
	this->Superclass::setup();
	d->StillErrorLabel->setVisible(false);

	//new
// 	d->subjectTreeView->setModel(d->logic()->getTreeViewModel());
// 	d->subjectTreeView->header()->hide();

// 	contextMenu = new QMenu(d->subjectTreeView);
// 	contextMenuShowAction = new QAction("Show",contextMenu);
// 	contextMenu->addAction(contextMenuShowAction);
// 	connect(contextMenu, SIGNAL(triggered(QAction*)), this, SLOT(contextMenuClicked(QAction*)));

	// Make the ComboBox only show "Registration" nodes.
// 	d->InputSubjectComboBox->addAttribute("vtkMRMLSubjectHierarchyNode","*");
// 	connect(d->InputSubjectComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(subjectHierarchyChanged(vtkMRMLNode*)));


// 	d->subjectTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
// 	connect(d->subjectTreeView, SIGNAL(customContextMenuRequested(QPoint const&)), this, SLOT(treeViewContextMenu(QPoint const&)));

// 	connect(d->loadPhaseButton, SIGNAL(clicked(bool)), this, SLOT(loadPhaseClicked(bool)));

	//end new

	connect(d->ParameterComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(setRegistrationQualityParametersNode(vtkMRMLNode*)));
	
	connect(d->SaveScreenshotPushButton, SIGNAL(clicked()), this, SLOT(saveScreenshotClicked()));
	connect(d->SaveOutputFilePushButton, SIGNAL(clicked()), this, SLOT(saveOutputFileClicked()));
	
	connect(d->FalseColorPushButton, SIGNAL(clicked()), this, SLOT (falseColorClicked()));
	connect(d->CheckerboardPushButton, SIGNAL(clicked()), this, SLOT (checkerboardClicked()));
	connect(d->AbsoluteDiffPushButton, SIGNAL(clicked()), this, SLOT (absoluteDiffClicked()));
        connect(d->fiducialButton, SIGNAL(clicked()), this, SLOT (fiducialClicked()));
	connect(d->jacobianButton, SIGNAL(clicked()), this, SLOT (jacobianClicked()));
	connect(d->consistencyButton, SIGNAL(clicked()), this, SLOT (inverseConsistClicked()));

	connect(d->patternSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setCheckerboardPattern(int)));

	connect(d->MovieToggle, SIGNAL(clicked()), this, SLOT (movieToggle()));
	connect(d->movieBoxRed, SIGNAL(stateChanged(int)), this, SLOT (movieBoxRedStateChanged(int)));
	connect(d->movieBoxYellow, SIGNAL(stateChanged(int)), this, SLOT (movieBoxYellowStateChanged(int)));
	connect(d->movieBoxGreen, SIGNAL(stateChanged(int)), this, SLOT (movieBoxGreenStateChanged(int)));

	connect(d->FlickerToggle, SIGNAL(clicked()), this, SLOT (flickerToggle()));
	connect(flickerTimer, SIGNAL(timeout()), this, SLOT(flickerToggle1()));
// 	connect( d->checkBox_ScalarBar2D, SIGNAL(toggled(bool)), this, SLOT( setScalarBar2DVisibility(bool) ) );
	
	qSlicerApplication * app = qSlicerApplication::application();
	if (app && app->layoutManager()){
		QStringList sliceViewerNames = app->layoutManager()->sliceViewNames();
		qMRMLSliceWidget* sliceViewerWidgetRed = app->layoutManager()->sliceWidget(sliceViewerNames[0]);
		const qMRMLSliceView* sliceViewRed = sliceViewerWidgetRed->sliceView();
		d->ScalarBarWidget2DRed->SetInteractor(sliceViewerWidgetRed->interactorStyle()->GetInteractor());
		qMRMLSliceWidget* sliceViewerWidgetYellow = app->layoutManager()->sliceWidget(sliceViewerNames[1]);
		const qMRMLSliceView* sliceViewYellow = sliceViewerWidgetYellow->sliceView();
		d->ScalarBarWidget2DYellow->SetInteractor(sliceViewerWidgetYellow->interactorStyle()->GetInteractor());
		qMRMLSliceWidget* sliceViewerWidgetGreen = app->layoutManager()->sliceWidget(sliceViewerNames[2]);
		const qMRMLSliceView* sliceViewGreen = sliceViewerWidgetGreen->sliceView();
		d->ScalarBarWidget2DGreen->SetInteractor(sliceViewerWidgetGreen->interactorStyle()->GetInteractor());
// 		connect(d->checkBox_ScalarBar2D, SIGNAL(stateChanged(int)), sliceViewRed, SLOT(scheduleRender()));
// 		connect(d->checkBox_ScalarBar2D, SIGNAL(stateChanged(int)), sliceViewYellow, SLOT(scheduleRender()));
// 		connect(d->checkBox_ScalarBar2D, SIGNAL(stateChanged(int)), sliceViewGreen, SLOT(scheduleRender()));
	}
}

//-----------------------------------------------------------------------------
// Output File
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::saveScreenshotClicked() {
	Q_D(const qSlicerRegistrationQualityModuleWidget);
	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();

	
	
	if (! pNode->GetOutputDirectory() ) {
		this->outputDirectoyChanged();
	}
	
	QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
	
	bool ok;
	QString text = QInputDialog::getText(NULL, tr("QInputDialog::getText()"),
                                          tr("Screenshot description:"), QLineEdit::Normal,
                                        tr("Image of jacobian."), &ok);
	
	try {
		d->logic()->SaveScreenshot(text.toLatin1().constData());
	} catch (std::runtime_error e) {
		d->StillErrorLabel->setText(e.what());
		d->StillErrorLabel->setVisible(true);
		d->AbsoluteDiffPushButton->toggle();
		cerr << e.what() << endl;
		QApplication::restoreOverrideCursor();
		return;
	}
	QApplication::restoreOverrideCursor();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::saveOutputFileClicked() {
	Q_D(const qSlicerRegistrationQualityModuleWidget);
	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();

	
	
	if (! pNode->GetOutputDirectory() ) {
		this->outputDirectoyChanged();
	}
	
	QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
	try {
		d->logic()->SaveOutputFile();
	} catch (std::runtime_error e) {
		d->StillErrorLabel->setText(e.what());
		d->StillErrorLabel->setVisible(true);
		d->AbsoluteDiffPushButton->toggle();
		cerr << e.what() << endl;
		QApplication::restoreOverrideCursor();
		return;
	}
	QApplication::restoreOverrideCursor();
}
//-----------------------------------------------------------------------------
// Absolute Difference
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::absoluteDiffClicked() {
	Q_D(const qSlicerRegistrationQualityModuleWidget);

	QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
	try {
		d->logic()->CalculateDIRQAFrom(1);
	} catch (std::runtime_error e) {
		d->StillErrorLabel->setText(e.what());
		d->StillErrorLabel->setVisible(true);
		d->AbsoluteDiffPushButton->toggle();
		cerr << e.what() << endl;
		QApplication::restoreOverrideCursor();
		return;
	}
	d->StillErrorLabel->setText("");

	//Create legend:
	this->updateWidgetFromMRML();
	
	QApplication::restoreOverrideCursor();

}
//-----------------------------------------------------------------------------
// Fiducials distance
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::fiducialClicked() {
	Q_D(const qSlicerRegistrationQualityModuleWidget);
	
	//Check if we want to calculate reference or inverse distance
	int number = 4;
// 	if (reference) number = 4;
// 	else number = 5;

	QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
	try {
		d->logic()->CalculateDIRQAFrom(number);
	} catch (std::runtime_error e) {
		d->StillErrorLabel->setText(e.what());
		d->StillErrorLabel->setVisible(true);
		cerr << e.what() << endl;
		QApplication::restoreOverrideCursor();
		return;
	}
	d->StillErrorLabel->setText("");
	
	
	QApplication::restoreOverrideCursor();

}
//-----------------------------------------------------------------------------
// False Color
//-----------------------------------------------------------------------------

void qSlicerRegistrationQualityModuleWidget::falseColorClicked() {
	Q_D(const qSlicerRegistrationQualityModuleWidget);
        bool invertColor;
        
        if (d->ColorCheckBox->checkState() == 0) invertColor = false;
        else invertColor = true;

	try {
		d->logic()->FalseColor(invertColor);
	} catch (std::runtime_error e) {
		d->StillErrorLabel->setText(e.what());
		d->StillErrorLabel->setVisible(true);
		d->FalseColorPushButton->toggle();
		cerr << e.what() << endl;
		return;
	}
	d->StillErrorLabel->setText("");
	d->consistencyButton->setChecked(false);
	d->CheckerboardPushButton->setChecked(false);
	d->AbsoluteDiffPushButton->setChecked(false);
	d->jacobianButton->setChecked(false);
}

//-----------------------------------------------------------------------------
// Checkerboard
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::checkerboardClicked(){
	Q_D(const qSlicerRegistrationQualityModuleWidget);
// 	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
	try {
		d->logic()->Checkerboard();
	} catch (std::runtime_error e) {
		d->StillErrorLabel->setText(e.what());
		d->StillErrorLabel->setVisible(true);
		d->CheckerboardPushButton->toggle();
		cerr << e.what() << endl;
		QApplication::restoreOverrideCursor();
		return;
	}
	d->StillErrorLabel->setText("");
	QApplication::restoreOverrideCursor();
}

void qSlicerRegistrationQualityModuleWidget::flickerToggle(){
	Q_D(const qSlicerRegistrationQualityModuleWidget);

	if(!flickerTimer->isActive()) {
// 		cerr << "Starting timer" << endl;
		flickerToggle1();
		flickerTimer->start(500);
	} else {
// 		cerr << "Stopping timer" << endl;
		flickerTimer->stop();
	}
}

// TODO: Move this to Logic
void qSlicerRegistrationQualityModuleWidget::flickerToggle1(){
	Q_D(const qSlicerRegistrationQualityModuleWidget);

// 	cerr << "Timer timeout" << endl;
	vtkSlicerRegistrationQualityLogic *logic = d->logic();
	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();

	if (pNode->GetFlickerOpacity()!=0 && pNode->GetFlickerOpacity()!=1) {
		pNode->SetFlickerOpacity(1);
	}

	logic->Flicker(pNode->GetFlickerOpacity());

	// TODO: Perhaps just do pNode->SetFlickerOpacity(1-(pNode->GetFlickerOpacity()));
	if (pNode->GetFlickerOpacity()==0) {
		pNode->DisableModifiedEventOn();
		pNode->SetFlickerOpacity(1);
		pNode->DisableModifiedEventOff();
	} else {
		pNode->DisableModifiedEventOn();
		pNode->SetFlickerOpacity(0);
		pNode->DisableModifiedEventOff();
	}
}

void qSlicerRegistrationQualityModuleWidget::movieToggle(){
	Q_D(const qSlicerRegistrationQualityModuleWidget);

	d->MovieToggle->setEnabled(false);
	d->MovieToggle->setText("Stop");

	vtkSlicerRegistrationQualityLogic *logic = d->logic();
	logic->Movie();
	d->MovieToggle->setText("Start");
	d->MovieToggle->setEnabled(true);
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::movieBoxRedStateChanged(int state) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetMovieBoxRedState(state);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::movieBoxYellowStateChanged(int state) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetMovieBoxYellowState(state);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::movieBoxGreenStateChanged(int state) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetMovieBoxGreenState(state);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
// Vector checks
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::jacobianClicked(){
	Q_D(const qSlicerRegistrationQualityModuleWidget);
	QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
	try {
		d->logic()->CalculateDIRQAFrom(2);
	} catch (std::runtime_error e) {
		d->StillErrorLabel->setText(e.what());
		d->StillErrorLabel->setVisible(true);
		d->jacobianButton->toggle();
		cerr << e.what() << endl;
		QApplication::restoreOverrideCursor();
		return;
	}
	d->StillErrorLabel->setText("");

	this->updateWidgetFromMRML();

	QApplication::restoreOverrideCursor();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::inverseConsistClicked(){
	Q_D(const qSlicerRegistrationQualityModuleWidget);

	QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
	try {
		d->logic()->CalculateDIRQAFrom(3);
	} catch (std::runtime_error e) {
		d->StillErrorLabel->setText(e.what());
		d->StillErrorLabel->setVisible(true);
		d->consistencyButton->toggle();
		cerr << e.what() << endl;
		QApplication::restoreOverrideCursor();
		return;
	}
	d->StillErrorLabel->setText("");

	this->updateWidgetFromMRML();
	QApplication::restoreOverrideCursor();
}

//------------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setCheckerboardPattern(int checkboardPattern) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}

	pNode->DisableModifiedEventOn();
	pNode->SetCheckerboardPattern(checkboardPattern);
	pNode->DisableModifiedEventOff();
}
//------------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setScalarBar2DVisibility(bool visible)
{
  Q_D(qSlicerRegistrationQualityModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerRegistrationQualityModuleWidget::setScalarBar2DVisibility: Invalid scene!";
    return;
  }

  if (d->ScalarBarWidget2DRed == 0 || d->ScalarBarWidget2DYellow == 0 || d->ScalarBarWidget2DGreen == 0)
  {
    return;
  }
  if (visible)
  {
#if (VTK_MAJOR_VERSION <= 5)
    d->ScalarBarActor2DRed->UseColorNameAsLabelOn();
    d->ScalarBarActor2DYellow->UseColorNameAsLabelOn();
    d->ScalarBarActor2DGreen->UseColorNameAsLabelOn();
#else
    d->ScalarBarActor2DRed->UseAnnotationAsLabelOn();
    d->ScalarBarActor2DYellow->UseAnnotationAsLabelOn();
    d->ScalarBarActor2DGreen->UseAnnotationAsLabelOn();
#endif
  }
  vtkMRMLColorTableNode* selectedColorNode = d->logic()->GetRegistrationQualityNode()->GetColorTableNode();
  if (!selectedColorNode)
  {
    qCritical() << "qSlicerRegistrationQualityModuleWidget::setScalarBar2DVisibility: Invalid color table node!";
    return;
  }
  int numberOfColors = selectedColorNode->GetNumberOfColors();
  for (int i=0; i<numberOfColors; i++)
  {
#if (VTK_MAJOR_VERSION <= 5)
    d->ScalarBarActor2DRed->SetColorName(i, selectedColorNode->GetColorName(i));
    d->ScalarBarActor2DYellow->SetColorName(i, selectedColorNode->GetColorName(i));
    d->ScalarBarActor2DGreen->SetColorName(i, selectedColorNode->GetColorName(i));
#else
    d->ScalarBarActor2DRed->GetLookupTable()->SetAnnotation(i, vtkStdString(selectedColorNode->GetColorName(i)));
    d->ScalarBarActor2DYellow->GetLookupTable()->SetAnnotation(i, vtkStdString(selectedColorNode->GetColorName(i)));
    d->ScalarBarActor2DGreen->GetLookupTable()->SetAnnotation(i, vtkStdString(selectedColorNode->GetColorName(i)));
#endif
  }

  d->ScalarBarWidget2DRed->SetEnabled(visible);
  d->ScalarBarWidget2DYellow->SetEnabled(visible);
  d->ScalarBarWidget2DGreen->SetEnabled(visible);
}
//-----------------------------------------------------------
bool qSlicerRegistrationQualityModuleWidget::setEditedNode(
  vtkMRMLNode* node, QString role /* = QString()*/, QString context /* = QString() */)
{
  Q_D(qSlicerRegistrationQualityModuleWidget);
  Q_UNUSED(context);

  if ( QString::compare(role, "fixedImage", Qt::CaseInsensitive) == 0 ){
     this->referenceVolumeChanged(node);
  }
  else if ( QString::compare(role, "warpedImage", Qt::CaseInsensitive) == 0 ){
     this->warpedVolumeChanged(node);
  }
  else if ( QString::compare(role, "fixedVectorField", Qt::CaseInsensitive) == 0 ){
     this->vectorVolumeChanged(node);
  }
  else if ( QString::compare(role, "movingVectorField", Qt::CaseInsensitive) == 0 ){
     this->invVectorVolumeChanged(node);
  }
  else if ( QString::compare(role, "fixedFiducials", Qt::CaseInsensitive) == 0 ){
     this->fiducialChanged(node);
  }
  else if ( QString::compare(role, "movingFiducials", Qt::CaseInsensitive) == 0 ){
     this->fiducialChanged(node);
  }
  else {
     return false;
  }
  return true;
}
