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
		
		if (pNode->GetROINodeID()) {
			d->ROIInputComboBox->setCurrentNode(pNode->GetROINodeID());
		} else {
			this->ROIChanged(d->ROIInputComboBox->currentNode());
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

		d->movieBoxRed->setChecked(pNode->GetMovieBoxRedState());
		d->jacobianMeanSpinBox->setValue(pNode->GetJacobianStatistics()[0]);
		d->jacobianSTDSpinBox->setValue(pNode->GetJacobianStatistics()[1]);
// 		pNode->SetInverseConsistStatistics(0,0,0,0);

		d->inverseConsistMeanSpinBox->setValue(pNode->GetInverseConsistStatistics()[0]);
		d->inverseConsistSTDSpinBox->setValue(pNode->GetInverseConsistStatistics()[1]);


		//Update Visualization Parameters
		d->CheckerboardPattern->setValue(pNode->GetCheckerboardPattern());
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
		return;
	}

	d->JacobianCheckBox->setEnabled(true);
	d->InverseConsistCheckBox->setEnabled(true);

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
	connect(d->OutputModelComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(outputModelChanged(vtkMRMLNode*)));
	connect(d->ROIInputComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(ROIChanged(vtkMRMLNode*)));
//	connect(d->OutputCheckerboardComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(checkerboardVolumeChanged(vtkMRMLNode*)));
//	connect(d->AbsoluteDiffComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(absoluteDiffVolumeChanged(vtkMRMLNode*)));

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
}

//-----------------------------------------------------------------------------
// Squared Difference
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
	QApplication::restoreOverrideCursor();

}
//-----------------------------------------------------------------------------
// Image Checks
//-----------------------------------------------------------------------------

void qSlicerRegistrationQualityModuleWidget::falseColorClicked(bool state) {
	Q_D(const qSlicerRegistrationQualityModuleWidget);

	try {
	  std::cerr << "Trying like crazy!" << std::endl;
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
	QApplication::restoreOverrideCursor();
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
