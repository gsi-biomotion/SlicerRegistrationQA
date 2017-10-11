#include "vtkMRMLRegistrationQualityNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkObjectFactory.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLVectorVolumeNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLColorTableNode.h>

//----------------------------------------------------------------------------
static const char* COLOR_TABLE_REFERENCE_ROLE = "colorTableRef";
//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLRegistrationQualityNode);

//----------------------------------------------------------------------------
vtkMRMLRegistrationQualityNode::vtkMRMLRegistrationQualityNode() {

	this->VectorVolumeNodeID = NULL;
	this->InvVectorVolumeNodeID = NULL;
	this->TransformNodeID = NULL;
	this->MovingTransformNodeID = NULL;
	this->FixedVolumeNodeID = NULL;
	this->WarpedVolumeNodeID = NULL;
	this->OutputDirectory = NULL;
	this->ROINodeID = NULL;
	this->FixedFiducialNodeID = NULL;
	this->MovingFiducialNodeID = NULL;
	this->OutputModelNodeID = NULL;
	this->SubjectHierarchyNodeID = NULL;

	//Checkerboard Parameters
	this->CheckerboardPattern = 20;
	this->CheckerboardVolumeNodeID = NULL;

	this->FlickerOpacity = 1;

	this->AbsoluteDiffVolumeNodeID = NULL;
	AbsoluteDiffStatistics[0] = AbsoluteDiffStatistics[1] = AbsoluteDiffStatistics[2] = AbsoluteDiffStatistics[3] = 0;

	FiducialsStatistics[0] = FiducialsStatistics[1] = FiducialsStatistics[2] = FiducialsStatistics[3] = 0;
	InvFiducialsStatistics[0] = InvFiducialsStatistics[1] = InvFiducialsStatistics[2] = InvFiducialsStatistics[3] = 0;
	
	this->JacobianVolumeNodeID = NULL;
	JacobianStatistics[0] = JacobianStatistics[1] = JacobianStatistics[2] = JacobianStatistics[3] = 0;

	this->InverseConsistVolumeNodeID = NULL;
	InverseConsistStatistics[0] = InverseConsistStatistics[1] = InverseConsistStatistics[2] = InverseConsistStatistics[3] = 0;

	NumberOfScreenshots = 1;
	
	MovieBoxRedState = 0;
	MovieBoxYellowState = 0;
	MovieBoxGreenState = 0;
	MovieRun = 0;

}

