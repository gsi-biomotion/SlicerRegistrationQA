#ifndef __qSlicerRegistrationQualityModuleWidget_h
#define __qSlicerRegistrationQualityModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerRegistrationQualityModuleExport.h"

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

	void visualize();

protected slots:
	void onLogicModified();
	void updateSourceOptions(int);

	void vectorVolumeChanged(vtkMRMLNode*);
	void referenceVolumeChanged(vtkMRMLNode*);
	void warpedVolumeChanged(vtkMRMLNode*);
	void outputModelChanged(vtkMRMLNode*);

	void movieBoxRedStateChanged(int state);
	void movieBoxYellowStateChanged(int state);
	void movieBoxGreenStateChanged(int state);

	void checkerboardVolumeChanged(vtkMRMLNode*);
	// Image Checks
	void falseColorToggle();
	void checkerboardToggle();
	void movieToggle();
	void flickerToggle();

	// Parameters
	void setCheckerboardPattern(QString aText);
	// Glyph Parameters
	void setGlyphPointMax(double);
	void setGlyphScale(double);
	void setGlyphScaleDirectional(bool);
	void setGlyphScaleIsotropic(bool);
	void setGlyphThreshold(double, double);
	void setGlyphSeed(int);
	void setSeed();
	void setGlyphSourceOption(int);
	// Arrow Parameters
	void setGlyphArrowTipLength(double);
	void setGlyphArrowTipRadius(double);
	void setGlyphArrowShaftRadius(double);
	void setGlyphArrowResolution(double);
	// Cone Parameters
	void setGlyphConeHeight(double);
	void setGlyphConeRadius(double);
	void setGlyphConeResolution(double);
	// Sphere Parameters
	void setGlyphSphereResolution(double);

	// Grid Parameters
	void setGridScale(double);
	void setGridDensity(double);

	// Block Parameters
	void setBlockScale(double);
	void setBlockDisplacementCheck(int);

	// Contour Parameters
	void setContourNumber(double);
	void setContourRange(double, double);

	// Glyph Slice Parameters
	void setGlyphSliceNode(vtkMRMLNode*);
	void setGlyphSlicePointMax(double);
	void setGlyphSliceThreshold(double, double);
	void setGlyphSliceScale(double);
	void setGlyphSliceSeed(int);
	void setSeed2();

	// Grid Slice Parameters
	void setGridSliceNode(vtkMRMLNode*);
	void setGridSliceScale(double);
	void setGridSliceDensity(double);

protected:
	QScopedPointer<qSlicerRegistrationQualityModuleWidgetPrivate> d_ptr;

	virtual void setup();
	void onEnter();

private:
	Q_DECLARE_PRIVATE(qSlicerRegistrationQualityModuleWidget);
	Q_DISABLE_COPY(qSlicerRegistrationQualityModuleWidget);
};

#endif
