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

		if (pNode->GetVectorVolumeNodeID()) {
			d->InputFieldComboBox->setCurrentNode(pNode->GetVectorVolumeNodeID());
		} else {
			this->vectorVolumeChanged(d->InputFieldComboBox->currentNode());
		}
		if (pNode->GetInvVectorVolumeNodeID()) {
			d->InputInvFieldComboBox->setCurrentNode(pNode->GetInvVectorVolumeNodeID());
		} else {
			this->invVectorVolumeChanged(d->InputInvFieldComboBox->currentNode());
		}

		if (pNode->GetReferenceVolumeNodeID()) {
			d->InputReferenceComboBox->setCurrentNode(pNode->GetReferenceVolumeNodeID());
		} else {
			this->referenceVolumeChanged(d->InputReferenceComboBox->currentNode());
		}

		if (pNode->GetWarpedVolumeNodeID()) {
			d->InputWarpedComboBox->setCurrentNode(pNode->GetWarpedVolumeNodeID());
		} else {
			this->warpedVolumeChanged(d->InputWarpedComboBox->currentNode());
		}
		
		if (pNode->GetROINodeID()) {
			d->ROIInputComboBox->setCurrentNode(pNode->GetROINodeID());
		} else {
			this->ROIChanged(d->ROIInputComboBox->currentNode());
		}
		
		if (pNode->GetFiducialNodeID()) {
			d->FiducialInputComboBox->setCurrentNode(pNode->GetFiducialNodeID());
		} else {
			this->fiducialChanged(d->FiducialInputComboBox->currentNode());
		}
		
		if (pNode->GetInvFiducialNodeID()) {
			d->InvFiducialInputComboBox->setCurrentNode(pNode->GetInvFiducialNodeID());
		} else {
			this->invFiducialChanged(d->InvFiducialInputComboBox->currentNode());
		}

// 		if (pNode->GetCheckerboardNodeID()) {
// 			d->OutputCheckerboardComboBox->setCurrentNode(pNode->GetCheckerboardNodeID());
// 		} else {
// 			this->checkerboardVolumeChanged(d->OutputCheckerboardComboBox->currentNode());
// 		}
//
// 		if (pNode->GetAbsoluteDiffNodeID()) {
// 			d->AbsoluteDiffComboBox->setCurrentNode(pNode->GetAbsoluteDiffNodeID());
// 		} else {
// 			this->absoluteDiffVolumeChanged(d->AbsoluteDiffComboBox->currentNode());
// 		}

// 		pNode->SetFlickerOpacity(0);
		d->absoluteDiffMeanSpinBox->setValue(pNode->GetAbsoluteDiffStatistics()[0]);
		d->absoluteDiffSTDSpinBox->setValue(pNode->GetAbsoluteDiffStatistics()[1]);
		
		d->FiducialsMeanSpinBox->setValue(pNode->GetFiducialsStatistics()[0]);
		d->FiducialsMaxSpinBox->setValue(pNode->GetFiducialsStatistics()[2]);
		d->InvFiducialsMeanSpinBox->setValue(pNode->GetInvFiducialsStatistics()[0]);
		d->InvFiducialsMaxSpinBox->setValue(pNode->GetInvFiducialsStatistics()[2]);

		d->movieBoxRed->setChecked(pNode->GetMovieBoxRedState());
		d->jacobianMeanSpinBox->setValue(pNode->GetJacobianStatistics()[0]);
		d->jacobianSTDSpinBox->setValue(pNode->GetJacobianStatistics()[1]);