//----------------------------------------------------------------------------
vtkMRMLRegistrationQualityNode::~vtkMRMLRegistrationQualityNode() {
	this->SetVectorVolumeNodeID(NULL);
	this->SetInvVectorVolumeNodeID(NULL);
	this->SetTransformNodeID(NULL);
	this->SetMovingTransformNodeID(NULL);
	this->SetFixedVolumeNodeID(NULL);
	this->SetWarpedVolumeNodeID(NULL);
	this->SetOutputDirectory(NULL);
	this->SetROINodeID(NULL);
	this->SetFixedFiducialNodeID(NULL);
	this->SetMovingFiducialNodeID(NULL);
	this->SetSubjectHierarchyNodeID(NULL);
	this->SetOutputModelNodeID(NULL);
	this->SetCheckerboardVolumeNodeID(NULL);
	this->SetAbsoluteDiffVolumeNodeID(NULL);
	this->SetJacobianVolumeNodeID(NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityNode::ReadXMLAttributes(const char** atts) {
	std::cerr << "Reading RegistrationQuality parameter node" << std::endl;
	Superclass::ReadXMLAttributes(atts);

	const char* attName;
	const char* attValue;
	while (*atts != NULL) {
		attName = *(atts++);
		attValue = *(atts++);

		if (!strcmp(attName, "VectorVolumeNodeID")) {
			this->SetVectorVolumeNodeID(attValue);
			continue;
		}
		if (!strcmp(attName, "InvVectorVolumeNodeID")) {
			this->SetInvVectorVolumeNodeID(attValue);
			continue;
		}
		if (!strcmp(attName, "TransformNodeID")) {
			this->SetTransformNodeID(attValue);
			continue;
		}
		if (!strcmp(attName, "MovingTransformNodeID")) {
			this->SetMovingTransformNodeID(attValue);
			continue;
		}
		if (!strcmp(attName, "FixedVolumeNodeID")) {
			this->SetFixedVolumeNodeID(attValue);
			continue;
		}
		if (!strcmp(attName, "WarpedVolumeNodeID")) {
			this->SetWarpedVolumeNodeID(attValue);
			continue;
		}
		if (!strcmp(attName, "OutputDirectory")) {
			this->SetOutputDirectory(attValue);
			continue;
		}
		if (!strcmp(attName, "ROINodeID")) {
			this->SetROINodeID(attValue);
			continue;
		}
		if (!strcmp(attName, "FixedFiducialNodeID")) {
			this->SetFixedFiducialNodeID(attValue);
			continue;
		}
		if (!strcmp(attName, "MovingFiducialNodeID")) {
			this->SetMovingFiducialNodeID(attValue);
                        continue;
                }
		if (!strcmp(attName, "SubjectHierarchyNodeID")) {
			this->SetSubjectHierarchyNodeID(attValue);
			continue;
		}
		if (!strcmp(attName, "OutputModelNodeID")) {
			this->SetOutputModelNodeID(attValue);
			continue;
		}
		if (!strcmp(attName, "CheckerboardPattern")) {
			std::stringstream ss;
			ss << attValue;
			ss >> this->CheckerboardPattern;
			continue;
		}
// 		if (!strcmp(attName,"CheckerboardVolumeNodeID")) {
// 			this->SetCheckerboardVolumeNodeID(attValue);
// 			continue;
// 		}
// 		if (!strcmp(attName,"FlickerOpacity")) {
// 			std::stringstream ss;
// 			ss << attValue;
// 			ss >> this->FlickerOpacity;
// 			continue;
// 		}
// 		if (!strcmp(attName,"AbsoluteDiffVolumeNodeID")) {
// 			this->SetAbsoluteDiffVolumeNodeID(attValue);
// 			continue;
// 		}
// 		if (!strcmp(attName, "AbsoluteDiffMean")) {
// 			std::stringstream ss;
// 			ss << attValue;
// 			ss >> this->AbsoluteDiffMean;
// 			continue;
// 		}
// 		if (!strcmp(attName,"JacobianVolumeNodeID")) {
// 			this->SetJacobianVolumeNodeID(attValue);
// 			continue;
// 		}
// 		if (!strcmp(attName, "JacobianStatistics")) {
// 			std::stringstream ss;
// 			ss << attValue;
// 			ss >> this->JacobianStatistics;
// 			continue;
// 		}
	}

	this->WriteXML(std::cout,1);
}

//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityNode::WriteXML(ostream& of, int nIndent) {
	Superclass::WriteXML(of, nIndent);
	vtkIndent indent(nIndent);

	of << indent << " VectorVolumeNodeID=\""
			<< (this->VectorVolumeNodeID ? this->VectorVolumeNodeID : "NULL") << "\"";
	of << indent << " InvVectorVolumeNodeID=\""
			<< (this->InvVectorVolumeNodeID ? this->InvVectorVolumeNodeID : "NULL") << "\"";
	of << indent << " TransformNodeID=\""
			<< (this->TransformNodeID ? this->TransformNodeID : "NULL") << "\"";
	of << indent << " MovingTransformNodeID=\""
			<< (this->MovingTransformNodeID ? this->MovingTransformNodeID : "NULL") << "\"";
	of << indent << " FixedVolumeNodeID=\""
			<< (this->FixedVolumeNodeID ? this->FixedVolumeNodeID : "NULL") << "\"";
	of << indent << " WarpedVolumeNodeID=\""
			<< (this->WarpedVolumeNodeID ? this->WarpedVolumeNodeID : "NULL") << "\"";
	of << indent << " OutputDirectory=\""
			<< (this->OutputDirectory ? this->OutputDirectory : "NULL") << "\"";
	of << indent << " ROINodeID=\""
			<< (this->ROINodeID ? this->ROINodeID : "NULL") << "\"";
	of << indent << " FixedFiducialNodeID=\""
			<< (this->FixedFiducialNodeID ? this->FixedFiducialNodeID : "NULL") << "\"";
	of << indent << " MovingFiducialNodeID=\""
			<< (this->MovingFiducialNodeID ? this->MovingFiducialNodeID : "NULL") << "\"";
	of << indent << " SubjectHierarchyNodeID=\""
			<< (this->SubjectHierarchyNodeID ? this->SubjectHierarchyNodeID : "NULL") << "\"";
	of << indent << " OutputModelNodeID=\""
			<< (this->OutputModelNodeID ? this->OutputModelNodeID : "NULL") << "\"";
	of << indent << " CheckerboardPattern=\"" << this->CheckerboardPattern << "\"";
// 	of << indent << " CheckerboardVolumeNodeID=\""
// 			<< (this->CheckerboardVolumeNodeID ? this->CheckerboardVolumeNodeID : "NULL") << "\"";
//
//
// 	of << indent << " FlickerOpacity=\""<< this->FlickerOpacity << "\"";
//
// 	of << indent << " AbsoluteDiffVolumeNodeID=\""
// 			<< (this->AbsoluteDiffVolumeNodeID ? this->AbsoluteDiffVolumeNodeID : "NULL") << "\"";
// 	of << indent << " AbsoluteDiffMean=\"" << this->AbsoluteDiffMean << "\"";
// 	of << indent << " AbsoluteDiffSTD=\"" << this->AbsoluteDiffSTD << "\"";
//
// 	of << indent << " JacobianVolumeNodeID=\""
// 			<< (this->JacobianVolumeNodeID ? this->JacobianVolumeNodeID : "NULL") << "\"";
// 	of << indent << " JacobianStatistics=\"" << this->JacobianStatistics << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityNode::Copy(vtkMRMLNode *anode) {
	Superclass::Copy(anode);
	vtkMRMLRegistrationQualityNode *node = vtkMRMLRegistrationQualityNode::SafeDownCast(anode);
	this->DisableModifiedEventOn();

	this->SetVectorVolumeNodeID(node->GetVectorVolumeNodeID());
	this->SetInvVectorVolumeNodeID(node->GetInvVectorVolumeNodeID());
	this->SetTransformNodeID(node->GetTransformNodeID());
	this->SetMovingTransformNodeID(node->GetMovingTransformNodeID());
	this->SetFixedVolumeNodeID(node->GetFixedVolumeNodeID());
	this->SetWarpedVolumeNodeID(node->GetWarpedVolumeNodeID());
	this->SetOutputDirectory(node->GetOutputDirectory());
	this->SetROINodeID(node->GetROINodeID());
	this->SetFixedFiducialNodeID(node->GetFixedFiducialNodeID());
	this->SetMovingFiducialNodeID(node->GetMovingFiducialNodeID());
	this->SetSubjectHierarchyNodeID(node->GetSubjectHierarchyNodeID());
	this->SetOutputModelNodeID(node->GetOutputModelNodeID());

// 	this->SetCheckerboardVolumeNodeID(node->GetCheckerboardVolumeNodeID());
	this->CheckerboardPattern=node->CheckerboardPattern;
//
//
// 	this->FlickerOpacity = node->FlickerOpacity;
// 	this->SetAbsoluteDiffVolumeNodeID(node->GetAbsoluteDiffVolumeNodeID());
//
// 	this->SetJacobianVolumeNodeID(node->GetJacobianVolumeNodeID());
// // 	this->JacobianStatistics = node->JacobianStatistics;

	this->DisableModifiedEventOff();
	this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityNode::UpdateReferenceID(const char *oldID, const char *newID) {
	if (this->VectorVolumeNodeID && !strcmp(oldID, this->VectorVolumeNodeID)) {
		this->SetAndObserveVectorVolumeNodeID(newID);
	}
	if (this->InvVectorVolumeNodeID && !strcmp(oldID, this->InvVectorVolumeNodeID)) {
		this->SetAndObserveInvVectorVolumeNodeID(newID);
	}
	if (this->TransformNodeID && !strcmp(oldID, this->TransformNodeID)) {
		this->SetAndObserveTransformNodeID(newID);
	}
	if (this->MovingTransformNodeID && !strcmp(oldID, this->MovingTransformNodeID)) {
		this->SetAndObserveMovingTransformNodeID(newID);
	}

	if (this->OutputDirectory && !strcmp(oldID, this->OutputDirectory)) {
		this->SetAndObserveOutputDirectory(newID);
	}
}

//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityNode::SetAndObserveVectorVolumeNodeID(const char* id) {
	if (this->VectorVolumeNodeID) {
		this->Scene->RemoveReferencedNodeID(this->VectorVolumeNodeID, this);
	}
	this->SetVectorVolumeNodeID(id);

	if (id) {
		this->Scene->AddReferencedNodeID(this->VectorVolumeNodeID, this);
	}
}
//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityNode::SetAndObserveInvVectorVolumeNodeID(const char* id) {
	if (this->InvVectorVolumeNodeID) {
		this->Scene->RemoveReferencedNodeID(this->InvVectorVolumeNodeID, this);
	}
	this->SetInvVectorVolumeNodeID(id);

	if (id) {
		this->Scene->AddReferencedNodeID(this->InvVectorVolumeNodeID, this);
	}
}
//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityNode::SetAndObserveTransformNodeID(const char* id) {
	if (this->TransformNodeID) {
		this->Scene->RemoveReferencedNodeID(this->TransformNodeID, this);
	}
	this->SetTransformNodeID(id);

	if (id) {
		this->Scene->AddReferencedNodeID(this->TransformNodeID, this);
	}
}
//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityNode::SetAndObserveMovingTransformNodeID(const char* id) {
	if (this->MovingTransformNodeID) {
		this->Scene->RemoveReferencedNodeID(this->MovingTransformNodeID, this);
	}
	this->SetMovingTransformNodeID(id);

	if (id) {
		this->Scene->AddReferencedNodeID(this->MovingTransformNodeID, this);
	}
}
//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityNode::SetAndObserveFixedVolumeNodeID(const char* id) {
	if (this->FixedVolumeNodeID) {
		this->Scene->RemoveReferencedNodeID(this->FixedVolumeNodeID, this);
	}
	this->SetFixedVolumeNodeID(id);

	if (id) {
		this->Scene->AddReferencedNodeID(this->FixedVolumeNodeID, this);
	}
}

