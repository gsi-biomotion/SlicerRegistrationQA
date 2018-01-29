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


#include "qSlicerSubjectHierarchyRegistrationQualityPlugin.h"

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
// #include "vtkMRMLMarkupsNode.h"
#include "vtkSlicerRegistrationQualityLogic.h"
#include "vtkMRMLRegistrationQualityNode.h"
#include <vtkSlicerVolumesLogic.h>
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
class qSlicerSubjectHierarchyRegistrationQualityPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyRegistrationQualityPlugin);
protected:
  qSlicerSubjectHierarchyRegistrationQualityPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyRegistrationQualityPluginPrivate(qSlicerSubjectHierarchyRegistrationQualityPlugin& object);
  ~qSlicerSubjectHierarchyRegistrationQualityPluginPrivate();
  void init();
public:
  QMenu* DIRQAMenu;
  QActionGroup* DIRQAMenuActionGroup;
  QIcon DIRQAIcon;
  vtkSlicerRegistrationQualityLogic* DIRQALogic;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyRegistrationQualityPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyRegistrationQualityPluginPrivate::qSlicerSubjectHierarchyRegistrationQualityPluginPrivate(qSlicerSubjectHierarchyRegistrationQualityPlugin& object)
 : q_ptr(&object)
{
  this->DIRQAMenu = NULL;
  this->DIRQAMenuActionGroup = NULL;
  this->DIRQAIcon = QIcon(":Icons/ContourIcon.png");
  this->DIRQALogic = NULL;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyRegistrationQualityPlugin);

  
  
  this->DIRQAMenu = new QMenu("DIRQA Functions");
  this->DIRQAMenuActionGroup = new QActionGroup(q);
  
  QMenu* assignMenu = this->DIRQAMenu->addMenu("Assign node to:");
  QMenu* imageMenu = assignMenu->addMenu( "Image" );
  
  QAction* fixedImageAction = new QAction("Fixed Image",q);
  QObject::connect(fixedImageAction, SIGNAL(triggered()), q, SLOT(fixedImageSelected()));
  fixedImageAction->setActionGroup(this->DIRQAMenuActionGroup);
  fixedImageAction->setData(QVariant("Fixed Image"));
  imageMenu->addAction(fixedImageAction);
  
  QAction* movingImageAction = new QAction("Moving Image",q);
  QObject::connect(movingImageAction, SIGNAL(triggered()), q, SLOT(movingImageSelected()));
  movingImageAction->setActionGroup(this->DIRQAMenuActionGroup);
  movingImageAction->setData(QVariant("Moving Image"));
  imageMenu->addAction(movingImageAction);
  
  QAction* fbwarpedImageAction = new QAction("Forward Warped Image",q);
  QObject::connect(fbwarpedImageAction, SIGNAL(triggered()), q, SLOT(fwarpedImageSelected()));
  fbwarpedImageAction->setActionGroup(this->DIRQAMenuActionGroup);
  fbwarpedImageAction->setData(QVariant("Forward Warped Image"));
  imageMenu->addAction(fbwarpedImageAction);
  
  QAction* bwarpedImageAction = new QAction("Backward Warped Image",q);
  QObject::connect(bwarpedImageAction, SIGNAL(triggered()), q, SLOT(bwarpedImageSelected()));
  bwarpedImageAction->setActionGroup(this->DIRQAMenuActionGroup);
  bwarpedImageAction->setData(QVariant("Backward Warped Image"));
  imageMenu->addAction(bwarpedImageAction);

  QMenu* vectorMenu = assignMenu->addMenu( "Vector Field" );
  QAction* fixedVFAction = new QAction("Forward",q);
  QObject::connect(fixedVFAction, SIGNAL(triggered()), q, SLOT(fixedVectorFieldSelected()));
  fixedVFAction->setActionGroup(this->DIRQAMenuActionGroup);
  fixedVFAction->setData(QVariant("Forward"));
  vectorMenu->addAction(fixedVFAction);
  
  QAction* movingVFAction = new QAction("Backward",q);
  QObject::connect(movingVFAction, SIGNAL(triggered()), q, SLOT(movingVectorFieldSelected()));
  movingVFAction->setActionGroup(this->DIRQAMenuActionGroup);
  movingVFAction->setData(QVariant("Backward"));
  vectorMenu->addAction(movingVFAction);
  
  QMenu* fiducialMenu = assignMenu->addMenu( "Fiducial" );
  QAction* fixedFiducialsAction = new QAction("Fixed phase",q);
  QObject::connect(fixedFiducialsAction, SIGNAL(triggered()), q, SLOT(fixedFiducialsSelected()));
  fixedFiducialsAction->setActionGroup(this->DIRQAMenuActionGroup);
  fixedFiducialsAction->setData(QVariant("Fixed phase"));
  fiducialMenu->addAction(fixedFiducialsAction);
  
  QAction* movingFiducialsAction = new QAction("Moving phase",q);
  QObject::connect(movingFiducialsAction, SIGNAL(triggered()), q, SLOT(movingFiducialsSelected()));
  movingFiducialsAction->setActionGroup(this->DIRQAMenuActionGroup);
  movingFiducialsAction->setData(QVariant("Moving phase"));
  fiducialMenu->addAction(movingFiducialsAction);
  
  QMenu* contourMenu = assignMenu->addMenu( "Contour" );
  QAction* fixedContourAction = new QAction("Fixed phase",q);
  QObject::connect(fixedContourAction, SIGNAL(triggered()), q, SLOT(fixedContourSelected()));
  fixedContourAction->setActionGroup(this->DIRQAMenuActionGroup);
  fixedContourAction->setData(QVariant("Fixed phase"));
  contourMenu->addAction(fixedContourAction);
  
  QAction* movingContourAction = new QAction("Moving phase",q);
  QObject::connect(movingContourAction, SIGNAL(triggered()), q, SLOT(movingContourSelected()));
  movingContourAction->setActionGroup(this->DIRQAMenuActionGroup);
  movingContourAction->setData(QVariant("Moving phase"));
  contourMenu->addAction(movingContourAction);
  
  QAction* roiAction = new QAction("ROI",q);
  QObject::connect(roiAction, SIGNAL(triggered()), q, SLOT(ROISelected()));
  roiAction->setActionGroup(this->DIRQAMenuActionGroup);
  roiAction->setData(QVariant("ROI"));
  assignMenu->addAction(roiAction);
  
  QAction* calculateDIRQAAction = new QAction("Calculate DIRQA",q);
  QObject::connect(calculateDIRQAAction, SIGNAL(triggered()), q, SLOT(calcuateDIRQAForCurrentNode()));
  calculateDIRQAAction->setCheckable(true);
  calculateDIRQAAction->setActionGroup(this->DIRQAMenuActionGroup);
  calculateDIRQAAction->setData(QVariant("Calculate DIRQA"));
  this->DIRQAMenu->addAction(calculateDIRQAAction);
  
  QAction* loadVolumeAction = new QAction("Load from File",q);
  QObject::connect(loadVolumeAction, SIGNAL(triggered()), q, SLOT(loadFromFilenameForCurrentID()));
  loadVolumeAction->setCheckable(true);
  loadVolumeAction->setActionGroup(this->DIRQAMenuActionGroup);
  loadVolumeAction->setData(QVariant("Load Volume from File"));
  this->DIRQAMenu->addAction(loadVolumeAction);
  
  QAction* updateVolumeAction = new QAction("Load and Update",q);
  QObject::connect(updateVolumeAction, SIGNAL(triggered()), q, SLOT(updateRegNodeForCurrentNode()));
  updateVolumeAction->setCheckable(true);
  updateVolumeAction->setActionGroup(this->DIRQAMenuActionGroup);
  updateVolumeAction->setData(QVariant("Load and Update"));
  this->DIRQAMenu->addAction(updateVolumeAction);

  this->DIRQALogic = vtkSlicerRegistrationQualityLogic::New();
  vtkSmartPointer<vtkMRMLScene> scene; 
  scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  this->DIRQALogic->SetMRMLScene(scene);
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyRegistrationQualityPluginPrivate::~qSlicerSubjectHierarchyRegistrationQualityPluginPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyRegistrationQualityPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyRegistrationQualityPlugin::qSlicerSubjectHierarchyRegistrationQualityPlugin(QObject* parent)
 : Superclass(parent)
 , m_RegQAParametersID(QString())
 , d_ptr( new qSlicerSubjectHierarchyRegistrationQualityPluginPrivate(*this) )
{
  this->m_Name = QString("RegQuality");
  this->m_RegQAParametersID = QString("");

  Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
  d->init();
}
//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyRegistrationQualityPlugin::~qSlicerSubjectHierarchyRegistrationQualityPlugin()
{
}

