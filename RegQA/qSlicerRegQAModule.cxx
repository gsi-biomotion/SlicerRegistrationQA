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
#include <QDebug>

// Slicer includes
#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>

// RegQA Logic includes
#include <vtkSlicerRegQALogic.h>
#include <vtkSlicerVolumesLogic.h>
#include <vtkSlicerTransformLogic.h>
// #include <vtkSlicerDoseVolumeHistogramLogic.h>

// RegQA includes
#include "qSlicerRegQAModule.h"
#include "qSlicerRegQAModuleWidget.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyRegQAPlugin.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerRegQAModule, qSlicerRegQAModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_RegQA
class qSlicerRegQAModulePrivate {
public:
	qSlicerRegQAModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerRegQAModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerRegQAModulePrivate::qSlicerRegQAModulePrivate() {
}

//-----------------------------------------------------------------------------
// qSlicerRegQAModule methods

//-----------------------------------------------------------------------------
qSlicerRegQAModule::qSlicerRegQAModule(QObject* _parent)
	: Superclass(_parent)
	, d_ptr(new qSlicerRegQAModulePrivate) {
}

//-----------------------------------------------------------------------------
qSlicerRegQAModule::~qSlicerRegQAModule() {
}

//-----------------------------------------------------------------------------
QString qSlicerRegQAModule::helpText() const {
	return "TODO:\n-FalseColour\n-Flicker\n-Checkerboard\n-Movie\n-CURL\n-Jacobian\n-InverseConsistencyError";
}

//-----------------------------------------------------------------------------
QString qSlicerRegQAModule::acknowledgementText() const {
	return "This module is a project of the following institutes:\nGSI - HIT - UKER\n\nThe aim of this module is to estimate the quality of an image registration.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerRegQAModule::contributors() const {
	QStringList moduleContributors;
	moduleContributors << QString("Kristjan Anderle") << QString("Jens Woelfelschneider") << QString("Tobias Brandt") << QString("Daniel Richter");
	return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerRegQAModule::icon() const {
	return QIcon(":/Icons/RegQAIcon.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerRegQAModule::categories() const {
	return QStringList() << "Registration";
}
	
//-----------------------------------------------------------------------------
QStringList qSlicerRegQAModule::dependencies() const {
	return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerRegQAModule::setup() {
	this->Superclass::setup();
	// Register Subject Hierarchy core plugins
       qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(new qSlicerSubjectHierarchyRegQAPlugin());
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation *qSlicerRegQAModule
::createWidgetRepresentation() {
	return new qSlicerRegQAModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerRegQAModule::createLogic() {
	return vtkSlicerRegQALogic::New();
}
