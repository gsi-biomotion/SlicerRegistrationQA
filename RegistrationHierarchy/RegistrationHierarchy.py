import os
import unittest
from __main__ import vtk, qt, ctk, slicer
import numpy as np

#
# RegistrationHierarchy


#Constants that have to be the same in creating Registration Hierarchy
NAME_CT = 'CT'
NAME_WARP = 'WarpedImage' #Warped image from phase ( phase + vector field = warpedimage)
NAME_INVWARP = 'InvWarpedImage' #Warped image from reference phase (reference + invVector = invWarpedImage)
NAME_VECTOR = 'Vector' #Vector Field from phase to reference phase
NAME_INVVECTOR = 'InvVector' #Vector from reference phase to phase
NAME_ABSDIFF = 'AbsoluteDifference'
NAME_INVABSDIFF = 'InvAbsoluteDifference'
NAME_JACOBIAN = 'Jacobian'
NAME_INVJACOBIAN = 'InvJacobian'
NAME_INVCONSIST = 'InverseConsistency'
NAME_REFPHASE = 'ReferenceHierarchyNode'
NAME_ROIFILEPATH = 'RoiFilePath'
NAME_DIRQA = 'DIRQA'
NAME_DEFABS = "DefaultAbsoluteDifference"
NAME_PERCENT = 'Percentile' #Special case to store each contribution to vector field magnitude
NAME_INVPERCENT = 'InvPercentile'
NAME_JACCONSIST = 'JacobianConsistency'

STATISTIC_LIST = [NAME_VECTOR, NAME_INVVECTOR, NAME_DEFABS, NAME_ABSDIFF, NAME_INVABSDIFF, NAME_JACOBIAN,
		  NAME_INVJACOBIAN, NAME_INVCONSIST, NAME_PERCENT, NAME_INVPERCENT, NAME_JACCONSIST]

class RegistrationHierarchy:
  def __init__(self, parent):
    parent.title = "Registration Hierarchy" # TODO make this more human readable by adding spaces
    parent.categories = ["RegistrationQuality"]
    parent.dependencies = []
    parent.contributors = ["Kristjan Anderle (GSI)"] # replace with "Firstname Lastname (Org)"
    parent.helpText = """
    This is an example of scripted loadable module bundled in an extension.
    """
    parent.acknowledgementText = """
    This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc. and Steve Pieper, Isomics, Inc.  and was partially funded by NIH grant 3P41RR013218-12S1.
""" # replace with organization, grant and thanks.
    self.parent = parent

    # Add this test to the SeqMRMLSubjectHierarchyTreeViewlfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['RegistrationHierarchy'] = self.runTest

  def runTest(self):
    tester = RegistrationHierarchyTest()
    tester.runTest()

#
# qRegistrationHierarchyWidget
#

