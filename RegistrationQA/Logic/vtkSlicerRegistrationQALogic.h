#ifndef __vtkSlicerRegistrationQALogic_h
#define __vtkSlicerRegistrationQALogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
// #include <vtkMRMLSliceCompositeNode.h>
// #include <vtkMRMLScalarVolumeNode.h>
// #include <vtkMRMLVectorVolumeNode.h>
// #include <vtkMRMLGridTransformNode.h>
// #include <vtkMRMLAnnotationROINode.h>
// #include <vtkMRMLMarkupsFiducialNode.h>
// STD includes
#include <cstdlib>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkMatrix4x4.h>
#include <vtkSegment.h>


#include "vtkSlicerRegistrationQAModuleLogicExport.h"

class vtkMRMLRegistrationQANode;
class vtkMRMLVectorVolumeNode;
class vtkMRMLVolumeNode;
class vtkMRMLTableNode;
class vtkMRMLTransformNode;
class vtkMRMLGridTransformNode;
class vtkMRMLScalarVolumeNode;
class vtkMRMLAnnotationROINode;
class vtkMRMLMarkupsFiducialNode;
class vtkMRMLSliceCompositeNode;
class vtkMRMLSegmentationNode;

/// \ingroup Slicer_QtModules_RegistrationQA
class VTK_SLICER_REGISTRATIONQA_MODULE_LOGIC_EXPORT vtkSlicerRegistrationQALogic :
	
	public vtkSlicerModuleLogic {
public:
   public:
   /** Constants used **/
   static const char* SLICERRegistrationQA_EXTENSION_NAME;
   static const std::string INVERSE;
   static const std::string WARPED_IMAGE;
   static const std::string VECTOR_FIELD;
   static const std::string TRANSFORM;
   static const std::string ABSOLUTEDIFFERENCE;
   static const std::string JACOBIAN;
   static const std::string INVERSECONSISTENCY;
   static const std::string REFIMAGEID;
   static const std::string REGISTRATION_TYPE;
   static const std::string PHASENUMBER;
   static const std::string PHASETYPE;
   static const std::string CT;
   static const std::string SEGMENTATION;
   static const std::string SEGMENTATID;
   static const std::string ROI;
   static const std::string ROIITEMID;
   static const std::string ITEMID;
   static const std::string DIR;
   static const std::string FILEPATH;
   static const std::string FIXEDIMAGEID;
   static const std::string MOVINGIMAGEID;
   static const std::string VECTORITEMID;
   static const std::string TRANSFORMITEMID;
   static const std::string NODEITEMID;
   static const std::string ABSDIFFNODEITEMID;
   static const std::string JACOBIANITEMID;
   static const std::string INVERSECONSISTENCYITEMID;
   static const std::string TABLE;
   static const std::string ABSDIFFTABLE;
   static const std::string JACOBIANTABLE;
   static const std::string INVCONSISTTABLE;
   static const std::string FIDUCIAL;
   static const std::string FIDUCIALTABLE;
   static const std::string REFERENCENUMBER;
   static const std::string TRIPVF;
   static const std::string BACKWARD;
   static const std::string IMAGE;
   static const std::string RegistrationQANODEID;
public:
	static vtkSlicerRegistrationQALogic *New();
	vtkTypeMacro(vtkSlicerRegistrationQALogic, vtkSlicerModuleLogic);
	void PrintSelf(ostream& os, vtkIndent indent);

	void SaveScreenshot(const char*);
	void SaveOutputFile();
        void CalculateRegQAFrom(int number);
	
        void CalculateContourStatistic();
	
	void FalseColor(bool falseColor, bool movingImage, bool matchLevels);
	void Flicker(int opacity);
	
	void Movie();
	void Checkerboard();
        
        /** Calculates distance between reference and moving fiducials before and after transofrm.
         *  Transform can be either be a vtkMRMLTransformNode or vtkMRMLVolumeNode. The difference
         *  in distance is writen is statisticValues, with the 2N entries, where N = number of markups.
         *  Even entries are before and odd after registration, respectively.
         */
        bool CalculateFiducialsDistance(vtkMRMLMarkupsFiducialNode* referenceFiducals, vtkMRMLMarkupsFiducialNode* movingFiducials,vtkMRMLTransformNode *transofrm, double *statisticValues);
        bool CalculateFiducialsDistance(vtkMRMLMarkupsFiducialNode* referenceFiducals, vtkMRMLMarkupsFiducialNode* movingFiducials,vtkMRMLVectorVolumeNode *vectorNode, double *statisticValues);

        /** Calculates Absolute Difference between reference and warped image from registration
         *  quality node. Calculates statistic as well, if given 
         */
        bool AbsoluteDifference(vtkMRMLRegistrationQANode* regQANode, double statisticValues[4]);
        
        /**Calculates Jacobian from registration quality parameter node.
         * Calculates statistic as well, if given 
         */
        bool Jacobian(vtkMRMLRegistrationQANode* regQANode, double statisticValues[4]);
        
        /** Calculates inverse consistency from registration quality parameter node.
         * Calculates statistic as well, if given 
         */
        bool InverseConsist(vtkMRMLRegistrationQANode* regQANode, double statisticValues[4]);
        
        
	/** Set the background to fixed image, forground to warped image with opacity 0.5
         */
        void SetDefaultDisplay();
        
	void CalculateStatistics(vtkMRMLScalarVolumeNode*, double statisticValues[4]);
        vtkMRMLVolumeNode* LoadVolumeFromFile( std::string filePath, std::string volumeName);

	/** Change transform into vector volume
         */
        vtkMRMLGridTransformNode* CreateTransformFromVector(vtkMRMLVectorVolumeNode* vectorVolume);
        
        /** Change vector volume into transform
         */
	vtkMRMLVectorVolumeNode* CreateVectorFromTransform(vtkMRMLTransformNode* transform);
        
	/** Create new warped volume from moving volume by applying transformation on it
         */
	vtkMRMLScalarVolumeNode* GetWarpedFromMoving(vtkMRMLScalarVolumeNode *movingVolume, vtkMRMLTransformNode *transform);
        
        /** Helper function to load from registration parameters node
         */
        void CreateROI();
        
        /** Create ROI around segment. If there's only one segment, ROI will be created only around it.
         */
        vtkMRMLAnnotationROINode* CreateROIAroundSegments(vtkMRMLSegmentationNode* segmentation1Node,const char* segment1StringID,
                                                           vtkMRMLSegmentationNode* segmentation2Node,const char* segment2StringID);
        
        /** Update table with all the node names
         */
        void UpdateRegistrationQATable();
        
        /** Change in which direction (forward or backward) should
         *  the QA be performed.
         */
        void SwitchRegistrationDirection();
        
        /** Update registration quality node based on item attributes in
         *  given subject hierarchy node
         */
        void UpdateNodeFromSHNode(vtkIdType itemID);


public:
	void SetAndObserveRegistrationQANode(vtkMRMLRegistrationQANode *node);
	vtkGetObjectMacro(RegistrationQANode, vtkMRMLRegistrationQANode);

protected:
	vtkSlicerRegistrationQALogic();
	~vtkSlicerRegistrationQALogic();

	virtual void RegisterNodes();
	

	virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);

	virtual void UpdateFromMRMLScene();
	virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
	virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);
	virtual void OnMRMLSceneEndImport();
	virtual void OnMRMLSceneEndClose();
        
	void InvertXandY(vtkImageData* imageData);
        bool GetRadiusAndCenter(vtkMRMLSegmentationNode* segmentationNode, vtkSegment* segment, double radius[3], double center[3]);
        void UpdateTableWithStatisticalValues(double statisticValues[4], int row);
        void UpdateTableWithFiducialValues(vtkMRMLMarkupsFiducialNode* fiducial, double statisticValues[4]);
        void SetForegroundImage(vtkMRMLScalarVolumeNode*,vtkMRMLScalarVolumeNode*,double opacity);
        bool getSliceCompositeNodeRASBounds(vtkMRMLSliceCompositeNode *scn, double* minmax);
        /** Takes two scalar volumes and calls a CLI-module to calculate mean square error 
         *  between them either on the whole volume or on the ROI, if specified.
         */
        vtkMRMLScalarVolumeNode* CalculateAbsoluteDifference(vtkMRMLScalarVolumeNode*,vtkMRMLScalarVolumeNode*, vtkMRMLAnnotationROINode* inputROI = NULL);
        
        /** Takes vector field and calls a CLI-module to calculate jacobian determinant 
         *  either on the whole vector volume or on the ROI, if specified.
         */
        vtkMRMLScalarVolumeNode* CalculateJacobian(vtkMRMLVectorVolumeNode *vectorVolume,
                                          vtkMRMLAnnotationROINode *inputROI = NULL);
        /** Takes two vector fields and calls a CLI-module to calculate inverse
         *  consistency between them. The calculation is done either on the 
         *  whole vector volume or on the ROI, if specified.
         *  If tableNode is given, basic statistic values are writen into it.
         */
        vtkMRMLScalarVolumeNode* CalculateInverseConsist(vtkMRMLVectorVolumeNode *vectorVolume1,
                                                vtkMRMLVectorVolumeNode *vectorVolume2,
                                                vtkMRMLAnnotationROINode *inputROI=NULL);

protected:
	/// Parameter set MRML node
	vtkMRMLRegistrationQANode* RegistrationQANode;

private:
	vtkSlicerRegistrationQALogic(const vtkSlicerRegistrationQALogic&);// Not implemented
	void operator=(const vtkSlicerRegistrationQALogic&);// Not implemented
};

#endif
