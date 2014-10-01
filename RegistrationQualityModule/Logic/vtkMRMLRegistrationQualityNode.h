#pragma once
#ifndef __vtkMRMLRegistrationQualityNode_h
#define __vtkMRMLRegistrationQualityNode_h

#include <vtkMRML.h>
#include <vtkMRMLNode.h>

#include "vtkSlicerRegistrationQualityModuleLogicExport.h"

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

	vtkSetStringMacro(VectorVolumeNodeID);
	vtkGetStringMacro(VectorVolumeNodeID);
	void SetAndObserveVectorVolumeNodeID(const char* id);

	vtkSetStringMacro(InvVectorVolumeNodeID);
	vtkGetStringMacro(InvVectorVolumeNodeID);
	void SetAndObserveInvVectorVolumeNodeID(const char* id);

	vtkSetStringMacro(ReferenceVolumeNodeID);
	vtkGetStringMacro(ReferenceVolumeNodeID);
	void SetAndObserveReferenceVolumeNodeID(const char* id);

	vtkSetStringMacro(WarpedVolumeNodeID);
	vtkGetStringMacro(WarpedVolumeNodeID);
	void SetAndObserveWarpedVolumeNodeID(const char* id);

	vtkSetStringMacro(OutputDirectory);
	vtkGetStringMacro(OutputDirectory);
	void SetAndObserveOutputDirectory(const char* id);
	
	vtkSetStringMacro(ROINodeID);
	vtkGetStringMacro (ROINodeID);
	void SetAndObserveROINodeID(const char* id);

	vtkGetMacro(NumberOfScreenshots, int);
	vtkSetMacro(NumberOfScreenshots, int);
	// Movie parameters
	vtkGetMacro(MovieBoxRedState, int);
	vtkGetMacro(MovieBoxYellowState, int);
	vtkGetMacro(MovieBoxGreenState, int);

	// No set for this one
	vtkGetMacro(MovieRun, int);

	virtual void SetMovieBoxRedState(int state) {
		vtkDebugMacro(<< GetClassName() << " (" << this
				<< "): setting MovieBoxRedState to " << state);
		if(MovieBoxRedState != state) {
			MovieBoxRedState = state;
			MovieRun ^= 1<<0;
			Modified();
		}
	}

	virtual void SetMovieBoxYellowState(int state) {
		vtkDebugMacro(<< GetClassName() << " (" << this
				<< "): setting MovieBoxYellowState to " << state);
		if(MovieBoxYellowState != state) {
			MovieBoxYellowState = state;
			MovieRun ^= 1<<1;
			Modified();
		}
	}

	virtual void SetMovieBoxGreenState(int state) {
		vtkDebugMacro(<< GetClassName() << " (" << this
				<< "): setting MovieBoxGreenState to " << state);
		if(MovieBoxGreenState != state) {
			MovieBoxGreenState = state;
			MovieRun ^= 1<<2;
			Modified();
		}
	}

	// Checkerboard parameters
	vtkSetStringMacro(CheckerboardVolumeNodeID);
	vtkGetStringMacro(CheckerboardVolumeNodeID);
	void SetAndObserveCheckerboardVolumeNodeID(const char* id);

	vtkSetMacro(CheckerboardPattern, int);
	vtkGetMacro(CheckerboardPattern, int);


	vtkSetMacro(FlickerOpacity, int);
	vtkGetMacro(FlickerOpacity, int);

	// Squared Difference parameters:
	vtkSetStringMacro(AbsoluteDiffVolumeNodeID);
	vtkGetStringMacro(AbsoluteDiffVolumeNodeID);
	void SetAndObserveAbsoluteDiffVolumeNodeID(const char* id);

	vtkSetVector4Macro(AbsoluteDiffStatistics, double);
	vtkGetVector4Macro(AbsoluteDiffStatistics, double);


	// Jacobian parameters:
	vtkSetStringMacro(JacobianVolumeNodeID);
	vtkGetStringMacro(JacobianVolumeNodeID);
	void SetAndObserveJacobianVolumeNodeID(const char* id);


	vtkSetVector4Macro(JacobianStatistics, double);
	vtkGetVector4Macro(JacobianStatistics, double);

	// Inverse Consistenciy parameters:
	vtkSetStringMacro(InverseConsistVolumeNodeID);
	vtkGetStringMacro(InverseConsistVolumeNodeID);
	void SetAndObserveInverseConsistVolumeNodeID(const char* id);

	vtkSetVector4Macro(InverseConsistStatistics, double);
	vtkGetVector4Macro(InverseConsistStatistics, double);

protected:
	vtkMRMLRegistrationQualityNode();
	~vtkMRMLRegistrationQualityNode();

	vtkMRMLRegistrationQualityNode(const vtkMRMLRegistrationQualityNode&);
	void operator=(const vtkMRMLRegistrationQualityNode&);

	char* VectorVolumeNodeID;
	char* InvVectorVolumeNodeID;
	char* ReferenceVolumeNodeID;
	char* WarpedVolumeNodeID;
	char* OutputDirectory;
	char *ROINodeID;

	int NumberOfScreenshots;
	
	int MovieBoxRedState;
	int MovieBoxYellowState;
	int MovieBoxGreenState;
	int MovieRun;

	char* CheckerboardVolumeNodeID;
	int CheckerboardPattern;

	int FlickerOpacity;
	int FlickerDelay;

	char* AbsoluteDiffVolumeNodeID;
	double AbsoluteDiffStatistics[4];


	char* JacobianVolumeNodeID;
	double JacobianStatistics[4];

	char* InverseConsistVolumeNodeID;
	double InverseConsistStatistics[4];
};

#endif
