# -*- coding: iso-8859-1 -*-
import os
import unittest
from __main__ import vtk, qt, ctk, slicer

from vtkSlicerSubjectHierarchyModuleMRML import vtkMRMLSubjectHierarchyNode
from vtkSlicerSubjectHierarchyModuleLogic import vtkSlicerSubjectHierarchyModuleLogic
try:
  vtkMRMLSubjectHierarchyNode
  vtkSlicerSubjectHierarchyModuleLogic
except AttributeError:
  import sys
  sys.stderr.write('Unable to create SubjectHierarchy nodes: SubjectHierarchy module not found!')


#
# ReadRegistrationNode
#
#Constants that have to be the same in creating Registration Hierarchy
#



#Node names:
NAME_CT = 'CT'
NAME_WARP = 'WarpedImage' #Warped image from phase ( phase + vector field = warpedimage)
NAME_INVWARP = 'InvWarpedImage' #Warped image from reference phase (reference + invVector = invWarpedImage)
NAME_VECTOR = 'Vector' #Vector Field from phase to reference phase
NAME_INVVECTOR = 'InvVector' #Vector from reference phase to phase
NAME_ABSDIFF = 'AbsoluteDifference'
NAME_JACOBIAN = 'Jacobian'
NAME_INVCONSIST = 'InverseConsistency'
NAME_REFPHASE = 'ReferenceHierarchyNode'
#Roi should be named 'R'
NAME_ROIFILEPATH = 'RoiFilePath' #If there is region of intrest it should be put under reference phase hierarchy node.
NAME_DIRQA = 'DIRQA'

#Names for directories are just DIR + NAME_X. I.e. directory for CTs is stored under 'DIRCT' attribute

class ReadRegistrationNode:
  def __init__(self, parent):
    parent.title = "Read Registration Node" # TODO make this more human readable by adding spaces
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
    slicer.selfTests['ReadRegistrationNode'] = self.runTest

  def runTest(self):
    tester = ReadRegistrationNodeTest()
    tester.runTest()

#
# qReadRegistrationNodeWidget
#