class RegistrationHierarchyWidget:
  def __init__(self, parent = None):
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
    else:
      self.parent = parent
    self.layout = self.parent.layout()
    if not parent:
      self.setup()
      self.parent.show()

  def setup(self):
    # Instantiate and connect widgets ...

    #qMRMLSubjectHierarchyTreeView
    # Reload and Test area
    #
    reloadCollapsibleButton = ctk.ctkCollapsibleButton()
    reloadCollapsibleButton.text = "Reload && Test"
    self.layout.addWidget(reloadCollapsibleButton)
    reloadFormLayout = qt.QFormLayout(reloadCollapsibleButton)

    # reload button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.redirqaAllPhaseButton = qt.QPushButton("Reload")
    self.redirqaAllPhaseButton.toolTip = "Reload this module."
    self.redirqaAllPhaseButton.name = "RegistrationHierarchy Reload"
    reloadFormLayout.addWidget(self.redirqaAllPhaseButton)
    self.redirqaAllPhaseButton.connect('clicked()', self.onReload)

    # reload and test button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadAndTestButton = qt.QPushButton("Reload and Test")
    self.reloadAndTestButton.toolTip = "Reload this module and then run the self tests."
    reloadFormLayout.addWidget(self.reloadAndTestButton)
    self.reloadAndTestButton.connect('clicked()', self.onReloadAndTest)

    #
    # Parameters Area
    #
    parametersCollapsibleButton = ctk.ctkCollapsibleButton()
    parametersCollapsibleButton.text = "Parameters"
    self.layout.addWidget(parametersCollapsibleButton)

    # Layout within the dummy collapsible button
    self.parametersFormLayout = qt.QFormLayout(parametersCollapsibleButton)

    #
    # Select Subject Hierarchy
    #
    self.regHierarchyComboBox = slicer.qMRMLSubjectHierarchyTreeView()
    #self.regHierarchyComboBox.nodeTypes = ( ("vtkMRMLSubjectHierarchyNode"),"" )vtkMRMLSubjectHierarchyNode
    #self.regHierarchyComboBox.addAttribute( "vtkMRMLScalarVolumeNode", "Name", "Registration Node" )
    #self.regHierarchyComboBox.selectNodeUponCreation = True
    #self.regHierarchyComboBox.addEnabled = False
    #self.regHierarchyComboBox.removeEnabled = False
    #self.regHierarchyComboBox.noneEnabled = False
    #self.regHierarchyComboBox.showHidden = False
    #self.regHierarchyComboBox.showChildNodeTypes = False
    self.regHierarchyComboBox.setMRMLScene( slicer.mrmlScene )
    self.regHierarchyComboBox.setToolTip( "Select subject hierarchy with registration hierarchy." )
    self.parametersFormLayout.addRow("Select hirerachy: ", self.regHierarchyComboBox)
    
    self.columnView = qt.QTreeView()
    #self.columnView.setColumnWidths((150,150))
    self.columnView.setContextMenuPolicy(qt.Qt.CustomContextMenu)
    self.parametersFormLayout.addRow(self.columnView)
    
    #Model that stores all items
    self.model = qt.QStandardItemModel()
    self.columnView.setModel(self.model)
    
    #
    # Automatic registration
    #
    self.automaticRegistrationButton = qt.QPushButton("Automatic registration all phases.")
    self.automaticRegistrationButton.toolTip = "Will load and register all phases."
    self.automaticRegistrationButton.enabled = True
    self.parametersFormLayout.addRow(self.automaticRegistrationButton)
    
    
    #
    # Make DIRQA on all phases
    #
    self.dirqaAllPhaseButton = qt.QPushButton("Make DIRQA on all phases.")
    self.dirqaAllPhaseButton.toolTip = "Will make dirqa on all phases in registration hierarchy node."
    self.dirqaAllPhaseButton.enabled = True
    self.parametersFormLayout.addRow(self.dirqaAllPhaseButton)
    
    #
    # Make Inv Con on all phases
    #
    self.invConAllPhaseButton = qt.QPushButton("Load ROI.")
    self.invConAllPhaseButton.toolTip = "Will make inverse consistency on all phases in registration hierarchy node."
    self.invConAllPhaseButton.enabled = True
    self.parametersFormLayout.addRow(self.invConAllPhaseButton)
    
    # Resample 4D
    self.resample4DCheckBox = qt.QCheckBox()     
    self.resample4DCheckBox.setToolTip( "Check if you want 4DCT vector field to be resampled." )
    self.resample4DCheckBox.setCheckState(2)
    self.parametersFormLayout.addRow("Resample:", self.resample4DCheckBox)
    
    #Label map
    self.selectLabelMap = slicer.qMRMLNodeComboBox()
    self.selectLabelMap.nodeTypes = ["vtkMRMLLabelMapVolumeNode"]
    #self.selectLabelMap.addAttribute( "vtkMRMLScalarVolumeNode", "Name", "Registration Node" )
    self.selectLabelMap.selectNodeUponCreation = False
    self.selectLabelMap.addEnabled = False
    self.selectLabelMap.removeEnabled = False
    self.selectLabelMap.noneEnabled = False
    self.selectLabelMap.showHidden = False
    self.selectLabelMap.showChildNodeTypes = False
    self.selectLabelMap.setMRMLScene( slicer.mrmlScene )
    self.selectLabelMap.setToolTip( "Select Contour for calculation." )
    self.parametersFormLayout.addRow("Select Contour: ", self.selectLabelMap)
    
    #
    # Calculate statistics on label map
    #
    self.calcStatisticsButton = qt.QPushButton("Calculate statistics.")
    self.calcStatisticsButton.toolTip = "Load inverse consistency files and checks statistics for selected label map"
    self.calcStatisticsButton.enabled = True
    self.parametersFormLayout.addRow(self.calcStatisticsButton)
    
    #Right-click menu
    self.contextMenu = qt.QMenu()
    self.showAction = qt.QAction("Show", self.contextMenu)
    self.contextMenu.addAction(self.showAction)
    self.showAction.enabled = True
    self.showAction.visible = True
    self.computeAction = qt.QAction("Compute DIRQA", self.contextMenu)
    self.contextMenu.addAction(self.computeAction)
    
    self.computeAction.enabled = True
    self.computeAction.visible = True
    
    self.automaticRegistrationButton.connect('clicked(bool)', self.onAutomaticRegistrationButton)
    self.dirqaAllPhaseButton.connect('clicked(bool)', self.onDirqaAllPhaseButton)
    self.invConAllPhaseButton.connect('clicked(bool)', self.onInvConAllPhaseButton)
    self.calcStatisticsButton.connect('clicked(bool)', self.onCalcStatisticsButton)
    self.regHierarchyComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.setStandardModel)
    self.contextMenu.connect('triggered(QAction*)', self.onContextMenuClicked)
    
    #self.columnView.clicked.connect(self.itemSelection)
    self.columnView.customContextMenuRequested.connect(self.showMenu)
             
    
    
    
    # Add vertical spacer
    self.layout.addStretch(1)
    
    
  def onAutomaticRegistrationButton(self):
    logic = RegistrationHierarchyLogic()
    logic.automaticRegistration(self.regHierarchy)
    self.setStandardModel(self.regHierarchy)
    
  
  def onDirqaAllPhaseButton(self):
    logic = RegistrationHierarchyLogic()
    resample = []
    if self.resample4DCheckBox.checkState() == 2:
       resample = [2,2,2]
    logic.computeDIRQAfromHierarchyNode(self.regHierarchy, resample)
    logic.writeData(self.regHierarchy)
    self.setStandardModel(self.regHierarchy)


  def onInvConAllPhaseButton(self):
    logic = RegistrationHierarchyLogic()
    resample = []
    if self.resample4DCheckBox.checkState() == 2:
       resample = [2,2,2]
    logic.loadRoi(self.regHierarchy)
    #logic.computeInverseConsistencyAllPhases(self.regHierarchy, resample)
    #logic.writeInvConData(self.regHierarchy)
    #self.setStandardModel(self.regHierarchy)
    
  def onCalcStatisticsButton(self):
    logic = RegistrationHierarchyLogic()
    labelMap = self.selectLabelMap.currentNode()
    logic.calculateInvConStatistics(self.regHierarchy,labelMap)
  
  def showMenu(self, pos):
    #index = self.columnView.selectionModel().currentIndex()
    #For phase enable option to compute everything
    #if index.() == 0:
      #self.computeAction.visible = True
      #self.showAction.visible = False
    #if index.row() == 1:
      #self.computeAction.visible = False
      #self.showAction.visible = True     
    self.contextMenu.popup(self.columnView.mapToGlobal(pos))

  def onContextMenuClicked(self,action):
    logic = RegistrationHierarchyLogic()
    index = self.columnView.selectionModel().currentIndex()
    node = self.model.itemFromIndex(index).data()
    if action == self.showAction:
      if not node:
	print "Can't get node"
	return
      logic.showVolumeFromHierarchyNode(node)
	
      #logic.showVolumeFromHierarchyNode(node)
    if action == self.computeAction:
      if not node:
	return
      
      if node.GetID() == self.regHierarchy.GetAttribute(NAME_REFPHASE):
	print "Skiping reference phase"
	return
      refPhaseNode = logic.getReferencePhaseFromHierarchy(self.regHierarchy)
      
      if not refPhaseNode:
	print "Can't load reference phase"
	return
      logic.computeDIRQAfromHierarchyNode(node,refPhaseNode,[])
      
      
  #def itemSelection(self,item):
    #print "Wooh"
  
  def setStandardModel(self,regHierarchy):

    
    self.model.clear()
    if not regHierarchy:
      return

    if not regHierarchy.GetNameWithoutPostfix().find('Registration') > -1:
      return
    
    if regHierarchy.GetNumberOfChildrenNodes() < 1:
      print "Error, Selected Subject Hiearchy doesn't have any child contour nodes."
    
    
    self.model.setHorizontalHeaderLabels(['Registration Node: '])
    #List that holds all data
    self.phaseItems = []
    self.dirqaItems = []

    #Go through all phases
    for i in range(0,regHierarchy.GetNumberOfChildrenNodes()):
      phaseNode = regHierarchy.GetNthChildNode(i)
      #Create Item for each phase
      phaseItem = qt.QStandardItem()
      phaseItem.setData(phaseNode)
      phaseItem.setText(phaseNode.GetNameWithoutPostfix())
      #Loop through dirqa nodes
      if phaseNode.GetNumberOfChildrenNodes() > 0:
	dirqaList = []
	for j in range(0,phaseNode.GetNumberOfChildrenNodes()):
	  dirqaNode = phaseNode.GetNthChildNode(j)
	  dirqaName = dirqaNode.GetNameWithoutPostfix()
	  #Create item for each dirqa node
	  dirqaItem = qt.QStandardItem()
	  dirqaItem.setData(dirqaNode)
	  dirqaItem.setText(dirqaName)
	  phaseItem.appendRow(dirqaItem)
	  dirqaList.append(dirqaItem)
	  
	  #Look for statistics:
	  stringList = []
	  for check in STATISTIC_LIST:
	    if dirqaName == check:
	      stringList = ["Mean","STD","Max","Min"]
		  
	  if dirqaName == NAME_VECTOR or dirqaName == NAME_INVVECTOR:
	    stringList = ["x","y","z"]
	    
	  if stringList:
	    for i in range(0,len(stringList)):
	      if not dirqaNode.GetAttribute(stringList[i]):
	        continue
	      text = ''
	      text = dirqaItem.text() + " "
	      text += stringList[i] + ': ' + dirqaNode.GetAttribute(stringList[i]) + " "
	      dirqaItem.setText(text)
	    
	    
	self.dirqaItems.append(dirqaList)
      else:
	self.dirqaItems.append(None)
	  
      self.model.appendRow(phaseItem)
      self.phaseItems.append(phaseItem)
      self.regHierarchy = regHierarchy


  def cleanup(self):
    pass


  def onReload(self,moduleName="RegistrationHierarchy"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def onReloadAndTest(self,moduleName="RegistrationHierarchy"):
    try:
      self.onReload()
      evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
      tester = eval(evalString)
      tester.runTest()
    except Exception, e:
      import traceback
      traceback.print_exc()
      qt.QMessageBox.warning(slicer.util.mainWindow(),
          "Reload and Test", 'Exception!\n\n' + str(e) + "\n\nSee Python Console for Stack Trace")

#
# RegistrationHierarchyLogic
#

class RegistrationHierarchyLogic:
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def __init__(self):
    pass

  def automaticRegistration(self,regHierarchy, overwrite = True, resample = []):
    if not regHierarchy:
      print "No registration Hierarchy"
      return

    warpedOn = True
    cbtOn = False
    mhaOn = True
    patientName = regHierarchy.GetAttribute("PatientName")
    
    refPhaseNode = self.getReferencePhaseFromHierarchy(regHierarchy)
    referenceNumber = regHierarchy.GetAttribute("ReferenceNumber")
    if not refPhaseNode:
      print "Can't get reference node."
      return
      
    nPhases = regHierarchy.GetNumberOfChildrenNodes()
    if nPhases < 1:
      print "No children nodes."
      return
     
    
    regParameters = registrationParameters(patientName,refPhaseNode, referenceNumber, resample)
    
    regParameters.warpDirectory = regHierarchy.GetAttribute("DIR" + NAME_WARP )
    regParameters.vectorDirectory = regHierarchy.GetAttribute("DIR" + NAME_VECTOR )
    
    if not os.path.exists(regParameters.warpDirectory):
      print regParameters.warpDirectory + " directory doesn't exist - create!"
      return
      
    if not os.path.exists(regParameters.vectorDirectory):
      print regParameters.vectorDirectory + " directory doesn't exist - create!"
      return 
    
    if warpedOn:
      regParameters.setWarpVolume()
    
    if cbtOn:
      regParameters.setVectorVolume()
      
    regParameters.mhaOn = mhaOn
    
    beginPhase = 0

    
    #Loop through all phases:
    for i in range(beginPhase,nPhases):
      phaseHierarchyNode = regHierarchy.GetNthChildNode(i)   

      phaseNumber = phaseHierarchyNode.GetAttribute('PhaseNumber')
      
      #phaseNode = nextPhaseVolume
      
      #if not i == nPhases:
        #nextPhaseVolume = self.getVolumeFromChild(regHierarchy.GetNthChildNode(i+1),NAME_CT)
        
      if phaseHierarchyNode.GetID() == regHierarchy.GetAttribute(NAME_REFPHASE):
	print "Skipping reference phase"
	continue
      
      #If there's already vector hierarchy node, then eithen remove node or skip this phase :
      if phaseHierarchyNode.GetChildWithName(phaseHierarchyNode,NAME_VECTOR):
	#If inverse registration wasn't completed or if we have overwrite option, repeat registration
	#Otherwise skip.
	if overwrite or not phaseHierarchyNode.GetChildWithName(phaseHierarchyNode,NAME_INVVECTOR):
	  slicer.mrmlScene.RemoveNode(phaseHierarchyNode.GetChildWithName(phaseHierarchyNode,NAME_VECTOR))
	  if phaseHierarchyNode.GetChildWithName(phaseHierarchyNode,NAME_INVVECTOR):
	    slicer.mrmlScene.RemoveNode(phaseHierarchyNode.GetChildWithName(phaseHierarchyNode,NAME_INVVECTOR))
	  if phaseHierarchyNode.GetChildWithName(phaseHierarchyNode,NAME_WARP):
	    slicer.mrmlScene.RemoveNode(phaseHierarchyNode.GetChildWithName(phaseHierarchyNode,NAME_WARP))
	  if phaseHierarchyNode.GetChildWithName(phaseHierarchyNode,NAME_INVWARP):
	    slicer.mrmlScene.RemoveNode(phaseHierarchyNode.GetChildWithName(phaseHierarchyNode,NAME_INVWARP))
	else:
	  print "Skipping phase: " + phaseHierarchyNode.GetNameWithoutPostfix()
	  continue
      
      phaseNode = self.getVolumeFromChild(phaseHierarchyNode,NAME_CT)
      if not phaseNode:
	print "Can't load phase from: " + phaseHierarchyNode.GetNameWithoutPostfix()
	continue


      
      regParameters.movingNode = phaseNode
      
      regParameters.movingNumber = phaseNumber
      regParameters.movingHierarchy = phaseHierarchyNode
      
      #for i in paramets
        #regParameters.parameters = []
      
      
      
      

      regParameters.register()
      slicer.mrmlScene.RemoveNode(phaseNode)


    #Remove all volumes from scene, to free memory 
    slicer.mrmlScene.RemoveNode(regParameters.warpVolume)
    slicer.mrmlScene.RemoveNode(regParameters.vectorVolume)
    #slicer.mrmlScene.RemoveNode(refPhaseNode)
    
    self.createTrafo(regParameters.vectorDirectory,patientName,nPhases,int(referenceNumber))

      

  def createTrafo(self,directoryPath,patientName,numberOfPhases,referencePhase):
    output_str = '!filetype trptrf\n'
    output_str += '!fileversion 20081222\n'
    output_str += '!filedate 20110325\n'
    output_str += '!patientname ' + patientName + '\n'
    output_str += '!trafomethod SCBT\n'
    output_str += '!states ' + str(numberOfPhases) + '\n'
    output_str += '!refstate ' + str(referencePhase) + '\n'
    output_str += '!invert 0\n'
    output_str += '!FinalRegistrationMatrix 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1\n'
    output_str += '!PreTrafoMatrix 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1\n'
    
    filePath = directoryPath + patientName + '.trf'
    f = open(filePath,"wb+")
    f.write(output_str)
    f.close()
    
    
  def computeDIRQAfromHierarchyNode(self,regHierarchy, resample):
    #Look for DIRQA module
    try:
      DIRQALogic = slicer.modules.registrationquality.logic()
    except AttributeError:
      import sys
      sys.stderr.write('Cannot find registrationquality module')
      return
      
    removeOn = True
    resampledRef = False
    jacConsist = True
    
    if not regHierarchy:
      print "No registration Hierarchy"
      return

    #refPhaseNode = self.getReferencePhaseFromHierarchy(regHierarchy)
    #referenceHierarchyNode = slicer.util.getNode(regHierarchy.GetAttribute(NAME_REFPHASE))
    referenceNumber = int(regHierarchy.GetAttribute("ReferenceNumber"))

    #if not refPhaseNode:
      #print "Can't get reference node."
      #return
      
    roiNode = self.loadRoi(regHierarchy)
    if not roiNode:
       print "No ROI"
       return
     
    #if not roiNode:
      #roiFilePath = referenceHierarchyNode.GetAttribute(NAME_ROIFILEPATH)
      #if roiFilePath:
        #slicer.util.loadAnnotationFiducial(roiFilePath)
        #roiNode = slicer.util.getNode('R')
    
    nPhases = regHierarchy.GetNumberOfChildrenNodes()
    if nPhases < 1:
      print "No children nodes."
      return

    #Loop through all phases:
    vectorNode = None
    invVectorNode = None
    maxPhaseHierarchy = None
    for i in range(0,nPhases):
      if i == referenceNumber:
        continue
      #if not i == 9:
         #continue
      if i < 10:
         phaseName = "Phase_0" + str(i)
      else:
         phaseName = "Phase_" + str(i)
      phaseHierarchyNode = regHierarchy.GetChildWithName(regHierarchy,phaseName)
      
      if not phaseHierarchyNode:
	print "Can't get " + phaseName
	continue
      
      print "DIRQA phase " + str(i) + "/" + str(nPhases)
      
      vectorHierarchyNode = phaseHierarchyNode.GetChildWithName(phaseHierarchyNode,NAME_VECTOR)
      invVectorHierarchyNode = phaseHierarchyNode.GetChildWithName(phaseHierarchyNode,NAME_INVVECTOR)
      
      vectorNode = self.getVolumeFromChild(phaseHierarchyNode,NAME_VECTOR)
      invVectorNode = self.getVolumeFromChild(phaseHierarchyNode,NAME_INVVECTOR)
      
      if not vectorNode or not invVectorNode:
         print "Can't load vectors."
         continue
      else:
        print "Loaded Vector: " + vectorNode.GetName()
        print "Loaded InvVector: " + invVectorNode.GetName()
      
      #MHA or not?
      if vectorHierarchyNode.GetAttribute("FilePath").find('.mha') < 0:
	 resampleTRiP = False
	 if regHierarchy.GetAttribute('DIR' + NAME_VECTOR).find('Resample') > -1:
           resampleTRiP = True #Special case for resampling just cubes 
         print "Setting TRiP .cbt vector values to physical." + str(resampleTRiP)
         self.setVectorField(vectorNode, resampleTRiP)
         self.setVectorField(invVectorNode, resampleTRiP)
         
      #Set origins manually to zero! Due to trip
      #vectorNode.SetOrigin([0,0,0])
      #invVectorNode.SetOrigin([0,0,0])
      

      #Resample for computation friendly
      if vectorNode.GetImageData().GetDimensions()[1] > 300:
        vectorNode = self.resampleVolume(vectorNode,resample, vectorHierarchyNode)
        invVectorNode = self.resampleVolume(invVectorNode, resample, invVectorHierarchyNode)
      
      self.computeMaxVectorValues(phaseHierarchyNode, roiNode)

      self.computeInverseConsistency(phaseHierarchyNode, roiNode)
   
      self.computeJacobian(phaseHierarchyNode, roiNode, jacConsist)
   
      ##Load refNode
      refNode = self.getReferencePhaseFromHierarchy(regHierarchy)
      refNode.SetOrigin([0,0,0])
     
      if len(resample) == 3:
        refHierarchy = self.getReferenceHierarchy(regHierarchy)
        refNode = self.resampleVolume(refNode, resample, refHierarchy)
      
      self.computeAbsDifference(phaseHierarchyNode, refNode, roiNode,resample)
      
      self.removeHierarchyNode(refNode)
      
      self.writeData(regHierarchy)

      self.removeHierarchyNode(vectorNode)
      self.removeHierarchyNode(invVectorNode)
      
      
  def computeInverseConsistencyAllPhases(self, regHierarchy, resample):
    
    if not regHierarchy:
      print "No registration Hierarchy"
      return
      
    #refPhaseNode = self.getReferencePhaseFromHierarchy(regHierarchy)
    #referenceHierarchyNode = slicer.util.getNode(regHierarchy.GetAttribute(NAME_REFPHASE))
    referenceNumber = regHierarchy.GetAttribute("ReferenceNumber")
    #if not refPhaseNode:
      #print "Can't get reference node."
      #return
      
    patientName = regHierarchy.GetAttribute("PatientName")
    

    #TODO: This needs fix. Now the name of ROI has to be 'R' otherwise it doesn't work.
    roiNode = slicer.util.getNode('R')
    
    nPhases = regHierarchy.GetNumberOfChildrenNodes()
    if nPhases < 1:
      print "No children nodes."
      return
    vectorNode = None
    invVectorNode = None

    for i in range(0,nPhases):
      if i == referenceNumber:
        continue
      phaseHierarchyNode = regHierarchy.GetNthChildNode(i)
      self.computeInverseConsistency(phaseHierarchyNode, resample, patientName)

    print "Finished inverse Consistency dirqa"
  
  def computeInverseConsistency(self,phaseHierarchyNode, roiNode = None, resample = [], patientName = ""):
      
      try:
         DIRQALogic = slicer.modules.registrationquality.logic()
      except AttributeError:
         import sys
         sys.stderr.write('Cannot find registrationquality module')
         return
      
      if not phaseHierarchyNode:
         print "No phase node"
         return
      
      if self.checkIfStatisticsExist(phaseHierarchyNode, NAME_INVCONSIST):
         return
      
      
      
      vectorNode = self.getVolumeFromChild(phaseHierarchyNode,NAME_VECTOR)
      invVectorNode = self.getVolumeFromChild(phaseHierarchyNode,NAME_INVVECTOR)
      
      if vectorNode is None or invVectorNode is None:
        print "No vector for phase " + phaseHierarchyNode.GetNameWithoutPostfix()
        return
      
     
      if 1:
         vectorNode.SetOrigin(0,0,0)
         invVectorNode.SetOrigin(0,0,0)

      if len(resample) == 3:
         vectorHierarchyNode = phaseHierarchyNode.GetChildWithName(phaseHierarchyNode,NAME_VECTOR)
         invVectorHierarchyNode = phaseHierarchyNode.GetChildWithName(phaseHierarchyNode,NAME_INVVECTOR)
         vectorNode = self.resampleVolume(vectorNode,resample, vectorHierarchyNode)
         invVectorNode = self.resampleVolume(invVectorNode,resample, invVectorHierarchyNode)

      invConsistHierarchy = None
      
      invConsistHierarchy = phaseHierarchyNode.GetChildWithName(phaseHierarchyNode,NAME_INVCONSIST)
      if invConsistHierarchy:
         invConsistNode = self.getVolumeFromChild(phaseHierarchyNode,NAME_INVCONSIST)
      else:
          invConsistNode = DIRQALogic.InverseConsist(vectorNode,invVectorNode,roiNode)
          invConsistHierarchy = self.createChild(phaseHierarchyNode,NAME_INVCONSIST)
          self.setParentNodeFromHierarchy(invConsistNode, invConsistHierarchy)
          
      if invConsistNode:
            #if not removeOn:
              #invConsistHierarchy.SetAssociatedNodeID(invConsistNode.GetID())
            #Statistics
            statisticsArray = [0,0,0,0]
            DIRQALogic.CalculateStatistics(invConsistNode,statisticsArray)
            self.writeStatistics(invConsistHierarchy,statisticsArray)
      else:
            print "Can't compute Inverse Consistency."
          
      if len(patientName) > 0:
        directory = '/u/kanderle/InvCon'
        filePath = directory + "/InvCon_" + patientName + "_phase" + phaseHierarchyNode.GetAttribute("PhaseNumber") + "ref" + str(referenceNumber) + ".nhdr"
        if not slicer.util.saveNode(invConsistNode,filePath):
          print "Cannot save " + filePath

      self.removeHierarchyNode(invConsistNode)
      
  def computeMaxVectorValues(self,phaseHierarchyNode, roiNode = None):
      
      if self.checkIfStatisticsExist(phaseHierarchyNode, NAME_VECTOR):
	return
      
      vectorNode = self.getVolumeFromChild(phaseHierarchyNode,NAME_VECTOR)
      invVectorNode = self.getVolumeFromChild(phaseHierarchyNode,NAME_INVVECTOR)
      
      percentNode = self.createChild( phaseHierarchyNode, NAME_PERCENT)
      invPercentNode = self.createChild( phaseHierarchyNode, NAME_INVPERCENT)
      
      if vectorNode is None or invVectorNode is None:
        print "No vector for phase " + phaseHierarchyNode.GetNameWithoutPostfix()
        return
      
      arrayVector = slicer.util.array(vectorNode.GetID())
      arrayInvVector = slicer.util.array(invVectorNode.GetID())
      
      dim = arrayVector.shape
      dimInv = arrayInvVector.shape

      if not dim == dimInv:
	print "Vector and Inv Vector have different dimensions."
	return
      
      bounds = np.zeros([3,2])
      if roiNode:
	roiBounds = [0,0,0,0,0,0]
	roiNode.GetRASBounds(roiBounds)
	
	vectorBounds = [ [0,0,0,0],
		         [0,0,0,0]]
	
	matrix = vtk.vtkMatrix4x4()
	vectorNode.GetRASToIJKMatrix(matrix)
	vectorBounds[0][:] = matrix.MultiplyPoint([roiBounds[0],roiBounds[2],roiBounds[4],1])
	vectorBounds[1][:] = matrix.MultiplyPoint([roiBounds[1],roiBounds[3],roiBounds[5],1])
					     
	for i in range(3):
	  if vectorBounds[0][i] == vectorBounds[1][i]:
	    print "Error: " + str(vectorBounds)
	    return
	  
	  bounds[i][0] = int(vectorBounds[0][2-i]+0.5)
	  bounds[i][1] = int(vectorBounds[1][2-i]+0.5)
	    
	  bounds[i][:] = np.sort(bounds[i])
	    
	  if bounds[i][0] < 0:
	    bounds[i][0] = 0
	  if bounds[i][1] > dim[i]:
	    bounds[i][1] = dim[i]
	    
	    
      else:
	bounds[:,1] = dim
      
      data = np.zeros([arrayVector.size/3,2])
      
      percentile = np.zeros([4,2])
      bounds =  bounds.astype('int')
      
      n = 0
      for z in range(bounds[0][0],bounds[0][1]):
	print "Slice " + str(z+1) + " of " + str(bounds[0][1])
	for y in range(bounds[1][0],bounds[1][1]):
	  for x in range(bounds[2][0],bounds[2][1]):
	    data[n,0] = np.linalg.norm(arrayVector[z,y,x,:])
	    data[n,1] = np.linalg.norm(arrayInvVector[z,y,x,:])
	    
	    for i in range(dim[3]):
              value = abs(arrayVector[z,y,x,i])/data[n,0]
	      percentile[i,0] += value * value
	      value = abs(arrayInvVector[z,y,x,i])/data[n,1]
	      percentile[i,1] += value * value
	    n += 1

	    
      percentile /= n

      self.writeStatistics(percentNode,percentile[:,0],False)
      self.writeStatistics(invPercentNode,percentile[:,1],False)
      
      statistics = [0,0,0,0]
      statistics[0] = data[:,0].mean()
      statistics[1] = data[:,0].std()
      statistics[2] = data[:,0].max()
      statistics[3] = data[:,0].min()
      
      #print statistics

      #stats = vtk.vtkImageAccumulate()
      #stats.SetInputData(vectorNode.GetImageData())
      #stats.Update()
      #statistics = [stats.GetMean(),stats.GetStandardDeviation(),stats.GetMax(),stats.GetMin()]
      
      self.writeStatistics(phaseHierarchyNode.GetChildWithName(phaseHierarchyNode,NAME_VECTOR),statistics,False)
      
      #stats = vtk.vtkImageAccumulate()
      #stats.SetInputData(invVectorNode.GetImageData())
      #stats.Update()
      #statistics = [stats.GetMean(),stats.GetStandardDeviation(),stats.GetMax(),stats.GetMin()]
      statistics[0] = data[:,1].mean()
      statistics[1] = data[:,1].std()
      statistics[2] = data[:,1].max()
      statistics[3] = data[:,1].min()
      
      #print statistics
      
      self.writeStatistics(phaseHierarchyNode.GetChildWithName(phaseHierarchyNode,NAME_INVVECTOR),statistics,False)
  
  def computeJacobian(self,phaseHierarchyNode, roiNode = None, jacConsist = False, patientName = ""):
     try:
        DIRQALogic = slicer.modules.registrationquality.logic()
     except AttributeError:
        import sys
        sys.stderr.write('Cannot find registrationquality module')
        return
      
     if not phaseHierarchyNode:
        print "No phase node"
        return
         
     if self.checkIfStatisticsExist(phaseHierarchyNode, NAME_JACOBIAN):
       if jacConsist and self.checkIfStatisticsExist(phaseHierarchyNode, NAME_JACCONSIST):
         return
     
     vectorNode = self.getVolumeFromChild(phaseHierarchyNode,NAME_VECTOR)
     invVectorNode = self.getVolumeFromChild(phaseHierarchyNode,NAME_INVVECTOR)
      
     if vectorNode is None or invVectorNode is None:
        print "No vector for phase " + phaseHierarchyNode.GetNameWithoutPostfix()
        return
           
     jacobianNode = self.getVolumeFromChild(phaseHierarchyNode,NAME_JACOBIAN)
     if not jacobianNode:
        jacobianNode = DIRQALogic.Jacobian(vectorNode,roiNode)
        jacobianHierarchy = self.createChild(phaseHierarchyNode,NAME_JACOBIAN)
        self.setParentNodeFromHierarchy(jacobianNode,jacobianHierarchy)
     if jacobianNode:
          #Statistics
          statisticsArray = [0,0,0,0]
          DIRQALogic.CalculateStatistics(jacobianNode,statisticsArray)
          self.writeStatistics(jacobianHierarchy,statisticsArray)
     else:
          print "Can't compute Jacobian."
     
     #print str(statisticsArray) + " to " + jacobianHierarchy.GetName()
         
     
     invJacobianNode = self.getVolumeFromChild(phaseHierarchyNode,NAME_INVJACOBIAN)
     if not invJacobianNode:
        invJacobianNode = DIRQALogic.Jacobian(invVectorNode,roiNode)
        invJacobianHierarchy = self.createChild(phaseHierarchyNode,NAME_INVJACOBIAN)
        self.setParentNodeFromHierarchy(invJacobianNode, invJacobianHierarchy)
     if invJacobianNode:
          #Statistics
          statisticsArray = [0,0,0,0]
          DIRQALogic.CalculateStatistics(invJacobianNode,statisticsArray)
          self.writeStatistics(invJacobianHierarchy,statisticsArray)
     else:
          print "Can't compute Inverse Jacobian."
     
     if len(patientName) > 0:
        directory = '/u/kanderle/InvCon'
        filePath = directory + "/InvCon_" + patientName + "_invJacobian.nhdr"
        if not slicer.util.saveNode(invJacobianNode,filePath):
          print "Cannot save " + filePath
     
     #print str(statisticsArray) + " to " + invJacobianHierarchy.GetName()
     if jacConsist:
       if invJacobianNode and jacobianNode:
	 
	 arrayJacobian = slicer.util.array(jacobianNode.GetID())
	 arrayInvJacobian = slicer.util.array(invJacobianNode.GetID())
	 
	 arrayJacobian[:] =abs(arrayJacobian[:] - 1/arrayInvJacobian[:])
	 statisticsArray = [0,0,0,0]
	 statisticsArray[0] = arrayJacobian.mean()
	 statisticsArray[1] = arrayJacobian.std()
	 statisticsArray[2] = arrayJacobian.max()
	 statisticsArray[3] = arrayJacobian.min()
	 print "Jacobian consistency for " + jacobianNode.GetName() + " " + str(statisticsArray)
	 
	 jacConsistHierarchy = self.createChild(phaseHierarchyNode,NAME_JACCONSIST)
	 self.writeStatistics(jacConsistHierarchy,statisticsArray)
	 
       else:
	 print "Can't get jacobian nodes"
	 
     
     self.removeHierarchyNode(jacConsistHierarchy)
     self.removeHierarchyNode(jacobianNode) 
     self.removeHierarchyNode(invJacobianNode)

  
  def computeAbsDifference(self,phaseHierarchyNode, refNode, roiNode = None, resample = []):
     try:
        DIRQALogic = slicer.modules.registrationquality.logic()
     except AttributeError:
        import sys
        sys.stderr.write('Cannot find registrationquality module')
        return
      
     if not phaseHierarchyNode or not refNode:
        print "No phase node or reference node"
        return
         
     
     if self.checkIfStatisticsExist(phaseHierarchyNode, NAME_ABSDIFF):
        return
     
     transform = None
     invTransform = None
     
     vectorNode = self.getVolumeFromChild(phaseHierarchyNode,NAME_VECTOR)
     invVectorNode = self.getVolumeFromChild(phaseHierarchyNode,NAME_INVVECTOR)
     phaseNode = self.getVolumeFromChild(phaseHierarchyNode,NAME_CT)
     
     phaseNode.SetOrigin([0,0,0])

     
     if len(resample) == 3:
        phaseHierarchy = phaseHierarchyNode.GetChildWithName(phaseHierarchyNode,NAME_CT)
        phaseNode = self.resampleVolume(phaseNode, resample, phaseHierarchy)
      
     if vectorNode is None or invVectorNode is None or phaseNode is None:
        print "No vector for phase " + phaseHierarchyNode.GetNameWithoutPostfix()
        return

     
     absDiffNode = self.getVolumeFromChild(phaseHierarchyNode,NAME_ABSDIFF)
     if not absDiffNode:
       warpNode = self.getVolumeFromChild(phaseHierarchyNode,NAME_WARP)
       
       if len(resample) == 3 and warpNode:
        warpHierarchy = phaseHierarchyNode.GetChildWithName(phaseHierarchyNode,NAME_WARP)
        warpNode = self.resampleVolume(warpNode, resample, warpHierarchy)

       
       if not warpNode:
         #Get transform
         transform = DIRQALogic.CreateTransformFromVector(vectorNode)
         warpNode = DIRQALogic.GetWarpedFromMoving(phaseNode, transform)
         
         if not warpNode:
            print "Can't create warped Image"
            return
         
       warpNode.SetOrigin([0,0,0])
       
       absDiffNodeWarp = DIRQALogic.AbsoluteDifference(refNode,warpNode,roiNode)
       absDiffNodePhase = DIRQALogic.AbsoluteDifference(refNode,phaseNode,roiNode)
       if absDiffNodeWarp and absDiffNodePhase:
         #TODO: Use ITK filters
         #arrayWarp = slicer.util.array(absDiffNodeWarp.GetID())
         #arrayPhase = slicer.util.array(absDiffNodePhase.GetID())
        
         ##Find relative change of Warped image
         #arrayWarp[:] = arrayPhase[:] - arrayWarp[:]
         #maxArray = arrayWarp.max()
         #minArray = arrayWarp.min()
         #normFactor = 100 / ( maxArray - minArray)
         #arrayWarp[:] = (arrayWarp[:] - minArray) * normFactor
         #absDiffNodeWarp.GetImageData().Modified()
        
         absDiffHierarchy = self.createChild(phaseHierarchyNode,NAME_ABSDIFF)
         defabsDiffHierarchy = self.createChild(phaseHierarchyNode,NAME_DEFABS)
         self.setParentNodeFromHierarchy(absDiffNodeWarp, absDiffHierarchy)
         self.setParentNodeFromHierarchy(absDiffNodePhase, defabsDiffHierarchy)
         #Statistics
         statisticsArrayWarp = [0,0,0,0]
         statisticsArrayPhase = [0,0,0,0]
         DIRQALogic.CalculateStatistics(absDiffNodeWarp,statisticsArrayWarp)
         DIRQALogic.CalculateStatistics(absDiffNodePhase, statisticsArrayPhase)
         
         print statisticsArrayWarp
         print statisticsArrayPhase
         
         self.writeStatistics(absDiffHierarchy,statisticsArrayWarp)
         self.writeStatistics(defabsDiffHierarchy,statisticsArrayPhase)
         ##Compare mean and STD with phase hierarchy
         #for i in range(0,2):
           #if not statisticsArrayPhase[i] == 0:
             #statisticsArrayWarp[i] = 1 - (statisticsArrayWarp[i]/statisticsArrayPhase[i])
        
       else:
         print "Can't compute Absolute Difference."
     
     print "Statis: " + str(statisticsArrayPhase)
     print "sta: " + str(statisticsArrayWarp)
     self.removeHierarchyNode(absDiffNodeWarp)
     self.removeHierarchyNode(absDiffNodePhase)
     if transform:
        self.removeHierarchyNode(transform)
     self.removeHierarchyNode(warpNode)

     invAbsDiffNode = self.getVolumeFromChild(phaseHierarchyNode,NAME_INVABSDIFF)
     if not absDiffNode:
       invWarpNode = self.getVolumeFromChild(phaseHierarchyNode,NAME_INVWARP)
       if len(resample) == 3 and invWarpNode:
	print "Loaded " + invWarpNode.GetName()
        invWarpHierarchy = phaseHierarchyNode.GetChildWithName(phaseHierarchyNode,NAME_INVWARP)
        invWarpNode = self.resampleVolume(invWarpNode, resample, invWarpHierarchy)
       if not invWarpNode:
         #Get transform
         invTransform = DIRQALogic.CreateTransformFromVector(invVectorNode)
         invWarpNode = DIRQALogic.GetWarpedFromMoving(refNode, invTransform)
         
         if not invWarpNode:
            print "Can't create warped Image"
            return
         
       invWarpNode.SetOrigin([0,0,0])
       
       invAbsDiffNodeWarp = DIRQALogic.AbsoluteDifference(phaseNode,invWarpNode,roiNode)
       if invAbsDiffNodeWarp:

         invAbsDiffHierarchy = self.createChild(phaseHierarchyNode,NAME_INVABSDIFF)
         self.setParentNodeFromHierarchy(invAbsDiffNodeWarp,invAbsDiffHierarchy)
         #Statistics
         statisticsArrayWarp = [0,0,0,0]
         DIRQALogic.CalculateStatistics(invAbsDiffNodeWarp,statisticsArrayWarp)
         
         print statisticsArrayWarp
         
         self.writeStatistics(invAbsDiffHierarchy,statisticsArrayWarp)
        
       else:
         print "Can't compute Inverse Absolute Difference."
         
     print "Stat " + str(statisticsArrayWarp)
     
     self.removeHierarchyNode(invAbsDiffNodeWarp)
     if invTransform:
        self.removeHierarchyNode(invTransform)
     self.removeHierarchyNode(invWarpNode)
     
     self.removeHierarchyNode(phaseNode)

  def calculateInvConStatistics(self, regHierarchy, labelMap):
    
    import LabelStatistics
    
    referenceNumber = regHierarchy.GetAttribute("ReferenceNumber")
    #if not refPhaseNode:
      #print "Can't get reference node."
      #return
      
    patientName = regHierarchy.GetAttribute("PatientName")
    volumesLogic = slicer.modules.volumes.logic()
    output_str = ("DIRQA Inverse Consistency for: " + regHierarchy.GetNameWithoutPostfix() + 
    " at " + labelMap.GetName() +" \n")
   
    nPhases = regHierarchy.GetNumberOfChildrenNodes()
    if nPhases < 1:
      print "No children nodes."
      return

    for i in range(0,nPhases):
      if i == referenceNumber:
        continue
      phaseHierarchyNode = regHierarchy.GetNthChildNode(i)
      invConNode = self.getVolumeFromChild(phaseHierarchyNode,NAME_INVCONSIST)

      if invConNode is None:
        print "Can't load inverse consistency for phase " + str(i)
        continue
      
      warnings = volumesLogic.CheckForLabelVolumeValidity(invConNode, labelMap)
      resampledLabelNode = None
      if warnings != "":
	if 'mismatch' in warnings:
	  resampledLabelNode = volumesLogic.ResampleVolumeToReferenceVolume(labelMap, invConNode)
	  # resampledLabelNode does not have a display node, therefore the colorNode has to be passed to it
	  stats = LabelStatistics.LabelStatisticsLogic(invConNode, resampledLabelNode, colorNode=labelMap.GetDisplayNode().GetColorNode(), nodeBaseName=labelMap.GetName())
	else:
	   qt.QMessageBox.warning(slicer.util.mainWindow(),
            "Label Statistics", "Volumes do not have the same geometry.\n%s" % warnings)
           return
      else:
	stats = LabelStatistics.LabelStatisticsLogic(invConNode, labelMap)

      if resampledLabelNode:
	slicer.mrmlScene.RemoveNode(resampledLabelNode)
	
      #for i in stats.labelStats["Labels"]:
	#for k in stats.keys:
	  #print stats.labelStats[i,k]
      #print "Phase " + str(i)
      #print stats.labelStats
      output_str += "Phase " + str(i) +"\n"
      output_str += "Mean " + str(round(stats.labelStats[1,'Mean'],2))+"\n"
      output_str += "STD " + str(round(stats.labelStats[1,'StdDev'],2))+"\n"
      output_str += "Max " + str(round(stats.labelStats[1,'Max'],2))+"\n"
      output_str += "Min " + str(round(stats.labelStats[1,'Min'],2))+"\n"
     
    directoryPath = '/u/kanderle/InvCon/'
    filePath = directoryPath + patientName + '_InvConStatsfor_' + labelMap.GetName()
    f = open(filePath,"wb+")
    f.write(output_str)
    f.close()
    print output_str
    print "Wrote dirqa data to: " + filePath
       
     

  
  #Function to set vector values from TRiP to world
  def setVectorField(self, vectorNode, resampleTRiP = False):
    spacing = vectorNode.GetSpacing()
    vectorArray = slicer.util.array(vectorNode.GetID())
    
    for i in range(0,3):
      #Special case, because resampling cube with trip doesn't correct values
      if resampleTRiP:
         vectorArray[:,:,:,i] = vectorArray[:,:,:,i] * spacing[i] / 2
      else:
         vectorArray[:,:,:,i] = vectorArray[:,:,:,i] * spacing[i]
    vectorNode.GetImageData().Modified()
  
  def writeData(self,regHierarchy):
    output_str = "DIRQA for: " + regHierarchy.GetNameWithoutPostfix() + " \n"
    for phase in range(0,regHierarchy.GetNumberOfChildrenNodes()):
      phaseNode = regHierarchy.GetNthChildNode(phase)
      if phaseNode.GetNumberOfChildrenNodes() > 0:
        output_str += "Phase " + str(phase) + "\n"
	for node in range(0,phaseNode.GetNumberOfChildrenNodes()):
	  dirqaNode = phaseNode.GetNthChildNode(node)
	  dirqaName = dirqaNode.GetNameWithoutPostfix()
	  #Look for statistics:
	  stringList = []
	  for check in STATISTIC_LIST:
	    if check == dirqaName:
	      stringList = ["Mean","STD","Max","Min"]
		  
	  if dirqaName == NAME_VECTOR or dirqaName == NAME_INVVECTOR:
	    directionList = ["x","y","z"]
	    if stringList and dirqaNode.GetAttribute(stringList[0] + directionList[0]):
	      output_str += dirqaName + " \n"
              for i in range(0,len(directionList)):
                for j in range(len(stringList)):
                 output_str += stringList[j] + '_' + directionList[i] + ': ' + dirqaNode.GetAttribute(stringList[j]+directionList[i]) + " "
                 output_str += " \n"
            #if stringList and dirqaNode.GetAttribute(stringList[0]):
              #output_str += stringList[i] + '_abs: ' + dirqaNode.GetAttribute(stringList[i]) + " "
              #output_str += " \n"
	  
	  if stringList and dirqaNode.GetAttribute(stringList[0]):
            if dirqaName == NAME_VECTOR or dirqaName == NAME_INVVECTOR:
               output_str += "Abs" + dirqaName + "\n"
            else:
               output_str += dirqaName + " \n"
	    for i in range(0,len(stringList)):
	      if not dirqaNode.GetAttribute(stringList[i]):
	        continue
	      output_str += stringList[i] + ': ' + dirqaNode.GetAttribute(stringList[i]) + " "
	      output_str += " \n"
	output_str += "\n"
    directoryPath = regHierarchy.GetAttribute("DIR" + NAME_DIRQA)
    #print output_str
    if not directoryPath or os.path.isdir(directoryPath) == False:
      print "Can't get Dirqa directory."
      return
    filePath = directoryPath + 'DirqaData.txt'
    f = open(filePath,"wb+")
    f.write(output_str)
    f.close()
    print "Wrote dirqa data to: " + filePath
    
  def writeStatistics(self,hierarchyNode,statisticsArray, vector = False):
    stringList = ["Mean","STD","Max","Min"]
    direction = ["x","y","z"]
    if len(stringList) > len(statisticsArray):
      print "Cannot write statistics, not enough data."
      return
    for i in range(0,len(stringList)):
      if vector:
         for j in range(len(direction)):
           hierarchyNode.SetAttribute(stringList[i] + direction[j],str(round(statisticsArray[i][j],2)))
      else:
         hierarchyNode.SetAttribute(stringList[i],str(round(statisticsArray[i],2)))

  def writeInvConData(self,regHierarchy):
    output_str = "DIRQA for Inverse Consistency\n"
    for i in range(0,regHierarchy.GetNumberOfChildrenNodes()):
      phaseNode = regHierarchy.GetNthChildNode(i)
      if phaseNode.GetNumberOfChildrenNodes() > 0:
        for j in range(0,phaseNode.GetNumberOfChildrenNodes()):
          dirqaNode = phaseNode.GetNthChildNode(j)
          dirqaName = dirqaNode.GetNameWithoutPostfix()
          #Look for statistics:
          stringList = []
          stringList = ["Mean","STD","Max","Min"]
          if stringList and dirqaNode.GetAttribute(stringList[0]):
            output_str += "Phase " +str(i) + " \n"
            for i in range(0,len(stringList)):
              if not dirqaNode.GetAttribute(stringList[i]):
                continue
              output_str += stringList[i] + ': ' + dirqaNode.GetAttribute(stringList[i]) + " "
              output_str += " \n"
          #output_str += " \n"
    directoryPath = '/u/kanderle/InvCon/'
    print output_str
    if not directoryPath:
      print "Can't get Dirqa directory."
      return
    filePath = directoryPath + 'DirqaData_' + regHierarchy.GetAttribute('PatientName') + '.txt'
    f = open(filePath,"wb+")
    f.write(output_str)
    f.close()
    print "Wrote dirqa data to: " + filePath  
  
  def createChild(self,hierarchyNode,string):
    newHierarchy = slicer.vtkMRMLSubjectHierarchyNode()
    newHierarchy.SetParentNodeID(hierarchyNode.GetID())
    newHierarchy.SetName(string)
    newHierarchy.SetLevel('Subseries')
    #TODO: Addd directories
    #newHierarchy.SetAttribute('FilePath',ctDirectory+fileName)
    #newHierarchy.SetOwnerPluginName('Volumes')
    slicer.mrmlScene.AddNode(newHierarchy)
    return newHierarchy
  
  def showVolumeFromHierarchyNode(self,hierarchyNode):
    
    volume = self.loadVolumeFromHierarchyNode(hierarchyNode)

    if not volume:
      print "No volume"
      return
      
    if volume.IsA('vtkMRMLVolumeNode'):
      if not volume.GetDisplayNodeID():
	displayNode = None
        if volume.IsA('vtkMRMLScalarVolumeNode'):
	  displayNode = slicer.vtkMRMLScalarVolumeDisplayNode()
          displayNode.SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey")
        if volume.IsA('vtkMRMLVectorVolumeNode'):
	  displayNode = slicer.vtkMRMLVectorVolumeDisplayNode()
          displayNode.SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey")
        if displayNode:
	  slicer.mrmlScene.AddNode(displayNode)
	  volume.SetAndObserveDisplayNodeID(displayNode.GetID())
      else:
	volume.GetDisplayNode().SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey")
      selectionNode = slicer.app.applicationLogic().GetSelectionNode()
      selectionNode.SetReferenceActiveVolumeID(volume.GetID())
      slicer.app.applicationLogic().PropagateVolumeSelection(0)
  
  def getReferencePhaseFromHierarchy(self,hierarchyNode):
    
    referenceHierarchyNode = self.getReferenceHierarchy(hierarchyNode)
      
    referenceNode = self.getVolumeFromChild(referenceHierarchyNode,NAME_CT)
    if not referenceNode:
      print "Can't load reference volume from: " + referenceHierarchyNode.GetNameWithoutPostfix()
      return None

    return referenceNode
  
  def getReferenceHierarchy(self, hierarchyNode):
    #Find out reference phase
    referenceID = hierarchyNode.GetAttribute(NAME_REFPHASE)
    if not referenceID:
      print "Can't find reference node from: " + hierarchyNode.GetNameWithoutPostfix()
      return None
      
    referenceHierarchyNode = slicer.util.getNode(referenceID)
    if not referenceHierarchyNode:
      print "Can't get reference Hierarchy node"
      return None
    return referenceHierarchyNode
     
  
  def checkIfStatisticsExist(self, hierarchyNode, string):
     childNode = hierarchyNode.GetChildWithName(hierarchyNode,string)
     if not childNode:
        return False
     meanAtt = childNode.GetAttribute('Mean')
     if meanAtt:
        return True
     else:
        return False
        
  def loadAllChildren(self,hierarchyNode,string):
    for i in range(0,hierarchyNode.GetNumberOfChildrenNodes()):
       volume = self.getVolumeFromChild(hierarchyNode.GetNthChildNode(i),string)
  
  def getVolumeFromChild(self,hierarchyNode,string):
    volume = None
    childNode = hierarchyNode.GetChildWithName(hierarchyNode,string)
    if not childNode:
      print "Can't get childNode: " + string + " from " + hierarchyNode.GetNameWithoutPostfix()
      return None
    volume = self.loadVolumeFromHierarchyNode(childNode)
    if not volume:
       print "Can't load  " + string + " from hierarchy childNode: " + childNode.GetNameWithoutPostfix()
       return None
    return volume

  #Loads volume from hierarchyNode. Can find it there or tries to load it from disk.
  def loadVolumeFromHierarchyNode(self,hierarchyNode):
     #Look for existing associated nodes: 
    if hierarchyNode.GetNumberOfChildrenNodes() > 0:
      volume = hierarchyNode.GetNthChildNode(0).GetAssociatedNode()
      if volume:
	return volume

    filePath = hierarchyNode.GetAttribute('FilePath')
    volume = None
    if filePath:
      #Special case for ctx files
      if filePath.find('ctx') > -1:
	import LoadCTX
        loadLogic = LoadCTX.LoadCTXLogic()
        volume = slicer.util.getNode(loadLogic.loadCube(filePath,0))
        if volume:
	   #Write it in hierarchy node for later use
	  hierarchyNode.SetAssociatedNodeID(volume.GetID())
	  return volume
      elif filePath.find('cbt') > -1:
	import LoadCTX
        loadLogic = LoadCTX.LoadCTXLogic()
        print filePath
        print loadLogic.loadCube(filePath,3)
        volume = slicer.util.getNode(loadLogic.loadCube(filePath,3))
        if volume:
	   #Write it in hierarchy node for later use
	  hierarchyNode.SetAssociatedNodeID(volume.GetID())
	  return volume
      else:
	volumesLogic = slicer.vtkSlicerVolumesLogic()
	volumesLogic.SetMRMLScene(slicer.mrmlScene)
	slicerVolumeName = os.path.splitext(os.path.basename(filePath))[0]
	volume = volumesLogic.AddArchetypeVolume(filePath,slicerVolumeName)
	if not volume:
	  print "Can't load volume " + os.path.basename(filePath)
	  return None
	
	#Find hierarchy
	self.setParentNodeFromHierarchy(volume, hierarchyNode)
    else:
      print "Can't get file Path from: " + hierarchyNode.GetNameWithoutPostfix()
      return None

    return volume

  def setParentNodeFromHierarchy(self, volume, hierarchyNode):
     if not volume or not hierarchyNode:
        print "No input volume or hierarchy node"
        return
     #Loop through all subject hierarchies if it exist
     subjectHierarchyList = slicer.util.getNodes('vtkMRMLSubjectHierarchyNode*')
     for subjectHierarchy in subjectHierarchyList:
        if subjectHierarchyList[subjectHierarchy].GetAssociatedNodeID() == volume.GetID():
           subjectHierarchyList[subjectHierarchy].SetParentNodeID(hierarchyNode.GetID())
           return
           
     #Create, if it doesn't exist
     volumeHierarchy = self.createChild(hierarchyNode, volume.GetName())
     volumeHierarchy.SetAssociatedNodeID(volume.GetID()) 
  
  def removeHierarchyNode(self, volume):
     subjectHierarchyList = slicer.util.getNodes('vtkMRMLSubjectHierarchyNode*')
     #Loop through all subject hierarchies, delete if you find the right one
     for subjectHierarchy in subjectHierarchyList:
        if subjectHierarchyList[subjectHierarchy].GetAssociatedNodeID() == volume.GetID():
           slicer.mrmlScene.RemoveNode(subjectHierarchyList[subjectHierarchy])
           slicer.mrmlScene.RemoveNode(volume)
           return
  
  #Save node to disk and write it to subject hierarchy
  def saveAndWriteNode(self,node,hierarchyNode,string,filePath,cbtOn = False,resample = []):
    if not node or not hierarchyNode or not string:
      print "Not enough input parameters."
      return False
      
    if 0:
      childNode = self.createChild(hierarchyNode,string)
      childNode.SetAttribute("FilePath",filePath)
    
    directory = os.path.dirname(os.path.realpath(filePath))
    if not os.path.exists(directory):
      print "No path: " + directory
      return False
      
    print "Saving " + node.GetName()
    #Special Case
    if cbtOn:
      #-----Convert transform node into vector node
      if node.IsA('vtkMRMLGridTransformNode'):
	print "Converting Transform Node to Vector Field"
	trans = node.GetTransformFromParent()#.GetConcatenatedTransform(0)
	#trans.Inverse()
	#trans.Update()
	im = trans.GetDisplacementGrid()
	if im is None:
	  print "Can't get image data. " + str(im)
	  return
	  
	vectorNode = slicer.vtkMRMLVectorVolumeNode()
	slicer.mrmlScene.AddNode( vectorNode )
	vectorNode.SetAndObserveImageData( im )
	vectorNode.SetName( node.GetName() )
	vectorNode.SetSpacing( im.GetSpacing() )
	
	#vectorNode.SetOrigin( im.GetOrigin() )
	
	#Get Right Direction For Vector Volume
	#matrix = vtk.vtkMatrix4x4()
	matrix = trans.GetGridDirectionMatrix()
        #matrix.DeepCopy((-1,0,0,0,0,-1,0,0,0,0,1,0,0,0,0,1))
        vectorNode.SetIJKToRASDirectionMatrix(matrix)

        if not resample == []:
	  newVectorNode = self.resampleVolume(vectorNode,resample)
	  slicer.mrmlScene.RemoveNode(vectorNode)
	  vectorNode = newVectorNode

      #----- Save vector node ----
      spacing = vectorNode.GetSpacing()
      #Changes values due to TRiP demands
      vectorArray = slicer.util.array(vectorNode.GetID())
      for i in range(0,3):
        vectorArray[:,:,:,i] = vectorArray[:,:,:,i] / spacing[i]
      vectorNode.GetImageData().Modified()
      
      scalarNode = slicer.vtkMRMLScalarVolumeNode()
      slicer.mrmlScene.AddNode( scalarNode )
      
      storageNode = scalarNode.CreateDefaultStorageNode()
      storageNode.SetUseCompression(0)
      slicer.mrmlScene.AddNode( storageNode )
      scalarNode.SetAndObserveStorageNodeID(  storageNode.GetID() )

      scalarNode.SetOrigin(vectorNode.GetOrigin())
      scalarNode.SetSpacing(spacing)

      ijkToRAS = vtk.vtkMatrix4x4()
      vectorNode.GetIJKToRASMatrix(ijkToRAS)
      scalarNode.SetIJKToRASMatrix(ijkToRAS)

      extract = vtk.vtkImageExtractComponents()
      extract.SetInput(vectorNode.GetImageData())

      names = [vectorNode.GetName() + "_x",vectorNode.GetName() + "_y",vectorNode.GetName() + "_z"]
      
      for i in range(0,3):
        print "Saving vector field " + names[i]
        scalarNode.SetName(names[i])
        extract.SetComponents(i)
        extract.Update()
        scalarNode.SetAndObserveImageData( extract.GetOutput() )
        filePath = directory + "/" + scalarNode.GetName() + ".nhdr"
        if not slicer.util.saveNode(scalarNode,filePath):
	  print "Cannot save " + filePath
        
      #-----Remove nodes to clear memory
      slicer.mrmlScene.RemoveNode(vectorNode)
      slicer.mrmlScene.RemoveNode(scalarNode)
      
      return True
    
    if slicer.util.saveNode(node,filePath):
      #childNode.SetAttribute("FilePath",filePath)
      return True

  
  def loadRoi(self, regHierarchy):
     annotationLogic = slicer.modules.annotations.logic()
     name = regHierarchy.GetAttribute('PatientName') + "_roi"
     filePath = regHierarchy.GetAttribute(NAME_ROIFILEPATH)
     nodeID = annotationLogic.LoadAnnotation(filePath, name, 3)
     if not nodeID:
        print "Can't load " + filePath + " as ROI."
        return None
     node = slicer.util.getNode(nodeID)
     if node:
        print "Loaded ROI from " + filePath
        return node
     else:
        return None
  
  
  def resampleVolume(self,volumeNode,resample, parentNode = None):
      if not volumeNode:
        print "No node for resampling."
        return
      
      if resample == []:
	print "No resample values."
	return
	
      if not len(resample) == 3:
        print "Too many values for resampling."
        return
      
      oldVolumeNode = volumeNode
      
      #Create new vector volume
      if volumeNode.IsA('vtkMRMLVectorVolumeNode'):
         newVolumeNode = slicer.vtkMRMLVectorVolumeNode()
      elif volumeNode.IsA('vtkMRMLScalarVolumeNode'):
         newVolumeNode = slicer.vtkMRMLScalarVolumeNode()
      else:
         print "Unknown type of " + volumeNode.GetID()
         return None
         
      newVolumeNode.SetName(oldVolumeNode.GetName())
      slicer.mrmlScene.AddNode(newVolumeNode)
      
      newStorageNode = newVolumeNode.CreateDefaultStorageNode()
      newVolumeNode.SetAndObserveStorageNodeID(newStorageNode.GetID())
      
      #Create strings for resampling
      spacing = ''
      size = ''
      for i in range(0,len(resample)):
        spacing += str(oldVolumeNode.GetSpacing()[i]*resample[i])
        #extent = oldVolumeNode.GetImageData().GetExtent[2*i+1]
        extent = oldVolumeNode.GetImageData().GetExtent()[2*i+1]+1
        size += str(extent/resample[i])
        if i < 2:
	  spacing += ','
	  size += ','

      print "Resampling " + oldVolumeNode.GetName() + " to new pixel size " + size 
      
      #Set parameters
      parameters = {} 
      parameters["inputVolume"] = oldVolumeNode.GetID()
      parameters["outputVolume"] = newVolumeNode.GetID()
      parameters["referenceVolume"] = ''
      parameters["outputImageSpacing"] = spacing
      parameters["outputImageSize"] = size
      
      #Do resampling
      resampleScalarVolume = slicer.modules.resamplescalarvectordwivolume
      clNode = slicer.cli.run(resampleScalarVolume, None, parameters, wait_for_completion=True)
      
      #Remove old vector node and set new:
      slicer.mrmlScene.RemoveNode(oldVolumeNode)
      
      if parentNode:
        self.setParentNodeFromHierarchy(newVolumeNode,parentNode)
         
      return newVolumeNode

class RegistrationHierarchyTest(unittest.TestCase):
  """
  This is the test case for your scripted module.
  """

  def delayDisplay(self,message,msec=1000):
    """This utility method displays a small dialog and waits.
    This does two things: 1) it lets the event loop catch up
    to the state of the test so that rendering and widget updates
    have all taken place before the test continues and 2) it
    shows the user/developer/tester the state of the test
    so that we'll know when it breaks.
    """
    print(message)
    self.info = qt.QDialog()
    self.infoLayout = qt.QVBoxLayout()
    self.info.setLayout(self.infoLayout)
    self.label = qt.QLabel(message,self.info)
    self.infoLayout.addWidget(self.label)
    qt.QTimer.singleShot(msec, self.info.close)
    self.info.exec_()

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_RegistrationHierarchy1()

  def test_RegistrationHierarchy1(self):
    """ Ideally you should have several levels of tests.  At the lowest level
    tests sould exercise the functionality of the logic with different inputs
    (both valid and invalid).  At higher levels your tests should emulate the
    way the user would interact with your code and confirm that it still works
    the way you intended.
    One of the most important features of the tests is that it should alert other
    developers when their changes will have an impact on the behavior of your
    module.  For example, if a developer removes a feature that you depend on,
    your test should break so they know that the feature is needed.
    """

    self.delayDisplay("Starting the test")
    #
    # first, get some data
    #
    import urllib
    downloads = (
        ('http://slicer.kitware.com/midas3/download?items=5767', 'FA.nrrd', slicer.util.loadVolume),
        )

    for url,name,loader in downloads:
      filePath = slicer.app.temporaryPath + '/' + name
      if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
        print('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)
      if loader:
        print('Loading %s...\n' % (name,))
        loader(filePath)
    self.delayDisplay('Finished with download and loading\n')

    volumeNode = slicer.util.getNode(pattern="FA")
    logic = RegistrationHierarchyLogic()
    self.assertTrue( logic.hasImageData(volumeNode) )
    self.delayDisplay('Test passed!')
    
#Class that holds all info for registration 
class registrationParameters():
    def __init__(self,patientName,referenceNode,referenceNumber,resample = []):
      self.patientName = patientName
      self.referenceNode = referenceNode
      self.referenceNumber = referenceNumber
      self.movingNode = None
      self.movingNumber = ''
      self.movingHierarchy = None
      self.parameters = {}
      self.warpVolume = None
      self.warpDirectory = ''
      self.vectorVolume = None
      self.vectorDirectory = ''
      self.vf_F_name = ''
      self.mhaOn = False
      self.resample = resample

    def setWarpVolume(self):
      warpVolume = slicer.vtkMRMLScalarVolumeNode()
      slicer.mrmlScene.AddNode( warpVolume )
      storageNode = warpVolume.CreateDefaultStorageNode()
      slicer.mrmlScene.AddNode ( storageNode )
      warpVolume.SetAndObserveStorageNodeID( storageNode.GetID() )
      self.warpVolume = warpVolume
      
    def setVectorVolume(self):
      vectorVolume = slicer.vtkMRMLGridTransformNode()
      slicer.mrmlScene.AddNode( vectorVolume )
      storageNode = vectorVolume.CreateDefaultStorageNode()
      slicer.mrmlScene.AddNode ( storageNode )
      vectorVolume.SetAndObserveStorageNodeID( storageNode.GetID() )
      self.vectorVolume = vectorVolume
      
    def register(self):
      if not self.referenceNode or not self.movingNode:
	print "Not enough parameters"
	return
	
      registrationName = self.patientName + "_"  + self.movingNumber + "_" + self.referenceNumber
      if self.warpVolume:
	self.warpVolume.SetName(registrationName+"_warped")
      if self.vectorVolume:
	self.vectorVolume.SetName(registrationName)
      if self.mhaOn:
	self.vf_F_name = self.vectorDirectory + registrationName + "_vf.mha"

      self.setParameters()
      #run plastimatch registration
      plmslcRegistration= slicer.modules.plastimatch_slicer_bspline
      slicer.cli.run(plmslcRegistration, None, self.parameters, wait_for_completion=True)
      #Resample if neccesary
      #TODO: Descripton in process.
      #self.resampleVolume()
      #save nodes
      self.saveNodes()
      #Switch
      
      
      registrationName = self.patientName + "_"  + self.referenceNumber+ "_" + self.movingNumber
      if self.warpVolume:
	self.warpVolume.SetName(registrationName+"_warped")
      if self.vectorVolume:
	self.vectorVolume.SetName(registrationName)
      if self.mhaOn:
	self.vf_F_name = self.vectorDirectory + registrationName + "_vf.mha"

      self.setParameters()
      self.switchPhase()
      
      slicer.cli.run(plmslcRegistration, None, self.parameters, wait_for_completion=True)
      #Resample if neccesary
      #TODO: Descripton in process.
      #self.resampleVolume()
      #Save nodes
      self.saveNodes(switch = True)
      
	  
    def saveNodes(self,switch = False):
      logic = RegistrationHierarchyLogic()
      if self.warpVolume:
	if not self.warpDirectory:
	  print "No directory"
	  return	  
	if switch:
	  name = NAME_INVWARP
	else:
	  name = NAME_WARP
	filePath = self.warpDirectory + self.warpVolume.GetName() + ".nrrd"
	if logic.saveAndWriteNode(self.warpVolume,self.movingHierarchy,name,filePath):
	  print "Saved Warped Image " + self.warpVolume.GetName()
      
      if self.vectorVolume:
	if not self.vectorDirectory:
	  print "No directory"
	  return	  
	if switch:
	  name = NAME_VECTOR
	else:
	  name = NAME_INVVECTOR
	filePath = self.vectorDirectory + self.vectorVolume.GetName() + "_x.ctx"
	if logic.saveAndWriteNode(self.vectorVolume,self.movingHierarchy,name,filePath,True,self.resample):
	  print "Saved vector field."
      
    
    def switchPhase(self):
      if not self.parameters:
	print "No parameters"
	return
	
      self.parameters["plmslc_fixed_volume"] = self.movingNode.GetID()
      self.parameters["plmslc_moving_volume"] = self.referenceNode.GetID()

    def setParameters(self):
      parameters = {}
      
      parameters["plmslc_fixed_volume"] = self.referenceNode.GetID()
      parameters["plmslc_moving_volume"] = self.movingNode.GetID()

      parameters["plmslc_fixed_fiducials"] = ''
      parameters["plmslc_moving_fiducials"] = ''
      
      parameters["metric"] = "MSE" #"MI

      if self.warpVolume:
	parameters["plmslc_output_warped"] = self.warpVolume
      else:
	parameters["plmslc_output_warped"] = ''
      if self.vectorVolume:
        parameters["plmslc_output_vf"] = self.vectorVolume
      else:
	parameters["plmslc_output_vf"] = ''
      
      if not self.vectorVolume and self.vf_F_name:
        parameters["plmslc_output_vf_f"] = self.vf_F_name
      else:
	parameters["plmslc_output_vf_f"] = self.vf_F_name
      
      parameters["enable_stage_0"] = False
      
      parameters["stage_1_resolution"] = '4,4,2'
      parameters["stage_1_grid_size"] = '50'
      parameters["stage_1_regularization"] = '0.005'
      parameters["stage_1_its"] = '200'
      parameters["plmslc_output_warped_1"] = ''
      
      parameters["enable_stage_2"] = True
      parameters["stage_2_resolution"] = '2,2,1'
      parameters["stage_2_grid_size"] = '15'
      parameters["stage_1_regularization"] = '0.005'
      parameters["stage_2_its"] = '100'
      parameters["plmslc_output_warped_2"] = ''
	
      parameters["enable_stage_3"] = False
      parameters["stage_3_resolution"] = '1,1,1'
      parameters["stage_3_grid_size"] = '15'
      parameters["stage_1_regularization"] = '0.005'
      parameters["stage_3_its"] = '100'
      parameters["plmslc_output_warped_3"] = ''
      self.parameters = parameters
      
    def resampleVolume(self):
      if not self.vectorVolume or not self.vectorVolume.IsA('vtkMRMLVectorVolumeNode'):
        print "No vector volume for resampling."
        return
      
      if self.resample == []:
	print "No resample values."
	return
	
      if not len(self.resample) == 3:
        print "Too many values for resampling."
        return
      
      oldVectorVolume = self.vectorVolume
      
      #Create new vector volume
      newVectorVolume = slicer.vtkMRMLVectorVolumeNode()
      newVectorVolume.SetName(oldVectorVolume.GetName())
      slicer.mrmlScene.AddNode(newVectorVolume)
      
      #Create strings for resampling
      spacing = ''
      size = ''
      for i in range(0,len(self.resample)):
        spacing += str(oldVectorVolume.GetSpacing()[i]*self.resample[i])
        #extent = oldVectorVolume.GetImageData().GetExtent[2*i+1]
        extent = oldVectorVolume.GetImageData().GetExtent()[2*i+1]+1
        size += str(extent/self.resample[i])
        if i < 2:
	  spacing += ','
	  size += ','

      print "Resampling " + oldVectorVolume.GetName() + " to new pixel size " + size 
      
      #Set parameters
      parameters = {} 
      parameters["inputVolume"] = oldVectorVolume.GetID()
      parameters["outputVolume"] = newVectorVolume.GetID()
      parameters["referenceVolume"] = ''
      parameters["outputImageSpacing"] = spacing
      parameters["outputImageSize"] = size
      
      #Do resampling
      resampleScalarVolume = slicer.modules.resamplescalarvectordwivolume
      clNode = slicer.cli.run(resampleScalarVolume, None, parameters, wait_for_completion=True)
      
      #Remove old vector node and set new:
      self.vectorVolume = newVectorVolume
      slicer.mrmlScene.RemoveNode(oldVectorVolume)
      
    
def progressBar(message):
    progressBar = qt.QProgressDialog(slicer.util.mainWindow())
    progressBar.setModal(True)
    progressBar.setMinimumDuration(150)
    progressBar.setLabelText(message)
    qt.QApplication.processEvents()
    return progressBar