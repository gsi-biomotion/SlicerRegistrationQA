#ifndef __qSlicerRegQAModule_h
#define __qSlicerRegQAModule_h

// SlicerQt includes
#include "qSlicerLoadableModule.h"

#include "vtkSlicerConfigure.h" // For Slicer_HAVE_QT5

#include "qSlicerRegQAModuleExport.h"

class qSlicerRegQAModulePrivate;

/// \ingroup Slicer_QtModules_RegQA
class Q_SLICER_QTMODULES_REGQA_EXPORT qSlicerRegQAModule :
	public qSlicerLoadableModule {

	Q_OBJECT
#ifdef Slicer_HAVE_QT5
    Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
#endif
	Q_INTERFACES(qSlicerLoadableModule);

public:
	typedef qSlicerLoadableModule Superclass;
	explicit qSlicerRegQAModule(QObject *parent=0);
	virtual ~qSlicerRegQAModule();

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
	QScopedPointer<qSlicerRegQAModulePrivate> d_ptr;

private:
	Q_DECLARE_PRIVATE(qSlicerRegQAModule);
	Q_DISABLE_COPY(qSlicerRegQAModule);

};

#endif
