/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/


#include "qSlicerSubjectHierarchyRegistrationQAPlugin.h"

// SubjectHierarchy MRML includes
#include "vtkMRMLSubjectHierarchyConstants.h"
#include "vtkMRMLSubjectHierarchyNode.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLVectorVolumeNode.h>
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLSegmentationNode.h>
#include <vtkMRMLTableNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLGridTransformNode.h>
// #include "vtkMRMLMarkupsNode.h"
#include "vtkSlicerRegistrationQALogic.h"
#include "vtkMRMLRegistrationQANode.h"
#include <vtkSlicerVolumesLogic.h>
#include <vtkSlicerSegmentationsModuleLogic.h>
#include <vtkSlicerAnnotationModuleLogic.h>
#include <vtkMRMLColorTableNode.h>

// #include <vtkSlicerDoseVolumeHistogramModuleLogic.h>
// #include <vtkMRMLDoseVolumeHistogramNode.h>


#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLVectorVolumeDisplayNode.h>
#include <vtkMRMLLabelMapVolumeDisplayNode.h>
#include <vtkMRMLVolumeArchetypeStorageNode.h>
#include <vtkMRMLAnnotationROINode.h>
#include <vtkSlicerMarkupsLogic.h>

#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>
#include <vtkSlicerScriptedLoadableModuleLogic.h>
#include <qSlicerAbstractCoreModule.h>


// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkNew.h>
#include <vtkMatrix4x4.h>
#include <vtkIdList.h>
#include <vtkSegment.h>
#include <vtkDataObject.h>
#include <vtkOrientedImageData.h>
#include <vtksys/SystemTools.hxx>
#include <vtkStringArray.h>
#include <vtkAbstractArray.h>
#include <vtkTable.h>
#include <vtkVariant.h>
#include <vtkImageData.h>
#include <vtkImageAppendComponents.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageMathematics.h>
#include <vtkCollection.h>
#include <vtkObject.h>


// Qt includes
#include <QDebug>
#include <QStandardItem>
#include <QAction>
#include <QMenu>


// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerAbstractModule.h"
#include "qSlicerModuleManager.h"
#include "qSlicerAbstractModuleWidget.h"


// MRML widgets includes
#include "qMRMLNodeComboBox.h"


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Plugins
class qSlicerSubjectHierarchyRegistrationQAPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyRegistrationQAPlugin);
protected:
  qSlicerSubjectHierarchyRegistrationQAPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyRegistrationQAPluginPrivate(qSlicerSubjectHierarchyRegistrationQAPlugin& object);
  ~qSlicerSubjectHierarchyRegistrationQAPluginPrivate();
  void init();
public:
  QMenu* regQAMenu;
  QActionGroup* regQAMenuActionGroup;
  QIcon regQAIcon;
  vtkSlicerRegistrationQALogic* regQALogic;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyRegistrationQAPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyRegistrationQAPluginPrivate::qSlicerSubjectHierarchyRegistrationQAPluginPrivate(qSlicerSubjectHierarchyRegistrationQAPlugin& object)
 : q_ptr(&object)
{
  this->regQAMenu = NULL;
  this->regQAMenuActionGroup = NULL;
  this->regQAIcon = QIcon(":Icons/ContourIcon.png");
  this->regQALogic = NULL;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyRegistrationQAPlugin);

  
  
  this->regQAMenu = new QMenu("Registration QA");
  this->regQAMenuActionGroup = new QActionGroup(q);
  
  QMenu* assignMenu = this->regQAMenu->addMenu("Assign node to:");
  QMenu* imageMenu = assignMenu->addMenu( "Image" );
  
  QAction* referenceImageAction = new QAction("Reference Image",q);
  QObject::connect(referenceImageAction, SIGNAL(triggered()), q, SLOT(referenceImageSelected()));
  referenceImageAction->setActionGroup(this->regQAMenuActionGroup);
  referenceImageAction->setData(QVariant("Reference Image"));
  imageMenu->addAction(referenceImageAction);
  
  QAction* movingImageAction = new QAction("Moving Image",q);
  QObject::connect(movingImageAction, SIGNAL(triggered()), q, SLOT(movingImageSelected()));
  movingImageAction->setActionGroup(this->regQAMenuActionGroup);
  movingImageAction->setData(QVariant("Moving Image"));
  imageMenu->addAction(movingImageAction);
  
  QAction* fbwarpedImageAction = new QAction("Forward Warped Image",q);
  QObject::connect(fbwarpedImageAction, SIGNAL(triggered()), q, SLOT(fwarpedImageSelected()));
  fbwarpedImageAction->setActionGroup(this->regQAMenuActionGroup);
  fbwarpedImageAction->setData(QVariant("Forward Warped Image"));
  imageMenu->addAction(fbwarpedImageAction);
  
  QAction* bwarpedImageAction = new QAction("Backward Warped Image",q);
  QObject::connect(bwarpedImageAction, SIGNAL(triggered()), q, SLOT(bwarpedImageSelected()));
  bwarpedImageAction->setActionGroup(this->regQAMenuActionGroup);
  bwarpedImageAction->setData(QVariant("Backward Warped Image"));
  imageMenu->addAction(bwarpedImageAction);

  QMenu* vectorMenu = assignMenu->addMenu( "Vector Field" );
  QAction* referenceVFAction = new QAction("Forward",q);
  QObject::connect(referenceVFAction, SIGNAL(triggered()), q, SLOT(referenceVectorFieldSelected()));
  referenceVFAction->setActionGroup(this->regQAMenuActionGroup);
  referenceVFAction->setData(QVariant("Forward"));
  vectorMenu->addAction(referenceVFAction);
  
  QAction* movingVFAction = new QAction("Backward",q);
  QObject::connect(movingVFAction, SIGNAL(triggered()), q, SLOT(movingVectorFieldSelected()));
  movingVFAction->setActionGroup(this->regQAMenuActionGroup);
  movingVFAction->setData(QVariant("Backward"));
  vectorMenu->addAction(movingVFAction);
  
  QMenu* fiducialMenu = assignMenu->addMenu( "Fiducial" );
  QAction* referenceFiducialsAction = new QAction("Reference phase",q);
  QObject::connect(referenceFiducialsAction, SIGNAL(triggered()), q, SLOT(referenceFiducialsSelected()));
  referenceFiducialsAction->setActionGroup(this->regQAMenuActionGroup);
  referenceFiducialsAction->setData(QVariant("Reference phase"));
  fiducialMenu->addAction(referenceFiducialsAction);
  
  QAction* movingFiducialsAction = new QAction("Moving phase",q);
  QObject::connect(movingFiducialsAction, SIGNAL(triggered()), q, SLOT(movingFiducialsSelected()));
  movingFiducialsAction->setActionGroup(this->regQAMenuActionGroup);
  movingFiducialsAction->setData(QVariant("Moving phase"));
  fiducialMenu->addAction(movingFiducialsAction);
  
  QMenu* contourMenu = assignMenu->addMenu( "Contour" );
  QAction* referenceContourAction = new QAction("Reference phase",q);
  QObject::connect(referenceContourAction, SIGNAL(triggered()), q, SLOT(referenceContourSelected()));
  referenceContourAction->setActionGroup(this->regQAMenuActionGroup);
  referenceContourAction->setData(QVariant("Reference phase"));
  contourMenu->addAction(referenceContourAction);
  
  QAction* movingContourAction = new QAction("Moving phase",q);
  QObject::connect(movingContourAction, SIGNAL(triggered()), q, SLOT(movingContourSelected()));
  movingContourAction->setActionGroup(this->regQAMenuActionGroup);
  movingContourAction->setData(QVariant("Moving phase"));
  contourMenu->addAction(movingContourAction);
  
  QAction* roiAction = new QAction("ROI",q);
  QObject::connect(roiAction, SIGNAL(triggered()), q, SLOT(ROISelected()));
  roiAction->setActionGroup(this->regQAMenuActionGroup);
  roiAction->setData(QVariant("ROI"));
  assignMenu->addAction(roiAction);
  
  QAction* calculateregQAAction = new QAction("Calculate Registration QA",q);
  QObject::connect(calculateregQAAction, SIGNAL(triggered()), q, SLOT(calcuateregQAForCurrentNode()));
  calculateregQAAction->setActionGroup(this->regQAMenuActionGroup);
  calculateregQAAction->setData(QVariant("Calculate regQA"));
  this->regQAMenu->addAction(calculateregQAAction);
  
  QAction* loadAction = new QAction("Load from disk",q);
  QObject::connect(loadAction, SIGNAL(triggered()), q, SLOT(updateRegNodeForCurrentNode()));
  loadAction->setActionGroup(this->regQAMenuActionGroup);
  loadAction->setData(QVariant("Load from disk"));
  this->regQAMenu->addAction(loadAction);
  
  QAction* convertAction = new QAction("Convert",q);
  QObject::connect(convertAction, SIGNAL(triggered()), q, SLOT(convertCurrentNode()));
  convertAction->setActionGroup(this->regQAMenuActionGroup);
  convertAction->setData(QVariant("Convert"));
  this->regQAMenu->addAction(convertAction);

  this->regQALogic = vtkSlicerRegistrationQALogic::New();
  vtkSmartPointer<vtkMRMLScene> scene; 
  scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  this->regQALogic->SetMRMLScene(scene);
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyRegistrationQAPluginPrivate::~qSlicerSubjectHierarchyRegistrationQAPluginPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyRegistrationQAPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyRegistrationQAPlugin::qSlicerSubjectHierarchyRegistrationQAPlugin(QObject* parent)
 : Superclass(parent)
 , m_RegistrationQAParametersID(QString())
 , d_ptr( new qSlicerSubjectHierarchyRegistrationQAPluginPrivate(*this) )
{
  this->m_Name = QString("RegQuality");
  this->m_RegistrationQAParametersID = QString("");

  Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
  d->init();
}
//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyRegistrationQAPlugin::~qSlicerSubjectHierarchyRegistrationQAPlugin()
{
}

