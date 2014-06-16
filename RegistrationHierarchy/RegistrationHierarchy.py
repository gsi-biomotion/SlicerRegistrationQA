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
NAME_JACOBIAN = 'Jacobian'
NAME_INVCONSIST = 'InverseConsistency'
NAME_REFPHASE = 'ReferenceHierarchyNode'
NAME_ROIFILEPATH = 'RoiFilePath'
NAME_DIRQA = 'DIRQA'

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

    # Add this test to the SelfTest module's list for discovery when the module
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

    #
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
    self.regHierarchyComboBox = slicer.qMRMLNodeComboBox()
    self.regHierarchyComboBox.nodeTypes = ( ("vtkMRMLSubjectHierarchyNode"),"" )
    self.regHierarchyComboBox.addAttribute( "vtkMRMLScalarVolumeNode", "Name", "Registration Node" )
    #self.regHierarchyComboBox.selectNodeUponCreation = True
    self.regHierarchyComboBox.addEnabled = False
    self.regHierarchyComboBox.removeEnabled = False
    self.regHierarchyComboBox.noneEnabled = False
    self.regHierarchyComboBox.showHidden = False
    self.regHierarchyComboBox.showChildNodeTypes = False
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
    logic.computeDIRQAfromHierarchyNode(self.regHierarchy)


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
	
      logic.showVolumeFromHierarchyNode(node)
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
      logic.computeDIRQAfromHierarchyNode(node,refPhaseNode)
      self.setStandardModel(self.regHierarchy)
      
  #def itemSelection(self,item):
    #print "Wooh"
  
  def setStandardModel(self,regHierarchy):

    if not regHierarchy:
      return

    if not regHierarchy.GetNameWithoutPostfix().find('Registration') > -1:
      return
    
    if regHierarchy.GetNumberOfChildrenNodes() < 1:
      print "Error, Selected Subject Hiearchy doesn't have any child contour nodes."
    
    self.model.clear()
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
	  #Create item for each dirqa node
	  dirqaItem = qt.QStandardItem()
	  dirqaItem.setData(dirqaNode)
	  dirqaItem.setText(dirqaNode.GetNameWithoutPostfix())
	  phaseItem.appendRow(dirqaItem)
	  dirqaList.append(dirqaItem)
	  
	  #Look for statistics:
	  if dirqaNode.GetNameWithoutPostfix() == NAME_ABSDIFF:
	    text = ''
	    text = dirqaItem.text() + " "
	    text += 'Mean: ' + dirqaNode.GetAttribute('Mean') + " "
	    text += 'STD: ' + dirqaNode.GetAttribute('STD') + " "
	    text += 'Max: ' + dirqaNode.GetAttribute('Max') + " "
	    text += 'Min: ' + dirqaNode.GetAttribute('Min')
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

  def automaticRegistration(self,regHierarchy):
    if not regHierarchy:
      print "No registration Hierarchy"
      return

    warpedOn = True
    cbtOn = True
    mhaOn = False
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
     
    regParameters = registrationParameters(patientName,refPhaseNode, referenceNumber)
    
    regParameters.warpDirectory = regHierarchy.GetAttribute("DIR" + NAME_WARP )
    regParameters.vectorDirectory = regHierarchy.GetAttribute("DIR" + NAME_VECTOR )
    
    if warpedOn:
      regParameters.setWarpVolume()
    
    if cbtOn:
      regParameters.setVectorVolume()
      
    regParameters.mhaOn = mhaOn
    
    pbar = progressBar(("Registering all phases"))
    pbar.setValue(0)
    pbar.show()
    #Loop through all phases:
    for i in range(0,nPhases):
      phaseHierarchyNode = regHierarchy.GetNthChildNode(i)   
      pbar.setValue(i/nPhases)
      
      if phaseHierarchyNode.GetID() == regHierarchy.GetAttribute(NAME_REFPHASE):
	print "Skiping reference phase"
	continue
  
      
      phaseNumber = phaseHierarchyNode.GetAttribute('PhaseNumber')
      
      phaseNode = self.getVolumeFromChild(phaseHierarchyNode,NAME_CT)
      if not phaseNode:
	print "Can't load phase from: " + phaseHierarchyNode.GetNameWithoutPostfix()
	continue
      
      regParameters.movingNode = phaseNode
      regParameters.movingNumber = phaseNumber
      regParameters.movingHierarchy = phaseHierarchyNode
      

      regParameters.register()
      slicer.mrmlScene.RemoveNode(phaseNode)

    slicer.mrmlScene.RemoveNode(regParameters.warpVolume)
    slicer.mrmlScene.RemoveNode(regParameters.vectorVolume)
    pbar.close()
      

  def computeDIRQAfromHierarchyNode(self,regHierarchy):
    #Look for DIRQA module
    try:
      DIRQALogic = slicer.modules.registrationquality.logic()
    except AttributeError:
      import sys
      sys.stderr.write('Cannot find registrationquality module')
      return
      
    if not regHierarchy:
      print "No registration Hierarchy"
      return

    refPhaseNode = self.getReferencePhaseFromHierarchy(regHierarchy)
    referenceHierarchyNode = slicer.util.getNode(regHierarchy.GetAttribute(NAME_REFPHASE))
    referenceNumber = regHierarchy.GetAttribute("ReferenceNumber")
    if not refPhaseNode:
      print "Can't get reference node."
      return
      
    #TODO: This needs fix. Now the name of ROI has to be 'R' otherwise it doesn't work.
    roiNode = slicer.util.getNode('R')
    if not roiNode:
      roiFilePath = referenceHierarchyNode.GetAttribute(NAME_ROIFILEPATH)
      if roiFilePath:
        slicer.util.loadAnnotationFiducial(roiFilePath)
        roiNode = slicer.util.getNode('R')
    
    nPhases = regHierarchy.GetNumberOfChildrenNodes()
    if nPhases < 1:
      print "No children nodes."
      return

    #Loop through all phases, find biggest changes in vector field:
    maxVectorNode = None
    maxPhaseHierarchy = slicer.util.getNode(regHierarchy.GetAttribute('MaxPhaseHierarchy'))
    
    maxVectorValue = -1
    if not maxPhaseHierarchy:
      #Loading from python module (LoadCTX) takes time. Solution is to load all volumes in advance:
      self.loadAllChildren(regHierarchy,NAME_VECTOR)
      print "Starting Loop"
      for i in range(0,nPhases):
        phaseHierarchyNode = regHierarchy.GetNthChildNode(i)   
      
        if phaseHierarchyNode.GetID() == regHierarchy.GetAttribute(NAME_REFPHASE):
	  print "Skiping reference phase"
	  continue
      
        vectorNode = self.getVolumeFromChild(phaseHierarchyNode,NAME_VECTOR)
        if not vectorNode:
	  print "Can't load phase from: " + phaseHierarchyNode.GetNameWithoutPostfix()
	  continue
      
        #Find out phase with the biggest max value in vector field
        array = abs(slicer.util.array(vectorNode.GetID()))
        vectorMax = array.max()
        if vectorMax > maxVectorValue:
	  maxVectorNode = vectorNode
	  print "New value is " + str(maxVectorValue)
	  maxPhaseHierarchy = phaseHierarchyNode
	  maxVectorValue = vectorMax

    else:
      maxVectorNode = self.getVolumeFromChild(maxPhaseHierarchy,NAME_VECTOR)
      print maxVectorNode.GetName()
      phaseHierarchyNode = maxPhaseHierarchy
      array = abs(slicer.util.array(maxVectorNode.GetID()))
      maxVectorValue = array.max()
	
    if not maxVectorNode:
      print "Phase with max vector field could not be found."
      return
    else:
      print "Phase with max vector value: " + phaseHierarchyNode.GetNameWithoutPostfix() + " with: " + str(maxVectorValue) + "mm"
    
    
    regHierarchy.SetAttribute('MaxPhaseHierarchy',maxPhaseHierarchy.GetID())
    #TODO: Find max value in 3D and compare it with spacing 
    spacing = maxVectorNode.GetSpacing()
    if maxVectorValue < spacing[0] and maxVectorValue < spacing[2]:
      print "All vector field values are smaller than spacing."
      return
      
    
    #AbsoluteDifference
    
    #Check if it's already computed
    absDiffNode = self.getVolumeFromChild(maxPhaseHierarchy,NAME_ABSDIFF)
    if not absDiffNode:
      warpNode = self.getVolumeFromChild(maxPhaseHierarchy,NAME_WARP)
      if warpNode:
        absDiffNodeWarp = DIRQALogic.AbsoluteDifference(refPhaseNode,warpNode,roiNode)
      phaseNode = self.getVolumeFromChild(maxPhaseHierarchy,NAME_CT)
      if phaseNode:
	absDiffNodePhase = DIRQALogic.AbsoluteDifference(refPhaseNode,phaseNode,roiNode)
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
	
	#absDiffHierarchy = self.createChild(maxPhaseHierarchy,NAME_ABSDIFF)
	#absDiffHierarchy.SetAssociatedNodeID(absDiffNodeWarp.GetID())
	
	#Statistics
	statisticsArray = [0,0,0,0]
	DIRQALogic.CalculateStatistics(absDiffNodeWarp,statisticsArray)
	self.writeStatistics(absDiffHierarchy,statisticsArray)
      else:
	print "Can't compute Absolute Difference."

    return
    #Jacobian
    
    #Check if it's already computed
    jacobianNode = self.getVolumeFromChild(maxPhaseHierarchy,NAME_JACOBIAN)
    if not jacobianNode:
      jacobianNode = DIRQALogic.Jacobian(maxVectorNode,roiNode)
      if jacobianNode:
	jacobianHierarchy = self.createChild(maxPhaseHierarchy,NAME_JACOBIAN)
	jacobianHierarchy.SetAssociatedNodeID(jacobianNode.GetID())
	
	#Statistics
	statisticsArray = [0,0,0,0]
	DIRQALogic.CalculateStatistics(jacobianNode,statisticsArray)
	self.writeStatistics(jacobianHierarchy,statisticsArray)
      else:
	print "Can't compute Jacobian."
	
    #Inverse Consistency
    
    #Check if it's already computed
    invConsistNode = self.getVolumeFromChild(maxPhaseHierarchy,NAME_INVCONSIST)
    if not invConsistNode:
      invVectorNode = self.getVolumeFromChild(maxPhaseHierarchy,NAME_INVVECTOR)
      if invVectorNode:
	invConsistNode = DIRQALogic.InverseConsist(maxVectorNode,invVectorNode,roiNode)
	if invConsistNode:
	  invConsistHierarchy = self.createChild(maxPhaseHierarchy,NAME_INVCONSIST)
	  invConsistHierarchy.SetAssociatedNodeID(invConsistNode.GetID())
	  
	  #Statistics
	  statisticsArray = [0,0,0,0]
	  DIRQALogic.CalculateStatistics(invConsistNode,statisticsArray)
	  self.writeStatistics(invConsistHierarchy,statisticsArray)
	else:
	  print "Can't compute Inverse Consistency."
      else:
	print "Can't load inverse vector field."
	   
  def writeStatistics(self,hierarchyNode,statisticsArray):
    hierarchyNode.SetAttribute("Mean",str(round(statisticsArray[0],2)))
    hierarchyNode.SetAttribute("STD",str(round(statisticsArray[1],2)))
    hierarchyNode.SetAttribute("Max",str(round(statisticsArray[2],2)))
    hierarchyNode.SetAttribute("Min",str(round(statisticsArray[3],2)))
    
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
      selectionNode = slicer.app.applicationLogic().GetSelectionNode()
      selectionNode.SetReferenceActiveVolumeID(volume.GetID())
      slicer.app.applicationLogic().PropagateVolumeSelection(0)
  
  def getReferencePhaseFromHierarchy(self,hierarchyNode):
    
    #Find out reference phase
    referenceID = hierarchyNode.GetAttribute(NAME_REFPHASE)
    if not referenceID:
      print "Can't find reference node from: " + hierarchyNode.GetNameWithoutPostfix()
      return None
      
    referenceHierarchyNode = slicer.util.getNode(referenceID)
    if not referenceHierarchyNode:
      print "Can't get reference Hierarchy node"
      return None
      
    referenceNode = self.getVolumeFromChild(referenceHierarchyNode,NAME_CT)
    if not referenceNode:
      print "Can't load reference volume from: " + referenceHierarchyNode.GetNameWithoutPostfix()
      return None

    return referenceNode
  
  def loadAllChildren(self,hierarchyNode,string):
    for i in range(0,hierarchyNode.GetNumberOfChildrenNodes()):
       volume = self.getVolumeFromChild(hierarchyNode.GetNthChildNode(i),string)
  
  def getVolumeFromChild(self,hierarchyNode,string):
    volume = None
    childNode = hierarchyNode.GetChildWithName(hierarchyNode,string)
    if not childNode:
      print "Can't get childNode: " + string + "from " + hierarchyNode.GetNameWithoutPostfix()
      return None
    volume = self.loadVolumeFromHierarchyNode(childNode)
    if not volume:
       print "Can't load  " + string + " from hierarchy childNode: " + childNode.GetNameWithoutPostfix()
       return None
    return volume
  
  #Loads volume from hierarchyNode. Can find it there or tries to load it from disk.
  def loadVolumeFromHierarchyNode(self,hierarchyNode):
    volume = None
     #Look for existing associated nodes:
    if hierarchyNode.GetAssociatedNodeID():
      volume = slicer.util.getNode(hierarchyNode.GetAssociatedNodeID())
      if volume:
	return volume

    filePath = hierarchyNode.GetAttribute('FilePath')
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
        volume = slicer.util.getNode(loadLogic.loadCube(filePath,3))
        if volume:
	   #Write it in hierarchy node for later use
	  hierarchyNode.SetAssociatedNodeID(volume.GetID())
	  return volume
      else:
	if slicer.util.loadVolume(filePath):
	  #TODO: Not good solution, needs fix"
	  volume = slicer.util.getNode(os.path.splitext(os.path.basename(filePath))[0])
	  if not volume:
	    print "Can't find volume " + os.path.basename(filePath)
	    return None
	  #Write it in hierarchy node for later use
	  hierarchyNode.SetAssociatedNodeID(volume.GetID())
	else:
	  print "Can't load volume from: " + filePath
	  return None
    else:
      print "Can't get file Path from: " + hierarchyNode.GetNameWithoutPostfix()
      return False
    return volume
    
  #Save node to disk and write it to subject hierarchy
  def saveAndWriteNode(self,node,hierarchyNode,string,filePath,cbtOn = False):
    if not node or not hierarchyNode or not string:
      print "Not enough input parameters."
      return False
      
    childNode = self.createChild(hierarchyNode,string)
    childNode.SetAttribute("FilePath",filePath)
    
    directory = os.path.dirname(os.path.realpath(filePath))
    if not os.path.exists(directory):
      print "No path: " + directory
      return False
      
    print "Saving " + node.GetName()
    #Special Case
    if cbtOn:
      
      import SaveTRiP
      saveTripLogic = SaveTRiP.SaveTRiPLogic()
      saveTripLogic.writeTRiPdata(directory,extension='.cbt',nodeID = node.GetID() , aix = True)
      return True
    
    if slicer.util.saveNode(node,filePath):
      childNode.SetAttribute("FilePath",filePath)
      return True

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
    def __init__(self,patientName,referenceNode,referenceNumber):
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
	
      registrationName = self.patientName + "_" + self.movingNumber + "_" + self.referenceNumber
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
      #save nodes
      self.saveNodes()
      #Switch
      self.switchPhase()
      
      registrationName = self.patientName + "_" + self.referenceNumber + "_"+ self.movingNumber 
      if self.warpVolume:
	self.warpVolume.SetName(registrationName+"_warped")
      if self.vectorVolume:
	self.vectorVolume.SetName(registrationName)
      if self.mhaOn:
	self.vf_F_name = self.vectorDirectory + registrationName + "_vf.mha"

      slicer.cli.run(plmslcRegistration, None, self.parameters, wait_for_completion=True)
      #save nodes
      self.saveNodes()
      
	  
    def saveNodes(self):
      logic = RegistrationHierarchyLogic()
      if self.warpVolume:
	if not self.warpDirectory:
	  print "No directory"
	  return	  
	filePath = self.warpDirectory + self.warpVolume.GetName() + ".nrrd"
	if logic.saveAndWriteNode(self.warpVolume,self.movingHierarchy,NAME_WARP,filePath):
	  print "Saved Warped Image " + self.warpVolume.GetName()
      
      if self.vectorVolume:
	if not self.vectorDirectory:
	  print "No directory"
	  return	  
	filePath = self.vectorDirectory + self.vectorVolume.GetName() + "_x.ctx"
	if logic.saveAndWriteNode(self.vectorVolume,self.movingHierarchy,NAME_VECTOR,filePath,True):
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
      parameters["stage_2_resolution"] = '1,1,1'
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
    
def progressBar(message):
    progressBar = qt.QProgressDialog(slicer.util.mainWindow())
    progressBar.setModal(True)
    progressBar.setMinimumDuration(150)
    progressBar.setLabelText(message)
    qt.QApplication.processEvents()
    return progressBar