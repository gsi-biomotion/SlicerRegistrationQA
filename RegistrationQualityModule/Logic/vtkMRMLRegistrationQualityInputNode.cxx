#include "vtkMRMLRegistrationQualityInputNode.h"

// VTK includes
// #include <vtkCommand.h>
#include <vtkObjectFactory.h>

// MRML includes
#include <vtkMRMLScene.h>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLRegistrationQualityInputNode);

//----------------------------------------------------------------------------
vtkMRMLRegistrationQualityInputNode::vtkMRMLRegistrationQualityInputNode() {

// Set all (private) variables

}

//----------------------------------------------------------------------------
vtkMRMLRegistrationQualityInputNode::~vtkMRMLRegistrationQualityInputNode() {

// Set all IDs to NULL

}

//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityInputNode::ReadXMLAttributes(const char** atts) {
	std::cerr << "Reading RegistrationQuality input node" << std::endl;
	Superclass::ReadXMLAttributes(atts);

// 	const char* attName;
// 	const char* attValue;
// 	while (*atts != NULL) {
// 		attName = *(atts++);
// 		attValue = *(atts++);
//
// 		if (!strcmp(attName, "VectorVolumeNodeID")) {
// 			this->SetVectorVolumeNodeID(attValue);
// 			continue;
// 		}
// 		...
// 	}

	this->WriteXML(std::cout,1);
}

//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityInputNode::WriteXML(ostream& of, int nIndent) {
	Superclass::WriteXML(of, nIndent);
	vtkIndent indent(nIndent);

// 	of << indent << " VectorVolumeNodeID=\""
// 	<< (this->VectorVolumeNodeID ? this->VectorVolumeNodeID : "NULL") << "\"";
// 	...
}

//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityInputNode::Copy(vtkMRMLNode *anode) {
	Superclass::Copy(anode);
// 	vtkMRMLRegistrationQualityInputNode *node = vtkMRMLRegistrationQualityInputNode::SafeDownCast(anode);

	this->DisableModifiedEventOn();
// 	this->SetVectorVolumeNodeID(node->GetVectorVolumeNodeID());
// 	...
	this->DisableModifiedEventOff();
	this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityInputNode::UpdateReferenceID(const char */*oldID*/, const char */*newID*/) {
// 	if (this->VectorVolumeNodeID && !strcmp(oldID, this->VectorVolumeNodeID)) {
// 		this->SetAndObserveVectorVolumeNodeID(newID);
// 	}
// 	...
}

//----------------------------------------------------------------------------
// void vtkMRMLRegistrationQualityInputNode::SetAndObserveVectorVolumeNodeID(const char* id) {
// 	if (this->VectorVolumeNodeID) {
// 		this->Scene->RemoveReferencedNodeID(this->VectorVolumeNodeID, this);
// 	}
// 	this->SetVectorVolumeNodeID(id);
//
// 	if (id) {
// 		this->Scene->AddReferencedNodeID(this->VectorVolumeNodeID, this);
// 	}
// }
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityInputNode::PrintSelf(ostream& os, vtkIndent indent){
	Superclass::PrintSelf(os,indent);

// 	os << indent << " VectorVolumeNodeID = "
// 	<< (this->VectorVolumeNodeID ? this->VectorVolumeNodeID : "NULL") << "\n";
}

