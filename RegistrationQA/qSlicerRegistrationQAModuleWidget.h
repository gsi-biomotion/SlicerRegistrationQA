#ifndef __qSlicerRegistrationQAModuleWidget_h
#define __qSlicerRegistrationQAModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerRegistrationQAModuleExport.h"

#include <vtkMRMLVectorVolumeNode.h>
#include <vtkMRMLGridTransformNode.h>

class qSlicerRegistrationQAModuleWidgetPrivate;
class vtkMRMLNode;
class QMenu;

/// \ingroup Slicer_QtModules_RegistrationQA
class Q_SLICER_QTMODULES_REGISTRATIONQA_EXPORT qSlicerRegistrationQAModuleWidget :
	public qSlicerAbstractModuleWidget {

	Q_OBJECT
	QVTK_OBJECT

public:
	typedef qSlicerAbstractModuleWidget Superclass;
	qSlicerRegistrationQAModuleWidget(QWidget *parent=0);
	virtual ~qSlicerRegistrationQAModuleWidget();

	virtual void enter();
        virtual bool setEditedNode(vtkMRMLNode* node, QString role = QString(), QString context = QString());
        

public slots:
	virtual void setMRMLScene(vtkMRMLScene*);
	void onSceneImportedEvent();
	void setRegistrationQAParametersNode(vtkMRMLNode *node);
	void updateWidgetFromMRML();
        

protected slots:
	void onLogicModified();
        void updateButtonsAndTable();

	// new

	void fixedVolumeChanged(vtkMRMLNode*);
        void movingVolumeChanged(vtkMRMLNode*);
        void forwardWarpedVolumeChanged(vtkMRMLNode*);
        void backwardWarpedVolumeChanged(vtkMRMLNode*);

        void vectorVolumeChanged(vtkMRMLNode*);
        void invVectorVolumeChanged(vtkMRMLNode*);
        
        void outputModelChanged(vtkMRMLNode*);
        void outputDirectoyChanged();
        void ROIChanged(vtkMRMLNode*);
        void fixedFiducialChanged(vtkMRMLNode*);
        void movingFiducialChanged(vtkMRMLNode*);
        
        void FixedSegmentIDChanged(QString);
        void MovingSegmentIDChanged(QString);
        void fixedSegmentationNodeChanged(vtkMRMLNode*);
        void movingSegmentationNodeChanged(vtkMRMLNode*);
        
        void movieBoxRedStateChanged(int state);
        void movieBoxYellowStateChanged(int state);
        void movieBoxGreenStateChanged(int state);
        void ROIaroundSegmentClicked();
        void RegistrationDirectionChanged();
        void saveScreenshotClicked();
        void saveOutputFileClicked();

        // Image Checks
        void absoluteDiffClicked();
        void fiducialClicked();
        void falseColor1Clicked();
        void falseColor2Clicked();
        void checkerboardClicked();
        void movieToggle();
        void flickerToggle();
        void flickerToggle1();

        // Vector checks
        void jacobianClicked();
        void inverseConsistClicked();
        
        // Contour checks
        void contourButtonClicked();
        
        // Parameters
        void setCheckerboardPattern(int);

protected:
	QScopedPointer<qSlicerRegistrationQAModuleWidgetPrivate> d_ptr;
	QTimer *flickerTimer;

	virtual void setup();
	void onEnter();

private:
	Q_DECLARE_PRIVATE(qSlicerRegistrationQAModuleWidget);
	Q_DISABLE_COPY(qSlicerRegistrationQAModuleWidget);
};

#endif