//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityNode::SetAndObserveWarpedVolumeNodeID(const char* id) {
	if (this->WarpedVolumeNodeID) {
		this->Scene->RemoveReferencedNodeID(this->WarpedVolumeNodeID, this);
	}
	this->SetWarpedVolumeNodeID(id);

	if (id) {
		this->Scene->AddReferencedNodeID(this->WarpedVolumeNodeID, this);
	}
}

//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityNode::SetAndObserveOutputDirectory(const char* id) {
	if (this->OutputDirectory) {
		this->Scene->RemoveReferencedNodeID(this->OutputDirectory, this);
	}
	this->SetOutputDirectory(id);

	if (id) {
		this->Scene->AddReferencedNodeID(this->OutputDirectory, this);
	}
}
//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityNode::SetAndObserveROINodeID(const char* id) {
	if (this->ROINodeID) {
		this->Scene->RemoveReferencedNodeID(this->ROINodeID, this);
	}
	this->SetROINodeID(id);

	if (id) {
		this->Scene->AddReferencedNodeID(this->ROINodeID, this);
	}
}
//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityNode::SetAndObserveFixedFiducialNodeID(const char* id) {
	if (this->FixedFiducialNodeID) {
		this->Scene->RemoveReferencedNodeID(this->FixedFiducialNodeID, this);
	}
	this->SetFixedFiducialNodeID(id);

	if (id) {
		this->Scene->AddReferencedNodeID(this->FixedFiducialNodeID, this);
	}
}
//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityNode::SetAndObserveMovingFiducialNodeID(const char* id) {
	if (this->MovingFiducialNodeID) {
		this->Scene->RemoveReferencedNodeID(this->MovingFiducialNodeID, this);
	}
	this->SetMovingFiducialNodeID(id);

	if (id) {
		this->Scene->AddReferencedNodeID(this->MovingFiducialNodeID, this);
	}
}
//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityNode::SetAndObserveOutputModelNodeID(const char* id) {
        if (this->OutputModelNodeID) {
                this->Scene->RemoveReferencedNodeID(this->OutputModelNodeID, this);
        }
        this->SetOutputModelNodeID(id);

        if (id) {
                this->Scene->AddReferencedNodeID(this->OutputModelNodeID, this);
        }
}
//----------------------------------------------------------------------------
vtkMRMLColorTableNode* vtkMRMLRegistrationQualityNode::GetColorTableNode()
{
  return vtkMRMLColorTableNode::SafeDownCast( this->GetNodeReference(COLOR_TABLE_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityNode::SetAndObserveColorTableNode(vtkMRMLColorTableNode* node)
{
  this->SetNodeReferenceID(COLOR_TABLE_REFERENCE_ROLE, (node ? node->GetID() : NULL));
}
//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityNode::SetAndObserveCheckerboardVolumeNodeID(const char* id) {
	if (this->CheckerboardVolumeNodeID) {
		this->Scene->RemoveReferencedNodeID(this->CheckerboardVolumeNodeID, this);
	}
	this->SetCheckerboardVolumeNodeID(id);

	if (id) {
		this->Scene->AddReferencedNodeID(this->CheckerboardVolumeNodeID, this);
	}
}

//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityNode::SetAndObserveAbsoluteDiffVolumeNodeID(const char* id) {
	if (this->AbsoluteDiffVolumeNodeID) {
		this->Scene->RemoveReferencedNodeID(this->AbsoluteDiffVolumeNodeID, this);
	}
	this->SetAbsoluteDiffVolumeNodeID(id);

	if (id) {
		this->Scene->AddReferencedNodeID(this->AbsoluteDiffVolumeNodeID, this);
	}
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityNode::SetAndObserveJacobianVolumeNodeID(const char* id) {
	if (this->JacobianVolumeNodeID) {
		this->Scene->RemoveReferencedNodeID(this->JacobianVolumeNodeID, this);
	}
	this->SetJacobianVolumeNodeID(id);

	if (id) {
	  this->Scene->AddReferencedNodeID(this->JacobianVolumeNodeID, this);
	}
}

//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityNode::SetAndObserveInverseConsistVolumeNodeID(const char* id) {
	if (this->InverseConsistVolumeNodeID) {
		this->Scene->RemoveReferencedNodeID(this->InverseConsistVolumeNodeID, this);
	}
	this->SetInverseConsistVolumeNodeID(id);

	if (id) {
		this->Scene->AddReferencedNodeID(this->InverseConsistVolumeNodeID, this);
	}
}

//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityNode::SetAndObserveSubjectHierarchyNodeID(const char* id) {
	cout << "SetAndObserveSubjectHierarchyNodeID(" << (id?id:"NULL") << ")" << endl;
	if (this->SubjectHierarchyNodeID) {
		cout << "Old node not NULL (" << SubjectHierarchyNodeID << ")" << endl;
		this->Scene->RemoveReferencedNodeID(this->SubjectHierarchyNodeID, this);
	}
	cout << "Set new node" << endl;
	this->SetSubjectHierarchyNodeID(id);

	if (id) {
		cout << "AddReferencedNodeID" << endl;
		this->Scene->AddReferencedNodeID(this->SubjectHierarchyNodeID, this);
	}
	cout << "Done" << endl;
}