//----------------------------------------------------------------------------
// double d->regQALogic->canAddNodeToSubjectHierarchy(
//   vtkSmartPointer<vtkMRMLNode> node;, vtkIdType parentItemID/*=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID*/)const
// {
//   Q_UNUSED(parentItemID);
//   if (!node)
//     {
//     qCritical() << Q_FUNC_INFO << ": Input node is NULL";
//     return 0.0;
//     }
//   const char* registrationQuality = node->GetAttribute("RegQuality");
//   if ( registrationQuality != NULL ) {
//     return 1.0; // Only this plugin can handle this node
//   }
//   return 0.0;
// }

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyRegistrationQAPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID)const
{
  if (!itemID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return 0.0;
  }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return 0.0;
  }

  // RT Dose
  vtkMRMLNode* associatedNode = shNode->GetItemDataNode(itemID);
//   const char* registrationQuality = associatedNode->GetAttribute("RegQuality");
  
  if ( associatedNode && associatedNode->IsA("vtkMRMLScalarVolumeNode")
     )
  {
    return 0.1; // Only this plugin can handle this node
  }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyRegistrationQAPlugin::roleForPlugin()const
{
  return "Registration Quality";
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyRegistrationQAPlugin::tooltip(vtkIdType itemID)const
{
  return qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName("Volumes")->tooltip(itemID);
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyRegistrationQAPlugin::icon(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);

  if (!itemID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QIcon();
  }

  if (this->canOwnSubjectHierarchyItem(itemID))
  {
    return d->regQAIcon;
  }

  // Node unknown by plugin
  return QIcon();
}

QIcon qSlicerSubjectHierarchyRegistrationQAPlugin::visibilityIcon(int visible)
{
  // Have the default plugin (which is not registered) take care of this
  return qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin()->visibilityIcon(visible);
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyRegistrationQAPlugin::itemContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyRegistrationQAPlugin);

  QList<QAction*> actions;
  actions << d->regQAMenu->menuAction();
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::showContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);

  if (!itemID)
  {
    // There are no scene actions in this plugin
    return;
  }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
  }

  // Volume but not RT dose or labelmap
//   std::string level = shNode->GetItemLevel(itemID);
//   if ( level == "Folder" )
//   {
    d->regQAMenu->menuAction()->setVisible(true);
