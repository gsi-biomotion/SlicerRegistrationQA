// SlicerQt includes
#include "qSlicerRegistrationQAModuleWidget.h"
#include "ui_qSlicerRegistrationQAModule.h"
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
#include "vtkSlicerRegistrationQALogic.h"
#include "vtkMRMLRegistrationQANode.h"


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
#include <vtkMRMLSegmentationNode.h>

// qMRMLWidget includes
#include "qMRMLSliceWidget.h"
#include "qMRMLSliceView.h"


// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkMRMLTableNode.h>
#include <vtkPointData.h>
#include <vtkNew.h>
#include <vtkDataArray.h>
#include <vtkGeneralTransform.h>
#include <vtkSmartPointer.h>
#include <vtkColorTransferFunction.h>

//C include
#include <time.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_RegistrationQA
// TODO: Keeping private for now until after fixes and enhancements
class qSlicerRegistrationQAModuleWidgetPrivate: public Ui_qSlicerRegistrationQAModule {
	Q_DECLARE_PUBLIC(qSlicerRegistrationQAModuleWidget);
protected:
	qSlicerRegistrationQAModuleWidget* const q_ptr;
public:
	qSlicerRegistrationQAModuleWidgetPrivate(qSlicerRegistrationQAModuleWidget& object);
	~qSlicerRegistrationQAModuleWidgetPrivate();
	vtkSlicerRegistrationQALogic* logic() const;
};

//-----------------------------------------------------------------------------
// qSlicerRegistrationQAModuleWidgetPrivate methods
//-----------------------------------------------------------------------------
qSlicerRegistrationQAModuleWidgetPrivate::qSlicerRegistrationQAModuleWidgetPrivate(
	qSlicerRegistrationQAModuleWidget& object) : q_ptr(&object) {
}
//-----------------------------------------------------------------------------
qSlicerRegistrationQAModuleWidgetPrivate::~qSlicerRegistrationQAModuleWidgetPrivate()
{
}
//-----------------------------------------------------------------------------
vtkSlicerRegistrationQALogic* qSlicerRegistrationQAModuleWidgetPrivate::logic() const {
	Q_Q( const qSlicerRegistrationQAModuleWidget );
	return vtkSlicerRegistrationQALogic::SafeDownCast( q->logic() );
}
//-----------------------------------------------------------------------------
// qSlicerRegistrationQAModuleWidget methods
//-----------------------------------------------------------------------------
qSlicerRegistrationQAModuleWidget::qSlicerRegistrationQAModuleWidget(QWidget* _parent)
	: Superclass( _parent )
	, d_ptr(new qSlicerRegistrationQAModuleWidgetPrivate(*this))
	, flickerTimer(new QTimer(this)) {
}

