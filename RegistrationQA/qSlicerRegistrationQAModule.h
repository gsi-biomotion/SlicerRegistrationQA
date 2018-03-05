#ifndef __qSlicerRegistrationQAModule_h
#define __qSlicerRegistrationQAModule_h

// SlicerQt includes
#include "qSlicerLoadableModule.h"


#include "qSlicerRegistrationQAModuleExport.h"

class qSlicerRegistrationQAModulePrivate;

/// \ingroup Slicer_QtModules_RegistrationQA
class Q_SLICER_QTMODULES_REGISTRATIONQA_EXPORT qSlicerRegistrationQAModule :
	public qSlicerLoadableModule {

	Q_OBJECT
	Q_INTERFACES(qSlicerLoadableModule);

public:
	typedef qSlicerLoadableModule Superclass;
	explicit qSlicerRegistrationQAModule(QObject *parent=0);
	virtual ~qSlicerRegistrationQAModule();

	qSlicerGetTitleMacro(QTMODULE_TITLE);

	virtual QString helpText()const;
	virtual QString acknowledgementText()const;
	virtual QStringList contributors()const;

	virtual QIcon icon()const;

	virtual QStringList categories()const;
	virtual QStringList dependencies() const;

protected:
	/// Initialize the module. Register the volumes reader/writer
	virtual void setup();

	/// Create and return the widget representation associated to this module
	virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

	/// Create and return the logic associated to this module
	virtual vtkMRMLAbstractLogic* createLogic();

protected:
	QScopedPointer<qSlicerRegistrationQAModulePrivate> d_ptr;

private:
	Q_DECLARE_PRIVATE(qSlicerRegistrationQAModule);
	Q_DISABLE_COPY(qSlicerRegistrationQAModule);

};

#endif