//----------------------------------------------------------------------------
void vtkMRMLRegistrationQualityNode::PrintSelf(ostream& os, vtkIndent indent){
	Superclass::PrintSelf(os,indent);

	os << indent << " VectorVolumeNodeID = "
			<< (this->VectorVolumeNodeID ? this->VectorVolumeNodeID : "NULL") << "\n";
	os << indent << " InvVectorVolumeNodeID = "
			<< (this->InvVectorVolumeNodeID ? this->InvVectorVolumeNodeID : "NULL") << "\n";
	os << indent << " TransformNodeID = "
			<< (this->TransformNodeID ? this->TransformNodeID : "NULL") << "\n";
	os << indent << " MovingTransformNodeID = "
			<< (this->MovingTransformNodeID ? this->MovingTransformNodeID : "NULL") << "\n";
	os << indent << " ReferenceNodeID = "
			<< (this->FixedVolumeNodeID ? this->FixedVolumeNodeID : "NULL") << "\n";
	os << indent << " WarpedNodeID = "
			<< (this->WarpedVolumeNodeID ? this->WarpedVolumeNodeID : "NULL") << "\n";
	os << indent << " SubjectHierarchyNodeID = "
			<< (this->SubjectHierarchyNodeID ? this->SubjectHierarchyNodeID : "NULL") << "\n";

	os << indent << " OutputDirectory = "
			<< (this->OutputDirectory ? this->OutputDirectory : "NULL") << "\n";
	os << indent << " ROINodeID = "
			<< (this->ROINodeID ? this->ROINodeID : "NULL") << "\n";
	os << indent << " FixedFiducialNodeID = "
			<< (this->FixedFiducialNodeID ? this->FixedFiducialNodeID : "NULL") << "\n";
	os << indent << " MovingFiducialNodeID = "
			<< (this->MovingFiducialNodeID ? this->MovingFiducialNodeID : "NULL") << "\n";
// 	os << indent << " CheckerboardVolumeNodeID = "
// 			<< (this->CheckerboardVolumeNodeID ? this->CheckerboardVolumeNodeID : "NULL") << "\n";
	os << indent << " CheckerboardPattern = " << this->CheckerboardPattern << "\n";
//
// 	os << indent << " FlickerOpacity = " << this->FlickerOpacity << "\n";
//
// 	os << indent << " AbsoluteDiffVolumeNodeID = "
// 			<< (this->AbsoluteDiffVolumeNodeID ? this->AbsoluteDiffVolumeNodeID : "NULL") << "\n";
// 	os << indent << " AbsoluteDiffMean = " << this->AbsoluteDiffMean << "\n";
// 	os << indent << " AbsoluteDiffSTD = " << this->AbsoluteDiffMean << "\n";
//
// 	os << indent << " JacobianVolumeNodeID = "
// 			<< (this->JacobianVolumeNodeID ? this->JacobianVolumeNodeID : "NULL") << "\n";
// // 	os << indent << " JacobianStatistics = " << this->JacobianStatistics << "\n";
}