//----------------------------------------------------------------------------
// double d->DIRQALogic->canAddNodeToSubjectHierarchy(
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
double qSlicerSubjectHierarchyRegistrationQualityPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID)const
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
const QString qSlicerSubjectHierarchyRegistrationQualityPlugin::roleForPlugin()const
{
  return "Registration Quality";
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyRegistrationQualityPlugin::tooltip(vtkIdType itemID)const
{
  return qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName("Volumes")->tooltip(itemID);
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyRegistrationQualityPlugin::icon(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);

  if (!itemID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QIcon();
  }

  if (this->canOwnSubjectHierarchyItem(itemID))
  {
    return d->DIRQAIcon;
  }

  // Node unknown by plugin
  return QIcon();
}

QIcon qSlicerSubjectHierarchyRegistrationQualityPlugin::visibilityIcon(int visible)
{
  // Have the default plugin (which is not registered) take care of this
  return qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin()->visibilityIcon(visible);
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyRegistrationQualityPlugin::itemContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyRegistrationQualityPlugin);

  QList<QAction*> actions;
  actions << d->DIRQAMenu->menuAction();
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::showContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);

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
    d->DIRQAMenu->menuAction()->setVisible(true);
//     d->DIRQAMenu->selectSegment()->setVisible(false);
//     d->RegisterThisAction->setVisible(true);
//   }
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::loadFromFilenameForCurrentID()
{
  Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
  }
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  this->loadFromFilenameForItemID(currentItemID);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::loadFromFilenameForItemID(vtkIdType itemID)
{
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   
   if ( ! shNode->HasItemAttribute(itemID, d->DIRQALogic->FILEPATH.c_str())){
      qCritical() << Q_FUNC_INFO << ": No filepath for: " << shNode->GetItemName(itemID).c_str();
      return;
   }
  
   vtkSmartPointer<vtkMRMLNode> node;
   std::string regTypeIdentifier = shNode->GetItemAttribute(itemID,
                    d->DIRQALogic->REGISTRATION_TYPE.c_str());
   bool backward = shNode->HasItemAttribute(itemID, d->DIRQALogic->INVERSE.c_str());
   
   if ( regTypeIdentifier.compare(d->DIRQALogic->FIDUCIAL) == 0){
      node = vtkMRMLNode::SafeDownCast(this->loadMarkups(itemID));
   }
   else if ( regTypeIdentifier.compare(d->DIRQALogic->ROI) == 0){
      node = vtkMRMLNode::SafeDownCast(this->loadROI(itemID));
   }
   else{
      node = vtkMRMLNode::SafeDownCast(this->loadVolumeFromItemId(itemID));
   }
   
   if (node){
      this->InputSelected(regTypeIdentifier,backward,itemID,node);
   }
  
}
// //---------------------------------------------------------------------------
// void qSlicerSubjectHierarchyRegistrationQualityPlugin::assignNodeToRegQANode(vtkMRMLNode* node, ){
//    
// }
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::calcuateDIRQAForCurrentNode()
{
  
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  
  if (!currentItemID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
  }
  
  this->calcuateDIRQAForID(currentItemID, false);
}
void qSlicerSubjectHierarchyRegistrationQualityPlugin::updateRegNodeForCurrentNode(){
   vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  
  if (!currentItemID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
  }
  
  this->updateRegNodeForID(currentItemID);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::calcuateDIRQAForID(vtkIdType itemID, bool removeNodes) {
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   /* Find out which kind ot image we have */
   std::string regTypeIdentifier = shNode->GetItemAttribute(itemID,d->DIRQALogic->REGISTRATION_TYPE.c_str());

   if ( regTypeIdentifier.compare(d->DIRQALogic->WARPED_IMAGE) == 0 ) {
      this->calculateAbsoluteDifference(itemID,removeNodes);
   }
   else if ( regTypeIdentifier.compare(d->DIRQALogic->VECTOR_FIELD) == 0){
      this->calculateJacobian(itemID,false); /* No need to remove nodes here, we do it in invconsist */
      this->calculateInverseConsistency(itemID,false);
      if (removeNodes){
         this->removeNode(itemID);
      }
   }
   else if ( regTypeIdentifier.compare(d->DIRQALogic->FIDUCIAL) == 0){
      this->calculateFiducialDistance(itemID,removeNodes);
   }
   else{
  
   /* And check, if there are any children items and run DIRQA on them */
      vtkSmartPointer<vtkIdList> IdList = vtkSmartPointer<vtkIdList>::New();
      shNode->GetItemChildren(itemID,IdList);
      if ( IdList->GetNumberOfIds() > 0 ){
         for (int i=0; i < IdList->GetNumberOfIds(); i++){
            this->calcuateDIRQAForID(IdList->GetId(i), true); // Remove nodes is set on, so that multiple vectors can be loaded
         }
      }
   }
   
   
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::updateRegNodeForID(vtkIdType itemID){
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   /* Check all children, load them and write them in regQA node */
   std::string regTypeIdentifier = shNode->GetItemAttribute(itemID,
                    d->DIRQALogic->REGISTRATION_TYPE);
   
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
void qSlicerSubjectHierarchyRegistrationQualityPlugin::calculateFiducialDistance(vtkIdType itemID, bool removeNodes){
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
   
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   
   /* Check if previously calculated */
   vtkIdType fiducialFolder = shNode->GetItemChildWithName(itemID,
      d->DIRQALogic->FIDUCIAL.c_str());
   if ( fiducialFolder ){
      return;
   }
   
   vtkSmartPointer<vtkMRMLMarkupsFiducialNode> fiducialFixedNode;
   vtkSmartPointer<vtkMRMLMarkupsFiducialNode> fiducialReferenceNode;
   fiducialFixedNode = this->loadMarkups(itemID);
   
   
   std::string fixedImageStringID  = shNode->GetItemAttribute(
      itemID,d->DIRQALogic->FIXEDIMAGEID.c_str());
   
   std::string referenceImageStringID = shNode->GetAttributeFromItemAncestor(itemID,
        d->DIRQALogic->REFIMAGEID.c_str());
   
   if (fixedImageStringID.compare(referenceImageStringID) == 0 ){
      // Fiducial distance can't be calculated for reference case
      return;
   }
   
   //Find reference markups
   vtkIdType markupsFieldFolderID = shNode->GetItemParent(itemID);
   vtkIdType refMarkupID;
   bool bFound = false;
   vtkSmartPointer<vtkIdList> IdMarkupList = vtkSmartPointer<vtkIdList>::New();
   shNode->GetItemChildren(markupsFieldFolderID,IdMarkupList);
   for(int i=0;i<IdMarkupList->GetNumberOfIds();i++){
      refMarkupID = IdMarkupList->GetId(i);
      std::string fixedImageID1  = shNode->GetItemAttribute(
         refMarkupID,d->DIRQALogic->FIXEDIMAGEID.c_str());
      if ( fixedImageID1.compare(referenceImageStringID) == 0 ){
         bFound = true;
         break;
      }
   }
   
   if (! bFound ){
      qCritical() << Q_FUNC_INFO << ": Can't find markup with fixed ID " << referenceImageStringID.c_str();
      return;
   }
   
   fiducialReferenceNode = this->loadMarkups(refMarkupID);
   
   //Find appropriate vector field
   
   std::string vectorFieldFolderStringID = shNode->GetAttributeFromItemAncestor(itemID,
        d->DIRQALogic->VECTORITEMID.c_str());
   
   if ( vectorFieldFolderStringID.empty() ){
      qCritical() << Q_FUNC_INFO << ": Failed to vector field folder";
      return;
   }
   
   vtkIdType vectorFieldFolderID = atoll(vectorFieldFolderStringID.c_str());
   vtkIdType vectorItemID;
   bFound = false;
   vtkSmartPointer<vtkIdList> IdList = vtkSmartPointer<vtkIdList>::New();
   shNode->GetItemChildren(vectorFieldFolderID,IdList);
   
   for(int i=0;i<IdList->GetNumberOfIds();i++){
      vectorItemID = IdList->GetId(i);
      std::string fixedImageID2  = shNode->GetItemAttribute(
         vectorItemID,d->DIRQALogic->FIXEDIMAGEID.c_str());
      std::string movingImageID  = shNode->GetItemAttribute(
         vectorItemID,d->DIRQALogic->MOVINGIMAGEID.c_str());
      if (fixedImageID2.compare(referenceImageStringID) == 0 && movingImageID.compare(fixedImageStringID) == 0){
         bFound = true;
         break;
      }
   }
   
   if (! bFound ){
      qCritical() << Q_FUNC_INFO << ": Can't find vector field with fixed/moving phase " << referenceImageStringID.c_str() <<
      "/" << fixedImageStringID.c_str();
      return;
   }
   
   vtkSmartPointer<vtkMRMLVectorVolumeNode> vectorNode;
   vectorNode = vtkMRMLVectorVolumeNode::SafeDownCast(this->loadVolumeFromItemId(vectorItemID));

   double statisticValues[12];
   
   if ( ! d->DIRQALogic->CalculateFiducialsDistance(fiducialReferenceNode, fiducialFixedNode, 
      vectorNode, statisticValues) ){
      qCritical() << Q_FUNC_INFO << ": Can't calculate Fiducial distance.";
      return;
   }
   
   this->writeInTable(itemID,
         d->DIRQALogic->FIDUCIALTABLE.c_str(), statisticValues, fiducialFixedNode);
   
   fiducialFolder = shNode->CreateFolderItem(itemID,
            d->DIRQALogic->FIDUCIAL.c_str());      
//    vtkIdType jacobianID = shNode->CreateItem(jacobianFolder,jacobianNode);
   
   if ( removeNodes){
//       shNode->RemoveItem(jacobianID);
      this->removeNode(vtkMRMLNode::SafeDownCast(vectorNode));
      this->removeNode(vtkMRMLNode::SafeDownCast(fiducialReferenceNode));
      this->removeNode(vtkMRMLNode::SafeDownCast(fiducialFixedNode));
   }
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::calculateJacobian(vtkIdType itemID, bool removeNodes){
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
   
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   
   /* Check if previously calculated */
   vtkIdType jacobianFolder = shNode->GetItemChildWithName(itemID,
      d->DIRQALogic->JACOBIAN.c_str());
   if ( jacobianFolder ){
      return;
   }
   
   

   /* Load vector for current ItemID */
   this->loadFromFilenameForItemID(itemID);
   
//    vtkSmartPointer<vtkMRMLAnnotationROINode> ROINode;
   vtkSmartPointer<vtkMRMLScene> scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
   vtkSmartPointer<vtkMRMLScalarVolumeNode> jacobianNode;
//    ROINode = this->loadROI(itemID);
   
   vtkMRMLRegistrationQualityNode* regQANode = this->loadRegQANode(itemID);
   if ( shNode->HasItemAttribute(itemID, d->DIRQALogic->INVERSE)){
      regQANode = regQANode->GetBackwardRegQAParameters();
   }
   
   /* Set old jacobian to zero in regqanode */
   regQANode->SetAndObserveJacobianVolumeNodeID(NULL);
   regQANode->Modified();
   
   double statisticValues[4];
   if ( !d->DIRQALogic->Jacobian(regQANode, statisticValues) ){
      qCritical() << Q_FUNC_INFO << ": Failed to calculate jacobian for" << shNode->GetItemName(itemID).c_str();
      return;
   }
   
//    jacobianNode = d->DIRQALogic->Jacobian(vectorNode, ROINode);
   jacobianNode = vtkMRMLScalarVolumeNode::SafeDownCast(scene->GetNodeByID(
      regQANode->GetJacobianVolumeNodeID()));
      
   if (jacobianNode == NULL ){
      qCritical() << Q_FUNC_INFO << ": Can't calculate Jacobian";
      return;
   }
   
   jacobianFolder = shNode->CreateFolderItem(itemID,
                          d->DIRQALogic->JACOBIAN.c_str());
   
   
//    d->DIRQALogic->CalculateStatistics(jacobianNode, statisticValues);
   this->writeInTable(itemID,
         d->DIRQALogic->JACOBIANTABLE.c_str(), statisticValues);
   vtkIdType jacobianID = shNode->CreateItem(jacobianFolder,jacobianNode);

   

   shNode->SetItemAttribute(jacobianID, d->DIRQALogic->REGISTRATION_TYPE.c_str(), 
                              d->DIRQALogic->JACOBIAN.c_str());
   std::stringstream jacobianStringID;
   jacobianStringID << jacobianID;
   shNode->SetItemAttribute(jacobianFolder, d->DIRQALogic->JACOBIANITEMID.c_str(), 
      jacobianStringID.str().c_str());
   
   if ( removeNodes){
//       shNode->RemoveItem(jacobianID);
      this->removeNode(itemID);
   }

}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::calculateInverseConsistency(vtkIdType itemID, bool removeNodes){
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
   
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
      d->DIRQALogic->INVERSECONSISTENCY.c_str());
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
   
   vtkMRMLRegistrationQualityNode* regQANode = this->loadRegQANode(itemID);
   regQANode->SetAndObserveInverseConsistVolumeNodeID(NULL);
   regQANode->Modified();

   double statisticValues[4];

   if ( !d->DIRQALogic->InverseConsist(regQANode, statisticValues) ){
      qCritical() << Q_FUNC_INFO << ": Failed to calcualte inverse consistency for" << shNode->GetItemName(itemID).c_str();
      return;
   }
   backwardConsistNode = vtkMRMLScalarVolumeNode::SafeDownCast(scene->GetNodeByID(
      regQANode->GetInverseConsistVolumeNodeID()));
      
   if (backwardConsistNode == NULL ){
      qCritical() << Q_FUNC_INFO << ": Can't calculate inverse consistency";
      return;
   }
 
   this->writeInTable(itemID,
      d->DIRQALogic->INVCONSISTTABLE.c_str(), statisticValues);

   /* Write all the necessary data in subject hierarchy*/
   if (backwardConsistNode == NULL ){
      qCritical() << Q_FUNC_INFO << ": Can't calculate Inverse Consistency";
      return;
   }
   
   invConsistFolder = shNode->CreateFolderItem(itemID,
                   d->DIRQALogic->INVERSECONSISTENCY.c_str());

   vtkIdType backwardConsistID = shNode->CreateItem(invConsistFolder,backwardConsistNode);

   shNode->SetItemAttribute(backwardConsistID, d->DIRQALogic->REGISTRATION_TYPE.c_str(), 
                              d->DIRQALogic->INVERSECONSISTENCY.c_str());
   std::stringstream backwardConsistStringID;
   backwardConsistStringID <<  backwardConsistID;
   shNode->SetItemAttribute(invConsistFolder, d->DIRQALogic->INVERSECONSISTENCYITEMID.c_str(), 
      backwardConsistStringID.str().c_str());
   
   /* Check if backward vector has inverse consistency */
   vtkIdType backInvConsistFolder = shNode->GetItemChildWithName(backwardVectorID,
      d->DIRQALogic->INVERSECONSISTENCY.c_str());
   
   if ( ! backInvConsistFolder ){
      backInvConsistFolder = shNode->CreateFolderItem(backwardVectorID,
                   d->DIRQALogic->INVERSECONSISTENCY.c_str());
      shNode->SetItemAttribute(backInvConsistFolder, d->DIRQALogic->INVERSECONSISTENCYITEMID.c_str(), 
            backwardConsistStringID.str().c_str());
   }
   
//       this->removeNode(itemID);
   this->removeNode(backwardVectorID);
}
//---------------------------------------------------------------------------
vtkIdType qSlicerSubjectHierarchyRegistrationQualityPlugin::findInverseVectorID(vtkIdType itemID){
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return 0;
   }
   
   /* find fixed and moving image ID */
   std::string fixedImage1ID  = shNode->GetItemAttribute(
      itemID,d->DIRQALogic->FIXEDIMAGEID.c_str());
   std::string movingImage1ID  = shNode->GetItemAttribute(
      itemID,d->DIRQALogic->MOVINGIMAGEID.c_str());
   

   /* find items with reverse fixed and moving image ID */
   vtkIdType parentItemID = shNode->GetItemParent(itemID);
   vtkSmartPointer<vtkIdList> IdList = vtkSmartPointer<vtkIdList>::New();
   shNode->GetItemChildren(parentItemID,IdList);
   
   vtkIdType backwardVectorItemID;
   
   for(int i=0;i<IdList->GetNumberOfIds();i++){
      backwardVectorItemID = IdList->GetId(i);
      std::string fixedImage2ID  = shNode->GetItemAttribute(
         backwardVectorItemID,d->DIRQALogic->FIXEDIMAGEID.c_str());
      std::string movingImage2ID  = shNode->GetItemAttribute(
         backwardVectorItemID,d->DIRQALogic->MOVINGIMAGEID.c_str());
      if (fixedImage1ID.compare(movingImage2ID) == 0 && movingImage1ID.compare(fixedImage2ID) == 0){
         return backwardVectorItemID;
      }
   }
   return 0;
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::calculateAbsoluteDifference(vtkIdType itemID, bool removeNodes){
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
   
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   
   /* Check if previously calculated */
   vtkIdType absDiffFolder = shNode->GetItemChildWithName(itemID,
      d->DIRQALogic->ABSOLUTEDIFFERENCE.c_str());
   if ( absDiffFolder ){
      return;
   }
   
   
   std::string fixedImageCharID  = shNode->GetItemAttribute(itemID,
                        d->DIRQALogic->FIXEDIMAGEID.c_str());
 
   if ( fixedImageCharID.c_str() == NULL){
      qCritical() << Q_FUNC_INFO << ": Failed to find image IDs";
      return;
   }
   
   vtkIdType fixedImageID = atoll(fixedImageCharID.c_str());
   vtkSmartPointer<vtkMRMLScalarVolumeNode> AbsDiffNode;
   vtkSmartPointer<vtkMRMLScene> scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
   double statisticValues[4];
   
   this->loadFromFilenameForItemID(itemID);
   this->loadFromFilenameForItemID(fixedImageID);
   
   vtkMRMLRegistrationQualityNode* regQANode = this->loadRegQANode(itemID);
   
   if ( !d->DIRQALogic->AbsoluteDifference(regQANode, statisticValues) ){
      qCritical() << Q_FUNC_INFO << ": Failed to calcualte absolute difference for" << shNode->GetItemName(itemID).c_str();
      return;
   }
   
   AbsDiffNode = vtkMRMLScalarVolumeNode::SafeDownCast(scene->GetNodeByID(
      regQANode->GetAbsoluteDiffVolumeNodeID()));
   if (AbsDiffNode == NULL ){
      qCritical() << Q_FUNC_INFO << ": Can't calculate Absolute Difference";
      return;
   }

   absDiffFolder = shNode->CreateFolderItem(itemID,
         d->DIRQALogic->ABSOLUTEDIFFERENCE.c_str());
   
   this->writeInTable(itemID,
         d->DIRQALogic->ABSDIFFTABLE.c_str(), statisticValues);           
   vtkIdType absDiffID = shNode->CreateItem(absDiffFolder,AbsDiffNode);
   
   shNode->SetItemAttribute(absDiffID, d->DIRQALogic->REGISTRATION_TYPE.c_str(), 
                  d->DIRQALogic->ABSOLUTEDIFFERENCE.c_str());
   std::stringstream absDiffStringID;
   absDiffStringID <<  absDiffID;
   shNode->SetItemAttribute(absDiffFolder, d->DIRQALogic->ABSDIFFNODEITEMID.c_str(), 
      absDiffStringID.str().c_str());

   if (removeNodes){
      this->removeNode(itemID);
      this->removeNode(fixedImageID);
   }
 
      
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::removeNode(vtkMRMLNode* node){
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   
   vtkIdType id = shNode->GetItemByDataNode(node);
   vtkIdType parentID = shNode->GetItemParent(id);
   shNode->RemoveItemAttribute(parentID,
          d->DIRQALogic->NODEITEMID.c_str());
   shNode->RemoveItem(id);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::removeNode(vtkIdType itemID){
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
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
// void qSlicerSubjectHierarchyRegistrationQualityPlugin::calculateStatistics(vtkMRMLScalarVolumeNode* scalarNode, double statisticValues[4]){
//    Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
//    
//    vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
//    if (!shNode)
//    {
//       qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
//       return NULL;
//    }
//    
//    /* Calculate statistics */
//    d->DIRQALogic->CalculateStatistics(scalarNode, statisticValues);
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
void qSlicerSubjectHierarchyRegistrationQualityPlugin::writeInTable(vtkIdType itemID,char const* tableName, double* statisticValues, vtkMRMLMarkupsFiducialNode* fiducalsNode/*=NULL*/){
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
vtkMRMLTableNode* qSlicerSubjectHierarchyRegistrationQualityPlugin::createTable(vtkIdType itemID,char const* tableName,bool fiducialsTable/*=false*/){
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
// vtkMRMLAnnotationROINode* qSlicerSubjectHierarchyRegistrationQualityPlugin::calculateVolumeHistogram(vtkMRMLScalarVolumeNode* scalarNode){
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
vtkMRMLAnnotationROINode* qSlicerSubjectHierarchyRegistrationQualityPlugin::loadROI(vtkIdType itemID){
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return NULL;
   }
   
   std::string roiCharID = shNode->GetAttributeFromItemAncestor(itemID,d->DIRQALogic->ROIITEMID.c_str());
   
   if (roiCharID.c_str() == NULL){
      qCritical() << Q_FUNC_INFO << ": Can't find ROI item";
      return NULL;
   }
   
   vtkIdType roiItemID = atoll(roiCharID.c_str());
   
   /* check if its already loaded */
   std::string nodeStringItemID = shNode->GetItemAttribute(roiItemID,
         d->DIRQALogic->NODEITEMID.c_str());
   
   if ( ! nodeStringItemID.empty() ){
      vtkIdType nodeItemID = atoll(nodeStringItemID.c_str());
      vtkSmartPointer<vtkMRMLNode> node; 
      node = shNode->GetItemDataNode(nodeItemID);
      if ( node ){
         return vtkMRMLAnnotationROINode::SafeDownCast(node);
      }
      else{
         shNode->RemoveItemAttribute(roiItemID,
         d->DIRQALogic->NODEITEMID.c_str());
      }
   }
   
   vtkNew<vtkSlicerAnnotationModuleLogic> logic;
   vtkSmartPointer<vtkMRMLScene> scene; 
   scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
   logic->SetMRMLScene(scene);
   
   std::string filePath = shNode->GetItemAttribute(roiItemID, d->DIRQALogic->FILEPATH.c_str());
   
   if (filePath.c_str() == NULL){
      qCritical() << Q_FUNC_INFO << ": Can't find ROI filepath";
      return NULL;
   }
   
   char *RoiNodeID = logic->LoadAnnotation(filePath.c_str(), d->DIRQALogic->ROI.c_str(), 3);
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
   shNode->SetItemAttribute(roiItemID,d->DIRQALogic->NODEITEMID.c_str(),
      currentRoiIDString.str().c_str());

//    shNode->SetItemParent(currentRoiItemID,roiItemID);
   return RoiNode;

}
//---------------------------------------------------------------------------
vtkMRMLVolumeNode* qSlicerSubjectHierarchyRegistrationQualityPlugin::loadVolumeFromItemId(vtkIdType itemID){
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
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
         d->DIRQALogic->NODEITEMID.c_str());
   
   if ( ! nodeStringItemID.empty() ){
      vtkIdType nodeItemID = atoll(nodeStringItemID.c_str());
      vtkSmartPointer<vtkMRMLNode> node;
      node = shNode->GetItemDataNode(nodeItemID);
      if ( node ){
         return vtkMRMLVolumeNode::SafeDownCast(node);
      }
      else{
         shNode->RemoveItemAttribute(itemID,
             d->DIRQALogic->NODEITEMID.c_str());
      }
   }
  
   vtkNew<vtkSlicerVolumesLogic> logic;
   vtkSmartPointer<vtkMRMLScene> scene; 
   scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  
   logic->SetMRMLScene(scene);
  
   std::string filePath = shNode->GetItemAttribute(itemID, 
      d->DIRQALogic->FILEPATH.c_str());
   
   
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
      d->DIRQALogic->TRIPVF.c_str());
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
      d->DIRQALogic->NODEITEMID.c_str(),newVolumeStringID.str().c_str());
   
   //Careful!!
//    volume->SetOrigin(0,0,0);
   
   return volume;
}
//---------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* qSlicerSubjectHierarchyRegistrationQualityPlugin::loadMarkups(vtkIdType itemID){
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
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
         d->DIRQALogic->NODEITEMID.c_str());
   
   if ( ! nodeStringItemID.empty() ){
      vtkIdType nodeItemID = atoll(nodeStringItemID.c_str());
      vtkSmartPointer<vtkMRMLNode> node;
      node = shNode->GetItemDataNode(nodeItemID);
      if ( node ){
         return vtkMRMLMarkupsFiducialNode::SafeDownCast(node);
      }
      else{
         shNode->RemoveItemAttribute(itemID,
             d->DIRQALogic->NODEITEMID.c_str());
      }
   }
  
   vtkNew<vtkSlicerMarkupsLogic> logic;
   vtkSmartPointer<vtkMRMLScene> scene; 
   scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  
   logic->SetMRMLScene(scene);
  
   std::string filePath = shNode->GetItemAttribute(itemID, 
      d->DIRQALogic->FILEPATH.c_str());
   
   
   if (filePath.c_str() == NULL){
      qCritical() << Q_FUNC_INFO << ": Can't find ROI filepath";
      return NULL;
   }
   
   std::string fiducialName = vtksys::SystemTools::GetFilenameName(filePath.c_str());
   std::size_t lastindex = fiducialName.find_last_of("."); 
   std::string rawname = fiducialName.substr(0, lastindex);
   fiducialName = scene->GetUniqueNameByString(rawname.c_str());
   
   char *FixedFiducialNodeID = logic->LoadMarkupsFiducials(filePath.c_str(), fiducialName.c_str());
   
   if (FixedFiducialNodeID == NULL) {
      qCritical() << Q_FUNC_INFO << ": Can't load Roi from" << filePath.c_str();
      return NULL;
   }
   
   vtkSmartPointer<vtkMRMLMarkupsFiducialNode> fiducialNode;
   fiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( scene->GetNodeByID(FixedFiducialNodeID) );
   vtkIdType newFiducialID = shNode->CreateItem(itemID,fiducialNode);
   std::stringstream newFiducialStringID;
   newFiducialStringID <<  newFiducialID;
   shNode->SetItemAttribute(newFiducialID,"FilePath",filePath);
   shNode->SetItemAttribute(itemID,
      d->DIRQALogic->NODEITEMID.c_str(),newFiducialStringID.str().c_str());

//    shNode->SetItemParent(currentRoiItemID,roiItemID);
   return fiducialNode;
}
//---------------------------------------------------------------------------
vtkMRMLRegistrationQualityNode* qSlicerSubjectHierarchyRegistrationQualityPlugin::loadRegQANode(vtkIdType itemID){
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
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
   
   vtkSmartPointer<vtkMRMLRegistrationQualityNode> regQANode;
   vtkSmartPointer<vtkMRMLScene> scene; 
   scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();

   std::string regQANodeIDString = shNode->GetAttributeFromItemAncestor(itemID, 
         d->DIRQALogic->REGQANODEID.c_str() );
   if ( regQANodeIDString.empty() ){
      QString regQAID = this->m_RegQAParametersID;
      if ( regQAID.isEmpty() ){
         // Let's check first, if parameters exist in scene
         vtkSmartPointer<vtkCollection> collection = scene->GetNodesByClass("vtkMRMLRegistrationQualityNode");
         if (collection->GetNumberOfItems() > 0){
            qCritical() << Q_FUNC_INFO << ": Found collection regQA: ";
            for(int item = 0; item < collection->GetNumberOfItems(); item++){
               vtkSmartPointer<vtkObject> object = collection->GetItemAsObject(item);
               regQANode = vtkMRMLRegistrationQualityNode::SafeDownCast(object);
               // Take only forward paramees
               if (! regQANode->GetBackwardRegistration() ){
                  regQAID = regQANode->GetID();
                  break;
               }
            }
         }
         else{
            //Create new reg quality
            qCritical() << Q_FUNC_INFO << ": Creating new regQA: ";
            regQANode = vtkMRMLRegistrationQualityNode::New();
            scene->AddNode( regQANode );
            regQAID = regQANode->GetID();
         }
      }
      else{
         qCritical() << Q_FUNC_INFO << ": Found regQA: " << regQAID.toUtf8().constData();
      }
      regQANodeIDString = regQAID.toUtf8().constData();
   }

   regQANode = vtkMRMLRegistrationQualityNode::SafeDownCast(
         scene->GetNodeByID( regQANodeIDString ));

   
   if (!regQANode){
      qCritical() << Q_FUNC_INFO << "Can't get " << regQANodeIDString.c_str();
      return NULL;
   }
   
   if ( regQANode->GetBackwardRegQAParameters() == NULL ){
      d->DIRQALogic->CreateBackwardParameters(regQANode);
   }
   
   this->m_RegQAParametersID = QString(regQANodeIDString.c_str());
   
   return regQANode;
   
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::InputSelected(std::string name, bool backward, vtkIdType itemID, vtkMRMLNode* associatedNode)
{
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
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
   
   vtkMRMLRegistrationQualityNode* regQANode = this->loadRegQANode(itemID);
   if ( ! regQANode ){
      qCritical() << Q_FUNC_INFO << ": Can't get regQANode.";
      return; // Only this plugin can handle this node
   }
   if ( backward ){
      regQANode = regQANode->GetBackwardRegQAParameters();
   }
   
   regQANode->DisableModifiedEventOn();
   if ( name.compare(d->DIRQALogic->IMAGE) == 0 ){
      if (!associatedNode->IsA("vtkMRMLScalarVolumeNode")){
         qCritical() << associatedNode->GetName() << ": Not a scalar volume.";
      }
      regQANode->SetAndObserveVolumeNodeID(associatedNode->GetID());
   }
   else if ( name.compare(d->DIRQALogic->WARPED_IMAGE) == 0 ){
      if ( !associatedNode->IsA("vtkMRMLScalarVolumeNode")){
         qCritical() << associatedNode->GetName() << ": Not a scalar volume.";
      }
      regQANode->SetAndObserveWarpedVolumeNodeID(associatedNode->GetID());
   }
   else if ( name.compare(d->DIRQALogic->VECTOR_FIELD) == 0 ){
      if ( !(associatedNode->IsA("vtkMRMLVectorVolumeNode")) ){
         qCritical() << associatedNode->GetName() << ": Not a vector volume.";
      }
      regQANode->SetAndObserveVectorVolumeNodeID(associatedNode->GetID());
   }
   else if ( name.compare(d->DIRQALogic->FIDUCIAL) == 0){
      if (!associatedNode->IsA("vtkMRMLMarkupsNode")){
         qCritical() << associatedNode->GetName() << ": Not a markups node.";
      }
      regQANode->SetAndObserveFiducialNodeID(associatedNode->GetID());
   }
   else if ( name.compare(d->DIRQALogic->ROI) == 0){
      if (!associatedNode->IsA("vtkMRMLAnnotationROINode")){
         qCritical() << associatedNode->GetName() << ": Not a annotation ROI node.";
      }
      regQANode->SetAndObserveROINodeID(associatedNode->GetID());
   }
   else{
      qCritical() << associatedNode->GetName() << ": with an unnkown reg type: " << name.c_str();
      return;
   }
     
   regQANode->DisableModifiedEventOff();
   
   /* Update backward node */
   if ( backward ){
      regQANode->ChangeFromBackwardToFoward();
   }
   else{
      regQANode->GetBackwardRegQAParameters()->ChangeFromBackwardToFoward();
   }
   
   //Try to update widget

   qSlicerAbstractCoreModule* module = qSlicerApplication::application()->moduleManager()->module("RegistrationQuality");
   qSlicerAbstractModule* moduleWithAction = qobject_cast<qSlicerAbstractModule*>(module);
   if (!moduleWithAction)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to get RegistrationQuality module";
      return;
   }
   qSlicerAbstractModuleRepresentation* widget = moduleWithAction->widgetRepresentation();
   if (!widget)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to get RegistrationQuality module widget";
      return;
   }
   if (! widget->setEditedNode(associatedNode) ){
      qCritical() << Q_FUNC_INFO << ": Failed to do something";
      return;
   }
   
   qCritical() << Q_FUNC_INFO << ":set" << associatedNode->GetName() << " to " << name.c_str() << "\n";
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::InputSelected(std::string name, bool backward)
{
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
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
void qSlicerSubjectHierarchyRegistrationQualityPlugin::fixedImageSelected()
{
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
   this->InputSelected(d->DIRQALogic->IMAGE,false);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::movingImageSelected()
{
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
   this->InputSelected(d->DIRQALogic->IMAGE, true);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::fwarpedImageSelected()
{
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
   this->InputSelected(d->DIRQALogic->WARPED_IMAGE, false);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::bwarpedImageSelected()
{
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
   this->InputSelected(d->DIRQALogic->WARPED_IMAGE, true);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::fixedVectorFieldSelected()
{
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
   this->InputSelected(d->DIRQALogic->VECTOR_FIELD, false);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::movingVectorFieldSelected()
{
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
   this->InputSelected(d->DIRQALogic->VECTOR_FIELD, true);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::fixedFiducialsSelected()
{
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
   this->InputSelected(d->DIRQALogic->FIDUCIAL, false);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::movingFiducialsSelected()
{
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
   this->InputSelected(d->DIRQALogic->FIDUCIAL, true);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::ROISelected()
{
   Q_D(qSlicerSubjectHierarchyRegistrationQualityPlugin);
   this->InputSelected(d->DIRQALogic->ROI, false);
}
void qSlicerSubjectHierarchyRegistrationQualityPlugin::loadContourSelected(bool backward){
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
   
   vtkMRMLRegistrationQualityNode* regQANode = this->loadRegQANode(currentItemID);
   if (backward){
      regQANode = regQANode->GetBackwardRegQAParameters();
   }
   
   regQANode->SetSegmentID(segmentStringID.c_str());
   regQANode->SetAndObserveSegmentationNode(vtkMRMLSegmentationNode::SafeDownCast(segmentationNode));
   
   //Try to update widget
    // Get Registration Quality module
   qSlicerAbstractCoreModule* module = qSlicerApplication::application()->moduleManager()->module("RegistrationQuality");
   qSlicerAbstractModule* moduleWithAction = qobject_cast<qSlicerAbstractModule*>(module);
   if (!moduleWithAction)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to get RegistrationQuality module";
      return;
   }
   qSlicerAbstractModuleRepresentation* widget = moduleWithAction->widgetRepresentation();
   if (!widget)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to get RegistrationQuality module widget";
      return;
   }
   if (! widget->setEditedNode(segmentationNode) ){
      return; // Only this plugin can handle this node
   }

}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::fixedContourSelected()
{
   this->loadContourSelected(false);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::movingContourSelected()
{
   this->loadContourSelected(true);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::setNodeInRegQA(vtkMRMLNode* node, QString role, QString context)
{

    // Get Registration Quality module
   qSlicerAbstractCoreModule* module = qSlicerApplication::application()->moduleManager()->module("RegistrationQuality");
   qSlicerAbstractModule* moduleWithAction = qobject_cast<qSlicerAbstractModule*>(module);
   if (!moduleWithAction)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to get RegistrationQuality module";
      return;
   }
   // Select dose volume node in Dose Volume Histogram module
   qSlicerAbstractModuleRepresentation* widget = moduleWithAction->widgetRepresentation();
   if (!widget)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to get RegistrationQuality module widget";
      return;
   }
   if (! widget->setEditedNode(node,role,context) ){
      qCritical() << Q_FUNC_INFO << ": Can't add " << node->GetName() << " to " << role;
      return; // Only this plugin can handle this node
   }
}