import os
import unittest
from __main__ import vtk, qt, ctk, slicer

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
    # Load CTX File Button
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
    
    self.dirqaAllPhaseButton.connect('clicked(bool)', self.onDirqaAllPhaseButton)
    self.regHierarchyComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.setStandardModel)
    self.contextMenu.connect('triggered(QAction*)', self.onContextMenuClicked)
    
    #self.columnView.clicked.connect(self.itemSelection)
    self.columnView.customContextMenuRequested.connect(self.showMenu)
             
    
    
    
    # Add vertical spacer
    self.layout.addStretch(1)
    
    
  def onDirqaAllPhaseButton(self):
    logic = RegistrationHierarchyLogic()
    
    qt.QApplication.setOverrideCursor(qt.QCursor(3))
    pbar = self.progressBar(("DIRQA on all phases"))
    pbar.setValue(0)
    pbar.show()
    
    nPhases = len(self.phaseItems)
    n = 0
    
    #Load reference phaseHierarchyNode
    refPhaseNode = logic.getReferencePhaseFromHierarchy(self.regHierarchy)
    #Loop through all phases:
    for phaseItem in self.phaseItems:     
      pbar.setValue(n/nPhases)
      phaseHierarchyNode = phaseItem.data()
      if phaseHierarchyNode.GetID() == self.regHierarchy.GetAttribute(NAME_REFPHASE):
	print "Skiping reference phase"
	continue
      logic.computeDIRQAfromHierarchyNode(phaseHierarchyNode,refPhaseNode)
      n += 1
    pbar.close()
    self.setStandardModel(self.regHierarchy)
    qt.QApplication.restoreOverrideCursor()
      
    

    
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


  def progressBar(self,message):
    progressBar = qt.QProgressDialog(slicer.util.mainWindow())
    progressBar.setModal(True)
    progressBar.setMinimumDuration(150)
    progressBar.setLabelText(message)
    qt.QApplication.processEvents()
    return progressBar
  
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
  
  def computeDIRQAfromHierarchyNode(self,hierarchyNode,referenceNode):
    #Look for DIRQA module
    try:
      DIRQALogic = slicer.modules.registrationquality.logic()
    except AttributeError:
      import sys
      sys.stderr.write('Cannot find registrationquality module')
      return
      
    #for i in range(0,hierarchyNode.GetNumberOfChildrenNodes()):
      #phaseHierarchyNode = regHierarchy.GetNthChildNode(i)

    #Do checks if nodes already exist, so you can skip everything
    warpNode = self.getVolumeFromChild(hierarchyNode,NAME_WARP)
    if not warpNode:
      print "Can't load " + NAME_WARP + " from hierarchy node: " + hierarchyNode.GetNameWithoutPostfix()
      return
      
    #AbsoluteDifference
    
    #Check if it's already computed
    absDiffNode = self.getVolumeFromChild(hierarchyNode,NAME_ABSDIFF)
    if not absDiffNode:
      absDiffNode = DIRQALogic.AbsoluteDifference(referenceNode,warpNode)
      if absDiffNode:
	absDiffHierarchy = self.createChild(hierarchyNode,NAME_ABSDIFF)
	absDiffHierarchy.SetAssociatedNodeID(absDiffNode.GetID())
	
	#Statistics
	statisticsArray = [0,0,0,0]
	DIRQALogic.CalculateStatistics(absDiffNode,statisticsArray)
	self.writeStatistics(absDiffHierarchy,statisticsArray)
      else:
	print "Can't compute Absolute Difference."

    ##Jacobian
    ##Check if it's already computed
    #jacobianNode = self.getVolumeFromChild(hierarchyNode,NAME_JACOBIAN)
    #if not absDiffNode:
      #jacobianNode = DIRQALogic.Jacobian(vectorNode)
      #if jacobianNode:
	#jacobianHierarchy = self.createChild(hierarchyNode,NAME_ABSDIFF)
	#jacobianHierarchy.SetAssociatedNodeID(jacobianNode.GetID())
      #else:
	#print "Can't compute Jacobian."
	   
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
    newHierarchy.SetAttribute('DICOMHierarchy.SeriesModality','CT')
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