// 		pNode->SetInverseConsistStatistics(0,0,0,0);

		d->inverseConsistMeanSpinBox->setValue(pNode->GetInverseConsistStatistics()[0]);
		d->inverseConsistSTDSpinBox->setValue(pNode->GetInverseConsistStatistics()[1]);

		//Update Visualization Parameters
		d->CheckerboardPattern->setValue(pNode->GetCheckerboardPattern());
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

	d->JacobianCheckBox->setEnabled(true);
	d->InverseConsistCheckBox->setEnabled(true);

	pNode->DisableModifiedEventOn();
	if (node->IsA("vtkMRMLTransformNode")){
	  pNode->SetAndObserveTransformNodeID(node->GetID());
	  
	  //Convert transform to vector
	  vtkMRMLTransformNode *transform = vtkMRMLTransformNode::SafeDownCast(
				    this->mrmlScene()->GetNodeByID(
					  node->GetID()));
	  if (transform) {
	    vtkMRMLVectorVolumeNode *vectorVolume = d->logic()->CreateVectorFromTransform(transform);
	    if (vectorVolume){
	      pNode->SetAndObserveVectorVolumeNodeID(vectorVolume->GetID());
	    }
	  }
	  else{
	    std::cerr << "Widget: Transform not set, no creation of vector volume." << pNode->GetTransformNodeID() << std::endl;
	  }  
	}
	else if (node->IsA("vtkMRMLVectorVolumeNode")){
	  pNode->SetAndObserveVectorVolumeNodeID(node->GetID());

	  //Convert transform to vector
	  vtkMRMLVectorVolumeNode *vectorVolume= vtkMRMLVectorVolumeNode::SafeDownCast(
					  this->mrmlScene()->GetNodeByID(
						node->GetID()));
	  if (vectorVolume) {
	    vtkMRMLGridTransformNode *transform = d->logic()->CreateTransformFromVector(vectorVolume);
	    if( transform ){
	      pNode->SetAndObserveTransformNodeID(transform->GetID());
	    }
	  }
	  else{
	    std::cerr << "Widget: Vector not set, no creation of transform." << pNode->GetTransformNodeID() << std::endl;
	  }
	}
	pNode->DisableModifiedEventOff();
	
	// Calculate fiducial distance if possible
	if ( pNode->GetFiducialNodeID() != NULL && pNode->GetInvFiducialNodeID() != NULL ){
		this->fiducialCalculate(true);
	}

// 	double maxNorm = 0;

	// What to do if there is more than one array? Would there be more than one array?
// 	if (strcmp(node->GetClassName(), "vtkMRMLVectorVolumeNode") == 0) {
// 		d->InputReferenceComboBox->setEnabled(false);
// 		maxNorm = vtkMRMLVectorVolumeNode::SafeDownCast(node)->GetImageData()->GetPointData()->GetArray(0)->GetMaxNorm();
// 	} else if (	strcmp(node->GetClassName(), "vtkMRMLLinearTransformNode") == 0 ||
// 				strcmp(node->GetClassName(), "vtkMRMLBSplineTransformNode") == 0 ||
// 				strcmp(node->GetClassName(), "vtkMRMLGridTransformNode") == 0) {
// 		d->InputReferenceComboBox->setEnabled(true);
//
// 		vtkSmartPointer<vtkMRMLVolumeNode> referenceVolumeNode = vtkMRMLVolumeNode::SafeDownCast(this->mrmlScene()->GetNodeByID(pNode->GetReferenceVolumeNodeID()));
// 		if (referenceVolumeNode == NULL) {
// 			return;
// 		}
//
// 		//TODO: Remake progress dialog and add detail (update progress from actual steps occurring in logic)
// 		QProgressDialog *convertProgress = new QProgressDialog(qSlicerApplication::application()->mainWindow());
// 		convertProgress->setModal(true);
// 		convertProgress->setMinimumDuration(100); //will matter a bit more after progress dialog is remade
// 		convertProgress->show();
// 		convertProgress->setLabelText("Converting transform to vector volume...");
//
// 		convertProgress->setValue(20);
// 		d->logic()->GenerateTransformField();
//
// 		convertProgress->setValue(80);
// 		maxNorm = d->logic()->GetFieldMaxNorm() + 1;
//
// 		convertProgress->setValue(100);
// 		delete convertProgress;
// 	}

