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

	vtkSetStringMacro(OutputModelNodeID);
	vtkGetStringMacro(OutputModelNodeID);
	void SetAndObserveOutputModelNodeID(const char* id);

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
	vtkSetStringMacro(SquaredDiffVolumeNodeID);
	vtkGetStringMacro(SquaredDiffVolumeNodeID);
	void SetAndObserveSquaredDiffVolumeNodeID(const char* id);
	
	vtkSetVector4Macro(SquaredDiffStatistics, double);
	vtkGetVector4Macro(SquaredDiffStatistics, double);
	
	
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
	
	
	// Glyph Parameters
	vtkSetMacro(GlyphPointMax, int);
	vtkGetMacro(GlyphPointMax, int);
	vtkSetMacro(GlyphScale, float);
	vtkGetMacro(GlyphScale, float);
	vtkSetMacro(GlyphScaleDirectional, bool);
	vtkGetMacro(GlyphScaleDirectional, bool);
	vtkSetMacro(GlyphScaleIsotropic, bool);
	vtkGetMacro(GlyphScaleIsotropic, bool);
	vtkSetMacro(GlyphThresholdMax, double);
	vtkGetMacro(GlyphThresholdMax, double);
	vtkSetMacro(GlyphThresholdMin, double);
	vtkGetMacro(GlyphThresholdMin, double);
	vtkSetMacro(GlyphSeed, int);
	vtkGetMacro(GlyphSeed, int);
	vtkSetMacro(GlyphSourceOption, int);
	vtkGetMacro(GlyphSourceOption, int);
	// Arrow Parameters
	vtkSetMacro(GlyphArrowTipLength, float);
	vtkGetMacro(GlyphArrowTipLength, float);
	vtkSetMacro(GlyphArrowTipRadius, float);
	vtkGetMacro(GlyphArrowTipRadius, float);
	vtkSetMacro(GlyphArrowShaftRadius, float);
	vtkGetMacro(GlyphArrowShaftRadius, float);
	vtkSetMacro(GlyphArrowResolution, int);
	vtkGetMacro(GlyphArrowResolution, int);
	// Cone Parameters
	vtkSetMacro(GlyphConeHeight, float);
	vtkGetMacro(GlyphConeHeight, float);
	vtkSetMacro(GlyphConeRadius, float);
	vtkGetMacro(GlyphConeRadius, float);
	vtkSetMacro(GlyphConeResolution, int);
	vtkGetMacro(GlyphConeResolution, int);
	// Sphere Parameters
	vtkSetMacro(GlyphSphereResolution, float);
	vtkGetMacro(GlyphSphereResolution, float);

	// Grid Parameters
	vtkSetMacro(GridScale, float);
	vtkGetMacro(GridScale, float);
	vtkSetMacro(GridDensity, int);
	vtkGetMacro(GridDensity, int);

	// Block Parameters
	vtkSetMacro(BlockScale, float);
	vtkGetMacro(BlockScale, float);
	vtkSetMacro(BlockDisplacementCheck, int);
	vtkGetMacro(BlockDisplacementCheck, int);

	// Contour Parameters
	vtkSetMacro(ContourNumber, int);
	vtkGetMacro(ContourNumber, int);
	vtkSetMacro(ContourMin, float);
	vtkGetMacro(ContourMin, float);
	vtkSetMacro(ContourMax, float);
	vtkGetMacro(ContourMax, float);

	// Glyph Slice Parameters
	vtkSetStringMacro(GlyphSliceNodeID);
	vtkGetStringMacro(GlyphSliceNodeID);
	void SetAndObserveGlyphSliceNodeID(const char* id);
	vtkSetMacro(GlyphSlicePointMax, int);
	vtkGetMacro(GlyphSlicePointMax, int);
	vtkSetMacro(GlyphSliceThresholdMax, double);
	vtkGetMacro(GlyphSliceThresholdMax, double);
	vtkSetMacro(GlyphSliceThresholdMin, double);
	vtkGetMacro(GlyphSliceThresholdMin, double);
	vtkSetMacro(GlyphSliceScale, float);
	vtkGetMacro(GlyphSliceScale, float);
	vtkSetMacro(GlyphSliceSeed, int);
	vtkGetMacro(GlyphSliceSeed, int);

	//Grid Slice Parameters
	vtkSetStringMacro(GridSliceNodeID);
	vtkGetStringMacro(GridSliceNodeID);
	void SetAndObserveGridSliceNodeID(const char* id);
	vtkSetMacro(GridSliceScale, float);
	vtkGetMacro(GridSliceScale, float);
	vtkSetMacro(GridSliceDensity, int);
	vtkGetMacro(GridSliceDensity, int);

protected:
	vtkMRMLRegistrationQualityNode();
	~vtkMRMLRegistrationQualityNode();

	vtkMRMLRegistrationQualityNode(const vtkMRMLRegistrationQualityNode&);
	void operator=(const vtkMRMLRegistrationQualityNode&);

	char* VectorVolumeNodeID;
	char* InvVectorVolumeNodeID;
	char* ReferenceVolumeNodeID;
	char* WarpedVolumeNodeID;
	char* OutputModelNodeID;

	int MovieBoxRedState;
	int MovieBoxYellowState;
	int MovieBoxGreenState;
	int MovieRun;

	char* CheckerboardVolumeNodeID;
	int CheckerboardPattern;

	int FlickerOpacity;
	int FlickerDelay;
	
	char* SquaredDiffVolumeNodeID;
	double SquaredDiffStatistics[4];

	
	char* JacobianVolumeNodeID;
	double JacobianStatistics[4];
	
	char* InverseConsistVolumeNodeID;
	double InverseConsistStatistics[4];

	
	// Glyph Parameters
	int GlyphPointMax;
	//TODO: Need to change the UI into float too
	float GlyphScale;
	bool GlyphScaleDirectional;
	bool GlyphScaleIsotropic;
	double GlyphThresholdMax;
	double GlyphThresholdMin;
	int GlyphSeed;
	int GlyphSourceOption; //0 - Arrow, 1 - Cone, 2 - Sphere
	// Arrow Parameters
	float GlyphArrowTipLength;
	float GlyphArrowTipRadius;
	float GlyphArrowShaftRadius;
	int GlyphArrowResolution;

	// Cone Parameters
	float GlyphConeHeight;
	float GlyphConeRadius;
	int GlyphConeResolution;

	// Sphere Parameters
	float GlyphSphereResolution;

	// Grid Parameters
	float GridScale;
	int GridDensity;

	// Block Parameters
	float BlockScale;
	int BlockDisplacementCheck;

	// Contour Parameters
	int ContourNumber;
	float ContourMin;
	float ContourMax;

	// Glyph Slice Parameters
	char* GlyphSliceNodeID;
	int GlyphSlicePointMax;
	double GlyphSliceThresholdMax;
	double GlyphSliceThresholdMin;
	float GlyphSliceScale;
	int GlyphSliceSeed;

	// Grid Slice Parameters
	char* GridSliceNodeID;
	float GridSliceScale;
	int GridSliceDensity;
};

#endif
