#ifndef __qSlicerRegistrationQualityModule_h
#define __qSlicerRegistrationQualityModule_h

// SlicerQt includes
#include "qSlicerLoadableModule.h"

#include "qSlicerRegistrationQualityModuleExport.h"

class qSlicerRegistrationQualityModulePrivate;

/// \ingroup Slicer_QtModules_RegistrationQuality
class Q_SLICER_QTMODULES_REGISTRATIONQUALITY_EXPORT qSlicerRegistrationQualityModule :
	public qSlicerLoadableModule {

	Q_OBJECT
	Q_INTERFACES(qSlicerLoadableModule);

public:
	typedef qSlicerLoadableModule Superclass;
	explicit qSlicerRegistrationQualityModule(QObject *parent=0);
	virtual ~qSlicerRegistrationQualityModule();

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
	QScopedPointer<qSlicerRegistrationQualityModulePrivate> d_ptr;

private:
	Q_DECLARE_PRIVATE(qSlicerRegistrationQualityModule);
	Q_DISABLE_COPY(qSlicerRegistrationQualityModule);

};

#endif