// 	pNode->SetGlyphThresholdMax(maxNorm);
// 	d->InputGlyphThreshold->setMaximum(maxNorm);
// 	d->InputGlyphThreshold->setMaximumValue(maxNorm);
//
// 	pNode->SetContourMax(maxNorm);
// 	d->InputContourRange->setMaximum(maxNorm);
// 	d->InputContourRange->setMaximumValue(maxNorm);
//
// 	pNode->SetGlyphSliceThresholdMax(maxNorm);
// 	d->InputGlyphSliceThreshold->setMaximum(maxNorm);
// 	d->InputGlyphSliceThreshold->setMaximumValue(maxNorm);
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::invVectorVolumeChanged(vtkMRMLNode* node) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	// TODO: Move into updatefrommrml?
	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene() || !node) {
		return;
	}

	d->InverseConsistCheckBox->setEnabled(true);


	pNode->DisableModifiedEventOn();
	if (node->IsA("vtkMRMLTransformNode")){
	  pNode->SetAndObserveInvTransformNodeID(node->GetID());
	  	  
	  //Convert transform to vector
	  vtkMRMLTransformNode *transform = vtkMRMLTransformNode::SafeDownCast(
				    this->mrmlScene()->GetNodeByID(
					  node->GetID()));
	  if (transform) {
	    vtkMRMLVectorVolumeNode *vectorVolume = d->logic()->CreateVectorFromTransform(transform);
	    if (vectorVolume){
	      pNode->SetAndObserveVectorVolumeNodeID(vectorVolume->GetID());
	    }
	  }
	  else{
	    std::cerr << "Widget: Transform not set, no creation of vector volume." << pNode->GetTransformNodeID() << std::endl;
	  }	  
	}
	else if (node->IsA("vtkMRMLVectorVolumeNode")){
	  pNode->SetAndObserveInvVectorVolumeNodeID(node->GetID());
	  
	  //Convert transform to vector
	  vtkMRMLVectorVolumeNode *vectorVolume= vtkMRMLVectorVolumeNode::SafeDownCast(
					  this->mrmlScene()->GetNodeByID(
						node->GetID()));
	  if (vectorVolume) {
	    vtkMRMLGridTransformNode *transform = d->logic()->CreateTransformFromVector(vectorVolume);
	    if( transform ){
	      pNode->SetAndObserveInvTransformNodeID(transform->GetID());
	    }
	  }
	  else{
	    std::cerr << "Widget: Vector not set, no creation of transform." << pNode->GetTransformNodeID() << std::endl;
	  }
	}
	pNode->DisableModifiedEventOff();
	// Calculate fiducial distance if possible
	if ( pNode->GetFiducialNodeID() != NULL && pNode->GetInvFiducialNodeID() != NULL ){
		this->fiducialCalculate(false);
	}

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
	pNode->SetAndObserveReferenceVolumeNodeID(node->GetID());
	pNode->DisableModifiedEventOff();

	bool currentState;
	vtkSmartPointer<vtkMRMLVolumeNode> warpedVolumeNode = vtkMRMLVolumeNode::SafeDownCast(this->mrmlScene()->GetNodeByID(pNode->GetWarpedVolumeNodeID()));
	if (warpedVolumeNode == NULL) {
	    currentState=false;
	}
	else{
	  currentState=true;
	}

	d->AbsoluteDiffCheckBox->setEnabled(currentState);
	d->FalseColorCheckBox->setEnabled(currentState);
	d->CheckerboardCheckBox->setEnabled(currentState);
	d->MovieToggle->setEnabled(currentState);
	d->FlickerToggle->setEnabled(currentState);

// 	vtkSmartPointer<vtkMRMLTransformNode> vectorVolumeNode = vtkMRMLTransformNode::SafeDownCast(this->mrmlScene()->GetNodeByID(pNode->GetVectorVolumeNodeID()));
// 	if (vectorVolumeNode == NULL) {
// 		return;
// 	}
//
// 	if ( strcmp(vectorVolumeNode->GetClassName(), "vtkMRMLLinearTransformNode") == 0 ||
// 		 strcmp(vectorVolumeNode->GetClassName(), "vtkMRMLBSplineTransformNode") == 0 ||
// 		 strcmp(vectorVolumeNode->GetClassName(), "vtkMRMLGridTransformNode") == 0) {
//
// 		//TODO: Remake progress dialog and add detail (update progress from actual steps occurring in logic)
// 		QProgressDialog *convertProgress =  new QProgressDialog(qSlicerApplication::application()->mainWindow());
// 		convertProgress->setModal(true);
// 		convertProgress->setMinimumDuration(100); //will matter a bit more after progress dialog is remade
// 		convertProgress->show();
// 		convertProgress->setLabelText("Converting transform to vector volume...");
//
// 		convertProgress->setValue(20);
// 		d->logic()->GenerateTransformField();
//
// 		convertProgress->setValue(80);
// 		maxNorm = d->logic()->GetFieldMaxNorm() + 1;
//
// 		convertProgress->setValue(100);
// 		delete convertProgress;
// 	}
//
// 	pNode->SetGlyphThresholdMax(maxNorm);
// 	d->InputGlyphThreshold->setMaximum(maxNorm);
// 	d->InputGlyphThreshold->setMaximumValue(maxNorm);
//
// 	pNode->SetContourMax(maxNorm);
// 	d->InputContourRange->setMaximum(maxNorm);
// 	d->InputContourRange->setMaximumValue(maxNorm);
//
// 	pNode->SetGlyphSliceThresholdMax(maxNorm);
// 	d->InputGlyphSliceThreshold->setMaximum(maxNorm);
// 	d->InputGlyphSliceThreshold->setMaximumValue(maxNorm);

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
	vtkSmartPointer<vtkMRMLVolumeNode> referenceVolumeNode = vtkMRMLVolumeNode::SafeDownCast(this->mrmlScene()->GetNodeByID(pNode->GetReferenceVolumeNodeID()));
	if (referenceVolumeNode == NULL) {
	    currentState=false;
	}
	else{
	  currentState=true;
	}

	d->AbsoluteDiffCheckBox->setEnabled(currentState);
	d->FalseColorCheckBox->setEnabled(currentState);
	d->CheckerboardCheckBox->setEnabled(currentState);
	d->MovieToggle->setEnabled(currentState);
	d->FlickerToggle->setEnabled(currentState);
