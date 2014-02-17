// SlicerQt includes
#include "qSlicerRegistrationQualityModuleWidget.h"
#include "ui_qSlicerRegistrationQualityModule.h"
#include <qSlicerApplication.h>

// Qt includes
#include <QProgressDialog>
#include <QMainWindow>
#include <QDebug>
#include <QFileDialog>
#include <QCheckBox>
#include <QTimer>

// SlicerQt includes
#include <qSlicerAbstractCoreModule.h>

// DeformationVisualizer includes
#include "vtkSlicerRegistrationQualityLogic.h"
#include "vtkMRMLRegistrationQualityNode.h"

// MMRL includes
#include <vtkMRMLScene.h>
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLVectorVolumeNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLBSplineTransformNode.h>
#include <vtkMRMLGridTransformNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSelectionNode.h>

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkNew.h>
#include <vtkDataArray.h>
#include <vtkGeneralTransform.h>
#include <vtkSmartPointer.h>

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
	vtkSlicerRegistrationQualityLogic* logic() const;
};

//-----------------------------------------------------------------------------
// qSlicerRegistrationQualityModuleWidgetPrivate methods
//-----------------------------------------------------------------------------
qSlicerRegistrationQualityModuleWidgetPrivate::qSlicerRegistrationQualityModuleWidgetPrivate(
	qSlicerRegistrationQualityModuleWidget& object) : q_ptr(&object) {
}

