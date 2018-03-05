#pragma once
#ifndef __vtkMRMLRegistrationQANode_h
#define __vtkMRMLRegistrationQANode_h

#include <vtkMRML.h>
#include <vtkMRMLNode.h>

#include "vtkSlicerRegistrationQAModuleLogicExport.h"
class vtkMRMLSegmentationNode;
class vtkMRMLTableNode;

/// \ingroup Slicer_QtModules_RegistrationQA
class VTK_SLICER_REGISTRATIONQA_MODULE_LOGIC_EXPORT vtkMRMLRegistrationQANode :
	public vtkMRMLNode {

public:
	static vtkMRMLRegistrationQANode *New();
	vtkTypeMacro(vtkMRMLRegistrationQANode, vtkMRMLNode);
	void PrintSelf(ostream& os, vtkIndent indent);

	virtual vtkMRMLNode* CreateNodeInstance();
	virtual void ReadXMLAttributes( const char** atts);
	virtual void WriteXML(ostream& of, int indent);
	virtual void Copy(vtkMRMLNode *node);
	virtual const char* GetNodeTagName() {return "RegistrationQAParameters";};
	virtual void UpdateReferenceID(const char *oldID, const char *newID);

	vtkGetStringMacro(VectorVolumeNodeID);
        vtkSetStringMacro(VectorVolumeNodeID);
	void SetAndObserveVectorVolumeNodeID(const char* id);
	
	vtkGetStringMacro(TransformNodeID);
        vtkSetStringMacro(TransformNodeID);
	void SetAndObserveTransformNodeID(const char* id);

	vtkGetStringMacro(VolumeNodeID);
        vtkSetStringMacro(VolumeNodeID);
	void SetAndObserveVolumeNodeID(const char* id);

	vtkGetStringMacro(WarpedVolumeNodeID);
        vtkSetStringMacro(WarpedVolumeNodeID);
	void SetAndObserveWarpedVolumeNodeID(const char* id);

	vtkGetStringMacro(OutputDirectory);
        vtkSetStringMacro(OutputDirectory);
	void SetAndObserveOutputDirectory(const char* id);
	
	vtkGetStringMacro(ROINodeID);
        vtkSetStringMacro(ROINodeID);
        void SetAndObserveROINodeID(const char* id);
	
	vtkGetStringMacro (FiducialNodeID);
        vtkSetStringMacro (FiducialNodeID);
	void SetAndObserveFiducialNodeID(const char* id);

        /// Get segmentation node
        vtkMRMLSegmentationNode* GetSegmentationNode();
        /// Set segmentation node
        void SetAndObserveSegmentationNode(vtkMRMLSegmentationNode* node);

        /// Get  segment ID
        vtkGetStringMacro(SegmentID);
        /// Set  segment ID
        vtkSetStringMacro(SegmentID);

        /// Contour data table node
        vtkMRMLTableNode* GetRegistrationQATableNode();
        void SetAndObserveRegistrationQATableNode(vtkMRMLTableNode* node);
	
	vtkGetMacro(NumberOfScreenshots, int);
	vtkSetMacro(NumberOfScreenshots, int);

	vtkGetStringMacro(OutputModelNodeID);
        vtkSetStringMacro(OutputModelNodeID);
	void SetAndObserveOutputModelNodeID(const char* id);

	// Checkerboard parameters
	vtkGetStringMacro(CheckerboardVolumeNodeID);
        vtkSetStringMacro(CheckerboardVolumeNodeID);
	void SetAndObserveCheckerboardVolumeNodeID(const char* id);

	vtkSetMacro(CheckerboardPattern, int);
	vtkGetMacro(CheckerboardPattern, int);


	vtkSetMacro(FlickerOpacity, int);
	vtkGetMacro(FlickerOpacity, int);

	// Absolute Difference parameters:
	vtkGetStringMacro(AbsoluteDiffVolumeNodeID);
        vtkSetStringMacro(AbsoluteDiffVolumeNodeID);
	void SetAndObserveAbsoluteDiffVolumeNodeID(const char* id);

	// Jacobian volume node:
	vtkGetStringMacro(JacobianVolumeNodeID);
        vtkSetStringMacro(JacobianVolumeNodeID);
	void SetAndObserveJacobianVolumeNodeID(const char* id);

	// Inverse Consistenciy parameters:
	vtkGetStringMacro(InverseConsistVolumeNodeID);
        vtkSetStringMacro(InverseConsistVolumeNodeID);
	void SetAndObserveInverseConsistVolumeNodeID(const char* id);
        
        /// Forward flag
        vtkGetMacro(BackwardRegistration, bool);
        vtkSetMacro(BackwardRegistration, bool);
        vtkBooleanMacro(BackwardRegistration, bool);
        
        /// Backward Registration table node
        vtkMRMLRegistrationQANode* GetBackwardRegistrationQAParameters();
        void SetAndObserveBackwardRegistrationQAParameters(vtkMRMLRegistrationQANode* node);
        
        /// Exchange forward and backward parameters
        bool ChangeFromBackwardToFoward();
        
        // Movie parameters
        vtkGetMacro(MovieBoxRedState, int);
        vtkSetMacro(MovieBoxRedState, int);
        vtkGetMacro(MovieBoxYellowState, int);
        vtkSetMacro(MovieBoxYellowState, int);
        vtkGetMacro(MovieBoxGreenState, int);
        vtkSetMacro(MovieBoxGreenState, int);

        // No set for this one
        vtkGetMacro(MovieRun, bool);
        vtkSetMacro(MovieRun, bool);
        vtkBooleanMacro(MovieRun, bool);
        
        /// Get Volume Color node ID
        vtkGetStringMacro(VolumeColorNodeID);
        /// Se Volume Color node ID
        vtkSetStringMacro(VolumeColorNodeID);
        
        /// Get Warped Color node ID
        vtkGetStringMacro(WarpedColorNodeID);
        /// Se Warped Color node ID
        vtkSetStringMacro(WarpedColorNodeID);


        
protected:
	vtkMRMLRegistrationQANode();
	~vtkMRMLRegistrationQANode();

	vtkMRMLRegistrationQANode(const vtkMRMLRegistrationQANode&);
	void operator=(const vtkMRMLRegistrationQANode&);

	char* VectorVolumeNodeID;
	char* TransformNodeID;
	char* VolumeNodeID;
	char* WarpedVolumeNodeID;
	char* OutputDirectory;
	char* ROINodeID;
	char* FiducialNodeID;
              
        char* SegmentID;

	char* OutputModelNodeID;
	int NumberOfScreenshots;
	
	bool MovieRun;
        int MovieBoxRedState;
	int MovieBoxYellowState;
	int MovieBoxGreenState;

	char* CheckerboardVolumeNodeID;
	int CheckerboardPattern;

	int FlickerOpacity;
	int FlickerDelay;

	char* AbsoluteDiffVolumeNodeID;
	char* JacobianVolumeNodeID;
	char* InverseConsistVolumeNodeID;
        bool BackwardRegistration;
        
        char* VolumeColorNodeID;
        char* WarpedColorNodeID;
        
};

#endif