// 	vtkSlicerRegistrationQualityLogic *logic = d->logic();
// 	logic->ImageDifference();
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
	pNode->SetAndObserveFiducialNodeID(node->GetID());
	pNode->DisableModifiedEventOff();
	// Calculate fiducial distance if possible
	if ( pNode->GetTransformNodeID() != NULL && pNode->GetInvFiducialNodeID() != NULL ){
		this->fiducialCalculate(true);
	}
	if ( pNode->GetInvTransformNodeID() != NULL && pNode->GetInvFiducialNodeID() != NULL ){
		this->fiducialCalculate(false);
	}
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
	pNode->SetAndObserveInvFiducialNodeID(node->GetID());
	pNode->DisableModifiedEventOff();
	// Calculate fiducial distance if possible
	if ( pNode->GetTransformNodeID() != NULL && pNode->GetFiducialNodeID() != NULL ){
		this->fiducialCalculate(true);
	}
	if ( pNode->GetInvTransformNodeID() != NULL && pNode->GetFiducialNodeID() != NULL ){
		this->fiducialCalculate(false);
	}
}

void qSlicerRegistrationQualityModuleWidget::setup() {
	Q_D(qSlicerRegistrationQualityModuleWidget);
	d->setupUi(this);
	this->Superclass::setup();
	d->StillErrorLabel->setVisible(false);

	connect(d->ParameterComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(setRegistrationQualityParametersNode(vtkMRMLNode*)));

	connect(d->InputFieldComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(vectorVolumeChanged(vtkMRMLNode*)));
	connect(d->InputInvFieldComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(invVectorVolumeChanged(vtkMRMLNode*)));
	connect(d->InputReferenceComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(referenceVolumeChanged(vtkMRMLNode*)));
	connect(d->InputWarpedComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(warpedVolumeChanged(vtkMRMLNode*)));
	connect(d->ROIInputComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(ROIChanged(vtkMRMLNode*)));
	connect(d->FiducialInputComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(fiducialChanged(vtkMRMLNode*)));
	connect(d->InvFiducialInputComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(invFiducialChanged(vtkMRMLNode*)));
//	connect(d->OutputCheckerboardComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(checkerboardVolumeChanged(vtkMRMLNode*)));
//	connect(d->AbsoluteDiffComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(absoluteDiffVolumeChanged(vtkMRMLNode*)));

	connect(d->SaveScreenshotPushButton, SIGNAL(clicked()), this, SLOT(saveScreenshotClicked()));
	connect(d->SaveOutputFilePushButton, SIGNAL(clicked()), this, SLOT(saveOutputFileClicked()));
	
	connect(d->FalseColorCheckBox, SIGNAL(clicked(bool)), this, SLOT (falseColorClicked(bool)));
	connect(d->CheckerboardCheckBox, SIGNAL(clicked(bool)), this, SLOT (checkerboardClicked(bool)));
	connect(d->AbsoluteDiffCheckBox, SIGNAL(clicked(bool)), this, SLOT (absoluteDiffClicked(bool)));
	connect(d->JacobianCheckBox, SIGNAL(clicked(bool)), this, SLOT (jacobianClicked(bool)));
	connect(d->InverseConsistCheckBox, SIGNAL(clicked(bool)), this, SLOT (inverseConsistClicked(bool)));

	connect(d->CheckerboardPattern, SIGNAL(valueChanged(double)), this, SLOT(setCheckerboardPattern(double)));

	connect(d->MovieToggle, SIGNAL(clicked()), this, SLOT (movieToggle()));
	connect(d->movieBoxRed, SIGNAL(stateChanged(int)), this, SLOT (movieBoxRedStateChanged(int)));
	connect(d->movieBoxYellow, SIGNAL(stateChanged(int)), this, SLOT (movieBoxYellowStateChanged(int)));
	connect(d->movieBoxGreen, SIGNAL(stateChanged(int)), this, SLOT (movieBoxGreenStateChanged(int)));

	connect(d->FlickerToggle, SIGNAL(clicked()), this, SLOT (flickerToggle()));
	connect(flickerTimer, SIGNAL(timeout()), this, SLOT(flickerToggle1()));
	
	connect( d->checkBox_ScalarBar2D, SIGNAL(toggled(bool)), this, SLOT( setScalarBar2DVisibility(bool) ) );
	
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


		connect(d->checkBox_ScalarBar2D, SIGNAL(stateChanged(int)), sliceViewRed, SLOT(scheduleRender()));
		connect(d->checkBox_ScalarBar2D, SIGNAL(stateChanged(int)), sliceViewYellow, SLOT(scheduleRender()));
		connect(d->checkBox_ScalarBar2D, SIGNAL(stateChanged(int)), sliceViewGreen, SLOT(scheduleRender()));
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
		d->AbsoluteDiffCheckBox->toggle();
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
		d->AbsoluteDiffCheckBox->toggle();
		cerr << e.what() << endl;
		QApplication::restoreOverrideCursor();
		return;
	}
	QApplication::restoreOverrideCursor();
}
// Image Checks
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Absolute Difference
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::absoluteDiffClicked(bool state) {
	Q_D(const qSlicerRegistrationQualityModuleWidget);
	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();

	QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
	try {
		d->logic()->CalculateDIRQAFrom(1,state);
	} catch (std::runtime_error e) {
		d->StillErrorLabel->setText(e.what());
		d->StillErrorLabel->setVisible(true);
		d->AbsoluteDiffCheckBox->toggle();
		cerr << e.what() << endl;
		QApplication::restoreOverrideCursor();
		return;
	}
	d->StillErrorLabel->setText("");
	d->FalseColorCheckBox->setChecked(false);
	d->CheckerboardCheckBox->setChecked(false);
	
	d->JacobianCheckBox->setChecked(false);
	d->InverseConsistCheckBox->setChecked(false);

	if (state){
	  d->absoluteDiffMeanSpinBox->setValue(pNode->GetAbsoluteDiffStatistics()[0]);
	  d->absoluteDiffSTDSpinBox->setValue(pNode->GetAbsoluteDiffStatistics()[1]);
	  }
	else{
	  d->absoluteDiffMeanSpinBox->setValue(0);
	  d->absoluteDiffSTDSpinBox->setValue(0);
	  }
	//Create legend:
	this->updateWidgetFromMRML();
	
	QApplication::restoreOverrideCursor();

}
//-----------------------------------------------------------------------------
// Fiducials distance
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::fiducialCalculate(bool reference) {
	Q_D(const qSlicerRegistrationQualityModuleWidget);
	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	
	//Check if we want to calculate reference or inverse distance
	int number;
	if (reference) number = 4;
	else number = 5;

	QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
	try {
		d->logic()->CalculateDIRQAFrom(number,true);
	} catch (std::runtime_error e) {
		d->StillErrorLabel->setText(e.what());
		d->StillErrorLabel->setVisible(true);
		cerr << e.what() << endl;
		QApplication::restoreOverrideCursor();
		return;
	}
	d->StillErrorLabel->setText("");
	
	if ( reference ) {
		d->FiducialsMeanSpinBox->setValue(pNode->GetFiducialsStatistics()[0]);
		d->FiducialsMaxSpinBox->setValue(pNode->GetFiducialsStatistics()[2]);
	} 
	else{
		d->InvFiducialsMeanSpinBox->setValue(pNode->GetInvFiducialsStatistics()[0]);
		d->InvFiducialsMaxSpinBox->setValue(pNode->GetInvFiducialsStatistics()[2]);
	}
	
	QApplication::restoreOverrideCursor();

}
//-----------------------------------------------------------------------------
// False Color
//-----------------------------------------------------------------------------