vtkSlicerRegistrationQualityLogic* qSlicerRegistrationQualityModuleWidgetPrivate::logic() const {
	Q_Q( const qSlicerRegistrationQualityModuleWidget );
	return vtkSlicerRegistrationQualityLogic::SafeDownCast( q->logic() );
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

		if (pNode->GetOutputModelNodeID()) {
			d->OutputModelComboBox->setCurrentNode(pNode->GetOutputModelNodeID());
		} else {
			this->outputModelChanged(d->OutputModelComboBox->currentNode());
		}

// 		if (pNode->GetCheckerboardNodeID()) {
// 			d->OutputCheckerboardComboBox->setCurrentNode(pNode->GetCheckerboardNodeID());
// 		} else {
// 			this->checkerboardVolumeChanged(d->OutputCheckerboardComboBox->currentNode());
// 		}
// 		
// 		if (pNode->GetSquaredDiffNodeID()) {
// 			d->SquaredDiffComboBox->setCurrentNode(pNode->GetSquaredDiffNodeID());
// 		} else {
// 			this->squaredDiffVolumeChanged(d->SquaredDiffComboBox->currentNode());
// 		}

// 		pNode->SetFlickerOpacity(0);
		d->squaredDiffMeanSpinBox->setValue(pNode->GetSquaredDiffStatistics()[0]);
		d->squaredDiffSTDSpinBox->setValue(pNode->GetSquaredDiffStatistics()[1]);
		
		d->movieBoxRed->setChecked(pNode->GetMovieBoxRedState());
		d->jacobianMeanSpinBox->setValue(pNode->GetJacobianStatistics()[0]);
		d->jacobianSTDSpinBox->setValue(pNode->GetJacobianStatistics()[1]);
// 		pNode->SetInverseConsistStatistics(0,0,0,0);
		
		d->inverseConsistMeanSpinBox->setValue(pNode->GetInverseConsistStatistics()[0]);
		d->inverseConsistSTDSpinBox->setValue(pNode->GetInverseConsistStatistics()[1]);

	  
		//Update Visualization Parameters
		d->CheckerboardPattern->setValue(pNode->GetCheckerboardPattern());
		// Glyph Parameters
		d->InputGlyphPointMax->setValue(pNode->GetGlyphPointMax());
		d->InputGlyphSeed->setValue(pNode->GetGlyphSeed());
		d->InputGlyphScale->setValue(pNode->GetGlyphScale());
		d->InputGlyphScaleDirectional->setChecked(pNode->GetGlyphScaleDirectional());
		d->InputGlyphScaleIsotropic->setChecked(pNode->GetGlyphScaleIsotropic());
		d->InputGlyphThreshold->setMaximumValue(pNode->GetGlyphThresholdMax());
		d->InputGlyphThreshold->setMinimumValue(pNode->GetGlyphThresholdMin());
		d->GlyphSourceComboBox->setCurrentIndex(pNode->GetGlyphSourceOption());
		// Arrow Parameters
		d->InputGlyphArrowTipLength->setValue(pNode->GetGlyphArrowTipLength());
		d->InputGlyphArrowTipRadius->setValue(pNode->GetGlyphArrowTipRadius());
		d->InputGlyphArrowShaftRadius->setValue(pNode->GetGlyphArrowShaftRadius());
		d->InputGlyphArrowResolution->setValue(pNode->GetGlyphArrowResolution());
		// Cone Parameters
		d->InputGlyphConeHeight->setValue(pNode->GetGlyphConeHeight());
		d->InputGlyphConeRadius->setValue(pNode->GetGlyphConeRadius());
		d->InputGlyphConeResolution->setValue(pNode->GetGlyphConeResolution());
		// Sphere Parameters
		d->InputGlyphSphereResolution->setValue(pNode->GetGlyphSphereResolution());

		// Grid Parameters
		d->InputGridScale->setValue(pNode->GetGridScale());
		d->InputGridDensity->setValue(pNode->GetGridDensity());

		// Block Parameters
		d->InputBlockScale->setValue(pNode->GetBlockScale());
		d->InputBlockDisplacementCheck->setChecked(pNode->GetBlockDisplacementCheck());

		// Contour Parameters
		d->InputContourNumber->setValue(pNode->GetContourNumber());
		d->InputContourRange->setMaximumValue(pNode->GetContourMax());
		d->InputContourRange->setMinimumValue(pNode->GetContourMin());

		// Glyph Slice Parameters
		if (pNode->GetGlyphSliceNodeID()) {
			d->GlyphSliceComboBox->setCurrentNode(pNode->GetGlyphSliceNodeID());
		} else {
			this->setGlyphSliceNode(d->GlyphSliceComboBox->currentNode());
		}
		d->InputGlyphSlicePointMax->setValue(pNode->GetGlyphSlicePointMax());
		d->InputGlyphSliceThreshold->setMaximumValue(pNode->GetGlyphSliceThresholdMax());
		d->InputGlyphSliceThreshold->setMinimumValue(pNode->GetGlyphSliceThresholdMin());
		d->InputGlyphSliceScale->setValue(pNode->GetGlyphSliceScale());
		d->InputGlyphSliceSeed->setValue(pNode->GetGlyphSliceSeed());

		// Grid Slice Parameters
		if (pNode->GetGridSliceNodeID()) {
			d->GridSliceComboBox->setCurrentNode(pNode->GetGridSliceNodeID());
		} else {
			this->setGridSliceNode(d->GridSliceComboBox->currentNode());
		}
		d->InputGridSliceScale->setValue(pNode->GetGridSliceScale());
		d->InputGridSliceDensity->setValue(pNode->GetGridSliceDensity());
	}
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
		d->ApplyButton->setEnabled(false);
		d->VolumeDisabledLabel->show();
		return;
	}

	d->ApplyButton->setEnabled(true);
	d->JacobianCheckBox->setEnabled(true);
	d->InverseConsistCheckBox->setEnabled(true);
	d->VolumeDisabledLabel->hide();

	pNode->DisableModifiedEventOn();
	pNode->SetAndObserveVectorVolumeNodeID(node->GetID());
	pNode->DisableModifiedEventOff();

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
		d->ApplyButton->setEnabled(false);
		d->VolumeDisabledLabel->show();
		return;
	}

	d->InverseConsistCheckBox->setEnabled(true);


	pNode->DisableModifiedEventOn();
	pNode->SetAndObserveInvVectorVolumeNodeID(node->GetID());
	pNode->DisableModifiedEventOff();

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
		
	d->SquaredDiffCheckBox->setEnabled(currentState);
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
		
	d->SquaredDiffCheckBox->setEnabled(currentState);
	d->FalseColorCheckBox->setEnabled(currentState);
	d->CheckerboardCheckBox->setEnabled(currentState);
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
		d->ApplyButton->setEnabled(false);
		d->ModelDisabledLabel->show();
		return;
	}

	d->ApplyButton->setEnabled(true);
	d->ModelDisabledLabel->hide();

	pNode->DisableModifiedEventOn();
	pNode->SetAndObserveOutputModelNodeID(node->GetID());
	pNode->DisableModifiedEventOff();
}
//-----------------------------------------------------------------------------
// void qSlicerRegistrationQualityModuleWidget::checkerboardVolumeChanged(vtkMRMLNode* node)
// {
// 	Q_D(qSlicerRegistrationQualityModuleWidget);
// 
// 	//TODO: Move into updatefrommrml?
// 	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
// 	if (!pNode || !this->mrmlScene() || !node) {
// 		return;
// 	}
// 	pNode->DisableModifiedEventOn();
// 	pNode->SetAndObserveCheckerboardNodeID(node->GetID());
// 	pNode->DisableModifiedEventOff();
// }
// //-----------------------------------------------------------------------------
// void qSlicerRegistrationQualityModuleWidget::squaredDiffVolumeChanged(vtkMRMLNode* node)
// {
// 	Q_D(qSlicerRegistrationQualityModuleWidget);
// 
// 	//TODO: Move into updatefrommrml?
// 	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
// 	if (!pNode || !this->mrmlScene() || !node) {
// 		return;
// 	}
// 
// 	pNode->DisableModifiedEventOn();
// 	pNode->SetAndObserveSquaredDiffNodeID(node->GetID());
// 	pNode->DisableModifiedEventOff();
// }
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::updateSourceOptions(int option) {
	Q_D(qSlicerRegistrationQualityModuleWidget);
	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();

	//TODO: Move into updatefrommrml?
	if (option == 0) {
		d->ArrowSourceOptions->setEnabled(true);
		d->ArrowSourceOptions->setVisible(true);
		d->ConeSourceOptions->setEnabled(false);
		d->ConeSourceOptions->setVisible(false);
		d->SphereSourceOptions->setEnabled(false);
		d->SphereSourceOptions->setVisible(false);

		if (!pNode || !this->mrmlScene()) {
			return;
		}
		pNode->DisableModifiedEventOn();
		pNode->SetGlyphScaleDirectional(true);
		pNode->SetGlyphScaleIsotropic(false);
		pNode->DisableModifiedEventOff();
	} else if (option == 1) {
		d->ArrowSourceOptions->setEnabled(false);
		d->ArrowSourceOptions->setVisible(false);
		d->ConeSourceOptions->setEnabled(true);
		d->ConeSourceOptions->setVisible(true);
		d->SphereSourceOptions->setEnabled(false);
		d->SphereSourceOptions->setVisible(false);

		if (!pNode || !this->mrmlScene()) {
			return;
		}
		pNode->DisableModifiedEventOn();
		pNode->SetGlyphScaleDirectional(true);
		pNode->SetGlyphScaleIsotropic(false);
		pNode->DisableModifiedEventOff();
	} else if (option == 2) {
		d->ArrowSourceOptions->setEnabled(false);
		d->ArrowSourceOptions->setVisible(false);
		d->ConeSourceOptions->setEnabled(false);
		d->ConeSourceOptions->setVisible(false);
		d->SphereSourceOptions->setEnabled(true);
		d->SphereSourceOptions->setVisible(true);

		if (!pNode || !this->mrmlScene()) {
			return;
		}
		pNode->DisableModifiedEventOn();
		pNode->SetGlyphScaleDirectional(false);
		pNode->SetGlyphScaleIsotropic(true);
		pNode->DisableModifiedEventOff();
	}

	d->InputGlyphScaleDirectional->setChecked(pNode->GetGlyphScaleDirectional());
	d->InputGlyphScaleIsotropic->setChecked(pNode->GetGlyphScaleIsotropic());
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::visualize() {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	if ( d->InputFieldComboBox->currentNodeId() != NULL &&
		 d->OutputModelComboBox->currentNodeId() != NULL) {

		//TODO: Remake progress dialog and add detail (update progress from
		//      actual steps occurring in logic)
		QProgressDialog *visualizeProgress = new QProgressDialog(
				qSlicerApplication::application()->mainWindow());
		visualizeProgress->setModal(true);
		//will matter a bit more after progress dialog is remade
		visualizeProgress->setMinimumDuration(100);
		visualizeProgress->show();
		visualizeProgress->setLabelText("Processing...");
		visualizeProgress->setValue(0);

		if (d->GlyphToggle->isChecked()) {
			visualizeProgress->setLabelText("Creating glyphs...");
			visualizeProgress->setValue(20);
			d->logic()->CreateVisualization(1);
		} else if (d->GridToggle->isChecked()) {
			visualizeProgress->setLabelText("Creating grid...");
			visualizeProgress->setValue(20);
			d->logic()->CreateVisualization(2);
		} else if (d->ContourToggle->isChecked()) {
			visualizeProgress->setLabelText("Creating contours...");
			visualizeProgress->setValue(20);
			d->logic()->CreateVisualization(3);
		} else if (d->BlockToggle->isChecked()) {
			visualizeProgress->setLabelText("Creating block...");
			visualizeProgress->setValue(20);
			d->logic()->CreateVisualization(4);
		} else if (d->GlyphSliceToggle->isChecked()) {
			visualizeProgress->setLabelText("Creating glyphs for slice view...");
			visualizeProgress->setValue(20);
			d->logic()->CreateVisualization(5);
		} else if (d->GridSliceToggle->isChecked()) {
			visualizeProgress->setLabelText("Creating grid for slice view...");
			visualizeProgress->setValue(20);
			d->logic()->CreateVisualization(6);
		}
		visualizeProgress->setValue(100);
		delete visualizeProgress;
	}
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setup() {
	Q_D(qSlicerRegistrationQualityModuleWidget);
	d->setupUi(this);
	this->Superclass::setup();

	connect(d->ParameterComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(setRegistrationQualityParametersNode(vtkMRMLNode*)));

	connect(d->InputFieldComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(vectorVolumeChanged(vtkMRMLNode*)));
	connect(d->InputInvFieldComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(invVectorVolumeChanged(vtkMRMLNode*)));
	connect(d->InputReferenceComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(referenceVolumeChanged(vtkMRMLNode*)));
	connect(d->InputWarpedComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(warpedVolumeChanged(vtkMRMLNode*)));
	connect(d->OutputModelComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(outputModelChanged(vtkMRMLNode*)));
//	connect(d->OutputCheckerboardComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(checkerboardVolumeChanged(vtkMRMLNode*)));
//	connect(d->SquaredDiffComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(squaredDiffVolumeChanged(vtkMRMLNode*)));
	
	// Squared Difference 
	connect(d->SquaredDiffCheckBox, SIGNAL(stateChanged(int)), this, SLOT (squaredDiffStateChanged(int)));
	// Image Checks
	
	connect(d->FalseColorCheckBox, SIGNAL(stateChanged(int)), this, SLOT (falseColorStateChanged(int)));
	connect(d->CheckerboardCheckBox, SIGNAL(stateChanged(int)), this, SLOT (checkerboardStateChanged(int)));

	connect(d->MovieToggle, SIGNAL(clicked()), this, SLOT (movieToggle()));
	connect(d->movieBoxRed, SIGNAL(stateChanged(int)), this, SLOT (movieBoxRedStateChanged(int)));
	connect(d->movieBoxYellow, SIGNAL(stateChanged(int)), this, SLOT (movieBoxYellowStateChanged(int)));
	connect(d->movieBoxGreen, SIGNAL(stateChanged(int)), this, SLOT (movieBoxGreenStateChanged(int)));

	connect(d->FlickerToggle, SIGNAL(clicked()), this, SLOT (flickerToggle()));
	connect(flickerTimer, SIGNAL(timeout()), this, SLOT(flickerToggle1()));

	connect(d->CheckerboardPattern, SIGNAL(valueChanged(double)), this, SLOT(setCheckerboardPattern(double)));
	
	connect(d->JacobianCheckBox, SIGNAL(stateChanged(int)), this, SLOT (jacobianStateChanged(int)));
	
	connect(d->InverseConsistCheckBox, SIGNAL(stateChanged(int)), this, SLOT (inverseConsistStateChanged(int)));

	// Glyph Parameters
	connect(d->InputGlyphPointMax, SIGNAL(valueChanged(double)), this, SLOT(setGlyphPointMax(double)));
	connect(d->InputGlyphScale, SIGNAL(valueChanged(double)), this, SLOT(setGlyphScale(double)));
	connect(d->InputGlyphScaleDirectional, SIGNAL(toggled(bool)), this, SLOT(setGlyphScaleDirectional(bool)));
	connect(d->InputGlyphScaleIsotropic, SIGNAL(toggled(bool)), this, SLOT(setGlyphScaleIsotropic(bool)));
	connect(d->InputGlyphThreshold, SIGNAL(valuesChanged(double, double)), this, SLOT(setGlyphThreshold(double, double)));
	connect(d->GenerateSeedButton, SIGNAL(clicked()), this, SLOT(setSeed()));
	connect(d->InputGlyphSeed, SIGNAL(valueChanged(int)), this, SLOT(setGlyphSeed(int)));
	connect(d->GlyphSourceComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setGlyphSourceOption(int)));
	// Arrow Parameters
	connect(d->InputGlyphArrowTipLength, SIGNAL(valueChanged(double)), this, SLOT(setGlyphArrowTipLength(double)));
	connect(d->InputGlyphArrowTipRadius, SIGNAL(valueChanged(double)), this, SLOT(setGlyphArrowTipRadius(double)));
	connect(d->InputGlyphArrowShaftRadius, SIGNAL(valueChanged(double)), this, SLOT(setGlyphArrowShaftRadius(double)));
	connect(d->InputGlyphArrowResolution, SIGNAL(valueChanged(double)), this, SLOT(setGlyphArrowResolution(double)));
	// Cone Parameters
	connect(d->InputGlyphConeHeight, SIGNAL(valueChanged(double)), this, SLOT(setGlyphConeHeight(double)));
	connect(d->InputGlyphConeRadius, SIGNAL(valueChanged(double)), this, SLOT(setGlyphConeRadius(double)));
	connect(d->InputGlyphConeResolution, SIGNAL(valueChanged(double)), this, SLOT(setGlyphConeResolution(double)));
	// Sphere Parameters
	connect(d->InputGlyphSphereResolution, SIGNAL(valueChanged(double)), this, SLOT(setGlyphSphereResolution(double)));

	// Grid Parameters
	connect(d->InputGridScale, SIGNAL(valueChanged(double)), this, SLOT(setGridScale(double)));
	connect(d->InputGridDensity, SIGNAL(valueChanged(double)), this, SLOT(setGridDensity(double)));

	// Block Parameters
	connect(d->InputBlockScale, SIGNAL(valueChanged(double)), this, SLOT(setBlockScale(double)));
	connect(d->InputBlockDisplacementCheck, SIGNAL(stateChanged(int)), this, SLOT(setBlockDisplacementCheck(int)));

	// Contour Parameters
	connect(d->InputContourNumber, SIGNAL(valueChanged(double)), this, SLOT(setContourNumber(double)));
	connect(d->InputContourRange, SIGNAL(valuesChanged(double, double)), this, SLOT(setContourRange(double, double)));

	// Glyph Slice Parameters
	connect(d->GlyphSliceComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(setGlyphSliceNode(vtkMRMLNode*)));
	connect(d->InputGlyphSlicePointMax, SIGNAL(valueChanged(double)), this, SLOT(setGlyphSlicePointMax(double)));
	connect(d->InputGlyphSliceThreshold, SIGNAL(valuesChanged(double, double)), this, SLOT(setGlyphSliceThreshold(double, double)));
	connect(d->InputGlyphSliceScale, SIGNAL(valueChanged(double)), this, SLOT(setGlyphSliceScale(double)));
	connect(d->InputGlyphSliceSeed, SIGNAL(valueChanged(int)), this, SLOT(setGlyphSliceSeed(int)));
	connect(d->GenerateSeedButton2, SIGNAL(clicked()), this, SLOT(setSeed2()));

	// Grid Slice Parameters
	connect(d->GridSliceComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(setGridSliceNode(vtkMRMLNode*)));
	connect(d->InputGridSliceScale, SIGNAL(valueChanged(double)), this, SLOT(setGridSliceScale(double)));
	connect(d->InputGridSliceDensity, SIGNAL(valueChanged(double)), this, SLOT(setGridSliceDensity(double)));

	connect(d->ApplyButton, SIGNAL(clicked()), this, SLOT(visualize()));
}

//-----------------------------------------------------------------------------
// Squared Difference
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::squaredDiffStateChanged(int state) {
	Q_D(const qSlicerRegistrationQualityModuleWidget);
	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	
	vtkSlicerRegistrationQualityLogic *logic = d->logic();
	logic->SquaredDifference(state);
	
	if (state){
	  d->squaredDiffMeanSpinBox->setValue(pNode->GetSquaredDiffStatistics()[0]);
	  d->squaredDiffSTDSpinBox->setValue(pNode->GetSquaredDiffStatistics()[1]);
	  }
	else{
	  d->squaredDiffMeanSpinBox->setValue(0);
	  d->squaredDiffSTDSpinBox->setValue(0);  
	  }

}
//-----------------------------------------------------------------------------
// Image Checks
//-----------------------------------------------------------------------------

void qSlicerRegistrationQualityModuleWidget::falseColorStateChanged(int state) {
	Q_D(const qSlicerRegistrationQualityModuleWidget);
	vtkSlicerRegistrationQualityLogic *logic = d->logic();
	logic->FalseColor(state);
}

void qSlicerRegistrationQualityModuleWidget::checkerboardStateChanged(int state){
	Q_D(const qSlicerRegistrationQualityModuleWidget);
	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();

	
	vtkSlicerRegistrationQualityLogic *logic = d->logic();
	logic->Checkerboard(state);

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
// Vector checks
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::jacobianStateChanged(int state){
	Q_D(const qSlicerRegistrationQualityModuleWidget);
	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	vtkSlicerRegistrationQualityLogic *logic = d->logic();
	
	logic->Jacobian(state);
	
	if (state){
	  d->jacobianMeanSpinBox->setValue(pNode->GetJacobianStatistics()[0]);
	  d->jacobianSTDSpinBox->setValue(pNode->GetJacobianStatistics()[1]);
	}
	else{
	  d->jacobianMeanSpinBox->setValue(0);
	  d->jacobianSTDSpinBox->setValue(0);	  
	}
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::inverseConsistStateChanged(int state){
	Q_D(const qSlicerRegistrationQualityModuleWidget);
	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	vtkSlicerRegistrationQualityLogic *logic = d->logic();
	
	logic->InverseConsist(state);
	
	if (state){
	  d->inverseConsistMeanSpinBox->setValue(pNode->GetInverseConsistStatistics()[0]);
	  d->inverseConsistSTDSpinBox->setValue(pNode->GetInverseConsistStatistics()[1]);
	}
	else{
	  d->inverseConsistMeanSpinBox->setValue(0);
	  d->inverseConsistSTDSpinBox->setValue(0);
	}
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

//-----------------------------------------------------------------------------
// Glyph parameters
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGlyphPointMax(double pointMax) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGlyphPointMax(pointMax);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setSeed() {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGlyphSeed((static_cast<long>(RAND_MAX)+1)*rand()+rand());
	d->InputGlyphSeed->setValue(pNode->GetGlyphSeed());
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGlyphSeed(int seed) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGlyphSeed(seed);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGlyphScale(double scale) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGlyphScale(scale);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGlyphScaleDirectional(bool state) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGlyphScaleDirectional(state);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGlyphScaleIsotropic(bool state) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGlyphScaleIsotropic(state);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGlyphThreshold(double min, double max) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGlyphThresholdMin(min);
	pNode->SetGlyphThresholdMax(max);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGlyphSourceOption(int option) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGlyphSourceOption(option);
	pNode->DisableModifiedEventOff();
	this->updateSourceOptions(option);
}

//-----------------------------------------------------------------------------
// Arrow parameters
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGlyphArrowTipLength(double length) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGlyphArrowTipLength(length);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGlyphArrowTipRadius(double radius) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGlyphArrowTipRadius(radius);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGlyphArrowShaftRadius(double radius) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGlyphArrowShaftRadius(radius);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGlyphArrowResolution(double resolution) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGlyphArrowResolution(resolution);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
// Cone Parameters
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGlyphConeHeight(double height) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGlyphConeHeight(height);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGlyphConeRadius(double radius) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGlyphConeRadius(radius);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGlyphConeResolution(double resolution) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGlyphConeResolution(resolution);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
// Sphere Parameters
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGlyphSphereResolution(double resolution) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGlyphSphereResolution(resolution);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGridScale(double scale) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGridScale(scale);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGridDensity(double density) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGridDensity(density);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setBlockScale(double scale) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetBlockScale(scale);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setBlockDisplacementCheck(int state) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetBlockDisplacementCheck(state);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setContourNumber(double number) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetContourNumber(number);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setContourRange(double min, double max) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetContourMin(min);
	pNode->SetContourMax(max);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGlyphSliceNode(vtkMRMLNode* node) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!node || !pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetAndObserveGlyphSliceNodeID(node->GetID());
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGlyphSlicePointMax(double pointMax) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGlyphSlicePointMax(pointMax);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGlyphSliceThreshold(double min, double max) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGlyphSliceThresholdMin(min);
	pNode->SetGlyphSliceThresholdMax(max);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGlyphSliceScale(double scale) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGlyphSliceScale(scale);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGlyphSliceSeed(int seed) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGlyphSliceSeed(seed);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setSeed2() {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGlyphSliceSeed((static_cast<long>(RAND_MAX)+1)*(long)rand()+rand()); //TODO: Integer overflow here. Why use two random numbers?
	d->InputGlyphSliceSeed->setValue(pNode->GetGlyphSliceSeed());
	pNode->DisableModifiedEventOff();
}


//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGridSliceNode(vtkMRMLNode* node) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!node || !pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetAndObserveGridSliceNodeID(node->GetID());
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGridSliceScale(double scale) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGridSliceScale(scale);
	pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::setGridSliceDensity(double density) {
	Q_D(qSlicerRegistrationQualityModuleWidget);

	vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
	if (!pNode || !this->mrmlScene()) {
		return;
	}
	pNode->DisableModifiedEventOn();
	pNode->SetGridSliceDensity(density);
	pNode->DisableModifiedEventOff();
}
