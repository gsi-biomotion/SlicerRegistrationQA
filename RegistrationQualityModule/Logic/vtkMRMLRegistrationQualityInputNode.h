#pragma once

#include <vtkMRML.h>
#include <vtkMRMLNode.h>

#include "DIRQAImage.h"
#include "vtkSlicerRegistrationQualityModuleLogicExport.h"

/// \ingroup Slicer_QtModules_RegistrationQuality
class VTK_SLICER_REGISTRATIONQUALITY_MODULE_LOGIC_EXPORT vtkMRMLRegistrationQualityInputNode :
	public vtkMRMLNode {

public:
	static vtkMRMLRegistrationQualityInputNode *New();
	vtkTypeMacro(vtkMRMLRegistrationQualityInputNode, vtkMRMLNode);
	void PrintSelf(ostream& os, vtkIndent indent);

	virtual vtkMRMLNode* CreateNodeInstance();
	virtual void ReadXMLAttributes( const char** atts);
	virtual void WriteXML(ostream& of, int indent);
	virtual void Copy(vtkMRMLNode *node);
	virtual const char* GetNodeTagName() {return "RegistrationQualityInput";};
	virtual void UpdateReferenceID(const char *oldID, const char *newID);

protected:

	std::vector<DIRQAImage> images;
	std::vector<DIRQAImage> warped;
	std::vector<DIRQAImage> vector;

	std::vector<DIRQAImage> difference;
	std::vector<DIRQAImage> checkerboard;
	std::vector<DIRQAImage> jacobian;
	std::vector<DIRQAImage> inverseConsistency;

	vtkMRMLRegistrationQualityInputNode();
	~vtkMRMLRegistrationQualityInputNode();

	vtkMRMLRegistrationQualityInputNode(const vtkMRMLRegistrationQualityInputNode&);
	void operator=(const vtkMRMLRegistrationQualityInputNode&);

};
