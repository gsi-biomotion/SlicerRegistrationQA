import os
import unittest
from __main__ import vtk, qt, ctk, slicer

#
# RegistrationHierarchy


#Constants that have to be the same in creating Registration Hierarchy
NAME_CT = 'CT'
NAME_WARP = 'WarpedImage'
NAME_VECTOR = 'Vector' #Vector Field from phase to reference phase
NAME_INVVECTOR = 'InvVector' #Vector from reference phase to phase
NAME_ABSDIFF = 'AbsoluteDifference'
NAME_JACOBIAN = 'Jacobian'
NAME_INVERSECONS = 'InverseConsistency'
NAME_REFPHASE = 'ReferenceHierarchyNode'

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
    self.reloadButton = qt.QPushButton("Reload")
    self.reloadButton.toolTip = "Reload this module."
    self.reloadButton.name = "RegistrationHierarchy Reload"
    reloadFormLayout.addWidget(self.reloadButton)
    self.reloadButton.connect('clicked()', self.onReload)

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
    self.regHierarchyComboBox.selectNodeUponCreation = True
    self.regHierarchyComboBox.addEnabled = False
    self.regHierarchyComboBox.removeEnabled = False
    self.regHierarchyComboBox.noneEnabled = False
    self.regHierarchyComboBox.showHidden = False
    self.regHierarchyComboBox.showChildNodeTypes = False
    self.regHierarchyComboBox.setMRMLScene( slicer.mrmlScene )
    self.regHierarchyComboBox.setToolTip( "Select subject hierarchy with registration hierarchy." )
    self.parametersFormLayout.addRow("Select hirerachy: ", self.regHierarchyComboBox)
    
    #
    # Load CTX File Button
    #
    self.loadButton = qt.QPushButton("Load TRiP Volume")
    self.loadButton.toolTip = "Load TRiP File (*.ctx,*.dos,*.binfo)."
    self.loadButton.enabled = True
    self.parametersFormLayout.addRow(self.loadButton)

    self.columnView = qt.QColumnView()
    self.columnView.setColumnWidths((150,150))
    self.columnView.setContextMenuPolicy(qt.Qt.CustomContextMenu)
    self.parametersFormLayout.addRow(self.columnView)
    
    #Model that stores all items
    self.model = qt.QStandardItemModel()
    self.columnView.setModel(self.model)
    
    #self.model = qt.QStandardItemModel()
    #self.columnView.setModel(self.model)
    #self.t = qt.QStandardItem()
    #self.t.setText("tooo")
    #self.model.appendRow(self.t)
    
    self.loadButton.connect('clicked(bool)', self.onLoadButton)
    self.regHierarchyComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.setStandardModel)
    
    #self.columnView.clicked.connect(self.itemSelection)
    self.columnView.customContextMenuRequested.connect(self.showMenu)
             
    self.contextMenu = qt.QMenu()
    self.showAction = qt.QAction("Show", self.contextMenu)
    self.contextMenu.addAction(self.showAction)
    self.showAction.enabled = True
    self.showAction.visible = False
    self.computeAction = qt.QAction("Compute DIRQA", self.contextMenu)
    self.contextMenu.addAction(self.computeAction)
    
    self.computeAction.enabled = True
    self.contextMenu.connect('triggered(QAction*)', self.onContextMenuClicked)
    
    # Add vertical spacer
    self.layout.addStretch(1)
    
    
  def onLoadButton(self):
    print "Wait!"

    
  def showMenu(self, pos):
    index = self.columnView.selectionModel().currentIndex()
    print index
    #For phase enable option to compute everything
    if index.column() == 0:
      self.computeAction.visible = True
      self.showAction.visible = False
    if index.column() == 1:
      self.computeAction.visible = False
      self.showAction.visible = True
      
    self.contextMenu.popup(self.columnView.mapToGlobal(pos))

  def onContextMenuClicked(self,action):
    logic = RegistrationHierarchyLogic()
    index = self.columnView.selectionModel().currentIndex()
    node = self.model.itemFromIndex(index).data()
    if action == self.showAction:
      if not node:
	print "Can't get node"
	return
      node.showVolumeFromHierarchyNode(node)
	
      logic.showVolumeFromHierarchyNode(node)
    if action == self.computeAction:
      print "We'll do it"
      if not node:
	return
      logic.computeDIRQAfromHierarchyNode(node)
      
    
	
      
  
  #def itemSelection(self,item):
    #print "Wooh"
  
  def setStandardModel(self,regHierarchy):

    if not regHierarchy:
      return
    if regHierarchy.GetNumberOfChildrenNodes() < 1:
      print "Error, Selected Subject Hiearchy doesn't have any child contour nodes."
    
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
	self.dirqaItems.append(dirqaList)
      else:
	self.dirqaItems.append(None)
	  
      self.model.appendRow(phaseItem)
      self.phaseItems.append(phaseItem)


  def cleanup(self):
    pass

  def onSelect(self):
    self.applyButton.enabled = self.inputSelector.currentNode() and self.outputSelector.currentNode()

  def onApplyButton(self):
    logic = RegistrationHierarchyLogic()
    enableScreenshotsFlag = self.enableScreenshotsFlagCheckBox.checked
    screenshotScaleFactor = int(self.screenshotScaleFactorSliderWidget.value)
    print("Run the algorithm")
    logic.run(self.inputSelector.currentNode(), self.outputSelector.currentNode(), enableScreenshotsFlag,screenshotScaleFactor)

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

  def computeDIRQAfromHierarchyNode(self,hierarchyNode):
    print "Let's do it"
    """ Names have to be same throughout Registration Hierarchy phases:
     1. CT is for CT
     2. AbsoluteDifference
     3. Jacobian
     4. InverseConsistency
     """
    #Look for DIRQA module
    DIRQALogic = slicer.modules.RegistrationQuality.Logic()
    if not DIRQALogic:
      print "Can't find RegistrationQuality module"
      return
    
    #Find out reference phase
    referenceID = hierarchyNode.GetAttribute(NAME_REFPHASE)
    if not referenceID:
      print "Can't find reference node from: " + hierarchyNode.GetNameWithoutPostfix()
      return
      
    referenceHierarchyNode = slicer.util.getNode(referenceID)
    if not referenceHierarchyNode:
      print "Can't get reference Hierarchy node"
      return
      
    referenceNode = None
    if not self.getVolumeFromChild(referenceHierarchyNode,referenceNode,NAME_CT):
      print "Can't load reference volume from: " + referenceHierarchyNode.GetNameWithoutPostfix()
      return
      
    for i in range(0,hierarchyNode.GetNumberOfChildrenNodes()):
      phaseHierarchyNode = regHierarchy.GetNthChildNode(i)

      #Do checks if nodes already exist, so you can skip everything
      phaseNode = None
      if not self.getVolumeFromChild(phaseHierarchyNode,phaseNode,NAME_CT):
	print "Can't load phase " + str(i) + " from hierarchy node: " + phaseHierarchyNode.GetNameWithoutPostfix()
	continue
      
      
      absDiffNode = None
      if not self.getVolumeFromChild(phaseHierarchyNode,absDiffNode,NAME_ABSDIFF):
        absDiffNode = DIRQALogic.AbsoluteDifference(referenceNode,phaseNode)
        if absDiffNode:
	  self.createChild(phaseHierarchyNode,NAME_ABSDIFF)
	  
    
  
 
  def createChild(hierarchyNode,string):
    newHierarchy = vtkMRMLSubjectHierarchyNode()
    newHierarchy.SetParentNodeID(hierarchyNode.GetID())
    newHierarchy.SetName(string)
    newHierarchy.SetLevel('Subseries')
    newHierarchy.SetAttribute('DICOMHierarchy.SeriesModality','CT')
    #TODO: Addd directories
    #newHierarchy.SetAttribute('FilePath',ctDirectory+fileName)
    #newHierarchy.SetOwnerPluginName('Volumes')
    slicer.mrmlScene.AddNode(newHierarchy)
  
  def showVolumeFromHierarchyNode(self,hierarchyNode,volume):
    
    volume = None
    if not self.loadVolumeFromHierarchyNode(hierarchyNode,volume):
        print "Can't get volume"
        return

    if not volume:
      print "No volume"
      return
      
    if volume.isA('vtkMRMLVolumeNode'):
      selectionNode = slicer.app.applicationLogic().GetSelectionNode()
      selectionNode.SetReferenceActiveVolumeID(volume.GetID())
      slicer.app.applicationLogic().PropagateVolumeSelection(0)
  
  def getVolumeFromChild(self,hierarchyNode,volume,string):
    childNode = hierarchyNode.GetChildWithName(hierarchyNode,string)
    if not childNode:
      print "Can't get childNode: " + string + "from " + hierarchyNode.GetNameWithoutPostfix()
      return False
    if not self.loadVolumeFromHierarchyNode(childNode,volume):
       print "Can't load  " + string + " from hierarchy childNode: " + childNode.GetNameWithoutPostfix()
       return False
    return True
  
  #Loads volume from hierarchyNode. Can find it there or tries to load it from disk.
  def loadVolumeFromHierarchyNode(self,hierarchyNode,volume):
     #Look for existing associated nodes:
    if hierarchyNode.GetAssociatedNodeID():
      volume = slicer.util.getNode(hierarchyNode.GetAssociatedNodeID())
      if volume:
	return True

    filePath = hierarchyNode.GetAttribute('FilePath')
    if filePath:
      if slicer.util.loadVolume(filePath):
	#TODO: Not good solution, perhaps look at last vtkMRMLnode?"
	volume = slicer.util.getNode(os.path.basename(filePath))
	if not volume:
	  print "Can't find volume " + os.path.basename(filePath)
	  return False
	#Write it in hierarchy node for later use
	hierarchyNode.SetAssociatedNodeID(volume.GetID())
      else:
	print "Can't load volume from: " + filePath
	return False
    else:
      print "Can't get file Path from: " + hierarchyNode.GetNameWithoutPostfix()
      return False
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
