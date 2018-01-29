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
};

//-----------------------------------------------------------------------------
// qSlicerRegistrationQualityModuleWidgetPrivate methods
//-----------------------------------------------------------------------------
qSlicerRegistrationQualityModuleWidgetPrivate::qSlicerRegistrationQualityModuleWidgetPrivate(
	qSlicerRegistrationQualityModuleWidget& object) : q_ptr(&object) {
}
//-----------------------------------------------------------------------------
qSlicerRegistrationQualityModuleWidgetPrivate::~qSlicerRegistrationQualityModuleWidgetPrivate()
{
}
//-----------------------------------------------------------------------------
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
      //Create backward RegQAParameters, if it isn't there yet
      if ( pNode->GetBackwardRegQAParameters() == NULL ){
         d->logic()->CreateBackwardParameters(pNode);
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
      vtkSmartPointer<vtkMRMLTableNode> tableNode = pNode->GetRegQATableNode();
      if ( ! tableNode ){
         tableNode = d->logic()->CreateDefaultRegQATable();
         if (tableNode == NULL){
            d->StillErrorLabel->setText("Can't create default RegQA table");
            return;
         }
         pNode->DisableModifiedEventOn();
         pNode->SetAndObserveRegQATableNode(tableNode);
         pNode->GetBackwardRegQAParameters()->SetAndObserveRegQATableNode(tableNode);
         pNode->DisableModifiedEventOff();
      }
      
      d->MRMLTableView->setMRMLTableNode(tableNode);
      d->MRMLTableView->hideRow(0);
   }

   this->updateButtonsAndTable();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::updateButtonsAndTable() {
   Q_D(qSlicerRegistrationQualityModuleWidget);

   vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
   
   if ( ! pNode ){
      return;
   }

   bool currentScalarVolumeState = pNode->GetVolumeNodeID() && pNode->GetWarpedVolumeNodeID();
// 
   d->AbsoluteDiffPushButton->setEnabled(currentScalarVolumeState);
   d->FalseColor1PushButton->setEnabled(currentScalarVolumeState);
   d->CheckerboardPushButton->setEnabled(currentScalarVolumeState);
   d->FlickerToggle->setEnabled(currentScalarVolumeState);
   
   if (pNode->GetVolumeNodeID() &&  pNode->GetBackwardRegQAParameters()->GetVolumeNodeID()){
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
   
   if ( pNode->GetBackwardRegQAParameters() ){
      bool movingVector = vector && pNode->GetBackwardRegQAParameters()->GetVectorVolumeNodeID();
      d->consistencyButton->setEnabled(movingVector);

      bool fiducials = pNode->GetFiducialNodeID() && pNode->GetBackwardRegQAParameters()->GetFiducialNodeID();
      d->fiducialButton->setEnabled(fiducials);
 
      bool contour = pNode->GetSegmentID() &&
                     pNode->GetSegmentationNode() &&
                     pNode->GetBackwardRegQAParameters()->GetSegmentID() &&
                     pNode->GetBackwardRegQAParameters()->GetSegmentationNode() &&
                     vector;
                 
      d->contourButton->setEnabled(contour);
   }
   d->logic()->UpdateRegQATable();
   //Change node to the right direction
   
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::vectorVolumeChanged(vtkMRMLNode* node) {
        Q_D(qSlicerRegistrationQualityModuleWidget);

        // TODO: Move into updatefrommrml?
        vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
        if (!pNode || !this->mrmlScene() || !node) {
                return;
        }

        pNode->DisableModifiedEventOn();
        if (node->IsA("vtkMRMLTransformNode")){
          if ( pNode->GetBackwardRegistration() ){
             pNode->GetBackwardRegQAParameters()->SetAndObserveTransformNodeID(node->GetID());
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
             pNode->GetBackwardRegQAParameters()->SetAndObserveVectorVolumeNodeID(node->GetID());
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
void qSlicerRegistrationQualityModuleWidget::invVectorVolumeChanged(vtkMRMLNode* node) {
  Q_D(qSlicerRegistrationQualityModuleWidget);
  //Change to backward

  
  // TODO: Move into updatefrommrml?
  vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
  if (!pNode || !this->mrmlScene() || !node) {
          return;
  }

  pNode->DisableModifiedEventOn();
  if (node->IsA("vtkMRMLTransformNode")){
    if ( pNode->GetBackwardRegistration() ){
       pNode->SetAndObserveTransformNodeID(node->GetID());
    }
    else{
       pNode->GetBackwardRegQAParameters()->SetAndObserveTransformNodeID(node->GetID());
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
       pNode->GetBackwardRegQAParameters()->SetAndObserveVectorVolumeNodeID(node->GetID());
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
void qSlicerRegistrationQualityModuleWidget::fixedVolumeChanged(vtkMRMLNode* node) {

        Q_D(qSlicerRegistrationQualityModuleWidget);

        //TODO: Move into updatefrommrml?
        vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();

        if (!pNode || !this->mrmlScene() || !node) {
                return;
        }
        
        pNode->DisableModifiedEventOn();
        if ( pNode->GetBackwardRegistration() ){
           pNode->GetBackwardRegQAParameters()->SetAndObserveVolumeNodeID(node->GetID());
        }
        else{
           pNode->SetAndObserveVolumeNodeID(node->GetID());
        }
        pNode->DisableModifiedEventOff();
        this->updateButtonsAndTable();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::movingVolumeChanged(vtkMRMLNode* node) {

        Q_D(qSlicerRegistrationQualityModuleWidget);

        vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();

        if (!pNode || !this->mrmlScene() || !node) {
                return;
        }

        pNode->DisableModifiedEventOn();
        if ( pNode->GetBackwardRegistration() ){
           pNode->SetAndObserveVolumeNodeID(node->GetID());
        }
        else{
           pNode->GetBackwardRegQAParameters()->SetAndObserveVolumeNodeID(node->GetID());
        }
        pNode->DisableModifiedEventOff();
        this->updateButtonsAndTable();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::forwardWarpedVolumeChanged(vtkMRMLNode* node) {
        Q_D(qSlicerRegistrationQualityModuleWidget);
        
        //TODO: Move into updatefrommrml?
        vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
        if (!pNode || !this->mrmlScene() || !node) {
                return;
        }
        
        pNode->DisableModifiedEventOn();
        if ( pNode->GetBackwardRegistration() ){
           pNode->GetBackwardRegQAParameters()->SetAndObserveWarpedVolumeNodeID(node->GetID());
           
        }
        else{
           pNode->SetAndObserveWarpedVolumeNodeID(node->GetID());
        }
        pNode->DisableModifiedEventOff();
        this->updateButtonsAndTable();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::backwardWarpedVolumeChanged(vtkMRMLNode* node) {
        Q_D(qSlicerRegistrationQualityModuleWidget);
        
        //TODO: Move into updatefrommrml?
        vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
        if (!pNode || !this->mrmlScene() || !node) {
                return;
        }

        pNode->DisableModifiedEventOn();
        if ( pNode->GetBackwardRegistration() ){
           pNode->SetAndObserveWarpedVolumeNodeID(node->GetID());
        }
        else{
           pNode->GetBackwardRegQAParameters()->SetAndObserveWarpedVolumeNodeID(node->GetID());
        }
        pNode->DisableModifiedEventOff();
        this->updateButtonsAndTable();
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
        this->updateButtonsAndTable();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::FixedSegmentIDChanged(QString segmentID) {
        Q_D(qSlicerRegistrationQualityModuleWidget);

        //TODO: Move into updatefrommrml?
        vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
        if (!pNode || !this->mrmlScene() || segmentID.isEmpty() ) {
                return;
        }

        pNode->DisableModifiedEventOn();
        if ( pNode->GetBackwardRegistration() ){
           pNode->GetBackwardRegQAParameters()->SetSegmentID(segmentID.toLatin1().constData());
           
        }
        else{
           pNode->SetSegmentID(segmentID.toLatin1().constData());
        }
        pNode->DisableModifiedEventOff();
        this->updateButtonsAndTable();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::MovingSegmentIDChanged(QString segmentID) {
  Q_D(qSlicerRegistrationQualityModuleWidget);

  //TODO: Move into updatefrommrml?
  vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
  if (!pNode || !this->mrmlScene() || segmentID.isEmpty() ) {
          return;
  }
  pNode->DisableModifiedEventOn();
  if ( pNode->GetBackwardRegistration() ){
     pNode->SetSegmentID(segmentID.toLatin1().constData());
  }
  else{
     pNode->GetBackwardRegQAParameters()->SetSegmentID(segmentID.toLatin1().constData());
  }

  
  pNode->DisableModifiedEventOff();
  this->updateButtonsAndTable();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::fixedSegmentationNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerRegistrationQualityModuleWidget);
  
  vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
  if (!pNode || !this->mrmlScene() || !node) {
          return;
  }

  pNode->DisableModifiedEventOn();
  if ( pNode->GetBackwardRegistration() ){
     pNode->GetBackwardRegQAParameters()->SetAndObserveSegmentationNode(vtkMRMLSegmentationNode::SafeDownCast(node));
     
  }
  else{
     pNode->SetAndObserveSegmentationNode(vtkMRMLSegmentationNode::SafeDownCast(node));
  }
  pNode->DisableModifiedEventOff();
  this->updateButtonsAndTable();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::movingSegmentationNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerRegistrationQualityModuleWidget);
  
  vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
  if (!pNode || !this->mrmlScene() || !node) {
          return;
  }

  pNode->DisableModifiedEventOn();
  if ( pNode->GetBackwardRegistration() ){
     pNode->SetAndObserveSegmentationNode(vtkMRMLSegmentationNode::SafeDownCast(node));
     
  }
  else{
     pNode->GetBackwardRegQAParameters()->SetAndObserveSegmentationNode(vtkMRMLSegmentationNode::SafeDownCast(node));
  }
  pNode->DisableModifiedEventOff();
  this->updateButtonsAndTable();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::fixedFiducialChanged(vtkMRMLNode* node) {
        Q_D(qSlicerRegistrationQualityModuleWidget);

        //TODO: Move into updatefrommrml?
        vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
        if (!pNode || !this->mrmlScene() || !node) {
                return;
        }
        pNode->DisableModifiedEventOn();
        
        if ( pNode->GetBackwardRegistration() ){
           pNode->GetBackwardRegQAParameters()->SetAndObserveFiducialNodeID(node->GetID());
        }
        else{
           pNode->SetAndObserveFiducialNodeID(node->GetID());
        }
        pNode->DisableModifiedEventOff();
        this->updateButtonsAndTable();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::movingFiducialChanged(vtkMRMLNode* node) {
        Q_D(qSlicerRegistrationQualityModuleWidget);

        vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
        
        if (!pNode || !this->mrmlScene() || !node) {
                return;
        }
        pNode->DisableModifiedEventOn();

        if ( pNode->GetBackwardRegistration() ){
           pNode->SetAndObserveFiducialNodeID(node->GetID());
           
        }
        else{
           pNode->GetBackwardRegQAParameters()->SetAndObserveFiducialNodeID(node->GetID());
        }
        pNode->DisableModifiedEventOff();
        this->updateButtonsAndTable();
}
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::onLogicModified() {
        this->updateWidgetFromMRML();
}
void qSlicerRegistrationQualityModuleWidget::setup() {
	Q_D(qSlicerRegistrationQualityModuleWidget);
	d->setupUi(this);
	this->Superclass::setup();
	d->StillErrorLabel->setVisible(false);

	connect(d->ParameterComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(setRegistrationQualityParametersNode(vtkMRMLNode*)));
	
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
void qSlicerRegistrationQualityModuleWidget::ROIaroundSegmentClicked() {
   Q_D(const qSlicerRegistrationQualityModuleWidget);
   d->logic()->CreateROI();
}
//-----------------------------------------------------------------------------
// Interchange forward and backward registration direction
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::RegistrationDirectionChanged() {
   Q_D(const qSlicerRegistrationQualityModuleWidget);
   d->logic()->UpdateRegistrationDirection();
   this->updateWidgetFromMRML();
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
//      if (reference) number = 4;
//      else number = 5;

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
// False Color 1
//-----------------------------------------------------------------------------

void qSlicerRegistrationQualityModuleWidget::falseColor1Clicked() {
        Q_D(const qSlicerRegistrationQualityModuleWidget);
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

void qSlicerRegistrationQualityModuleWidget::falseColor2Clicked() {
        Q_D(const qSlicerRegistrationQualityModuleWidget);
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
void qSlicerRegistrationQualityModuleWidget::checkerboardClicked(){
        Q_D(const qSlicerRegistrationQualityModuleWidget);
//      vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
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
void qSlicerRegistrationQualityModuleWidget::flickerToggle1(){
        Q_D(const qSlicerRegistrationQualityModuleWidget);

//      cerr << "Timer timeout" << endl;
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

        vtkMRMLRegistrationQualityNode* pNode = d->logic()->GetRegistrationQualityNode();
        
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

        vtkSlicerRegistrationQualityLogic *logic = d->logic();
        logic->Movie();
        this->updateButtonsAndTable();
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
//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModuleWidget::contourButtonClicked(){
        Q_D(const qSlicerRegistrationQualityModuleWidget);

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
//-----------------------------------------------------------
bool qSlicerRegistrationQualityModuleWidget::setEditedNode(
  vtkMRMLNode* node, QString role /* = QString()*/, QString context /* = QString() */)
{
//   Q_D(qSlicerRegistrationQualityModuleWidget);
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