class ReadRegistrationNodeWidget:
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
    self.reloadButton.name = "ReadRegistrationNode Reload"
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
    parametersFormLayout = qt.QFormLayout(parametersCollapsibleButton)

    ##
    ## input volume selector
    ##
    #self.inputSelector = slicer.qMRMLNodeComboBox()
    #self.inputSelector.nodeTypes = ( ("vtkMRMLScalarVolumeNode"), "" )
    #self.inputSelector.addAttribute( "vtkMRMLScalarVolumeNode", "LabelMap", 0 )
    #self.inputSelector.selectNodeUponCreation = True
    #self.inputSelector.addEnabled = False
    #self.inputSelector.removeEnabled = False
    #self.inputSelector.noneEnabled = False
    #self.inputSelector.showHidden = False
    #self.inputSelector.showChildNodeTypes = False
    #self.inputSelector.setMRMLScene( slicer.mrmlScene )
    #self.inputSelector.setToolTip( "Pick the input to the algorithm." )
    #parametersFormLayout.addRow("Input Volume: ", self.inputSelector)
    #
    # Patient Name
    #
    self.patientName = qt.QLineEdit()     
    self.patientName.setToolTip( "Input patient name" )
    self.patientName.text = 'Lung001'
    parametersFormLayout.addRow("Patient Name:", self.patientName)
    #
    # Find CT
    #
    #self.selectCT = qt.QPushButton("Create Default Directories")
    #self.selectCT.toolTip = "Creates default directories based on Patient Name."
    #self.selectCT.enabled = True
    #parametersFormLayout.addRow(self.selectCT)
    # CT Directory
    self.pathCT = qt.QLineEdit()     
    self.pathCT.setToolTip( "Input the path to CT Directory" )
    self.pathCT.text = ''
    parametersFormLayout.addRow("Patient Directory path:", self.pathCT)
    
    # Warp images ctDirectory
    #self.pathWarpedImages = qt.QLineEdit()     
    #self.pathWarpedImages.setToolTip( "Input the path to warp images ctDirectory" )
    #self.pathWarpedImages.text = ''
    #parametersFormLayout.addRow("Warp images directory path:", self.pathWarpedImages)
    
    ##Vector fields ctDirectory
    #self.pathRegistration = qt.QLineEdit()     
    #self.pathRegistration.setToolTip( "Input the path to vector fields ctDirectory" )
    #self.pathRegistration.text = ''
    #parametersFormLayout.addRow("Registration Directory", self.pathRegistration)
    
    # Overwrite option
    self.overwriteCheckBox = qt.QCheckBox()     
    self.overwriteCheckBox.setToolTip( "Check, if you want to overwrite old files." )
    self.overwriteCheckBox.setCheckState(0)
    parametersFormLayout.addRow("Overwrite:", self.overwriteCheckBox)
    
    ## Chart button
    #self.chartFrame = qt.QFrame()
    #self.chartFrame.setLayout(qt.QHBoxLayout())
    #self.parent.layout().addWidget(self.chartFrame)
    #self.chartButton = qt.QPushButton("Chart")
    #self.chartButton.toolTip = "Make a chart from the current statistics."
    #self.chartFrame.layout().addWidget(self.chartButton)
    #self.chartOption = qt.QComboBox()
    #self.chartOption.addItems(self.chartOptions)
    #self.chartFrame.layout().addWidget(self.chartOption)
    #self.chartIgnoreZero = qt.QCheckBox()
    #self.chartIgnoreZero.setText('Ignore Zero')
    #self.chartIgnoreZero.checked = False
    #self.chartIgnoreZero.setToolTip('Do not include the zero index in the chart to avoid dwarfing other bars')
    #self.chartFrame.layout().addWidget(self.chartIgnoreZero)
    #self.chartFrame.enabled = False
    
    
    # Registration From Contrast
    #self.chartFrame = qt.QFrame()
    #self.chartFrame.setLayout(qt.QHBoxLayout())
    #self.parent.layout().addWidget(self.chartFrame)
    self.fromContrastCheckBox = qt.QCheckBox()     
    self.fromContrastCheckBox.setToolTip( "Check for registration from contrast" )
    self.fromContrastCheckBox.setCheckState(2)
    parametersFormLayout.addRow("Registration from contrast CT:", self.fromContrastCheckBox)
    
    # Registration 4D Native
    self.native4DCheckBox = qt.QCheckBox()     
    self.native4DCheckBox.setToolTip( "Check for registration of 4DCT native" )
    self.native4DCheckBox.setCheckState(2)
    parametersFormLayout.addRow("Registration of 4DCT native:", self.native4DCheckBox)
    
    # Registration 4D Contrast
    self.contrast4DCheckBox = qt.QCheckBox()     
    self.contrast4DCheckBox.setToolTip( "Check for registration of 4DCT contrast" )
    self.contrast4DCheckBox.setCheckState(0)
    parametersFormLayout.addRow("Registration of 4DCT contrast:", self.contrast4DCheckBox)
    
    # Resample 4D
    self.resample4DCheckBox = qt.QCheckBox()     
    self.resample4DCheckBox.setToolTip( "Check if you want 4DCT vector field to be resampled." )
    self.resample4DCheckBox.setCheckState(2)
    parametersFormLayout.addRow("Resample vector fields in 4DCT?:", self.resample4DCheckBox)
    
    #
    # Create subject hierarchy
    #
    self.createHierarchyButton = qt.QPushButton("Create Subject Hierarchy")
    self.createHierarchyButton.toolTip = "Create Subject Hierarchy from the CT files on disk."
    self.createHierarchyButton.visible = True
    parametersFormLayout.addRow(self.createHierarchyButton)

    #
    # registerAndDirqa all
    #
    self.registerAllButton = qt.QPushButton("Register!")
    self.registerAllButton.toolTip = "Makes registration from contrast and 4D."
    self.registerAllButton.enabled = True
    parametersFormLayout.addRow(self.registerAllButton)
    
    #
    # Automatic DIRQA
    #
    self.dirqaButton = qt.QPushButton("DIRQA!")
    self.dirqaButton.toolTip = "Makes registration check."
    self.dirqaButton.enabled = False
    parametersFormLayout.addRow(self.dirqaButton)
    
    # connections
    #self.selectCT.connect('clicked(bool)', self.onSelectCT)
    self.createHierarchyButton.connect('clicked(bool)', self.onCreateHierarchyButton)
    self.registerAllButton.connect('clicked(bool)', self.onRegisterAllButton)
    self.dirqaButton.connect('clicked(bool)', self.onDirqaButton)
    #self.inputSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect)

    # Add vertical spacer
    self.layout.addStretch(1)

  def cleanup(self):
    pass

  def onSelectCT(self):
    self.createDefaultPatientDir()

  def onCreateHierarchyButton(self):
    patientName = self.patientName.text
    if patientName == '':
      print "Input Patient Name!"
      return 
    logic = ReadRegistrationNodeLogic()
    #logic.createHierarchy(self.pathCT.text,self.pathRegistration.text,self.pathRegistration.text,self.patientName.text)
    if self.pathCT.text == '':
      self.createDefaultPatientDir()
    parameters = self.checkCheckboxes()
    logic.registerAndDirqa(self.pathCT.text,patientName, parameters)

  def onRegisterAllButton(self):
    patientName = self.patientName.text
    if patientName == '':
      print "Input Patient Name!"
      return
    if self.pathCT.text == '':
      self.createDefaultPatientDir()
    parameters = self.checkCheckboxes()
    logic = ReadRegistrationNodeLogic()
    logic.registerAndDirqa(self.pathCT.text,patientName, parameters, register = True)
  
  def onDirqaButton(self):
    patientName = self.patientName.text
    if patientName == '':
      print "Input Patient Name!"
      return 
    if self.pathCT.text == '':
      self.createDefaultPatientDir()
    parameters = self.checkCheckboxes()
    logic = ReadRegistrationNodeLogic()
    logic.registerAndDirqa(self.pathCT.text,patientName, parameters, dirqa = True)
    
  def createDefaultPatientDir(self):
    patientName = self.patientName.text
    if patientName == '':
      print "Input Patient Name!"
      return 
    
    self.pathCT.text = '/u/kanderle/AIXd/Data/FC/' + patientName + '/'
    #self.pathCT.text = '/u/motion/Data/PatientData/HIT/' + patientName + '/25062014/'
    #self.pathCT.text = '/u/motion/Data/PatientData2/PIGS/' + patientName + '/'

  
  def checkCheckboxes(self):	  
    if self.overwriteCheckBox.checkState() == 0:
      overwrite = False
    else:
      overwrite = True
      
    if self.fromContrastCheckBox.checkState() == 0:
      fromContrast = False
    else:
      fromContrast = True
      
    if self.native4DCheckBox.checkState() == 0:
      native4D = False
    else:
      native4D = True
      
    if self.contrast4DCheckBox.checkState() == 0:
      contrast4D = False
    else:
      contrast4D = True
      
    if self.resample4DCheckBox.checkState() == 0:
      resample4D = False
    else:
      resample4D = True
      
    parameters = [ overwrite, fromContrast, native4D, contrast4D, resample4D ]
    return parameters
  
  def onReload(self,moduleName="ReadRegistrationNode"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def onReloadAndTest(self,moduleName="ReadRegistrationNode"):
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
# ReadRegistrationNodeLogic
#

class ReadRegistrationNodeLogic:
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def __init__(self):
    pass

  def registerAndDirqa(self,patientDirectory,patientName, parameters, register = False, dirqa = False):
    from RegistrationHierarchy import RegistrationHierarchyLogic
    try:
      RegistrationHierarchyLogic
    except AttributeError:
      import sys
      sys.stderr.write('Unable to find RegistrationHierarchyLogic!')
      return
     
    registrationLogic = RegistrationHierarchyLogic()
    
    overwrite = parameters[0]
    fromContrast = parameters[1]
    native4D = parameters[2]
    contrast4D = parameters[3]
    resample4D = parameters[4]
    
    #Set resample factor:
    if resample4D:
      resampleValue = [2,2,1]
    else:
      resampleValue = []
    
    #Reference phase remains 00 throughout registration
    referencePhase = "00"
    referencePhaseContrast = "01"
    
    #First make subject Hierarchy Node
    #Create Patient Node
    subjectNode = slicer.util.getNode(patientName)
    if not subjectNode or subjectNode.IsA('vtkMRMLSubjectHierarchyNode') == 0:
      subjectNode = vtkMRMLSubjectHierarchyNode()
      subjectNode.SetName(patientName)
      subjectNode.SetLevel('Subject')
      slicer.mrmlScene.AddNode(subjectNode)
    
    
    
    #From Contrast Registration
    
    
    ##ctDirectoryNative = patientDirectory + '4DCT_1/NRRD/'
    ##ctDirectoryContrast = patientDirectory + '4DCT_2/NRRD/'
    ctDirectoryNative = patientDirectory + 'CTX/'
    #ctDirectoryContrast = '/u/motion/AIXd/Data/PatientData/FC/' + patientName +'/4D/Phases/Plan/'
    #ctDirectoryNative = patientDirectory + '4DCT_1/CTX/'
    #ctDirectoryContrast = patientDirectory  + '4DCT_2/CTX/'
    
    
    if fromContrast:
      print "Making registration from contrast."
      registrationNodeFromContrast = subjectNode.GetChildWithName(subjectNode,'Registration Node From Contrast')
      if not registrationNodeFromContrast:
        #vectorDirectory = patientDirectory + 'Registration/Plan/'
        #vectorDirectory = patientDirectory + '4DCT_1/Registration/FromContrast/'
        vectorDirectory = '/u/kanderle/MHA/'
        warpDirectory = vectorDirectory
        dirqaDirectory = vectorDirectory
        #Next, create Registration Node for registration from Contrast
        #Create Registration node
        registrationNodeFromContrast = vtkMRMLSubjectHierarchyNode()
        registrationNodeFromContrast.SetName('Registration Node From Contrast')
        registrationNodeFromContrast.SetLevel('Study')
        registrationNodeFromContrast.SetParentNodeID(subjectNode.GetID())
        #Paths to directories
        registrationNodeFromContrast.SetAttribute('DIR' + NAME_CT,ctDirectoryNative)
        registrationNodeFromContrast.SetAttribute('DIR' + NAME_WARP,warpDirectory)
        registrationNodeFromContrast.SetAttribute('DIR' + NAME_VECTOR,vectorDirectory)
        registrationNodeFromContrast.SetAttribute('DIR' + NAME_DIRQA,dirqaDirectory)
      
        registrationNodeFromContrast.SetAttribute('ReferenceNumber',referencePhaseContrast)
        registrationNodeFromContrast.SetAttribute('PatientName',patientName)
        slicer.mrmlScene.AddNode(registrationNodeFromContrast)
      
        self.createFromContrastHierarchy(registrationNodeFromContrast,referencePhaseContrast, ctDirectoryContrast,ctDirectoryNative)
	  
      #Make registration
      if register:
        registrationLogic.automaticRegistration(registrationNodeFromContrast,overwrite = overwrite)
      
      if dirqa:
	registrationLogic.computeDIRQAfromHierarchyNode(registrationNodeFromContrast)
      print "Finished registration from contrast."
    
    
    #Register 4D Native
    if native4D:
      print "Register Native 4D."
      registrationNodeNative4D = subjectNode.GetChildWithName(subjectNode,'Registration Node Native 4D')
      if not registrationNodeNative4D:
        #First create registration node
        #vectorDirectory = patientDirectory + '4DCT_1/Registration/4D/'
        #vectorDirectory = patientDirectory + '/Registration/4D/'
        vectorDirectory = '/u/kanderle/MHA/'
        warpDirectory = vectorDirectory
        dirqaDirectory = vectorDirectory
        #Create Registration node
        registrationNodeNative4D = vtkMRMLSubjectHierarchyNode()
        registrationNodeNative4D.SetName('Registration Node Native 4D')
        registrationNodeNative4D.SetLevel('Study')
        registrationNodeNative4D.SetParentNodeID(subjectNode.GetID())
        #Paths to directories
        registrationNodeNative4D.SetAttribute('DIR' + NAME_CT,ctDirectoryNative)
        registrationNodeNative4D.SetAttribute('DIR' + NAME_WARP,warpDirectory)
        registrationNodeNative4D.SetAttribute('DIR' + NAME_VECTOR,vectorDirectory)
        registrationNodeNative4D.SetAttribute('DIR' + NAME_DIRQA,dirqaDirectory)
      
        registrationNodeNative4D.SetAttribute('ReferenceNumber',referencePhase)
        registrationNodeNative4D.SetAttribute('PatientName',patientName)
        slicer.mrmlScene.AddNode(registrationNodeNative4D)
      
        #Add CT paths:
        self.create4DHierarchy(registrationNodeNative4D,referencePhase)
      
      #Make registration
      if register:
        registrationLogic.automaticRegistration(registrationNodeNative4D,overwrite = overwrite, resample = resampleValue)  
      
      #Make dirqa
      if dirqa:
	registrationLogic.computeDIRQAfromHierarchyNode(registrationNodeNative4D)
      print "Finished Native 4D."
    
    #Register 4D Contrast
    if contrast4D:
      print "Register Contrast 4D."
      registrationNodeContrast4D = subjectNode.GetChildWithName(subjectNode,'Registration Node Contrast 4D')
      if not registrationNodeContrast4D:
        #First create registration node
        #vectorDirectory = patientDirectory + '4DCT_2/Registration/4D/'
        vectorDirectory = patientDirectory + 'contrast/Registration/4D/'
        warpDirectory = vectorDirectory
        dirqaDirectory = vectorDirectory
        #Create Registration node
        registrationNodeContrast4D = vtkMRMLSubjectHierarchyNode()
        registrationNodeContrast4D.SetName('Registration Node Contrast 4D')
        registrationNodeContrast4D.SetLevel('Study')
        registrationNodeContrast4D.SetParentNodeID(subjectNode.GetID())
        #Paths to directories
        registrationNodeContrast4D.SetAttribute('DIR' + NAME_CT,ctDirectoryContrast)
        registrationNodeContrast4D.SetAttribute('DIR' + NAME_WARP,warpDirectory)
        registrationNodeContrast4D.SetAttribute('DIR' + NAME_VECTOR,vectorDirectory)
        registrationNodeContrast4D.SetAttribute('DIR' + NAME_DIRQA,dirqaDirectory)
      
        registrationNodeContrast4D.SetAttribute('ReferenceNumber',referencePhase)
        registrationNodeContrast4D.SetAttribute('PatientName',patientName)
        slicer.mrmlScene.AddNode(registrationNodeContrast4D)
      
        #Add CT paths:
        self.create4DHierarchy(registrationNodeContrast4D,referencePhase)
      
      #Make registration
      if register:
        registrationLogic.automaticRegistration(registrationNodeContrast4D,overwrite = overwrite, resample = resampleValue)
      if dirqa:
	registrationLogic.computeDIRQAfromHierarchyNode(registrationNodeContrast4D)
      print "Finished Contrast 4D"
    
    print "Finished!"
    
  
  
  def createHierarchy(self,ctDirectory,warpDirectory,vectorDirectory,patientName):
      
    if not os.path.exists(ctDirectory):
      print ctDirectory + "Doesn't exist"
      return
      
    referencePhase = "00"
    #Manual created for now:
    dirqaDirectory = vectorDirectory
    
    #Create Patient Node
    subjectNode = vtkMRMLSubjectHierarchyNode()
    subjectNode.SetName(patientName)
    subjectNode.SetLevel('Subject')
    subjectNode.SetAttribute('Directory',ctDirectory)
    slicer.mrmlScene.AddNode(subjectNode)
    #Create Registration node
    registrationNode = vtkMRMLSubjectHierarchyNode()
    registrationNode.SetName('Registration Node')
    registrationNode.SetLevel('Study')
    registrationNode.SetParentNodeID(subjectNode.GetID())
    #Paths to directories
    registrationNode.SetAttribute('DIR' + NAME_CT,ctDirectory)
    registrationNode.SetAttribute('DIR' + NAME_WARP,warpDirectory)
    registrationNode.SetAttribute('DIR' + NAME_VECTOR,vectorDirectory)
    registrationNode.SetAttribute('DIR' + NAME_DIRQA,dirqaDirectory)
    
    registrationNode.SetAttribute('ReferenceNumber',referencePhase)
    registrationNode.SetAttribute('PatientName',patientName)
    slicer.mrmlScene.AddNode(registrationNode)
    
    self.create4DHierarchy(registrationNode,referencePhase)
    print "Subject Hierarchy Created"
    return
    
  def createFromContrastHierarchy(self,registrationNode,referencePhase, ctDirectoryContrast, ctDirectoryNative):
      if not os.path.exists(ctDirectoryContrast) or not os.path.exists(ctDirectoryNative): 
        print "No directories:"
        print ctDirectoryContrast
        print ctDirectoryNative
        return
      warpDirectory = registrationNode.GetAttribute("DIR" + NAME_WARP )
      vectorDirectory = registrationNode.GetAttribute("DIR" + NAME_VECTOR )
      #Create two phases - contrast (00) and native (01)
      phaseNode0 = vtkMRMLSubjectHierarchyNode()
      phaseNode0.SetParentNodeID(registrationNode.GetID())
      phaseNode0.SetName('4DRef')
      phaseNode0.SetLevel('Series')
      phaseNode0.SetAttribute('Directory',ctDirectoryContrast)
      phaseNode0.SetAttribute('PhaseNumber',"00")
      slicer.mrmlScene.AddNode(phaseNode0)
      
      phaseNode1 = vtkMRMLSubjectHierarchyNode()
      phaseNode1.SetParentNodeID(registrationNode.GetID())
      phaseNode1.SetName('Plan')
      phaseNode1.SetLevel('Series')
      phaseNode1.SetAttribute('Directory',ctDirectoryNative)
      phaseNode1.SetAttribute('PhaseNumber',"01")
      slicer.mrmlScene.AddNode(phaseNode1)
      
      self.checkWarpDirectory(warpDirectory, phaseNode0, "00")
      self.checkVectorDirectory(vectorDirectory, phaseNode0, "00")
      #Link reference phase
      registrationNode.SetAttribute('ReferenceHierarchyNode',phaseNode1.GetID())
      
      #Look for 00 files
      #TODO: Change to .mha
      for fileName in os.listdir(ctDirectoryNative):
	if fileName.find('_00.nrrd') > -1:
	  ctNode = self.createChild(phaseNode0,NAME_CT)
	  ctNode.SetAttribute('FilePath',ctDirectoryNative+fileName)
	  
      for fileName in os.listdir(ctDirectoryContrast):
	if fileName.find('_00.nrrd') > -1:
	  ctNode = self.createChild(phaseNode1,NAME_CT)
	  ctNode.SetAttribute('FilePath',ctDirectoryContrast+fileName)
	  
      
  
  def create4DHierarchy(self,registrationNode,referencePhase):
    ctDirectory = registrationNode.GetAttribute("DIR" + NAME_CT )
    warpDirectory = registrationNode.GetAttribute("DIR" + NAME_WARP )
    vectorDirectory = registrationNode.GetAttribute("DIR" + NAME_VECTOR )
    
   
    if not os.path.exists(ctDirectory): 
        print "No directories."
        return
    for fileName in os.listdir(ctDirectory):

      #Looking for file names that end with 0.0% or 0%
      #if not fileName.find('%') > -1:
        #print "Cannot find % in file name"
        #continue
      #numberEnding = '0.0'
      #if not fileName.find(numberEnding+'%') > -1:
        #numberEnding = '0'
      #index = fileName.find(numberEnding+'%')
      #if not index > -1:
        #print "No ending with 0 in fileName"
        #continue
      
      #index = fileName.find('.ctx')
      index = fileName.find('.nrrd')
      if not index > -1:
	continue
      #Try to find out, which phase do we have
      phase = fileName[index-2:index]
	#phase = 0
	
      #Create New phase in subject hierarchy
      phaseNode = vtkMRMLSubjectHierarchyNode()
      phaseNode.SetParentNodeID(registrationNode.GetID())
      phaseNode.SetName('Phase_'+phase)
      phaseNode.SetLevel('Series')
      #phaseNode.SetAttribute('DICOMHierarchy.SeriesModality','CT')
      phaseNode.SetAttribute('PhaseNumber',phase)
      #phaseNode.SetOwnerPluginName('Volumes')
      slicer.mrmlScene.AddNode(phaseNode)
      
      #Check if reference phase and link it
      
     #Create New volume in subject hierarchy
      ctNode = self.createChild(phaseNode,NAME_CT)
      ctNode.SetAttribute('FilePath',ctDirectory+fileName)
      
      #Skip for reference phase
      if phase == referencePhase:
        registrationNode.SetAttribute('ReferenceHierarchyNode',phaseNode.GetID())
        roiFilePath = ctDirectory + 'R.acsv'
        if roiFilePath:
	  phaseNode.SetAttribute('RoiFilePath',roiFilePath)
        continue
      
      numberEnding = '0'
      self.checkWarpDirectory(warpDirectory, phaseNode, phase)
      self.checkVectorDirectory(vectorDirectory, phaseNode, phase)
      
      
  def checkWarpDirectory(self, warpDirectory, phaseNode, phase):
    if os.path.exists(warpDirectory):
      for file in os.listdir(warpDirectory):
        index = file.find('_warped.nrrd')
	if index > -1:
	  #Find out warpedimage or invWarpedImage
	  if file[index-2:index] == phase:
	    warpNode = self.createChild(phaseNode,NAME_WARP)
	  elif file[index-8:index-6] == phase:
	    warpNode = self.createChild(phaseNode,NAME_INVWARP)
	  else:
	    continue
	  if warpNode:
	    warpNode.SetAttribute('FilePath',warpDirectory+file)

  def checkVectorDirectory(self, vectorDirectory, phaseNode, phase):
    if os.path.exists(vectorDirectory):
      for file in os.listdir(vectorDirectory):
	index = file.find('_vf.mha')
	if index > -1:
	  #Find out warpedimage or invWarpedImage
	  if file[index-2:index] == phase:
	    warpNode = self.createChild(phaseNode,NAME_VECTOR)
	  elif file[index-8:index-6] == phase:
	    warpNode = self.createChild(phaseNode,NAME_INVVECTOR)
	  else:
	    continue
	  if warpNode:
	    warpNode.SetAttribute('FilePath',vectorDirectory+file)
	#index = file.find('_x.cbt')
        #if index > -1:
	  #if file[index-5:index-3] == phase:
	    #vectorNode = self.createChild(phaseNode,NAME_VECTOR)
	  #elif file[index-2:index] == phase:
	    #vectorNode = self.createChild(phaseNode,NAME_INVVECTOR)
	  #else:
	    #continue
	  #if vectorNode:
	    #vectorNode.SetAttribute('FilePath',vectorDirectory+file)
      

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


class ReadRegistrationNodeTest(unittest.TestCase):
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
    self.test_ReadRegistrationNode1()

  def test_ReadRegistrationNode1(self):
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
    logic = ReadRegistrationNodeLogic()
    self.assertTrue( logic.hasImageData(volumeNode) )
    self.delayDisplay('Test passed!')