//-----------------------------------------------------------------------------
qSlicerRegistrationQAModuleWidget::~qSlicerRegistrationQAModuleWidget() {
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::setMRMLScene(vtkMRMLScene* scene) {
	Q_D(qSlicerRegistrationQAModuleWidget);
	this->Superclass::setMRMLScene(scene);

	// Find parameters node or create it if there is no one in the scene
	if (scene &&  d->logic()->GetRegistrationQANode() == 0) {
		vtkMRMLNode* node = scene->GetNthNodeByClass(0, "vtkMRMLRegistrationQANode");
		if (node) {
			this->setRegistrationQAParametersNode(
				vtkMRMLRegistrationQANode::SafeDownCast(node));
		}
	}
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::onSceneImportedEvent() {
	this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::enter() {
	this->onEnter();
	this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::onEnter() {
	if (!this->mrmlScene()) {
		return;
	}

	Q_D(qSlicerRegistrationQAModuleWidget);

	if (d->logic() == NULL) {
		return;
	}

	//Check for existing parameter node
	vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();

	if (pNode == NULL) {
		vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass(0,"vtkMRMLRegistrationQANode");
		if (node) {
			pNode = vtkMRMLRegistrationQANode::SafeDownCast(node);
			d->logic()->SetAndObserveRegistrationQANode(pNode);
		} else {
			vtkSmartPointer<vtkMRMLRegistrationQANode> newNode = vtkSmartPointer<vtkMRMLRegistrationQANode>::New();
			this->mrmlScene()->AddNode(newNode);
			d->logic()->SetAndObserveRegistrationQANode(newNode);
		}
	}
	this->updateWidgetFromMRML();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::setRegistrationQAParametersNode(vtkMRMLNode *node) {
	Q_D(qSlicerRegistrationQAModuleWidget);

	vtkMRMLRegistrationQANode* pNode = vtkMRMLRegistrationQANode::SafeDownCast(node);

	qvtkReconnect( d->logic()->GetRegistrationQANode(), pNode,
				   vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));

	d->logic()->SetAndObserveRegistrationQANode(pNode);
	this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::updateWidgetFromMRML() {
   Q_D(qSlicerRegistrationQAModuleWidget);

   vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();
   if (pNode && this->mrmlScene()) {
      d->ParameterComboBox->setCurrentNode(pNode);
      //Create backward RegistrationQAParameters, if it isn't there yet
      if ( pNode->GetBackwardRegistrationQAParameters() == NULL ){
         pNode->CreateBackwardParameters();
      }
      
      //Update Visualization Parameters
      d->patternSpinBox->setValue(pNode->GetCheckerboardPattern());
      
      if (pNode->GetBackwardRegistration()){
         d->BackwardButton->setText("Change to Forw. Reg.");
      }
      else{
         d->BackwardButton->setText("Change to Back. Reg.");
      }
      //Create table if necesarry
      vtkSmartPointer<vtkMRMLTableNode> tableNode = pNode->GetRegistrationQATableNode();
      if ( tableNode ){
//          tableNode = d->logic()->CreateDefaultRegistrationQATable();
//          if (tableNode == NULL){
//             d->StillErrorLabel->setText("Can't create default RegistrationQA table");
//             return;
//          }
//          pNode->DisableModifiedEventOn();
//          pNode->SetAndObserveRegistrationQATableNode(tableNode);
//          pNode->GetBackwardRegistrationQAParameters()->SetAndObserveRegistrationQATableNode(tableNode);
//          pNode->DisableModifiedEventOff();
//       }
      
         d->MRMLTableView->setMRMLTableNode(tableNode);
         d->MRMLTableView->hideRow(0);
      }
   }

   this->updateButtonsAndTable();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::updateButtonsAndTable() {
   Q_D(qSlicerRegistrationQAModuleWidget);

   vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();
   
   if ( ! pNode ){
      return;
   }

   bool currentScalarVolumeState = pNode->GetVolumeNodeID() && pNode->GetWarpedVolumeNodeID();
// 
   d->AbsoluteDiffPushButton->setEnabled(currentScalarVolumeState);
   d->FalseColor1PushButton->setEnabled(currentScalarVolumeState);
   d->CheckerboardPushButton->setEnabled(currentScalarVolumeState);
   d->FlickerToggle->setEnabled(currentScalarVolumeState);
   
   if (pNode->GetVolumeNodeID() &&  pNode->GetBackwardRegistrationQAParameters()->GetVolumeNodeID()){
      d->FalseColor2PushButton->setEnabled(true);
   }
   else{
      d->FalseColor2PushButton->setEnabled(false);
   }
   
   if ( pNode->GetMovieRun() ) {
      d->MovieToggle->setText("Stop Movie");
   }
   else{
      d->MovieToggle->setText("Start Movie");
   }

// //    
   bool vector = pNode->GetVectorVolumeNodeID();
   d->jacobianButton->setEnabled(vector);
   
   if ( pNode->GetBackwardRegistrationQAParameters() ){
      bool movingVector = vector && pNode->GetBackwardRegistrationQAParameters()->GetVectorVolumeNodeID();
      d->consistencyButton->setEnabled(movingVector);

      bool fiducials = pNode->GetFiducialNodeID() && pNode->GetBackwardRegistrationQAParameters()->GetFiducialNodeID();
      d->fiducialButton->setEnabled(fiducials);
 
      bool contour = pNode->GetSegmentID() &&
                     pNode->GetSegmentationNode() &&
                     pNode->GetBackwardRegistrationQAParameters()->GetSegmentID() &&
                     pNode->GetBackwardRegistrationQAParameters()->GetSegmentationNode() &&
                     vector;
                 
      d->contourButton->setEnabled(contour);
   }
   d->logic()->UpdateRegistrationQATable();
   //Change node to the right direction
   
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::vectorVolumeChanged(vtkMRMLNode* node) {
        Q_D(qSlicerRegistrationQAModuleWidget);

        // TODO: Move into updatefrommrml?
        vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();
        if (!pNode || !this->mrmlScene() || !node) {
                return;
        }

        pNode->DisableModifiedEventOn();
        if (node->IsA("vtkMRMLTransformNode")){
          if ( pNode->GetBackwardRegistration() ){
             pNode->GetBackwardRegistrationQAParameters()->SetAndObserveTransformNodeID(node->GetID());
          }
          else{
             pNode->SetAndObserveTransformNodeID(node->GetID());
          }
             
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
          if ( pNode->GetBackwardRegistration() ){
             pNode->GetBackwardRegistrationQAParameters()->SetAndObserveVectorVolumeNodeID(node->GetID());
          }
          else{
             pNode->SetAndObserveVectorVolumeNodeID(node->GetID());
          }

//        //Convert transform to vector
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
        this->updateButtonsAndTable();
        
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::invVectorVolumeChanged(vtkMRMLNode* node) {
  Q_D(qSlicerRegistrationQAModuleWidget);
  //Change to backward

  
  // TODO: Move into updatefrommrml?
  vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();
  if (!pNode || !this->mrmlScene() || !node) {
          return;
  }

  pNode->DisableModifiedEventOn();
  if (node->IsA("vtkMRMLTransformNode")){
    if ( pNode->GetBackwardRegistration() ){
       pNode->SetAndObserveTransformNodeID(node->GetID());
    }
    else{
       pNode->GetBackwardRegistrationQAParameters()->SetAndObserveTransformNodeID(node->GetID());
    }

            
//        //Convert transform to vector
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
    if ( pNode->GetBackwardRegistration() ){
       pNode->SetAndObserveVectorVolumeNodeID(node->GetID());
    }
    else{
       pNode->GetBackwardRegistrationQAParameters()->SetAndObserveVectorVolumeNodeID(node->GetID());
    }
    
//        //Convert transform to vector
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
  this->updateButtonsAndTable();

}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::fixedVolumeChanged(vtkMRMLNode* node) {

        Q_D(qSlicerRegistrationQAModuleWidget);

        //TODO: Move into updatefrommrml?
        vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();

        if (!pNode || !this->mrmlScene() || !node) {
                return;
        }
        
        pNode->DisableModifiedEventOn();
        if ( pNode->GetBackwardRegistration() ){
           pNode->GetBackwardRegistrationQAParameters()->SetAndObserveVolumeNodeID(node->GetID());
        }
        else{
           pNode->SetAndObserveVolumeNodeID(node->GetID());
        }
        pNode->DisableModifiedEventOff();
        this->updateButtonsAndTable();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::movingVolumeChanged(vtkMRMLNode* node) {

        Q_D(qSlicerRegistrationQAModuleWidget);

        vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();

        if (!pNode || !this->mrmlScene() || !node) {
                return;
        }

        pNode->DisableModifiedEventOn();
        if ( pNode->GetBackwardRegistration() ){
           pNode->SetAndObserveVolumeNodeID(node->GetID());
        }
        else{
           pNode->GetBackwardRegistrationQAParameters()->SetAndObserveVolumeNodeID(node->GetID());
        }
        pNode->DisableModifiedEventOff();
        this->updateButtonsAndTable();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::forwardWarpedVolumeChanged(vtkMRMLNode* node) {
        Q_D(qSlicerRegistrationQAModuleWidget);
        
        //TODO: Move into updatefrommrml?
        vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();
        if (!pNode || !this->mrmlScene() || !node) {
                return;
        }
        
        pNode->DisableModifiedEventOn();
        if ( pNode->GetBackwardRegistration() ){
           pNode->GetBackwardRegistrationQAParameters()->SetAndObserveWarpedVolumeNodeID(node->GetID());
           
        }
        else{
           pNode->SetAndObserveWarpedVolumeNodeID(node->GetID());
        }
        pNode->DisableModifiedEventOff();
        this->updateButtonsAndTable();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::backwardWarpedVolumeChanged(vtkMRMLNode* node) {
        Q_D(qSlicerRegistrationQAModuleWidget);
        
        //TODO: Move into updatefrommrml?
        vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();
        if (!pNode || !this->mrmlScene() || !node) {
                return;
        }

        pNode->DisableModifiedEventOn();
        if ( pNode->GetBackwardRegistration() ){
           pNode->SetAndObserveWarpedVolumeNodeID(node->GetID());
        }
        else{
           pNode->GetBackwardRegistrationQAParameters()->SetAndObserveWarpedVolumeNodeID(node->GetID());
        }
        pNode->DisableModifiedEventOff();
        this->updateButtonsAndTable();
}


//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::outputModelChanged(vtkMRMLNode* node) {
        Q_D(qSlicerRegistrationQAModuleWidget);

        //TODO: Move into updatefrommrml?
        vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();
        if (!pNode || !this->mrmlScene() || !node) {
                return;
        }

        pNode->DisableModifiedEventOn();
        pNode->SetAndObserveOutputModelNodeID(node->GetID());
        pNode->DisableModifiedEventOff();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::outputDirectoyChanged() {
        Q_D(qSlicerRegistrationQAModuleWidget);

        //TODO: Move into updatefrommrml?
        vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();
        if (!pNode || !this->mrmlScene()) {
                return;
        }
        QString fileName = QFileDialog::getExistingDirectory(NULL, QString( tr("Set directory for output file") ));
        
        if (fileName.isNull()) {
                return;
        }

        pNode->DisableModifiedEventOn();
        pNode->SetAndObserveOutputDirectory(fileName.toLatin1().data());
        pNode->DisableModifiedEventOff();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::ROIChanged(vtkMRMLNode* node) {
        Q_D(qSlicerRegistrationQAModuleWidget);

        //TODO: Move into updatefrommrml?
        vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();
        if (!pNode || !this->mrmlScene() || !node) {
                return;
        }

        pNode->DisableModifiedEventOn();
        pNode->SetAndObserveROINodeID(node->GetID());
        pNode->DisableModifiedEventOff();
        this->updateButtonsAndTable();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::FixedSegmentIDChanged(QString segmentID) {
        Q_D(qSlicerRegistrationQAModuleWidget);

        //TODO: Move into updatefrommrml?
        vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();
        if (!pNode || !this->mrmlScene() || segmentID.isEmpty() ) {
                return;
        }

        pNode->DisableModifiedEventOn();
        if ( pNode->GetBackwardRegistration() ){
           pNode->GetBackwardRegistrationQAParameters()->SetSegmentID(segmentID.toLatin1().constData());
           
        }
        else{
           pNode->SetSegmentID(segmentID.toLatin1().constData());
        }
        pNode->DisableModifiedEventOff();
        this->updateButtonsAndTable();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::MovingSegmentIDChanged(QString segmentID) {
  Q_D(qSlicerRegistrationQAModuleWidget);

  //TODO: Move into updatefrommrml?
  vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();
  if (!pNode || !this->mrmlScene() || segmentID.isEmpty() ) {
          return;
  }
  pNode->DisableModifiedEventOn();
  if ( pNode->GetBackwardRegistration() ){
     pNode->SetSegmentID(segmentID.toLatin1().constData());
  }
  else{
     pNode->GetBackwardRegistrationQAParameters()->SetSegmentID(segmentID.toLatin1().constData());
  }

  
  pNode->DisableModifiedEventOff();
  this->updateButtonsAndTable();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::fixedSegmentationNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerRegistrationQAModuleWidget);
  
  vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();
  if (!pNode || !this->mrmlScene() || !node) {
          return;
  }

  pNode->DisableModifiedEventOn();
  if ( pNode->GetBackwardRegistration() ){
     pNode->GetBackwardRegistrationQAParameters()->SetAndObserveSegmentationNode(vtkMRMLSegmentationNode::SafeDownCast(node));
     
  }
  else{
     pNode->SetAndObserveSegmentationNode(vtkMRMLSegmentationNode::SafeDownCast(node));
  }
  pNode->DisableModifiedEventOff();
  this->updateButtonsAndTable();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::movingSegmentationNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerRegistrationQAModuleWidget);
  
  vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();
  if (!pNode || !this->mrmlScene() || !node) {
          return;
  }

  pNode->DisableModifiedEventOn();
  if ( pNode->GetBackwardRegistration() ){
     pNode->SetAndObserveSegmentationNode(vtkMRMLSegmentationNode::SafeDownCast(node));
     
  }
  else{
     pNode->GetBackwardRegistrationQAParameters()->SetAndObserveSegmentationNode(vtkMRMLSegmentationNode::SafeDownCast(node));
  }
  pNode->DisableModifiedEventOff();
  this->updateButtonsAndTable();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::fixedFiducialChanged(vtkMRMLNode* node) {
        Q_D(qSlicerRegistrationQAModuleWidget);

        //TODO: Move into updatefrommrml?
        vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();
        if (!pNode || !this->mrmlScene() || !node) {
                return;
        }
        pNode->DisableModifiedEventOn();
        
        if ( pNode->GetBackwardRegistration() ){
           pNode->GetBackwardRegistrationQAParameters()->SetAndObserveFiducialNodeID(node->GetID());
        }
        else{
           pNode->SetAndObserveFiducialNodeID(node->GetID());
        }
        pNode->DisableModifiedEventOff();
        this->updateButtonsAndTable();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::movingFiducialChanged(vtkMRMLNode* node) {
        Q_D(qSlicerRegistrationQAModuleWidget);

        vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();
        
        if (!pNode || !this->mrmlScene() || !node) {
                return;
        }
        pNode->DisableModifiedEventOn();

        if ( pNode->GetBackwardRegistration() ){
           pNode->SetAndObserveFiducialNodeID(node->GetID());
           
        }
        else{
           pNode->GetBackwardRegistrationQAParameters()->SetAndObserveFiducialNodeID(node->GetID());
        }
        pNode->DisableModifiedEventOff();
        this->updateButtonsAndTable();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::onLogicModified() {
        this->updateWidgetFromMRML();
}
void qSlicerRegistrationQAModuleWidget::setup() {
	Q_D(qSlicerRegistrationQAModuleWidget);
	d->setupUi(this);
	this->Superclass::setup();
	d->StillErrorLabel->setVisible(false);

	connect(d->ParameterComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(setRegistrationQAParametersNode(vtkMRMLNode*)));
	
	connect(d->SaveScreenshotPushButton, SIGNAL(clicked()), this, SLOT(saveScreenshotClicked()));
	connect(d->SaveOutputFilePushButton, SIGNAL(clicked()), this, SLOT(saveOutputFileClicked()));
        
        connect(d->ROIaroundSegmentButton, SIGNAL(clicked()), this, SLOT (ROIaroundSegmentClicked()));
        connect(d->BackwardButton, SIGNAL(clicked()), this, SLOT (RegistrationDirectionChanged()));
	
	connect(d->FalseColor1PushButton, SIGNAL(clicked()), this, SLOT (falseColor1Clicked()));
        connect(d->FalseColor2PushButton, SIGNAL(clicked()), this, SLOT (falseColor2Clicked()));
	connect(d->CheckerboardPushButton, SIGNAL(clicked()), this, SLOT (checkerboardClicked()));
	connect(d->AbsoluteDiffPushButton, SIGNAL(clicked()), this, SLOT (absoluteDiffClicked()));
        connect(d->fiducialButton, SIGNAL(clicked()), this, SLOT (fiducialClicked()));
	connect(d->jacobianButton, SIGNAL(clicked()), this, SLOT (jacobianClicked()));
	connect(d->consistencyButton, SIGNAL(clicked()), this, SLOT (inverseConsistClicked()));
        connect(d->contourButton, SIGNAL(clicked()), this, SLOT (contourButtonClicked()));

	connect(d->patternSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setCheckerboardPattern(int)));

	connect(d->MovieToggle, SIGNAL(clicked()), this, SLOT (movieToggle()));
	connect(d->movieBoxRed, SIGNAL(stateChanged(int)), this, SLOT (movieBoxRedStateChanged(int)));
	connect(d->movieBoxYellow, SIGNAL(stateChanged(int)), this, SLOT (movieBoxYellowStateChanged(int)));
	connect(d->movieBoxGreen, SIGNAL(stateChanged(int)), this, SLOT (movieBoxGreenStateChanged(int)));

	connect(d->FlickerToggle, SIGNAL(clicked()), this, SLOT (flickerToggle()));
	connect(flickerTimer, SIGNAL(timeout()), this, SLOT(flickerToggle1()));
        this->updateButtonsAndTable();
}
//-----------------------------------------------------------------------------
// Create ROI around Segments
//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::ROIaroundSegmentClicked() {
   Q_D(const qSlicerRegistrationQAModuleWidget);
   d->logic()->CreateROI();
}
//-----------------------------------------------------------------------------
// Interchange forward and backward registration direction
//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::RegistrationDirectionChanged() {
   Q_D(const qSlicerRegistrationQAModuleWidget);
   d->logic()->SwitchRegistrationDirection();
   this->updateWidgetFromMRML();
}
//-----------------------------------------------------------------------------
// Output File
//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::saveScreenshotClicked() {
        Q_D(const qSlicerRegistrationQAModuleWidget);
        vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();

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
void qSlicerRegistrationQAModuleWidget::saveOutputFileClicked() {
        Q_D(const qSlicerRegistrationQAModuleWidget);
        vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();

        
        
        if (! pNode->GetOutputDirectory() ) {
                this->outputDirectoyChanged();
        }
      
        QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
        try {
                d->logic()->SaveOutputFile();
        } catch (std::runtime_error e) {
                d->StillErrorLabel->setText(e.what());
                d->StillErrorLabel->setVisible(true);
                cerr << e.what() << endl;
                QApplication::restoreOverrideCursor();
                return;
        }
        QApplication::restoreOverrideCursor();
}
//-----------------------------------------------------------------------------
// Absolute Difference
//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::absoluteDiffClicked() {
        Q_D(const qSlicerRegistrationQAModuleWidget);

        QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
        try {
                d->logic()->CalculateRegQAFrom(1);
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
void qSlicerRegistrationQAModuleWidget::fiducialClicked() {
        Q_D(const qSlicerRegistrationQAModuleWidget);
        
        //Check if we want to calculate reference or inverse distance
        int number = 4;
//      if (reference) number = 4;
//      else number = 5;

        QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
        try {
                d->logic()->CalculateRegQAFrom(number);
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
// False Color 1
//-----------------------------------------------------------------------------

void qSlicerRegistrationQAModuleWidget::falseColor1Clicked() {
        Q_D(const qSlicerRegistrationQAModuleWidget);
        bool invertColor, matchLevels;
        
        if (d->ColorCheckBox->checkState() == 0) invertColor = false;
        else invertColor = true;
        if (d->MatchCheckBox->checkState() == 0) matchLevels = false;
        else matchLevels = true;
        try {
                d->logic()->FalseColor(invertColor, false, matchLevels);
        } catch (std::runtime_error e) {
                d->StillErrorLabel->setText(e.what());
                d->StillErrorLabel->setVisible(true);
                d->FalseColor1PushButton->toggle();
                cerr << e.what() << endl;
                return;
        }
        d->StillErrorLabel->setText("");
}

//-----------------------------------------------------------------------------
// False Color 2
//-----------------------------------------------------------------------------

void qSlicerRegistrationQAModuleWidget::falseColor2Clicked() {
        Q_D(const qSlicerRegistrationQAModuleWidget);
        bool invertColor, matchLevels;
        
        if (d->ColorCheckBox->checkState() == 0) invertColor = false;
        else invertColor = true;
        if (d->MatchCheckBox->checkState() == 0) matchLevels = false;
        else matchLevels = true;
        try {
                d->logic()->FalseColor(invertColor, true, matchLevels);
        } catch (std::runtime_error e) {
                d->StillErrorLabel->setText(e.what());
                d->StillErrorLabel->setVisible(true);
                d->FalseColor2PushButton->toggle();
                cerr << e.what() << endl;
                return;
        }
        d->StillErrorLabel->setText("");
}

//-----------------------------------------------------------------------------
// Checkerboard
//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::checkerboardClicked(){
        Q_D(const qSlicerRegistrationQAModuleWidget);
//      vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();
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

void qSlicerRegistrationQAModuleWidget::flickerToggle(){
        if(!flickerTimer->isActive()) {
//              cerr << "Starting timer" << endl;
                flickerToggle1();
                flickerTimer->start(500);
        } else {
//              cerr << "Stopping timer" << endl;
                flickerTimer->stop();
        }
}

// TODO: Move this to Logic
void qSlicerRegistrationQAModuleWidget::flickerToggle1(){
        Q_D(const qSlicerRegistrationQAModuleWidget);

//      cerr << "Timer timeout" << endl;
        vtkSlicerRegistrationQALogic *logic = d->logic();
        vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();

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

void qSlicerRegistrationQAModuleWidget::movieToggle(){
        Q_D(const qSlicerRegistrationQAModuleWidget);

        vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();
        
        //Alternate between start and stop
        pNode->DisableModifiedEventOn();
        if ( pNode->GetMovieRun() ) {
           d->MovieToggle->setText("Start Movie");
           pNode->MovieRunOff();
        }
        else{
           d->MovieToggle->setText("Stop Movie");
           pNode->MovieRunOn();
        }
        pNode->DisableModifiedEventOff();

        vtkSlicerRegistrationQALogic *logic = d->logic();
        logic->Movie();
        pNode->MovieRunOff();
        this->updateButtonsAndTable();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::movieBoxRedStateChanged(int state) {
        Q_D(qSlicerRegistrationQAModuleWidget);

        vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();
        if (!pNode || !this->mrmlScene()) {
                return;
        }
        pNode->DisableModifiedEventOn();
        pNode->SetMovieBoxRedState(state);
        pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::movieBoxYellowStateChanged(int state) {
        Q_D(qSlicerRegistrationQAModuleWidget);

        vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();
        if (!pNode || !this->mrmlScene()) {
                return;
        }
        pNode->DisableModifiedEventOn();
        pNode->SetMovieBoxYellowState(state);
        pNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::movieBoxGreenStateChanged(int state) {
        Q_D(qSlicerRegistrationQAModuleWidget);

        vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();
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
void qSlicerRegistrationQAModuleWidget::jacobianClicked(){
        Q_D(const qSlicerRegistrationQAModuleWidget);
        QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
        try {
                d->logic()->CalculateRegQAFrom(2);
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
void qSlicerRegistrationQAModuleWidget::inverseConsistClicked(){
        Q_D(const qSlicerRegistrationQAModuleWidget);

        QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
        try {
                d->logic()->CalculateRegQAFrom(3);
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
//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModuleWidget::contourButtonClicked(){
        Q_D(const qSlicerRegistrationQAModuleWidget);

        QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
        try {
                d->logic()->CalculateContourStatistic();
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
void qSlicerRegistrationQAModuleWidget::setCheckerboardPattern(int checkboardPattern) {
        Q_D(qSlicerRegistrationQAModuleWidget);

        vtkMRMLRegistrationQANode* pNode = d->logic()->GetRegistrationQANode();
        if (!pNode || !this->mrmlScene()) {
                return;
        }

        pNode->DisableModifiedEventOn();
        pNode->SetCheckerboardPattern(checkboardPattern);
        pNode->DisableModifiedEventOff();
}
//-----------------------------------------------------------
bool qSlicerRegistrationQAModuleWidget::setEditedNode(
  vtkMRMLNode* node, QString role /* = QString()*/, QString context /* = QString() */)
{
//   Q_D(qSlicerRegistrationQAModuleWidget);
  this->updateButtonsAndTable();
  return true;
/*
  if ( QString::compare(role, "fixedImage", Qt::CaseInsensitive) == 0 ){
     this->fixedVolumeChanged(node);
  }
  else if ( QString::compare(role, "movingImage", Qt::CaseInsensitive) == 0 ){
     this->movingVolumeChanged(node);
  }
  else if ( QString::compare(role, "forwardWarpedImage", Qt::CaseInsensitive) == 0 ){
     this->forwardWarpedVolumeChanged(node);
  }
  else if ( QString::compare(role, "backwardWarpedImage", Qt::CaseInsensitive) == 0 ){
     this->backwardWarpedVolumeChanged(node);
  }
  else if ( QString::compare(role, "fixedVectorField", Qt::CaseInsensitive) == 0 ){
     this->vectorVolumeChanged(node);
  }
  else if ( QString::compare(role, "movingVectorField", Qt::CaseInsensitive) == 0 ){
     this->invVectorVolumeChanged(node);
  }
  else if ( QString::compare(role, "fixedFiducials", Qt::CaseInsensitive) == 0 ){
     this->fixedFiducialChanged(node);
  }
  else if ( QString::compare(role, "movingFiducials", Qt::CaseInsensitive) == 0 ){
     this->movingFiducialChanged(node);
  }
  else if ( QString::compare(role, "ROI", Qt::CaseInsensitive) == 0 ){
     this->ROIChanged(node);
  }
  else if ( QString::compare(role, "fixedSegmentationNode", Qt::CaseInsensitive) == 0 ){
     this->fixedSegmentationNodeChanged(node);
  }
  else if ( QString::compare(role, "movingSegmentationNode", Qt::CaseInsensitive) == 0 ){
     this->movingSegmentationNodeChanged(node);
  }
  else if ( QString::compare(role, "fixedSegmentID", Qt::CaseInsensitive) == 0 ){
     this->FixedSegmentIDChanged(context);
  }
  else if ( QString::compare(role, "movingSegmentID", Qt::CaseInsensitive) == 0 ){
     this->MovingSegmentIDChanged(context);
  }
//   else if ( QString::compare(role, "itemID", Qt::CaseInsensitive) == 0 ){
//      d->logic()->UpdateNodeFromSHNode(context);
//   }
  else {
     return false;
  }
  std::cerr << "Set: " << node->GetName() << " to " << role.toLatin1().constData() << " " << context.toLatin1().constData() << "\n";
  return true;*/
}