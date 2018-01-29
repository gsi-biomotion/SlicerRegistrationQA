#pragma once
#ifndef __vtkMRMLRegistrationQualityNode_h
#define __vtkMRMLRegistrationQualityNode_h

#include <vtkMRML.h>
#include <vtkMRMLNode.h>

#include "vtkSlicerRegistrationQualityModuleLogicExport.h"
class vtkMRMLSegmentationNode;
class vtkMRMLTableNode;

/// \ingroup Slicer_QtModules_RegistrationQuality
class VTK_SLICER_REGISTRATIONQUALITY_MODULE_LOGIC_EXPORT vtkMRMLRegistrationQualityNode :
	public vtkMRMLNode {

public:
	static vtkMRMLRegistrationQualityNode *New();
	vtkTypeMacro(vtkMRMLRegistrationQualityNode, vtkMRMLNode);
	void PrintSelf(ostream& os, vtkIndent indent);

	virtual vtkMRMLNode* CreateNodeInstance();
	virtual void ReadXMLAttributes( const char** atts);
	virtual void WriteXML(ostream& of, int indent);
	virtual void Copy(vtkMRMLNode *node);
	virtual const char* GetNodeTagName() {return "RegistrationQualityParameters";};
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
        vtkMRMLTableNode* GetRegQATableNode();
        void SetAndObserveRegQATableNode(vtkMRMLTableNode* node);
	
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
        vtkMRMLRegistrationQualityNode* GetBackwardRegQAParameters();
        void SetAndObserveBackwardRegQAParameters(vtkMRMLRegistrationQualityNode* node);
        
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
        /// Forward flag
        vtkGetMacro(MovieRun, bool);
        vtkSetMacro(MovieRun, bool);
        vtkBooleanMacro(MovieRun, bool);
//         vtkGetMacro(MovieRun, int);

//         virtual void SetMovieBoxRedState(int state) {
//                 vtkDebugMacro(<< GetClassName() << " (" << this
//                                 << "): setting MovieBoxRedState to " << state);
//                 if(MovieBoxRedState != state) {
//                         MovieBoxRedState = state;
//                         MovieRun ^= 1<<0;
//                         Modified();
//                 }
//         }
// 
//         virtual void SetMovieBoxYellowState(int state) {
//                 vtkDebugMacro(<< GetClassName() << " (" << this
//                                 << "): setting MovieBoxYellowState to " << state);
//                 if(MovieBoxYellowState != state) {
//                         MovieBoxYellowState = state;
//                         MovieRun ^= 1<<1;
//                         Modified();
//                 }
//         }
// 
//         virtual void SetMovieBoxGreenState(int state) {
//                 vtkDebugMacro(<< GetClassName() << " (" << this
//                                 << "): setting MovieBoxGreenState to " << state);
//                 if(MovieBoxGreenState != state) {
//                         MovieBoxGreenState = state;
//                         MovieRun ^= 1<<2;
//                         Modified();
//                 }
//         }

        
protected:
	vtkMRMLRegistrationQualityNode();
	~vtkMRMLRegistrationQualityNode();

	vtkMRMLRegistrationQualityNode(const vtkMRMLRegistrationQualityNode&);
	void operator=(const vtkMRMLRegistrationQualityNode&);

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
        
};

#endif