//     d->regQAMenu->selectSegment()->setVisible(false);
//     d->RegisterThisAction->setVisible(true);
//   }
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::loadFromFilenameForItemID(vtkIdType itemID)
{
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   
   if ( ! shNode->HasItemAttribute(itemID, d->regQALogic->FILEPATH.c_str())){
      qCritical() << Q_FUNC_INFO << ": No filepath for: " << shNode->GetItemName(itemID).c_str();
      return;
   }
  
   vtkSmartPointer<vtkMRMLNode> node;
   std::string regTypeIdentifier = shNode->GetItemAttribute(itemID,
                    d->regQALogic->REGISTRATION_TYPE.c_str());
   bool backward = shNode->HasItemAttribute(itemID, d->regQALogic->INVERSE.c_str());
   
   if ( regTypeIdentifier.compare(d->regQALogic->FIDUCIAL) == 0){
      node = vtkMRMLNode::SafeDownCast(this->loadMarkups(itemID));
   }
   else if ( regTypeIdentifier.compare(d->regQALogic->ROI) == 0){
      node = vtkMRMLNode::SafeDownCast(this->loadROI(itemID));
   }
   else if ( regTypeIdentifier.compare(d->regQALogic->SEGMENTATION) == 0){
//TODO: Test this
   qCritical() << Q_FUNC_INFO << ": Not implemented yet";
   return;
//       node = vtkMRMLNode::SafeDownCast( this->loadSegmentation(itemID));
   }
   else{
      node = vtkMRMLNode::SafeDownCast(this->loadVolumeFromItemId(itemID));
   }
   
   if (node){
      this->InputSelected(regTypeIdentifier,backward,itemID,node);
   }
  
}
// //---------------------------------------------------------------------------
// void qSlicerSubjectHierarchyRegistrationQAPlugin::assignNodeToRegistrationQANode(vtkMRMLNode* node, ){
//    
// }
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::calcuateregQAForCurrentNode()
{
  
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  
  if (!currentItemID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
  }
  
  this->calcuateregQAForID(currentItemID, false);
}
void qSlicerSubjectHierarchyRegistrationQAPlugin::updateRegNodeForCurrentNode(){
   vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  
  if (!currentItemID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
  }
  
  this->updateRegNodeForID(currentItemID);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::convertCurrentNode(){
   vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  
  if (!currentItemID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
  }
  
  this->convertItemID(currentItemID);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::calcuateregQAForID(vtkIdType itemID, bool removeNodes) {
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   /* Find out which kind ot image we have */
   std::string regTypeIdentifier = shNode->GetItemAttribute(itemID,d->regQALogic->REGISTRATION_TYPE.c_str());

   if ( regTypeIdentifier.compare(d->regQALogic->WARPED_IMAGE) == 0 ) {
      this->calculateAbsoluteDifference(itemID,removeNodes);
   }
   else if ( regTypeIdentifier.compare(d->regQALogic->VECTOR_FIELD) == 0){
      this->calculateJacobian(itemID,false); /* No need to remove nodes here, we do it in invconsist */
      this->calculateInverseConsistency(itemID,false);
      if (removeNodes){
         this->removeNode(itemID);
      }
   }
   else if ( regTypeIdentifier.compare(d->regQALogic->FIDUCIAL) == 0){
      this->calculateFiducialDistance(itemID,removeNodes);
   }
   else{
  
   /* And check, if there are any children items and run regQA on them */
      vtkSmartPointer<vtkIdList> IdList = vtkSmartPointer<vtkIdList>::New();
      shNode->GetItemChildren(itemID,IdList);
      if ( IdList->GetNumberOfIds() > 0 ){
         for (int i=0; i < IdList->GetNumberOfIds(); i++){
            this->calcuateregQAForID(IdList->GetId(i), true); // Remove nodes is set on, so that multiple vectors can be loaded
         }
      }
   }
   
   
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::updateRegNodeForID(vtkIdType itemID){
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   /* Check all children, load them and write them in registrationQA node */
   std::string regTypeIdentifier = shNode->GetItemAttribute(itemID,
                    d->regQALogic->REGISTRATION_TYPE);
   
   if ( !regTypeIdentifier.empty() ){
      this->loadFromFilenameForItemID(itemID);
   }
   vtkSmartPointer<vtkIdList> IdList = vtkSmartPointer<vtkIdList>::New();
   shNode->GetItemChildren(itemID,IdList);
   if ( IdList->GetNumberOfIds() > 0 ){
      for (int i=0; i < IdList->GetNumberOfIds(); i++){
         this->updateRegNodeForID(IdList->GetId(i)); //Continous loop throgh all grandchildren
      }
   }
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::calculateFiducialDistance(vtkIdType itemID, bool removeNodes){
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   
   /* Check if previously calculated */
   vtkIdType fiducialFolder = shNode->GetItemChildWithName(itemID,
      d->regQALogic->FIDUCIAL.c_str());
   if ( fiducialFolder ){
      return;
   }
   
   /* Skip for reference fiducial */
   if ( ! shNode->HasItemAttribute(itemID, d->regQALogic->INVERSE) ){
      qCritical() << Q_FUNC_INFO << ": Skipping reference fiducial";
   }
   
   vtkMRMLRegistrationQANode* registrationQANode = this->loadRegistrationQANode(itemID);
   
   /* Load Fiducial */
   this->loadFromFilenameForItemID(itemID);
   
   std::string imageStringID  = shNode->GetItemAttribute(
      itemID,d->regQALogic->FIXEDIMAGEID.c_str());
   
   std::string referenceImageStringID = shNode->GetAttributeFromItemAncestor(itemID,
        d->regQALogic->REFIMAGEID.c_str());
   
   
   //Find reference markups
   vtkIdType markupsFieldFolderID = shNode->GetItemParent(itemID);
   vtkIdType refMarkupID;
   bool bFound = false;
   vtkSmartPointer<vtkIdList> IdMarkupList = vtkSmartPointer<vtkIdList>::New();
   shNode->GetItemChildren(markupsFieldFolderID,IdMarkupList);
   for(int i=0;i<IdMarkupList->GetNumberOfIds();i++){
      refMarkupID = IdMarkupList->GetId(i);
      /* Only reference fiducal has inverse off */
      if ( ! shNode->HasItemAttribute(refMarkupID, d->regQALogic->INVERSE) ){
         bFound = true;
         break;
      }
   }
//       std::string referenceImageID1  = shNode->GetItemAttribute(
//          refMarkupID,d->regQALogic->FIXEDIMAGEID.c_str());
//       if ( referenceImageID1.compare(referenceImageStringID) == 0 ){
//          bFound = true;
//          break;
//       }
//    }
   
   if (! bFound ){
      qCritical() << Q_FUNC_INFO << ": Can't find markup with reference ID " << referenceImageStringID.c_str();
      return;
   }
   qCritical() << Q_FUNC_INFO << "Test1";
   this->loadFromFilenameForItemID(refMarkupID);
   qCritical() << Q_FUNC_INFO << "Test2";
   //Find appropriate vector field
   
   std::string vectorFieldFolderStringID = shNode->GetAttributeFromItemAncestor(itemID,
        d->regQALogic->VECTORITEMID.c_str());
   
   if ( vectorFieldFolderStringID.empty() ){
      qCritical() << Q_FUNC_INFO << ": Failed to vector field folder";
      return;
   }
   qCritical() << Q_FUNC_INFO << "Test3";
   vtkIdType vectorFieldFolderID = atoll(vectorFieldFolderStringID.c_str());
   vtkIdType vectorItemID;
   vtkIdType inverseVectorID;
   bFound = false;
   bool bFourD = false;
   vtkSmartPointer<vtkIdList> IdList = vtkSmartPointer<vtkIdList>::New();
   shNode->GetItemChildren(vectorFieldFolderID,IdList);
   
   /* Check if we have a 4DCT case */
   if (IdList->GetNumberOfIds() > 2){
      bFourD = true;
   }
   
   for(int i=0;i<IdList->GetNumberOfIds();i++){
      vectorItemID = IdList->GetId(i);
      /* Skip forward vector, will be found later */
      if ( !shNode->HasItemAttribute(vectorItemID, d->regQALogic->INVERSE) && !bFourD ){
         inverseVectorID = vectorItemID;
         continue;
      }
      if ( bFourD ) {
         std::string referenceImageID2  = shNode->GetItemAttribute(
            vectorItemID,d->regQALogic->FIXEDIMAGEID.c_str());
         std::string movingImageID  = shNode->GetItemAttribute(
            vectorItemID,d->regQALogic->MOVINGIMAGEID.c_str());
         if (referenceImageID2.compare(referenceImageStringID) == 0 && movingImageID.compare(imageStringID) == 0){
            bFound = true;
            break;
         }
      }
      else {
         bFound = true;
         break;
      }
   }
   qCritical() << Q_FUNC_INFO << "Test4";
   if (! bFound ){
      qCritical() << Q_FUNC_INFO << ": Can't find vector field ";
      return;
   }
   
   qCritical() << Q_FUNC_INFO << "Loading vector field";
   vtkSmartPointer<vtkMRMLVectorVolumeNode> vectorNode;
   this->loadFromFilenameForItemID(vectorItemID);
   //And inverse vector
   if ( bFourD ) {
      inverseVectorID = this->findInverseVectorID(vectorItemID);
   }
   this->loadFromFilenameForItemID(inverseVectorID);
//    vectorNode = vtkMRMLVectorVolumeNode::SafeDownCast(this->loadVolumeFromItemId(vectorItemID));

//    double* statisticValues;
   qCritical() << Q_FUNC_INFO << "Starting calculations";
   // This calculates fiducial distance and writes in table
   d->regQALogic->SetAndObserveRegistrationQANode(registrationQANode);
   d->regQALogic->CalculateRegQAFrom(4);
   
   //Repeat for other direction
   d->regQALogic->SwitchRegistrationDirection();
   d->regQALogic->CalculateRegQAFrom(4);
   
   
   // Also for other direction
   
//    if ( ! d->regQALogic->CalculateFiducialsDistance(fiducialReferenceNode, fiducialNode, 
//       vectorNode, statisticValues) ){
//       qCritical() << Q_FUNC_INFO << ": Can't calculate Fiducial distance.";
//       return;
//    }
   
//    this->writeInTable(itemID,
//          d->regQALogic->FIDUCIALTABLE.c_str(), statisticValues, fiducialReferenceNode);
   
   fiducialFolder = shNode->CreateFolderItem(itemID,
            d->regQALogic->FIDUCIAL.c_str());      
//    vtkIdType jacobianID = shNode->CreateItem(jacobianFolder,jacobianNode);
   
   if ( removeNodes){
//       shNode->RemoveItem(jacobianID);
//       this->removeNode(vtkMRMLNode::SafeDownCast(vectorNode));
//       this->removeNode(vtkMRMLNode::SafeDownCast(fiducialReferenceNode));
//       this->removeNode(vtkMRMLNode::SafeDownCast(fiducialReferenceNode));
   }
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::calculateJacobian(vtkIdType itemID, bool removeNodes){
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   
   /* Check if previously calculated */
   vtkIdType jacobianFolder = shNode->GetItemChildWithName(itemID,
      d->regQALogic->JACOBIAN.c_str());
   if ( jacobianFolder ){
      return;
   }
   
   

   /* Load vector for current ItemID */
   this->loadFromFilenameForItemID(itemID);
   
//    vtkSmartPointer<vtkMRMLAnnotationROINode> ROINode;
   vtkSmartPointer<vtkMRMLScene> scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
   vtkSmartPointer<vtkMRMLScalarVolumeNode> jacobianNode;
//    ROINode = this->loadROI(itemID);
   
   vtkMRMLRegistrationQANode* registrationQANode = this->loadRegistrationQANode(itemID);
   // If we have a backward registration
   if ( shNode->HasItemAttribute(itemID, d->regQALogic->INVERSE)){
      registrationQANode = registrationQANode->GetBackwardRegistrationQAParameters();
   }
   
   /* Set old jacobian to zero in regqanode */
   registrationQANode->SetAndObserveJacobianVolumeNodeID(NULL);
   registrationQANode->Modified();
   
   double statisticValues[4];
   // Calculate Jacobian with nodes in registrationQANode and write in its table
   d->regQALogic->SetAndObserveRegistrationQANode(registrationQANode);
//    d->regQALogic->CalculateRegQAFrom(2);
   if ( !d->regQALogic->Jacobian(registrationQANode, statisticValues) ){
      qCritical() << Q_FUNC_INFO << ": Failed to calculate jacobian for" << shNode->GetItemName(itemID).c_str();
      return;
   }
   
//    jacobianNode = d->regQALogic->Jacobian(vectorNode, ROINode);
   jacobianNode = vtkMRMLScalarVolumeNode::SafeDownCast(scene->GetNodeByID(
      registrationQANode->GetJacobianVolumeNodeID()));
      
   if (jacobianNode == NULL ){
      qCritical() << Q_FUNC_INFO << ": Can't calculate Jacobian";
      return;
   }
   
   jacobianFolder = shNode->CreateFolderItem(itemID,
                          d->regQALogic->JACOBIAN.c_str());
   
   
//    d->regQALogic->CalculateStatistics(jacobianNode, statisticValues);
   this->writeInTable(itemID,
         d->regQALogic->JACOBIANTABLE.c_str(), statisticValues);
   vtkIdType jacobianID = shNode->CreateItem(jacobianFolder,jacobianNode);

   

   shNode->SetItemAttribute(jacobianID, d->regQALogic->REGISTRATION_TYPE.c_str(), 
                              d->regQALogic->JACOBIAN.c_str());
   std::stringstream jacobianStringID;
   jacobianStringID << jacobianID;
   shNode->SetItemAttribute(jacobianFolder, d->regQALogic->JACOBIANITEMID.c_str(), 
      jacobianStringID.str().c_str());
   
   if ( removeNodes){
//       shNode->RemoveItem(jacobianID);
      this->removeNode(itemID);
   }

}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::calculateInverseConsistency(vtkIdType itemID, bool removeNodes){
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   
   vtkSmartPointer<vtkMRMLScalarVolumeNode> backwardConsistNode;
   vtkSmartPointer<vtkMRMLScene> scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
   
   /* Check if previously calculated */
   vtkIdType invConsistFolder = shNode->GetItemChildWithName(itemID,
      d->regQALogic->INVERSECONSISTENCY.c_str());
   if ( invConsistFolder ){
      if ( removeNodes ){
         this->removeNode(itemID);
      }
      return;
   }
   
   vtkIdType backwardVectorID = this->findInverseVectorID(itemID);
   
   if (backwardVectorID == 0){
      qCritical() << Q_FUNC_INFO << ": Can't find backward vector.";
      return;
   }
   
   /* Calc inv consist */
   this->loadFromFilenameForItemID(itemID);
   this->loadFromFilenameForItemID(backwardVectorID);
   
   vtkMRMLRegistrationQANode* registrationQANode = this->loadRegistrationQANode(itemID);
   registrationQANode->SetAndObserveInverseConsistVolumeNodeID(NULL);
   registrationQANode->Modified();

   double statisticValues[4];

   if ( !d->regQALogic->InverseConsist(registrationQANode, statisticValues) ){
      qCritical() << Q_FUNC_INFO << ": Failed to calcualte inverse consistency for" << shNode->GetItemName(itemID).c_str();
      return;
   }
   backwardConsistNode = vtkMRMLScalarVolumeNode::SafeDownCast(scene->GetNodeByID(
      registrationQANode->GetInverseConsistVolumeNodeID()));
      
   if (backwardConsistNode == NULL ){
      qCritical() << Q_FUNC_INFO << ": Can't calculate inverse consistency";
      return;
   }
 
   this->writeInTable(itemID,
      d->regQALogic->INVCONSISTTABLE.c_str(), statisticValues);

   /* Write all the necessary data in subject hierarchy*/
   if (backwardConsistNode == NULL ){
      qCritical() << Q_FUNC_INFO << ": Can't calculate Inverse Consistency";
      return;
   }
   
   invConsistFolder = shNode->CreateFolderItem(itemID,
                   d->regQALogic->INVERSECONSISTENCY.c_str());

   vtkIdType backwardConsistID = shNode->CreateItem(invConsistFolder,backwardConsistNode);

   shNode->SetItemAttribute(backwardConsistID, d->regQALogic->REGISTRATION_TYPE.c_str(), 
                              d->regQALogic->INVERSECONSISTENCY.c_str());
   std::stringstream backwardConsistStringID;
   backwardConsistStringID <<  backwardConsistID;
   shNode->SetItemAttribute(invConsistFolder, d->regQALogic->INVERSECONSISTENCYITEMID.c_str(), 
      backwardConsistStringID.str().c_str());
   
   /* Check if backward vector has inverse consistency */
   vtkIdType backInvConsistFolder = shNode->GetItemChildWithName(backwardVectorID,
      d->regQALogic->INVERSECONSISTENCY.c_str());
   
   if ( ! backInvConsistFolder ){
      backInvConsistFolder = shNode->CreateFolderItem(backwardVectorID,
                   d->regQALogic->INVERSECONSISTENCY.c_str());
      shNode->SetItemAttribute(backInvConsistFolder, d->regQALogic->INVERSECONSISTENCYITEMID.c_str(), 
            backwardConsistStringID.str().c_str());
   }
   
//       this->removeNode(itemID);
   this->removeNode(backwardVectorID);
}
//---------------------------------------------------------------------------
vtkIdType qSlicerSubjectHierarchyRegistrationQAPlugin::findInverseVectorID(vtkIdType itemID){
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return 0;
   }
   
   /* find reference and moving image ID */
   std::string referenceImage1ID  = shNode->GetItemAttribute(
      itemID,d->regQALogic->FIXEDIMAGEID.c_str());
   std::string movingImage1ID  = shNode->GetItemAttribute(
      itemID,d->regQALogic->MOVINGIMAGEID.c_str());
   

   /* find items with reverse reference and moving image ID */
   vtkIdType parentItemID = shNode->GetItemParent(itemID);
   vtkSmartPointer<vtkIdList> IdList = vtkSmartPointer<vtkIdList>::New();
   shNode->GetItemChildren(parentItemID,IdList);
   
   vtkIdType backwardVectorItemID;
   
   /* check if we have a 4DCT registration */
   bool bFourD = false;
   
   if ( IdList->GetNumberOfIds() > 2 ){
      bFourD = true;
   }
   
   if ( bFourD ){
      for(int i=0;i<IdList->GetNumberOfIds();i++){
         backwardVectorItemID = IdList->GetId(i);
         std::string referenceImage2ID  = shNode->GetItemAttribute(
            backwardVectorItemID,d->regQALogic->FIXEDIMAGEID.c_str());
         std::string movingImage2ID  = shNode->GetItemAttribute(
            backwardVectorItemID,d->regQALogic->MOVINGIMAGEID.c_str());
         if (referenceImage1ID.compare(movingImage2ID) == 0 && movingImage1ID.compare(referenceImage2ID) == 0){
            return backwardVectorItemID;
         }
      }
   }
   else{
      /* Get the other vector, check if it has other direction */
      
      if ( IdList->GetId(0) == itemID ) {
         backwardVectorItemID = IdList->GetId(1);
      }
      else{
         backwardVectorItemID = IdList->GetId(0);
      }
      bool bInverse1 = shNode->HasItemAttribute( itemID, d->regQALogic->INVERSE);
      bool bInverse2 = shNode->HasItemAttribute( backwardVectorItemID, d->regQALogic->INVERSE);
      if ( bInverse1 == !bInverse2 ) {
         return backwardVectorItemID;
      }
      else{
         qCritical() << Q_FUNC_INFO << ": Need one inverse and one normal vector field";
         return 0;
      }
   }
   return 0;
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::calculateAbsoluteDifference(vtkIdType itemID, bool removeNodes){
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   
   /* Check if previously calculated */
   vtkIdType absDiffFolder = shNode->GetItemChildWithName(itemID,
      d->regQALogic->ABSOLUTEDIFFERENCE.c_str());
   if ( absDiffFolder ){
      return;
   }
   
   
   std::string referenceImageCharID  = shNode->GetItemAttribute(itemID,
                        d->regQALogic->FIXEDIMAGEID.c_str());
 
   if ( referenceImageCharID.c_str() == NULL){
      qCritical() << Q_FUNC_INFO << ": Failed to find image IDs";
      return;
   }
   
   vtkIdType referenceImageID = atoll(referenceImageCharID.c_str());
   vtkSmartPointer<vtkMRMLScalarVolumeNode> AbsDiffNode;
   vtkSmartPointer<vtkMRMLScene> scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
   double statisticValues[4];
   
   this->loadFromFilenameForItemID(itemID);
   this->loadFromFilenameForItemID(referenceImageID);
   
   vtkMRMLRegistrationQANode* registrationQANode = this->loadRegistrationQANode(itemID);
   
   if ( !d->regQALogic->AbsoluteDifference(registrationQANode, statisticValues) ){
      qCritical() << Q_FUNC_INFO << ": Failed to calcualte absolute difference for" << shNode->GetItemName(itemID).c_str();
      return;
   }
   
   AbsDiffNode = vtkMRMLScalarVolumeNode::SafeDownCast(scene->GetNodeByID(
      registrationQANode->GetAbsoluteDiffVolumeNodeID()));
   if (AbsDiffNode == NULL ){
      qCritical() << Q_FUNC_INFO << ": Can't calculate Absolute Difference";
      return;
   }

   absDiffFolder = shNode->CreateFolderItem(itemID,
         d->regQALogic->ABSOLUTEDIFFERENCE.c_str());
   
   this->writeInTable(itemID,
         d->regQALogic->ABSDIFFTABLE.c_str(), statisticValues);           
   vtkIdType absDiffID = shNode->CreateItem(absDiffFolder,AbsDiffNode);
   
   shNode->SetItemAttribute(absDiffID, d->regQALogic->REGISTRATION_TYPE.c_str(), 
                  d->regQALogic->ABSOLUTEDIFFERENCE.c_str());
   std::stringstream absDiffStringID;
   absDiffStringID <<  absDiffID;
   shNode->SetItemAttribute(absDiffFolder, d->regQALogic->ABSDIFFNODEITEMID.c_str(), 
      absDiffStringID.str().c_str());

   if (removeNodes){
      this->removeNode(itemID);
      this->removeNode(referenceImageID);
   }
 
      
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::removeNode(vtkMRMLNode* node){
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   
   vtkIdType id = shNode->GetItemByDataNode(node);
   vtkIdType parentID = shNode->GetItemParent(id);
   shNode->RemoveItemAttribute(parentID,
          d->regQALogic->NODEITEMID.c_str());
   shNode->RemoveItem(id);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::removeNode(vtkIdType itemID){
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }

   vtkSmartPointer<vtkIdList> IdList = vtkSmartPointer<vtkIdList>::New();
   vtkMRMLNode* node;
   shNode->GetItemChildren(itemID,IdList);
   
   if ( IdList->GetNumberOfIds() > 0 ){
      for (int i=0; i < IdList->GetNumberOfIds(); i++){
//          this->removeNode(IdList->GetId(i));
         node = shNode->GetItemDataNode(IdList->GetId(i));
         if ( node ){
            this->removeNode(node);
         }
      }
   }
   
   
}

//---------------------------------------------------------------------------
// void qSlicerSubjectHierarchyRegistrationQAPlugin::calculateStatistics(vtkMRMLScalarVolumeNode* scalarNode, double statisticValues[4]){
//    Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
//    
//    vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
//    if (!shNode)
//    {
//       qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
//       return NULL;
//    }
//    
//    /* Calculate statistics */
//    d->regQALogic->CalculateStatistics(scalarNode, statisticValues);
//    
//    /* Write statistics */
// //    std::string values[] = {"mean","STD","max","min"};
// //    for (int i = 0;i<4;i++){
// //       std::stringstream statisticValue;
// //       statisticValue << statisticValues[i];
// //       shNode->SetItemAttribute(itemID, values[i].c_str(), statisticValue.str().c_str());
// //    }
// }
//----------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::writeInTable(vtkIdType itemID,char const* tableName, double* statisticValues, vtkMRMLMarkupsFiducialNode* fiducalsNode/*=NULL*/){
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   
   bool fiducialsTable = false;
   if ( fiducalsNode ) {
      fiducialsTable = true;
   }
   
   /* Find table first */
   vtkSmartPointer<vtkMRMLTableNode> tableNode;
   vtkIdType parentItemID = shNode->GetItemParent(itemID);
   std::string tableStringID = shNode->GetItemAttribute(parentItemID,
        tableName);
   if ( ! tableStringID.empty() ){
      vtkIdType tableItemID = atoll(tableStringID.c_str());
      vtkSmartPointer<vtkMRMLNode> node; 
      node = shNode->GetItemDataNode(tableItemID);
      if ( node ){
         tableNode =  vtkMRMLTableNode::SafeDownCast(node);
      }
      else{
         tableNode = this->createTable(parentItemID, tableName, fiducialsTable);
      }
   }
   else{
      tableNode = this->createTable(parentItemID, tableName, fiducialsTable);
   }
   
   if (tableNode == NULL ){
      qCritical() << Q_FUNC_INFO << ": Failed to access table node";
      return;
   }

   vtkSmartPointer<vtkTable> table;
   table = tableNode->GetTable();
   int tableRow = table->GetNumberOfRows();
   double distanceBefore;
   double distanceAfter;

   if ( fiducalsNode ){
      int numberOfMarkups = fiducalsNode->GetNumberOfMarkups();
      if ( numberOfMarkups < 1 ){
         qCritical() << Q_FUNC_INFO << ": No markups in node";
         return;
      }
      for (int i=0; i < numberOfMarkups; i++){
         table->InsertNextBlankRow();
         distanceBefore = statisticValues[2*i];
         distanceAfter = statisticValues[2*i+1];
         table->SetValue(tableRow, 0, vtkVariant(fiducalsNode->GetNthMarkupLabel(i).c_str()));
         table->SetValue(tableRow, 1, vtkVariant(distanceBefore));
         table->SetValue(tableRow, 2, vtkVariant(distanceAfter));
         table->SetValue(tableRow, 3, vtkVariant(distanceBefore-distanceAfter));
         tableRow++;
      }
   }
   else{
      std::string itemName = shNode->GetItemName(itemID);
      table->InsertNextBlankRow();
      table->SetValue(tableRow, 0, vtkVariant(itemName.c_str()));
      for (int column = 0; column < 4; column++){
         table->SetValue(tableRow, column+1, vtkVariant(statisticValues[column]));
      }
   }
}
//---------------------------------------------------------------------------
vtkMRMLTableNode* qSlicerSubjectHierarchyRegistrationQAPlugin::createTable(vtkIdType itemID,char const* tableName,bool fiducialsTable/*=false*/){
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return NULL;
   }
   vtkSmartPointer<vtkMRMLScene> scene; 
   scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
   
   vtkSmartPointer<vtkMRMLTableNode> tableNode = vtkSmartPointer<vtkMRMLTableNode>::New();

   std::string tableNodeName = scene->GenerateUniqueName(tableName);
   tableNode->SetName(tableNodeName.c_str());
   scene->AddNode(tableNode);
   vtkIdType tableItemID = shNode->CreateItem(itemID, tableNode);
   std::stringstream tableStringID;
   tableStringID <<  tableItemID;
   
   shNode->SetItemAttribute(itemID, tableName, tableStringID.str().c_str());
   
   /* Set names */
   if ( fiducialsTable ){
      vtkAbstractArray* fiducialName = tableNode->AddColumn();
      fiducialName->SetName("Fiducial name");
      vtkAbstractArray* distanceBefore = tableNode->AddColumn();
      distanceBefore->SetName("Distance before Reg");
      vtkAbstractArray* distanceAfter = tableNode->AddColumn();
      distanceAfter->SetName("Distance after registration");
      vtkAbstractArray* diff = tableNode->AddColumn();
      diff->SetName("Difference");
   }
   else{
      vtkAbstractArray* nodeName = tableNode->AddColumn();
      nodeName->SetName("Node name");
      
      vtkAbstractArray* meanName = tableNode->AddColumn();
      meanName->SetName("Max");
      
      vtkAbstractArray* stdName = tableNode->AddColumn();
      stdName->SetName("Min");
      
      vtkAbstractArray* maxName = tableNode->AddColumn();
      maxName->SetName("Mean");
      
      vtkAbstractArray* minName = tableNode->AddColumn();
      minName->SetName("STD");
   }

   return tableNode;
}
//---------------------------------------------------------------------------
// vtkMRMLAnnotationROINode* qSlicerSubjectHierarchyRegistrationQAPlugin::calculateVolumeHistogram(vtkMRMLScalarVolumeNode* scalarNode){
// 
//    vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
//    if (!shNode)
//    {
//       qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
//       return;
//    }
//    // Create and set up logic
//    vtkMRMLScene* mrmlScene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
//    
//    vtkIdType segmentationNodeID = shNode->GetItemParent(this->m_SelectSegment);
//    vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::
//       SafeDownCast(shNode->GetItemDataNode(segmentationNodeID));
//       
//    if (!segmentationNode){
//       qCritical() << Q_FUNC_INFO << ": Failed to access segmentation node";
//       return;
//    }
//    vtkSmartPointer<vtkSlicerDoseVolumeHistogramModuleLogic> dvhLogic = 
//       vtkSmartPointer<vtkSlicerDoseVolumeHistogramModuleLogic>::New();
//    dvhLogic->SetMRMLScene(mrmlScene);
// 
//    // Create and set up parameter set MRML node
//    bool automaticOversamplingCalculation = false;
//    vtkSmartPointer<vtkMRMLDoseVolumeHistogramNode> paramNode = vtkSmartPointer<vtkMRMLDoseVolumeHistogramNode>::New();
//    paramNode->SetAndObserveDoseVolumeNode(scalarNode);
//    paramNode->SetAndObserveSegmentationNode(segmentationNode);
//    paramNode->SetAutomaticOversampling(automaticOversamplingCalculation);
//    mrmlScene->AddNode(paramNode);
//    
//    // Setup chart node
//    vtkMRMLChartNode* chartNode = paramNode->GetChartNode();
//    if (!chartNode)
//    {
//       mrmlScene->Commit();
//       std::cerr << "ERROR: Chart node must exist for DVH parameter set node!" << std::endl;
//       return EXIT_FAILURE;
//    }
//    chartNode->SetProperty("default", "title", "Dose Volume Histogram");
//    chartNode->SetProperty("default", "xAxisLabel", "Dose [Gy]");
//    chartNode->SetProperty("default", "yAxisLabel", "Fractional volume [%]");
//    chartNode->SetProperty("default", "type", "Line");
// 
// }
//---------------------------------------------------------------------------
vtkMRMLAnnotationROINode* qSlicerSubjectHierarchyRegistrationQAPlugin::loadROI(vtkIdType itemID){
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return NULL;
   }
   
   std::string roiCharID = shNode->GetAttributeFromItemAncestor(itemID,d->regQALogic->ROIITEMID.c_str());
   
   if (roiCharID.c_str() == NULL){
      qCritical() << Q_FUNC_INFO << ": Can't find ROI item";
      return NULL;
   }
   
   vtkIdType roiItemID = atoll(roiCharID.c_str());
   
   /* check if its already loaded */
   std::string nodeStringItemID = shNode->GetItemAttribute(roiItemID,
         d->regQALogic->NODEITEMID.c_str());
   
   if ( ! nodeStringItemID.empty() ){
      vtkIdType nodeItemID = atoll(nodeStringItemID.c_str());
      vtkSmartPointer<vtkMRMLNode> node; 
      node = shNode->GetItemDataNode(nodeItemID);
      if ( node ){
         return vtkMRMLAnnotationROINode::SafeDownCast(node);
      }
      else{
         shNode->RemoveItemAttribute(roiItemID,
         d->regQALogic->NODEITEMID.c_str());
      }
   }
   
   vtkNew<vtkSlicerAnnotationModuleLogic> logic;
   vtkSmartPointer<vtkMRMLScene> scene; 
   scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
   logic->SetMRMLScene(scene);
   
   std::string filePath = shNode->GetItemAttribute(roiItemID, d->regQALogic->FILEPATH.c_str());
   
   if (filePath.c_str() == NULL){
      qCritical() << Q_FUNC_INFO << ": Can't find ROI filepath";
      return NULL;
   }
   
   char *RoiNodeID = logic->LoadAnnotation(filePath.c_str(), d->regQALogic->ROI.c_str(), 3);
   vtkSmartPointer<vtkMRMLAnnotationROINode> RoiNode;
   RoiNode = vtkMRMLAnnotationROINode::SafeDownCast( scene->GetNodeByID(RoiNodeID) );
   if (RoiNodeID == NULL) {
      qCritical() << Q_FUNC_INFO << ": Can't load Roi from" << filePath.c_str();
      return NULL;
   }
   
   vtkSmartPointer<vtkMRMLNode> RoiNormalNode;
   RoiNormalNode = vtkMRMLNode::SafeDownCast(scene->GetNodeByID(RoiNodeID));
   vtkIdType currentRoiItemID = shNode->GetItemByDataNode(RoiNormalNode);
   std::stringstream currentRoiIDString;
   currentRoiIDString <<  currentRoiItemID;
   shNode->SetItemAttribute(roiItemID,d->regQALogic->NODEITEMID.c_str(),
      currentRoiIDString.str().c_str());

//    shNode->SetItemParent(currentRoiItemID,roiItemID);
   return RoiNode;

}
//---------------------------------------------------------------------------
vtkMRMLVolumeNode* qSlicerSubjectHierarchyRegistrationQAPlugin::loadVolumeFromItemId(vtkIdType itemID){
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return NULL;
   }
  
   if (!itemID)
   {
      qCritical() << Q_FUNC_INFO << ": Invalid current item";
      return NULL;
   }
  
   /* check if its already loaded */
   std::string nodeStringItemID = shNode->GetItemAttribute(itemID,
         d->regQALogic->NODEITEMID.c_str());
   
   if ( ! nodeStringItemID.empty() ){
      vtkIdType nodeItemID = atoll(nodeStringItemID.c_str());
      vtkSmartPointer<vtkMRMLNode> node;
      node = shNode->GetItemDataNode(nodeItemID);
      if ( node ){
         return vtkMRMLVolumeNode::SafeDownCast(node);
      }
      else{
         shNode->RemoveItemAttribute(itemID,
             d->regQALogic->NODEITEMID.c_str());
      }
   }
  
   vtkNew<vtkSlicerVolumesLogic> logic;
   vtkSmartPointer<vtkMRMLScene> scene; 
   scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  
   logic->SetMRMLScene(scene);
  
   std::string filePath = shNode->GetItemAttribute(itemID, 
      d->regQALogic->FILEPATH.c_str());
   
   
   std::string volumeName = vtksys::SystemTools::GetFilenameName(filePath.c_str());
   std::size_t lastindex = volumeName.find_last_of("."); 
   std::string rawname = volumeName.substr(0, lastindex);
   volumeName = scene->GetUniqueNameByString(rawname.c_str());

   vtkSmartPointer<vtkMRMLVolumeNode> volume;
   volume = logic->AddArchetypeVolume(filePath.c_str(), volumeName.c_str());
  
   if (!volume){
      qCritical() << Q_FUNC_INFO << "Can't load volume: " << filePath.c_str();
      return NULL;
   }
   
   vtkSmartPointer< vtkMRMLVolumeArchetypeStorageNode > volumeStorageNode = 
         vtkSmartPointer< vtkMRMLVolumeArchetypeStorageNode >::New();
   scene->AddNode(volumeStorageNode.GetPointer());
   volume->SetAndObserveStorageNodeID(volumeStorageNode->GetID());
   
//    volume->CreateDefaultDisplayNodes();
   if ( volume->IsA("vtkMRMLVectorVolumeNode") ){
      vtkSmartPointer< vtkMRMLVectorVolumeDisplayNode > vectorVolumeDisplayNode = 
         vtkSmartPointer< vtkMRMLVectorVolumeDisplayNode >::New();
      scene->AddNode(vectorVolumeDisplayNode.GetPointer());
      volume->SetAndObserveNthDisplayNodeID(0,vectorVolumeDisplayNode->GetID());
      volume->CreateDefaultDisplayNodes();
      
      std::string tripVf = shNode->GetItemAttribute(itemID, 
      d->regQALogic->TRIPVF.c_str());
      if (! tripVf.empty() ){
         /* Convert to physical vf */
         double* spacing;
         spacing = volume->GetSpacing();

         vtkNew<vtkImageData> imageData_new;    
         vtkSmartPointer<vtkImageData> imageData_old = volume->GetImageData();
         imageData_new->DeepCopy(imageData_old);
         
         vtkNew<vtkImageAppendComponents> append;

         vtkNew<vtkImageExtractComponents> iec;
         iec->SetInputData(imageData_new.GetPointer());

         for (int i=0;i<3;i++){
            vtkNew<vtkImageMathematics> math;
            math->SetOperationToMultiplyByK();
            iec->SetComponents(i);
            iec->Update();
            math->SetInput1Data( iec->GetOutput() );
            math->SetConstantK(spacing[i]); 
            math->Update();
            append->AddInputData( math->GetOutput() );
            append->Update();
         }
         volume->SetAndObserveImageData(append->GetOutput());
      }
   }
   else if ( volume->IsA("vtkMRMLScalarVolumeNode") ){
      vtkSmartPointer< vtkMRMLScalarVolumeDisplayNode > scalarVolumeDisplayNode = 
         vtkSmartPointer< vtkMRMLScalarVolumeDisplayNode >::New();
      vtkMRMLColorTableNode *greyColorNode = vtkMRMLColorTableNode::SafeDownCast(
         scene->GetNodeByID("vtkMRMLColorTableNodeGrey"));
      if ( greyColorNode){
         scalarVolumeDisplayNode->SetAndObserveColorNodeID(greyColorNode->GetID());
      }
      scene->AddNode(scalarVolumeDisplayNode.GetPointer());
      volume->SetAndObserveNthDisplayNodeID(0,scalarVolumeDisplayNode->GetID());
   }

   vtkIdType newVolumeID = shNode->CreateItem(itemID,volume);
   std::stringstream newVolumeStringID;
   newVolumeStringID <<  newVolumeID;
   shNode->SetItemAttribute(newVolumeID,"FilePath",filePath);
   shNode->SetItemAttribute(itemID,
      d->regQALogic->NODEITEMID.c_str(),newVolumeStringID.str().c_str());
   
   //Careful!!
//    volume->SetOrigin(0,0,0);
   
   return volume;
}
//---------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* qSlicerSubjectHierarchyRegistrationQAPlugin::loadMarkups(vtkIdType itemID){
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return NULL;
   }
  
   if (!itemID)
   {
      qCritical() << Q_FUNC_INFO << ": Invalid current item";
      return NULL;
   }
  
   /* check if its already loaded */
   std::string nodeStringItemID = shNode->GetItemAttribute(itemID,
         d->regQALogic->NODEITEMID.c_str());
   
   if ( ! nodeStringItemID.empty() ){
      vtkIdType nodeItemID = atoll(nodeStringItemID.c_str());
      vtkSmartPointer<vtkMRMLNode> node;
      node = shNode->GetItemDataNode(nodeItemID);
      if ( node ){
         return vtkMRMLMarkupsFiducialNode::SafeDownCast(node);
      }
      else{
         shNode->RemoveItemAttribute(itemID,
             d->regQALogic->NODEITEMID.c_str());
      }
   }
  
   vtkNew<vtkSlicerMarkupsLogic> logic;
   vtkSmartPointer<vtkMRMLScene> scene; 
   scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  
   logic->SetMRMLScene(scene);
  
   std::string filePath = shNode->GetItemAttribute(itemID, 
      d->regQALogic->FILEPATH.c_str());
   
   
   if (filePath.c_str() == NULL){
      qCritical() << Q_FUNC_INFO << ": Can't find ROI filepath";
      return NULL;
   }
   
   std::string fiducialName = vtksys::SystemTools::GetFilenameName(filePath.c_str());
   std::size_t lastindex = fiducialName.find_last_of("."); 
   std::string rawname = fiducialName.substr(0, lastindex);
   fiducialName = scene->GetUniqueNameByString(rawname.c_str());
   
   char *ReferenceFiducialNodeID = logic->LoadMarkupsFiducials(filePath.c_str(), fiducialName.c_str());
   
   if (ReferenceFiducialNodeID == NULL) {
      qCritical() << Q_FUNC_INFO << ": Can't load fiducials from" << filePath.c_str();
      return NULL;
   }
   
   vtkSmartPointer<vtkMRMLMarkupsFiducialNode> fiducialNode;
   fiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( scene->GetNodeByID(ReferenceFiducialNodeID) );
   vtkIdType newFiducialID = shNode->CreateItem(itemID,fiducialNode);
   std::stringstream newFiducialStringID;
   newFiducialStringID <<  newFiducialID;
   shNode->SetItemAttribute(newFiducialID,"FilePath",filePath);
   shNode->SetItemAttribute(itemID,
      d->regQALogic->NODEITEMID.c_str(),newFiducialStringID.str().c_str());

//    shNode->SetItemParent(currentRoiItemID,roiItemID);
   return fiducialNode;
}
//---------------------------------------------------------------------------
vtkMRMLSegmentationNode* qSlicerSubjectHierarchyRegistrationQAPlugin::loadSegmentation(vtkIdType itemID){
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return NULL;
   }
  
   if (!itemID)
   {
      qCritical() << Q_FUNC_INFO << ": Invalid current item";
      return NULL;
   }
  
   /* check if its already loaded */
   std::string nodeStringItemID = shNode->GetItemAttribute(itemID,
         d->regQALogic->NODEITEMID.c_str());
   
   if ( ! nodeStringItemID.empty() ){
      vtkIdType nodeItemID = atoll(nodeStringItemID.c_str());
      vtkSmartPointer<vtkMRMLNode> node;
      node = shNode->GetItemDataNode(nodeItemID);
      if ( node ){
         return vtkMRMLSegmentationNode::SafeDownCast(node);
      }
      else{
         shNode->RemoveItemAttribute(itemID,
             d->regQALogic->NODEITEMID.c_str());
      }
   }
   
   vtkSmartPointer<vtkMRMLSegmentationNode> segmentationNode;
  
   vtkNew<vtkSlicerSegmentationsModuleLogic> logic;
   vtkSmartPointer<vtkMRMLScene> scene; 
   scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  
   logic->SetMRMLScene(scene);
  
   std::string filePath = shNode->GetItemAttribute(itemID, 
      d->regQALogic->FILEPATH.c_str());
   
   
   if (filePath.c_str() == NULL){
      qCritical() << Q_FUNC_INFO << ": Can't find ROI filepath";
      return NULL;
   }
   
   std::string segmentationName = vtksys::SystemTools::GetFilenameName(filePath.c_str());
   std::size_t lastindex = segmentationName.find_last_of("."); 
   std::string rawname = segmentationName.substr(0, lastindex);
   segmentationName = scene->GetUniqueNameByString(rawname.c_str());
   
   segmentationNode = logic->LoadSegmentationFromFile(filePath.c_str());
   
   if (segmentationNode == NULL) {
      qCritical() << Q_FUNC_INFO << ": Can't load segmentation from" << filePath.c_str();
      return NULL;
   }
   
   vtkIdType newSegmentationID = shNode->CreateItem(itemID,segmentationNode);
   std::stringstream newSegmentationStringID;
   newSegmentationStringID <<  newSegmentationID;
   shNode->SetItemAttribute(newSegmentationID,"FilePath",filePath);
   shNode->SetItemAttribute(itemID,
      d->regQALogic->NODEITEMID.c_str(),newSegmentationStringID.str().c_str());

