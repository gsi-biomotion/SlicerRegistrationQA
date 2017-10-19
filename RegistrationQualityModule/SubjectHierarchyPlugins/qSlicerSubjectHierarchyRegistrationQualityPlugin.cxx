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
 , m_SelectSegment(vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
 , d_ptr( new qSlicerSubjectHierarchyRegistrationQualityPluginPrivate(*this) )
{
  this->m_Name = QString("RegQuality");

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
  std::string regTypeIdentifier = shNode->GetItemAttribute(currentItemID,
                    d->DIRQALogic->REGISTRATION_TYPE); 
  if ( regTypeIdentifier.compare(d->DIRQALogic->FIDUCIAL) == 0){
     this->loadMarkups(currentItemID);
  }
  else{
     this->loadVolumeFromItemId(currentItemID);
  }
  
}
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
      return;
   }
   else if ( regTypeIdentifier.compare(d->DIRQALogic->VECTOR_FIELD) == 0){
      this->calculateJacobian(itemID,false); /* No need to remove nodes here, we do it in invconsist */
      this->calculateInverseConsistency(itemID,removeNodes);
      return;
   }
   else if ( regTypeIdentifier.compare(d->DIRQALogic->FIDUCIAL) == 0){
      this->calculateFiducialDistance(itemID,removeNodes);
      return;
   }
  
   /* Or check, if there are any children items and run DIRQA on them */
   vtkSmartPointer<vtkIdList> IdList = vtkSmartPointer<vtkIdList>::New();
   shNode->GetItemChildren(itemID,IdList);
   if ( IdList->GetNumberOfIds() > 0 ){
      for (int i=0; i < IdList->GetNumberOfIds(); i++){
         this->calcuateDIRQAForID(IdList->GetId(i), true); // Remove nodes is set on, so that multiple vectors can be loaded
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
      if ( regTypeIdentifier.compare(d->DIRQALogic->FIDUCIAL) == 0){
        this->loadMarkups(itemID);
      }
      else{
        this->loadVolumeFromItemId(itemID);
      }
//       std::stringstream stringItemID;
//       newFiducialStringID <<  stringItemID;
      d->DIRQALogic->UpdateNodeFromSHNode(itemID);
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
   
   vtkSmartPointer<vtkMRMLVectorVolumeNode> vectorNode;
   vectorNode = vtkMRMLVectorVolumeNode::SafeDownCast(
      this->loadVolumeFromItemId(itemID));
   
   if ( vectorNode == NULL ){
      qCritical() << Q_FUNC_INFO << ": Failed to load vector node";
      return;
   }
   
   vtkSmartPointer<vtkMRMLAnnotationROINode> ROINode; 
   ROINode = this->loadROI(itemID);
   
//    vtkNew<vtkSlicerRegistrationQualityLogic> DIRQALogic;
//    vtkSmartPointer<vtkMRMLScene> scene; = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
//    DIRQALogic->SetMRMLScene(scene);
   
   vtkSmartPointer<vtkMRMLScalarVolumeNode> jacobianNode; 
   jacobianNode = d->DIRQALogic->Jacobian(vectorNode, ROINode);
   if (jacobianNode == NULL ){
      qCritical() << Q_FUNC_INFO << ": Can't calculate Jacobian";
      return;
   }
   
   jacobianFolder = shNode->CreateFolderItem(itemID,
                          d->DIRQALogic->JACOBIAN.c_str());
   
   double statisticValues[4];
   d->DIRQALogic->CalculateStatistics(jacobianNode, statisticValues);
   this->writeInTable(itemID,
         d->DIRQALogic->JACOBIANTABLE.c_str(), statisticValues);
   vtkIdType jacobianID = shNode->CreateItem(jacobianFolder,jacobianNode);
   
   if ( removeNodes){
//       shNode->RemoveItem(jacobianID);
      this->removeNode(vtkMRMLNode::SafeDownCast(vectorNode));
   }
   else {
      shNode->SetItemAttribute(jacobianID, d->DIRQALogic->REGISTRATION_TYPE.c_str(), 
                                 d->DIRQALogic->JACOBIAN.c_str());
      std::stringstream jacobianStringID;
      jacobianStringID << jacobianID;
      shNode->SetItemAttribute(jacobianFolder, d->DIRQALogic->JACOBIANITEMID.c_str(), 
         jacobianStringID.str().c_str());
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
   
   /* Check if previously calculated */
   vtkIdType invConsistFolder = shNode->GetItemChildWithName(itemID,
      d->DIRQALogic->INVERSECONSISTENCY.c_str());
   if ( invConsistFolder ){
      return;
   }
   
   vtkIdType inverseVectorID = this->findInverseVectorID(itemID);
   
   if (inverseVectorID == 0){
      qCritical() << Q_FUNC_INFO << ": Can't find inverse vector.";
      return;
   }
   
   vtkSmartPointer<vtkMRMLVectorVolumeNode> vectorNode;
   vectorNode = vtkMRMLVectorVolumeNode::SafeDownCast(
      this->loadVolumeFromItemId(itemID));
   vtkSmartPointer<vtkMRMLVectorVolumeNode> inverseVectorNode; 
   inverseVectorNode= vtkMRMLVectorVolumeNode::SafeDownCast(
      this->loadVolumeFromItemId(inverseVectorID));
   
   vtkSmartPointer<vtkMRMLAnnotationROINode> ROINode;
   ROINode = this->loadROI(itemID);
   if ( vectorNode == NULL || inverseVectorNode == NULL ){
      qCritical() << Q_FUNC_INFO << ": Failed to load vector nodes";
      return;
   }
//    
   vtkSmartPointer<vtkMRMLScalarVolumeNode> inverseConsistNode;
   inverseConsistNode = d->DIRQALogic->InverseConsist(vectorNode,inverseVectorNode, ROINode);
   if (inverseConsistNode == NULL ){
      qCritical() << Q_FUNC_INFO << ": Can't calculate Inverse Consistency";
      return;
   }
   
   invConsistFolder = shNode->CreateFolderItem(itemID,
                   d->DIRQALogic->INVERSECONSISTENCY.c_str());
   

   double statisticValues[4];
   d->DIRQALogic->CalculateStatistics(inverseConsistNode, statisticValues);
   this->writeInTable(itemID,
         d->DIRQALogic->INVCONSISTTABLE.c_str(), statisticValues);
//    this->calculateStatistics(inverseConsistNode, invConsistFolder);
   vtkIdType inverseConsistID = shNode->CreateItem(invConsistFolder,inverseConsistNode);
   
   if (removeNodes){
//       shNode->RemoveItem(inverseConsistID);
      
      this->removeNode(vtkMRMLNode::SafeDownCast(vectorNode));
      this->removeNode(vtkMRMLNode::SafeDownCast(inverseVectorNode));

   }
   else{
      
      shNode->SetItemAttribute(inverseConsistID, d->DIRQALogic->REGISTRATION_TYPE.c_str(), 
                                 d->DIRQALogic->INVERSECONSISTENCY.c_str());
      std::stringstream inverseConsistStringID;
      inverseConsistStringID <<  inverseConsistID;
      shNode->SetItemAttribute(invConsistFolder, d->DIRQALogic->INVERSECONSISTENCYITEMID.c_str(), 
         inverseConsistStringID.str().c_str());
   }
   
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
   
   vtkIdType inverseVectorItemID;
   
   for(int i=0;i<IdList->GetNumberOfIds();i++){
      inverseVectorItemID = IdList->GetId(i);
      std::string fixedImage2ID  = shNode->GetItemAttribute(
         inverseVectorItemID,d->DIRQALogic->FIXEDIMAGEID.c_str());
      std::string movingImage2ID  = shNode->GetItemAttribute(
         inverseVectorItemID,d->DIRQALogic->MOVINGIMAGEID.c_str());
      if (fixedImage1ID.compare(movingImage2ID) == 0 && movingImage1ID.compare(fixedImage2ID) == 0){
         return inverseVectorItemID;
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
   
//    vtkMRMLVolumeNode* fixPhaseVolumeNode = this->loadVolumeFromItemId(fixedImageID);
//    vtkMRMLVolumeNode* movPhaseVolumeNode = this->loadVolumeFromItemId(movingImageID);
   
   vtkSmartPointer<vtkMRMLScalarVolumeNode> fixPhaseNode; 
   fixPhaseNode = vtkMRMLScalarVolumeNode::SafeDownCast(
      this->loadVolumeFromItemId(fixedImageID));
   vtkSmartPointer<vtkMRMLScalarVolumeNode> movPhaseNode; 
   movPhaseNode = vtkMRMLScalarVolumeNode::SafeDownCast(
      this->loadVolumeFromItemId(itemID));
   
   vtkSmartPointer<vtkMRMLAnnotationROINode> ROINode; 
   ROINode = this->loadROI(itemID);

   if ( fixPhaseNode == NULL or movPhaseNode == NULL){
      qCritical() << Q_FUNC_INFO << ": Failed to load image nodes";
      return;
   }
   
//    vtkNew<vtkSlicerRegistrationQualityLogic> DIRQALogic;
//    vtkSmartPointer<vtkMRMLScene> scene; = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
//    DIRQALogic->SetMRMLScene(scene);
   
   vtkSmartPointer<vtkMRMLScalarVolumeNode> AbsDiffNode;
   AbsDiffNode = d->DIRQALogic->AbsoluteDifference(fixPhaseNode, movPhaseNode, ROINode);
   if (AbsDiffNode == NULL ){
      qCritical() << Q_FUNC_INFO << ": Can't calculate Absolute Difference";
      return;
   }
   
   absDiffFolder = shNode->CreateFolderItem(itemID,
         d->DIRQALogic->ABSOLUTEDIFFERENCE.c_str());
   double statisticValues[4];
   d->DIRQALogic->CalculateStatistics(AbsDiffNode, statisticValues);
   this->writeInTable(itemID,
         d->DIRQALogic->ABSDIFFTABLE.c_str(), statisticValues);           
   vtkIdType absDiffID = shNode->CreateItem(absDiffFolder,AbsDiffNode);

   if (removeNodes){
      this->removeNode(vtkMRMLNode::SafeDownCast(movPhaseNode));
      this->removeNode(vtkMRMLNode::SafeDownCast(fixPhaseNode));
//       this->removeNode(vtkMRMLNode::SafeDownCast(AbsDiffNode));
//       AbsDiffNode->Delete();
//       shNode->RemoveItem(absDiffID);
//       AbsDiffNode = NULL;
   }
   else {
      shNode->SetItemAttribute(absDiffID, d->DIRQALogic->REGISTRATION_TYPE.c_str(), 
                  d->DIRQALogic->ABSOLUTEDIFFERENCE.c_str());
      std::stringstream absDiffStringID;
      absDiffStringID <<  absDiffID;
      shNode->SetItemAttribute(absDiffFolder, d->DIRQALogic->ABSDIFFNODEITEMID.c_str(), 
         absDiffStringID.str().c_str());
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
      meanName->SetName("Mean");
      
      vtkAbstractArray* stdName = tableNode->AddColumn();
      stdName->SetName("STD");
      
      vtkAbstractArray* maxName = tableNode->AddColumn();
      maxName->SetName("Max");
      
      vtkAbstractArray* minName = tableNode->AddColumn();
      minName->SetName("Min");
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
void qSlicerSubjectHierarchyRegistrationQualityPlugin::ScalarImageSelected(const char* name)
{
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
   
   /* First check, if it is a scalar volume */
   vtkMRMLNode* associatedNode = shNode->GetItemDataNode(currentItemID);
//   const char* registrationQuality = associatedNode->GetAttribute("RegQuality");
  
   if ( ! (associatedNode && associatedNode->IsA("vtkMRMLScalarVolumeNode")) )
   {
      qCritical() << Q_FUNC_INFO << ": Selected node not a scalar volume.";
      return; // Only this plugin can handle this node
   }
   
   this->setNodeInRegQA(associatedNode, name);
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::fixedImageSelected()
{
   this->ScalarImageSelected("fixedImage");
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::movingImageSelected()
{
   this->ScalarImageSelected("movingImage");
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::fwarpedImageSelected()
{
   this->ScalarImageSelected("forwardWarpedImage");
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::bwarpedImageSelected()
{
   this->ScalarImageSelected("backwardWarpedImage");
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::fixedVectorFieldSelected()
{
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
   
   /* First check, if it is a scalar volume */
   vtkMRMLNode* associatedNode = shNode->GetItemDataNode(currentItemID);
//   const char* registrationQuality = associatedNode->GetAttribute("RegQuality");
  
   if ( ! (associatedNode && associatedNode->IsA("vtkMRMLVectorVolumeNode")) )
   {
      qCritical() << Q_FUNC_INFO << ": Selected node not a vector volume.";
      return; // Only this plugin can handle this node
   }

   this->setNodeInRegQA(associatedNode, "fixedVectorField");
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::movingVectorFieldSelected()
{
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
   
   /* First check, if it is a scalar volume */
   vtkMRMLNode* associatedNode = shNode->GetItemDataNode(currentItemID);
//   const char* registrationQuality = associatedNode->GetAttribute("RegQuality");
  
   if ( ! (associatedNode && associatedNode->IsA("vtkMRMLVectorVolumeNode")) )
   {
      qCritical() << Q_FUNC_INFO << ": Selected node not a vector volume.";
      return; // Only this plugin can handle this node
   }

   this->setNodeInRegQA(associatedNode, "movingVectorField");
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::fixedFiducialsSelected()
{
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
   
   /* First check, if it is a scalar volume */
   vtkMRMLNode* associatedNode = shNode->GetItemDataNode(currentItemID);
//   const char* registrationQuality = associatedNode->GetAttribute("RegQuality");
  
   if ( ! (associatedNode && associatedNode->IsA("vtkMRMLMarkupsNode")) )
   {
      qCritical() << Q_FUNC_INFO << ": Selected node not a markups volume.";
      return; // Only this plugin can handle this node
   }

   this->setNodeInRegQA(associatedNode, "fixedFiducials");
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::movingFiducialsSelected()
{
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
   
   /* First check, if it is a scalar volume */
   vtkMRMLNode* associatedNode = shNode->GetItemDataNode(currentItemID);
//   const char* registrationQuality = associatedNode->GetAttribute("RegQuality");
  
   if ( ! (associatedNode && associatedNode->IsA("vtkMRMLMarkupsNode")) )
   {
      qCritical() << Q_FUNC_INFO << ": Selected node not a markups volume.";
      return; // Only this plugin can handle this node
   }

   this->setNodeInRegQA(associatedNode, "movingFiducials");
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::ROISelected()
{
   vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
   if (!shNode)
   {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      return;
   }
   vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
   
   /* First check, if it is a scalar volume */
   vtkMRMLNode* associatedNode = shNode->GetItemDataNode(currentItemID);
//   const char* registrationQuality = associatedNode->GetAttribute("RegQuality");
  
   if ( ! (associatedNode && associatedNode->IsA("vtkMRMLAnnotationROINode")) )
   {
      qCritical() << Q_FUNC_INFO << ": Selected node not a AnnotationROI node.";
      return; // Only this plugin can handle this node
   }

   this->setNodeInRegQA(associatedNode, "ROI");
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::fixedContourSelected()
{
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
   
   this->setNodeInRegQA(segmentationNode, "fixedSegmentID", segmentStringID.c_str());
   this->setNodeInRegQA(segmentationNode, "fixedSegmentationNode");
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRegistrationQualityPlugin::movingContourSelected()
{
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
   
   this->setNodeInRegQA(segmentationNode, "movingSegmentID", segmentStringID.c_str());
   this->setNodeInRegQA(segmentationNode, "movingSegmentationNode");
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