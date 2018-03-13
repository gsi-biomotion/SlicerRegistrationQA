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
#include <QDebug>

// Slicer includes
#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>

// RegistrationQA Logic includes
#include <vtkSlicerRegistrationQALogic.h>
#include <vtkSlicerVolumesLogic.h>
#include <vtkSlicerTransformLogic.h>
// #include <vtkSlicerDoseVolumeHistogramLogic.h>

// RegistrationQA includes
#include "qSlicerRegistrationQAModule.h"
#include "qSlicerRegistrationQAModuleWidget.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyRegistrationQAPlugin.h"

//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerRegistrationQAModule, qSlicerRegistrationQAModule);
#endif


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_RegistrationQA
class qSlicerRegistrationQAModulePrivate {
public:
	qSlicerRegistrationQAModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerRegistrationQAModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerRegistrationQAModulePrivate::qSlicerRegistrationQAModulePrivate() {
}

//-----------------------------------------------------------------------------
// qSlicerRegistrationQAModule methods

//-----------------------------------------------------------------------------
qSlicerRegistrationQAModule::qSlicerRegistrationQAModule(QObject* _parent)
	: Superclass(_parent)
	, d_ptr(new qSlicerRegistrationQAModulePrivate) {
}

//-----------------------------------------------------------------------------
qSlicerRegistrationQAModule::~qSlicerRegistrationQAModule() {
}

//-----------------------------------------------------------------------------
QString qSlicerRegistrationQAModule::helpText() const {
	return "TODO:\n-FalseColour\n-Flicker\n-Checkerboard\n-Movie\n-CURL\n-Jacobian\n-InverseConsistencyError";
}

//-----------------------------------------------------------------------------
QString qSlicerRegistrationQAModule::acknowledgementText() const {
	return "This module is a project of the following institutes:\nGSI - HIT - UKER\n\nThe aim of this module is to estimate the quality of an image registration.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerRegistrationQAModule::contributors() const {
	QStringList moduleContributors;
	moduleContributors << QString("Kristjan Anderle") << QString("Jens Woelfelschneider") << QString("Tobias Brandt") << QString("Daniel Richter");
	return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerRegistrationQAModule::icon() const {
	return QIcon(":/Icons/RegistrationQAIcon.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerRegistrationQAModule::categories() const {
	return QStringList() << "Registration";
}
	
//-----------------------------------------------------------------------------
QStringList qSlicerRegistrationQAModule::dependencies() const {
	return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerRegistrationQAModule::setup() {
	this->Superclass::setup();
	// Register Subject Hierarchy core plugins
       qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(new qSlicerSubjectHierarchyRegistrationQAPlugin());
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation *qSlicerRegistrationQAModule
::createWidgetRepresentation() {
	return new qSlicerRegistrationQAModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerRegistrationQAModule::createLogic() {
	return vtkSlicerRegistrationQALogic::New();
}
