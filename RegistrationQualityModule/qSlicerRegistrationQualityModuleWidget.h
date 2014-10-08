#ifndef __qSlicerRegistrationQualityModuleWidget_h
#define __qSlicerRegistrationQualityModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerRegistrationQualityModuleExport.h"

#include <vtkMRMLVectorVolumeNode.h>
#include <vtkMRMLGridTransformNode.h>

class qSlicerRegistrationQualityModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_RegistrationQuality
class Q_SLICER_QTMODULES_REGISTRATIONQUALITY_EXPORT qSlicerRegistrationQualityModuleWidget :
	public qSlicerAbstractModuleWidget {

	Q_OBJECT
	QVTK_OBJECT

public:
	typedef qSlicerAbstractModuleWidget Superclass;
	qSlicerRegistrationQualityModuleWidget(QWidget *parent=0);
	virtual ~qSlicerRegistrationQualityModuleWidget();

	virtual void enter();

public slots:
	virtual void setMRMLScene(vtkMRMLScene*);
	void onSceneImportedEvent();
	void setRegistrationQualityParametersNode(vtkMRMLNode *node);
	void updateWidgetFromMRML();

protected slots:
	void onLogicModified();

	void vectorVolumeChanged(vtkMRMLNode*);
	void invVectorVolumeChanged(vtkMRMLNode*);
	void referenceVolumeChanged(vtkMRMLNode*);
	void warpedVolumeChanged(vtkMRMLNode*);
	void outputDirectoyChanged();
	void ROIChanged(vtkMRMLNode*);
	void fiducialChanged(vtkMRMLNode*);
	void invFiducialChanged(vtkMRMLNode*);

	void movieBoxRedStateChanged(int state);
	void movieBoxYellowStateChanged(int state);
	void movieBoxGreenStateChanged(int state);

// 	void checkerboardVolumeChanged(vtkMRMLNode*);
// 	void absoluteDiffVolumeChanged(vtkMRMLNode*);

	void saveScreenshotClicked();
	void saveOutputFileClicked();

	// Image Checks
	void absoluteDiffClicked(bool state);
	void fiducialCalculate(bool reference);
	void falseColorClicked(bool state);
	void checkerboardClicked(bool state);
	void movieToggle();
	void flickerToggle();
	void flickerToggle1();

	// Vector checks
	void jacobianClicked(bool state);
	void inverseConsistClicked(bool state);
	
	// Slot for changing 2D scalar bar visibility
	void setScalarBar2DVisibility(bool,int);
	// Parameters
	void setCheckerboardPattern(double);

protected:
	QScopedPointer<qSlicerRegistrationQualityModuleWidgetPrivate> d_ptr;
	QTimer *flickerTimer;

	virtual void setup();
	void onEnter();

private:
	Q_DECLARE_PRIVATE(qSlicerRegistrationQualityModuleWidget);
	Q_DISABLE_COPY(qSlicerRegistrationQualityModuleWidget);
};

#endif