void qSlicerRegistrationQualityModuleWidget::falseColorClicked(bool state) {
	Q_D(const qSlicerRegistrationQualityModuleWidget);

	try {
		d->logic()->FalseColor(state);
	} catch (std::runtime_error e) {
		d->StillErrorLabel->setText(e.what());
		d->StillErrorLabel->setVisible(true);
		d->FalseColorCheckBox->toggle();
		cerr << e.what() << endl;
		return;
	}
	d->StillErrorLabel->setText("");
	d->InverseConsistCheckBox->setChecked(false);
	d->CheckerboardCheckBox->setChecked(false);
	d->AbsoluteDiffCheckBox->setChecked(false);
	d->JacobianCheckBox->setChecked(false);
}
//-----------------------------------------------------------------------------
// Checkerboard
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::checkerboardClicked(bool state){
	Q_D(const qSlicerRegistrationQualityModuleWidget);
// 	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
	try {
		d->logic()->Checkerboard(state);
	} catch (std::runtime_error e) {
		d->StillErrorLabel->setText(e.what());
		d->StillErrorLabel->setVisible(true);
		d->CheckerboardCheckBox->toggle();
		cerr << e.what() << endl;
		QApplication::restoreOverrideCursor();
		return;
	}
	d->StillErrorLabel->setText("");
	d->FalseColorCheckBox->setChecked(false);
	d->AbsoluteDiffCheckBox->setChecked(false);
	d->JacobianCheckBox->setChecked(false);
	d->InverseConsistCheckBox->setChecked(false);
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
void qSlicerRegistrationQualityModuleWidget::jacobianClicked(bool state){
	Q_D(const qSlicerRegistrationQualityModuleWidget);
	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();

	QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
	try {
		d->logic()->CalculateDIRQAFrom(2,state);
	} catch (std::runtime_error e) {
		d->StillErrorLabel->setText(e.what());
		d->StillErrorLabel->setVisible(true);
		d->JacobianCheckBox->toggle();
		cerr << e.what() << endl;
		QApplication::restoreOverrideCursor();
		return;
	}
	d->StillErrorLabel->setText("");
	d->FalseColorCheckBox->setChecked(false);
	d->CheckerboardCheckBox->setChecked(false);
	d->AbsoluteDiffCheckBox->setChecked(false);
	
	d->InverseConsistCheckBox->setChecked(false);

	if (state){
	  d->jacobianMeanSpinBox->setValue(pNode->GetJacobianStatistics()[0]);
	  d->jacobianSTDSpinBox->setValue(pNode->GetJacobianStatistics()[1]);
	}
	else{
	  d->jacobianMeanSpinBox->setValue(0);
	  d->jacobianSTDSpinBox->setValue(0);
	}
	this->updateWidgetFromMRML();
// 	d->updateScalarBarsFromSelectedColorTable()
	QApplication::restoreOverrideCursor();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::inverseConsistClicked(bool state){
	Q_D(const qSlicerRegistrationQualityModuleWidget);
	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();

	QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
	try {
		d->logic()->CalculateDIRQAFrom(3,state);
	} catch (std::runtime_error e) {
		d->StillErrorLabel->setText(e.what());
		d->StillErrorLabel->setVisible(true);
		d->InverseConsistCheckBox->toggle();
		cerr << e.what() << endl;
		QApplication::restoreOverrideCursor();
		return;
	}
	d->StillErrorLabel->setText("");
	d->FalseColorCheckBox->setChecked(false);
	d->CheckerboardCheckBox->setChecked(false);
	d->AbsoluteDiffCheckBox->setChecked(false);
	
	d->JacobianCheckBox->setChecked(false);

	if (state){
	  d->inverseConsistMeanSpinBox->setValue(pNode->GetInverseConsistStatistics()[0]);
	  d->inverseConsistSTDSpinBox->setValue(pNode->GetInverseConsistStatistics()[1]);
	}
	else{
	  d->inverseConsistMeanSpinBox->setValue(0);
	  d->inverseConsistSTDSpinBox->setValue(0);
	}
	this->updateWidgetFromMRML();
	QApplication::restoreOverrideCursor();
}

//------------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setCheckerboardPattern(double checkboardPattern) {
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