//    shNode->SetItemParent(currentRoiItemID,roiItemID);
   return segmentationNode;
}
//---------------------------------------------------------------------------
vtkMRMLRegistrationQANode* qSlicerSubjectHierarchyRegistrationQAPlugin::loadRegistrationQANode(vtkIdType itemID){
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return NULL;
   }
  
   if (!itemID)
   {
      qCritical() << Q_FUNC_INFO << ": Invalid item";
      return NULL;
   }
   
   vtkSmartPointer<vtkMRMLRegistrationQANode> registrationQANode;
   vtkSmartPointer<vtkMRMLScene> scene; 
   scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();

   std::string registrationQANodeIDString = shNode->GetAttributeFromItemAncestor(itemID, 
         d->regQALogic->RegistrationQANODEID.c_str() );
   if ( registrationQANodeIDString.empty() ){
      QString registrationQAID = this->m_RegistrationQAParametersID;
      
      //Check if it still exist
      if ( ! registrationQAID.isEmpty() ){
         registrationQANodeIDString = registrationQAID.toUtf8().constData();
         registrationQANode = vtkMRMLRegistrationQANode::SafeDownCast(
               scene->GetNodeByID( registrationQANodeIDString ));
         if ( ! registrationQANode ) {
            this->m_RegistrationQAParametersID = "";
            registrationQAID = "";
         }
      }
         
      if ( registrationQAID.isEmpty() ){
         // Let's check first, if parameters exist in scene
         vtkSmartPointer<vtkCollection> collection = scene->GetNodesByClass("vtkMRMLRegistrationQANode");
         if (collection->GetNumberOfItems() > 0){
//             qCritical() << Q_FUNC_INFO << ": Found collection registrationQA: ";
            for(int item = 0; item < collection->GetNumberOfItems(); item++){
               vtkSmartPointer<vtkObject> object = collection->GetItemAsObject(item);
               registrationQANode = vtkMRMLRegistrationQANode::SafeDownCast(object);
               // Take only forward paramees
               if (! registrationQANode->GetBackwardRegistration() ){
                  registrationQAID = registrationQANode->GetID();
                  break;
               }
            }
         }
         else{
            //Create new reg quality
            qCritical() << Q_FUNC_INFO << ": Creating new registrationQA: ";
            registrationQANode = vtkMRMLRegistrationQANode::New();
            scene->AddNode( registrationQANode );
            registrationQAID = registrationQANode->GetID();
         }
      }
      registrationQANodeIDString = registrationQAID.toUtf8().constData();
   }

   registrationQANode = vtkMRMLRegistrationQANode::SafeDownCast(
         scene->GetNodeByID( registrationQANodeIDString ));

   
   if (!registrationQANode){
      qCritical() << Q_FUNC_INFO << "Can't get " << registrationQANodeIDString.c_str();
      return NULL;
   }
   
   if ( registrationQANode->GetBackwardRegistrationQAParameters() == NULL ){
      registrationQANode->CreateBackwardParameters();
   }
   
   this->m_RegistrationQAParametersID = QString(registrationQANodeIDString.c_str());
   
   return registrationQANode;
   
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::convertItemID(vtkIdType itemID){
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   if ( !itemID ) {
      qCritical() << Q_FUNC_INFO << ": No item ID";
      return;
   }
   
   vtkMRMLNode* node = shNode->GetItemDataNode(itemID);
   
   if ( !node) {
      qCritical() << Q_FUNC_INFO << ": No node for item";
      return;
   }
   
   vtkIdType parentID = shNode->GetItemParent(itemID);
   
   vtkMRMLRegistrationQANode* pNode = this->loadRegistrationQANode(itemID);
   //Check for backward
   if ( shNode->HasItemAttribute(parentID,d->regQALogic->INVERSE) ){
      pNode = pNode->GetBackwardRegistrationQAParameters();
   }
   
   if ( node->IsA("vtkMRMLVectorVolumeNode") ){
      vtkSmartPointer<vtkMRMLGridTransformNode> transformNode;
      transformNode = d->regQALogic->CreateTransformFromVector(vtkMRMLVectorVolumeNode::SafeDownCast(node));
      if ( ! transformNode ){
         qCritical() << Q_FUNC_INFO << ": Can't transform vector " << node->GetName() << " to transform";
         return;
      }
      vtkIdType transformNodeItemID = shNode->GetItemByDataNode(vtkMRMLNode::SafeDownCast(transformNode));
      std::stringstream newtransformNodeStringID;
      newtransformNodeStringID <<  transformNodeItemID;
      shNode->SetItemParent(transformNodeItemID, parentID);
      shNode->SetItemAttribute( transformNodeItemID, d->regQALogic->REGISTRATION_TYPE.c_str(), 
                              d->regQALogic->TRANSFORM.c_str());
      shNode->SetItemAttribute( parentID, d->regQALogic->TRANSFORMITEMID, newtransformNodeStringID.str().c_str() );
      
      // Write in pNode, if exist
      if ( pNode ){
         pNode->SetTransformNodeID(transformNode->GetID());
      }
   }
      
   // TODO: Locate ref volume and load it
   if ( node->IsA("vtkMRMLTransformNode") ){
      vtkSmartPointer<vtkMRMLVolumeNode> vectorVolume;
//       vectorVolume = d->regQALogic->CreateVectorFromTransform(node, RefVolume?);
      if ( ! vectorVolume ){
         qCritical() << Q_FUNC_INFO << ": Can't transform vector " << node->GetName() << " to transform";
         return;
      }
      vtkIdType vectorVolumeItemID = shNode->GetItemByDataNode(vtkMRMLNode::SafeDownCast(vectorVolume));
      std::stringstream newvectorVolumeStringID;
      newvectorVolumeStringID <<  vectorVolumeItemID;
      shNode->SetItemParent(vectorVolumeItemID, parentID);
      shNode->SetItemAttribute( vectorVolumeItemID, d->regQALogic->REGISTRATION_TYPE.c_str(), 
                              d->regQALogic->VECTOR_FIELD.c_str());
      shNode->SetItemAttribute( parentID, d->regQALogic->VECTORITEMID, newvectorVolumeStringID.str().c_str() );
   }
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::InputSelected(std::string name, bool backward, vtkIdType itemID, vtkMRMLNode* associatedNode)
{
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   
   if ( !itemID ) {
      qCritical() << Q_FUNC_INFO << ": No item ID";
      return;
   }
   if ( !associatedNode )
   {
      qCritical() << Q_FUNC_INFO << ": Can't get associatedNode.";
      return; // Only this plugin can handle this node
   }
   
   vtkMRMLRegistrationQANode* registrationQANode = this->loadRegistrationQANode(itemID);
   if ( ! registrationQANode ){
      qCritical() << Q_FUNC_INFO << ": Can't get registrationQANode.";
      return; // Only this plugin can handle this node
   }
   if ( backward ){
      registrationQANode = registrationQANode->GetBackwardRegistrationQAParameters();
   }
   
   registrationQANode->DisableModifiedEventOn();
   if ( name.compare(d->regQALogic->IMAGE) == 0 ){
      if (!associatedNode->IsA("vtkMRMLScalarVolumeNode")){
         qCritical() << associatedNode->GetName() << ": Not a scalar volume.";
      }
      registrationQANode->SetAndObserveVolumeNodeID(associatedNode->GetID());
   }
   else if ( name.compare(d->regQALogic->WARPED_IMAGE) == 0 ){
      if ( !associatedNode->IsA("vtkMRMLScalarVolumeNode")){
         qCritical() << associatedNode->GetName() << ": Not a scalar volume.";
      }
      registrationQANode->SetAndObserveWarpedVolumeNodeID(associatedNode->GetID());
   }
   else if ( name.compare(d->regQALogic->VECTOR_FIELD) == 0 ){
      if ( !(associatedNode->IsA("vtkMRMLVectorVolumeNode")) ){
         qCritical() << associatedNode->GetName() << ": Not a vector volume.";
      }
      registrationQANode->SetAndObserveVectorVolumeNodeID(associatedNode->GetID());
   }
   else if ( name.compare(d->regQALogic->FIDUCIAL) == 0){
      if (!associatedNode->IsA("vtkMRMLMarkupsNode")){
         qCritical() << associatedNode->GetName() << ": Not a markups node.";
      }
      registrationQANode->SetAndObserveFiducialNodeID(associatedNode->GetID());
   }
   else if ( name.compare(d->regQALogic->ROI) == 0){
      if (!associatedNode->IsA("vtkMRMLAnnotationROINode")){
         qCritical() << associatedNode->GetName() << ": Not a annotation ROI node.";
      }
      registrationQANode->SetAndObserveROINodeID(associatedNode->GetID());
   }
   else{
      qCritical() << associatedNode->GetName() << ": with an unnkown reg type: " << name.c_str();
      return;
   }
     
   registrationQANode->DisableModifiedEventOff();
   
   /* Update backward node */
   if ( backward ){
      registrationQANode->CopyFromBackward();
   }
   else{
      registrationQANode->GetBackwardRegistrationQAParameters()->CopyFromBackward();
   }
   
   //Try to update widget

   qSlicerAbstractCoreModule* module = qSlicerApplication::application()->moduleManager()->module("RegistrationQA");
   qSlicerAbstractModule* moduleWithAction = qobject_cast<qSlicerAbstractModule*>(module);
   if (!moduleWithAction)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to get RegistrationQA module";
      return;
   }
   qSlicerAbstractModuleRepresentation* widget = moduleWithAction->widgetRepresentation();
   if (!widget)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to get RegistrationQA module widget";
      return;
   }
   if (! widget->setEditedNode(associatedNode) ){
      qCritical() << Q_FUNC_INFO << ": Failed to do something";
      return;
   }
   
   qCritical() << Q_FUNC_INFO << ":set" << associatedNode->GetName() << " to " << name.c_str() << "\n";
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::InputSelected(std::string name, bool backward)
{
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   vtkIdType itemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
   vtkMRMLNode* associatedNode = shNode->GetItemDataNode(itemID);
   
   if (!associatedNode){
      qCritical() << Q_FUNC_INFO << ": Failed to access associated node";
      return;
   }
   
   this->InputSelected(name, backward, itemID, associatedNode);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::referenceImageSelected()
{
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   this->InputSelected(d->regQALogic->IMAGE,false);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::movingImageSelected()
{
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   this->InputSelected(d->regQALogic->IMAGE, true);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::fwarpedImageSelected()
{
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   this->InputSelected(d->regQALogic->WARPED_IMAGE, false);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::bwarpedImageSelected()
{
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   this->InputSelected(d->regQALogic->WARPED_IMAGE, true);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::referenceVectorFieldSelected()
{
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   this->InputSelected(d->regQALogic->VECTOR_FIELD, false);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::movingVectorFieldSelected()
{
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   this->InputSelected(d->regQALogic->VECTOR_FIELD, true);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::referenceFiducialsSelected()
{
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   this->InputSelected(d->regQALogic->FIDUCIAL, false);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::movingFiducialsSelected()
{
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   this->InputSelected(d->regQALogic->FIDUCIAL, true);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::ROISelected()
{
   Q_D(qSlicerSubjectHierarchyRegistrationQAPlugin);
   this->InputSelected(d->regQALogic->ROI, false);
}
void qSlicerSubjectHierarchyRegistrationQAPlugin::loadContourSelected(bool backward){
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
   
   /* First check, if it is segment */
   std::string segmentStringID = shNode->GetItemAttribute(currentItemID,"segmentID");
   if ( segmentStringID.empty() ){
      qCritical() << Q_FUNC_INFO << ": Selected node not a segment.";
      return;
   }
   
   /* Check if parent node is segmentation node */
   vtkMRMLNode* segmentationNode = shNode->GetItemDataNode(shNode->GetItemParent(currentItemID));
   if ( ! (segmentationNode && segmentationNode->IsA("vtkMRMLSegmentationNode")) )
   {
      qCritical() << Q_FUNC_INFO << ": Selected node is not a segmentation node.";
      return; // Only this plugin can handle this node
   }
   
   vtkMRMLRegistrationQANode* registrationQANode = this->loadRegistrationQANode(currentItemID);
   if (backward){
      registrationQANode = registrationQANode->GetBackwardRegistrationQAParameters();
   }
   
   registrationQANode->SetSegmentID(segmentStringID.c_str());
   registrationQANode->SetAndObserveSegmentationNode(vtkMRMLSegmentationNode::SafeDownCast(segmentationNode));
   
   //Try to update widget
    // Get Registration Quality module
   qSlicerAbstractCoreModule* module = qSlicerApplication::application()->moduleManager()->module("RegistrationQA");
   qSlicerAbstractModule* moduleWithAction = qobject_cast<qSlicerAbstractModule*>(module);
   if (!moduleWithAction)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to get RegistrationQA module";
      return;
   }
   qSlicerAbstractModuleRepresentation* widget = moduleWithAction->widgetRepresentation();
   if (!widget)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to get RegistrationQA module widget";
      return;
   }
   if (! widget->setEditedNode(segmentationNode) ){
      return; // Only this plugin can handle this node
   }

}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::referenceContourSelected()
{
   this->loadContourSelected(false);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::movingContourSelected()
{
   this->loadContourSelected(true);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQAPlugin::setNodeInRegistrationQA(vtkMRMLNode* node, QString role, QString context)
{

    // Get Registration Quality module
   qSlicerAbstractCoreModule* module = qSlicerApplication::application()->moduleManager()->module("RegistrationQA");
   qSlicerAbstractModule* moduleWithAction = qobject_cast<qSlicerAbstractModule*>(module);
   if (!moduleWithAction)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to get RegistrationQA module";
      return;
   }
   // Select dose volume node in Dose Volume Histogram module
   qSlicerAbstractModuleRepresentation* widget = moduleWithAction->widgetRepresentation();
   if (!widget)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to get RegistrationQA module widget";
      return;
   }
   if (! widget->setEditedNode(node,role,context) ){
      qCritical() << Q_FUNC_INFO << ": Can't add " << node->GetName() << " to " << role;
      return; // Only this plugin can handle this node
   }
}