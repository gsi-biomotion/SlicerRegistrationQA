/*==============================================================================
 *
 *  Program: 3D Slicer
 *
 *  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.
 *
 *  See COPYRIGHT.txt
 *  or http://www.slicer.org/copyright/copyright.txt for details.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * ==============================================================================*/

// Qt includes
#include <QtPlugin>

// RegistrationQuality Logic includes
#include <vtkSlicerRegistrationQualityLogic.h>

// RegistrationQuality includes
#include "qSlicerRegistrationQualityModule.h"
#include "qSlicerRegistrationQualityModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerRegistrationQualityModule, qSlicerRegistrationQualityModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_RegistrationQuality
class qSlicerRegistrationQualityModulePrivate {
public:
	qSlicerRegistrationQualityModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerRegistrationQualityModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerRegistrationQualityModulePrivate::qSlicerRegistrationQualityModulePrivate() {
}

//-----------------------------------------------------------------------------
// qSlicerRegistrationQualityModule methods

//-----------------------------------------------------------------------------
qSlicerRegistrationQualityModule::qSlicerRegistrationQualityModule(QObject* _parent)
	: Superclass(_parent)
	, d_ptr(new qSlicerRegistrationQualityModulePrivate) {
}

//-----------------------------------------------------------------------------
qSlicerRegistrationQualityModule::~qSlicerRegistrationQualityModule() {
}

//-----------------------------------------------------------------------------
QString qSlicerRegistrationQualityModule::helpText() const {
	return "TODO:\n-FalseColour\n-Flicker\n-Checkerboard\n-Movie\n-CURL\n-Jacobian\n-InverseConsistencyError";
}

//-----------------------------------------------------------------------------
QString qSlicerRegistrationQualityModule::acknowledgementText() const {
	return "This module is a project of the following institutes:\nGSI - HIT - UKER\n\nThe aim of this module is to estimate the quality of an image registration.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerRegistrationQualityModule::contributors() const {
	QStringList moduleContributors;
	moduleContributors << QString("Jens Woelfelschneider") << QString("Tobias Brandt") << QString("Daniel Richter") << QString("Kristjan Anderle");
	return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerRegistrationQualityModule::icon() const {
	return QIcon(":/Icons/RegistrationQualityIcon.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerRegistrationQualityModule::categories() const {
	return QStringList() << "Registration";
}

//-----------------------------------------------------------------------------
QStringList qSlicerRegistrationQualityModule::dependencies() const {
	return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQualityModule::setup() {
	this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation *qSlicerRegistrationQualityModule
::createWidgetRepresentation() {
	return new qSlicerRegistrationQualityModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerRegistrationQualityModule::createLogic() {
	return vtkSlicerRegistrationQualityLogic::New();
}
