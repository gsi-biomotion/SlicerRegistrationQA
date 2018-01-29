# -*- coding: iso-8859-1 -*-
import os
import unittest
from __main__ import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *


#
# ReadRegistrationNode
#
#Constants that have to be the same in creating Registration Hierarchy
#

#CONSTANTS
SLICERREGQA_EXTENSION_NAME = "SlicerRegQA"
INVERSE = "Inverse"
WARPED_IMAGE =  "WarpedImage"
VECTOR_FIELD =  "VectorField"
FIDUCIAL = "Fiducial"
ABSOLUTEDIFFERENCE =  "AbsDiff"
REFIMAGEID = "ReferenceImageItemID"
REFERENCENUMBER = "ReferenceNumber"
REGISTRATION_TYPE = "RegistrationType"
PHASENUMBER = "PhaseNumber"
PHASETYPE=  "Phase"
CT =  "CT"
ITEMID = "_itemID"
DIR = 'DIR'
FILEPATH = 'FilePath'
ROI = "ROI"
DIR = "DIR";
FIXEDIMAGEID = "FixedImage" + ITEMID
MOVINGIMAGEID = "MovingImage" + ITEMID
TRIPVF = "TRiP_vf"
BACKWARD = "BackwardReg"
IMAGE = "Image"
REGQANODEID = "RegQualityNodeID"

#Node names:
NAME_DEFABS = "DefaultAbsoluteDifference"
NAME_JACOBIAN = 'Jacobian'
NAME_INVJACOBIAN = 'InvJacobian'
NAME_INVCONSIST = 'InverseConsistency'
NAME_REFPHASE = 'ReferenceHierarchyNode'
#Roi should be named 'R'
NAME_ROIFILEPATH = 'RoiFilePath' #If there is region of intrest it should be put under reference phase hierarchy node.
NAME_DIRQA = 'DIRQA'
DIRQAFILE = 'DIRQAFile'
NAME_PERCENT = 'Percentile' #Special case to store each contribution to vector field magnitude
NAME_INVPERCENT = 'InvPercentile'
NAME_JACCONSIST = 'JacobianConsistency'
REGITEM = 'RegFiles'
REGQAITEM = 'RegQA'

#Names for directories are just DIR + NAME_X. I.e. directory for CTs is stored under 'DIRCT' attribute

class ReadRegistrationNode(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
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

class ReadRegistrationNodeWidget(ScriptedLoadableModuleWidget):
  #def __init__(self, parent = None):
    #self.predefinedList = ["","FC","SingleFC","MDACC"]
    #if not parent:
      #self.parent = slicer.qMRMLWidget()
      #self.parent.setLayout(qt.QVBoxLayout())
      #self.parent.setMRMLScene(slicer.mrmlScene)
    #else:
      #self.parent = parent
    #self.layout = self.parent.layout()
    #if not parent:
      #self.setup()
      #self.parent.show()

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)
    self.dirPaths = RegistrationHierarchyHelp()
    self.predefinedList = ["","FC","SingleFC","MDACC"]
    # Instantiate and connect widgets ...

    ##
    ## Reload and Test area
    ##
    #reloadCollapsibleButton = ctk.ctkCollapsibleButton()
    #reloadCollapsibleButton.text = "Reload && Test"
    #self.layout.addWidget(reloadCollapsibleButton)
    #reloadFormLayout = qt.QFormLayout(reloadCollapsibleButton)

    ## reload button
    ## (use this during development, but remove it when delivering
    ##  your module to users)
    #self.reloadButton = qt.QPushButton("Reload")
    #self.reloadButton.toolTip = "Reload this module."
    #self.reloadButton.name = "ReadRegistrationNode Reload"
    #reloadFormLayout.addWidget(self.reloadButton)
    #self.reloadButton.connect('clicked()', self.onReload)

    ## reload and test button
    ## (use this during development, but remove it when delivering
    ##  your module to users)
    #self.reloadAndTestButton = qt.QPushButton("Reload and Test")
    #self.reloadAndTestButton.toolTip = "Reload this module and then run the self tests."
    #reloadFormLayout.addWidget(self.reloadAndTestButton)
    #self.reloadAndTestButton.connect('clicked()', self.onReloadAndTest)

    #
    # Parameters Area
    #
    parametersCollapsibleButton = ctk.ctkCollapsibleButton()
    parametersCollapsibleButton.text = "Parameters"
    self.layout.addWidget(parametersCollapsibleButton)

    # Layout within the dummy collapsible button
    parametersFormLayout = qt.QFormLayout(parametersCollapsibleButton)
    
    # Voi list
    self.predefPatBox = qt.QComboBox()
    self.predefPatBox.enabled = True
    parametersFormLayout.addRow("Templates: ", self.predefPatBox)
    for element in self.predefinedList:
       self.predefPatBox.addItem(element)

    #
    # Patient Name
    #
    self.patientName = qt.QLineEdit()     
    self.patientName.setToolTip( "Input patient name" )
    self.patientName.text = ''
    parametersFormLayout.addRow("Patient Name:", self.patientName)

    # Directories
    self.CTDIR = qt.QLineEdit()     
    self.CTDIR.setToolTip( "Input the path to CT Directory" )
    self.CTDIR.text = ''
    parametersFormLayout.addRow("CT Directory:", self.CTDIR)
    
    self.vectorDIR = qt.QLineEdit()     
    self.vectorDIR.setToolTip( "Input the path to vector Directory" )
    self.vectorDIR.text = ''
    parametersFormLayout.addRow("Vector Field Directory:", self.vectorDIR)
    
    self.warpedDIR = qt.QLineEdit()     
    self.warpedDIR.setToolTip( "Input the path to warped images Directory" )
    self.warpedDIR.text = ''
    parametersFormLayout.addRow("Warped Images Directory:", self.warpedDIR)
    
    self.ROIfilePath = qt.QLineEdit()     
    self.ROIfilePath.setToolTip( "Input the path to ROI" )
    self.ROIfilePath.text = ''
    parametersFormLayout.addRow("ROI FilePath:", self.ROIfilePath)
    
    # CT Directory
    self.fiducialsDIR = qt.QLineEdit()     
    self.fiducialsDIR.setToolTip( "Input the path to fiducials Directory" )
    self.fiducialsDIR.text = ''
    parametersFormLayout.addRow("Fiducials Directory:", self.fiducialsDIR)

    
    #
    # Create subject hierarchy
    #
    self.createHierarchyButton = qt.QPushButton("Create Subject Hierarchy")
    self.createHierarchyButton.toolTip = "Create Subject Hierarchy from the CT files on disk."
    self.createHierarchyButton.visible = True
    parametersFormLayout.addRow(self.createHierarchyButton)
    
    
    # connections
    self.predefPatBox.connect('currentIndexChanged(int)', self.setPreDef)
    self.createHierarchyButton.connect('clicked(bool)', self.onCreateHierarchyButton)
    self.patientName.connect('textChanged(QString)',self.patNameChanged)


    # Add vertical spacer
    self.layout.addStretch(1)
    
    

  def cleanup(self):
    pass

  def patNameChanged(self, newName):
     element = self.predefinedList[self.predefPatBox.currentIndex]
     if element == "":
        return
     self.dirPaths.createFromTemplate(element,newName)
     self.updateWidget()
  
  def setPreDef(self, index):
     element = self.predefinedList[index]
     if element == "":
        return
     self.dirPaths.createFromTemplate(element)
     self.updateWidget()
     
  def updateDirpaths(self):
    self.dirPaths.name = self.patientName.text
    self.dirPaths.ctDirectory =  self.CTDIR.text
    self.dirPaths.vectorDirectory = self.vectorDIR.text
    self.dirPaths.warpDirectory = self.warpedDIR.text
    self.dirPaths.roiFile = self.ROIfilePath.text
    self.dirPaths.fiducialsDirectory = self.fiducialsDIR.text
    
  def updateWidget(self):
    self.patientName.text = self.dirPaths.name
    self.CTDIR.text = self.dirPaths.ctDirectory
    self.vectorDIR.text = self.dirPaths.vectorDirectory
    self.warpedDIR.text = self.dirPaths.warpDirectory
    self.ROIfilePath.text = self.dirPaths.roiFile
    self.fiducialsDIR.text = self.dirPaths.fiducialsDirectory
     
  def onCreateHierarchyButton(self):
    self.updateDirpaths()
    element = self.predefinedList[self.predefPatBox.currentIndex]
    fourD = True
    if element == "SingleFC":
       fourD  = False
    logic = ReadRegistrationNodeLogic()
    logic.registerAndDirqa(self.dirPaths,fourD)

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

  def registerAndDirqa(self,dirPaths,fourD):
    patientName = dirPaths.name
    
    #Reference phase remains 00 throughout registration
    referencePhase = dirPaths.referencePhase
    
    #First make subject Hierarchy Node
    #Create Patient Node
    shNode = slicer.util.getNode('SubjectHierarchy')
    if not shNode:
      print "Can't find Subject Hierarchy node!"
      return
    sceneItemID = shNode.GetSceneItemID()
    patientItemID = shNode.GetItemByUID('DICOM',patientName)
    if not patientItemID:
      patientItemID = shNode.CreateSubjectItem(sceneItemID, patientName);
      shNode.SetItemUID(patientItemID, 'DICOM', patientName);

    #Create reg quality node
    regQualityNode = slicer.vtkMRMLRegistrationQualityNode()
    slicer.mrmlScene.AddNode( regQualityNode )
    regQualityNode.SetName( patientName + 'regQualityNode')
    if not regQualityNode is None:
       shNode.SetItemAttribute( patientItemID, REGQANODEID, regQualityNode.GetID() )
    
    dirList = [dirPaths.warpDirectory, dirPaths.vectorDirectory, dirPaths.roiFile]
    
    ctDIRItemID = self.createChild(patientItemID,CT)
    shNode.SetItemAttribute(ctDIRItemID,FILEPATH,dirPaths.ctDirectory)
    shNode.SetItemAttribute(patientItemID,CT + ITEMID,str(ctDIRItemID))
    
    doseDIRItemID = self.createChild(patientItemID,'Dose')
    shNode.SetItemAttribute(doseDIRItemID,FILEPATH,dirPaths.doseDirectory)
    shNode.SetItemAttribute(patientItemID,'Dose' + ITEMID,str(doseDIRItemID))
    
    fiducialsDIRItemID = self.createChild(patientItemID,'Fiducials')
    shNode.SetItemAttribute(fiducialsDIRItemID,FILEPATH,dirPaths.fiducialsDirectory)
    shNode.SetItemAttribute(patientItemID,FIDUCIAL + ITEMID,str(fiducialsDIRItemID))
     
     #Create directories
    modList = [WARPED_IMAGE, VECTOR_FIELD,ROI]
    modDictID = {}
    for i in range(len(dirList)):
      itemID = self.createChild(patientItemID,modList[i])
      shNode.SetItemAttribute(itemID,FILEPATH,dirList[i])
      shNode.SetItemAttribute( patientItemID, modList[i] + ITEMID, str(itemID) )
      modDictID[modList[i]] = itemID
      if modList[i] == ROI:
         shNode.SetItemAttribute( itemID, REGISTRATION_TYPE, ROI )
    
     
    #DIRQA
    #shNode.SetItemAttribute(patientItemID,DIRQAFILE,dirqaFile)
   
    shNode.SetItemAttribute(patientItemID,'ReferenceNumber',referencePhase)
    shNode.SetItemAttribute(patientItemID,'PatientName',patientName)
   
    #Populate Subject Hierarchy with all phases
    if not os.path.exists(dirPaths.ctDirectory): 
      print "No directories."
      return

    phaseDictID = {}
    refPhaseID = 0
     
    #Find CTs
    for fileName in os.listdir(dirPaths.ctDirectory):
      index = fileName.find('.nrrd')
      if not index > -1:
         index = fileName.find('.nhdr')
         if not index > -1:
           continue
      
      #Special check, so we have 4DCT files
      if fourD:
         if not fileName[index-3] == "_":
            print "Not part of 4DCT " + fileName
            continue
      
      #Try to find out, which phase do we have
      if fourD:
         phase = fileName[index-2:index]
         try:
            int(phase)
         except ValueError:
            print "No phase: " + phase
            continue
      else:
         phase = "UNKNOWN"
     
      #Create New phase in subject hierarchy
      filePrefix, fileExtension = os.path.splitext(fileName)
      ctItemID = self.createChild(ctDIRItemID,filePrefix)
      shNode.SetItemAttribute(ctItemID, FILEPATH, dirPaths.ctDirectory + fileName)
      shNode.SetItemAttribute(ctItemID, PHASENUMBER, phase)
      shNode.SetItemAttribute(ctItemID, REGISTRATION_TYPE, IMAGE)
      shNode.SetItemAttribute(ctItemID, INVERSE, "1")
      referencePhase
      #Which is the reference phase in a non 4D registration?
      if not fourD:
         if fileName.find("Plan") > -1:
            shNode.RemoveItemAttribute(ctItemID, INVERSE)

      phaseDictID[phase] = ctItemID

      #Skip for reference phase
      if fourD and phase == referencePhase:
         shNode.SetItemAttribute(patientItemID,'ReferenceImageItemID',str(ctItemID))
         phaseDictID['refPhaseID'] = ctItemID
         shNode.SetItemAttribute(patientItemID,REFERENCENUMBER,str(phaseDictID['refPhaseID']))
         shNode.RemoveItemAttribute(ctItemID, INVERSE)
     
    #Find warped images and vector fields
    for i in range(2):
      if i == 0:
         warpedImageOn = True
      else:
         warpedImageOn = False
      if not os.path.exists(dirList[i]):
         print "No directory " + dirList[i]
      else:
         for file in os.listdir(dirList[i]):
            #Read 4D-CT registration case
            if fourD:
               fixedPhase, movingPhase = self.checkDirectoryFor4D(file, warpedImageOn)
               if not fixedPhase == -1 and not movingPhase == -1:
                  #print file
                  filePrefix, fileExtension = os.path.splitext(file)
                  itemID = self.createChild(modDictID[modList[i]],filePrefix)
                  shNode.SetItemAttribute(itemID,REGISTRATION_TYPE,modList[i])
                  shNode.SetItemAttribute(itemID,FIXEDIMAGEID,str(phaseDictID[fixedPhase]))
                  shNode.SetItemAttribute(itemID,MOVINGIMAGEID,str(phaseDictID[movingPhase]))
                  shNode.SetItemAttribute(itemID,FILEPATH, dirList[i] + file)
                  if file.find('_x.nrrd') > -1:
                     shNode.SetItemAttribute(itemID,TRIPVF, "1")
                  if movingPhase == referencePhase:
                     shNode.SetItemAttribute(itemID,INVERSE,"1")
            else:
               #Default case
               fixedPhase, movingPhase = self.checkDirectoryForFiles(file, warpedImageOn)
               if not fixedPhase == -1 and not movingPhase == -1:
                  #print file
                  filePrefix, fileExtension = os.path.splitext(file)
                  itemID = self.createChild(modDictID[modList[i]],filePrefix)
                  shNode.SetItemAttribute(itemID,REGISTRATION_TYPE,modList[i])
                  shNode.SetItemAttribute(itemID,FILEPATH, dirList[i] + file)
                  if file.find("plan_refPhase") > -1:
                     shNode.SetItemAttribute(itemID, INVERSE,"1")

       #self.checkDirqaDirectory(dirqaDirectory, regQAItemID, phase)
       #self.checkDirqaFile(dirqaFile, regQAItemID, phase)

    #Find fiducials
    if not os.path.exists(dirPaths.fiducialsDirectory):
         print "No directory " + dirPaths.fiducialsDirectory
    else:
       for fileName in os.listdir(dirPaths.fiducialsDirectory):
          filePrefix, fileExtension = os.path.splitext(fileName)
          if fileExtension.lower() == '.fcsv':
            if fourD:
               phaseStr = fileName[1]
               try:
                  int(phaseStr)
               except ValueError:
                  print "No phase: " + phaseStr
                  continue
               phase = '0' + phaseStr
               shNode.SetItemAttribute(itemID,FIXEDIMAGEID,str(phaseDictID[phase]))
            else:
               if fileName.find('T00') < 0 and fileName.find('Plan') < 0:
                  continue
               
            itemID = self.createChild(fiducialsDIRItemID,filePrefix)
            if not fourD and fileName.find('T00') > -1:
               shNode.SetItemAttribute(itemID,INVERSE,"1")
            shNode.SetItemAttribute(itemID,REGISTRATION_TYPE,FIDUCIAL)
            shNode.SetItemAttribute(itemID,FILEPATH, dirPaths.fiducialsDirectory + fileName)
            
       
    shNode.SetItemExpanded(modDictID[WARPED_IMAGE],False)
    shNode.SetItemExpanded(modDictID[VECTOR_FIELD],False)
    shNode.SetItemExpanded(ctDIRItemID,False)
    shNode.SetItemExpanded(fiducialsDIRItemID,False)


  def checkDirectoryFor4D(self, file, warpedImageOn):
     fixedPhase = movingPhase = -1
     if warpedImageOn:
        index = file.find('_warped.nrrd')
        if index > -1:
             #Find out warpedimage or invWarpedImage
             fixedPhase = file[index-2:index] 
             if file.find('fix') > -1:
                movingPhase = file[index-8:index-6]
             else:
                movingPhase = file[index-5:index-3]
     else:
        index1 = file.find('_vf.mha')
        #index1 = -1
        index2 = file.find('_x.nrrd')
        #index2 = -1
        #if file.find('cuda') < 0:
           #index2 = -1
        #index2 = -1
        #if file.find('MI') > -1 and file.find('cuda') < 0 and file.find('122') > -1:
           #return (-1,-1)
        if index1 > -1 or index2 > -1:
           if index1 > -1:
              index = index1
           else:
              index = index2
           fixedPhase = file[index-2:index]
           movingPhase = file[index-5:index-3]
              
           indexFix = file.find('fix')
           indexMov = file.find('mov')
           if indexFix > -1:
              fixedPhase = file[indexFix+3:indexFix+5]
           if indexMov > -1:
              movingPhase = file[indexMov+3:indexMov+5]

     try:
         int(fixedPhase)
         int(movingPhase)
     except ValueError:
         return (-1,-1)
     
     return (fixedPhase, movingPhase)
      
  def checkDirectoryForFiles(self, file, warpedImageOn):
     fixedPhase = movingPhase = -1
     if warpedImageOn:
        index = file.find('.nrrd')
        if index > -1:
           return (0,0)          
     else:
        index = file.find('.mha')
        if index > -1:
           return (0,0)
     
     return (fixedPhase, movingPhase)
  
  def checkDirqaDirectory(self, dirqaDirectory, phaseItemID, phase):
    shNode = slicer.util.getNode('SubjectHierarchy')
    if os.path.exists(dirqaDirectory):
      for file in os.listdir(dirqaDirectory):
        index = file.find('ref00.nhdr')
        if index > -1:
          #Find out warpedimage or invWarpedImage
          print file
          if ('0' + file[index-1]) == phase:
            dirqaID = self.createChild(phaseItemID,NAME_INVCONSIST)
          else:
            print "Cannot find phase number. Index: " + str(index)
            continue
          if dirqaID:
            shNode.SetItemAttribute(dirqaID,'FilePath',dirqaDirectory+file)
  
  def checkDirqaFile(self, dirqaFile, phaseItemID, phase):
     from RegistrationHierarchy import RegistrationHierarchyLogic
     
     if not os.path.isfile(dirqaFile):
       return
     
     regHierarchyLogic = RegistrationHierarchyLogic()
     f = open(dirqaFile,"r")
     content = f.read().split('\n')
     i = 0
     checks = [VECTOR_FIELD, INVVECTOR_FIELD,NAME_INVCONSIST, NAME_JACOBIAN,
               NAME_INVJACOBIAN, ABSOLUTEDIFFERENCE, INVABSOLUTEDIFFERENCE, NAME_DEFABS,
               "Abs"+VECTOR_FIELD, "Abs"+INVVECTOR_FIELD, NAME_PERCENT, NAME_INVPERCENT, NAME_JACCONSIST]

     values = ["Mean","STD","Max","Min"]
     directions = ["x","y","z"]
     while i < len(content):
        if content[i].find('Phase') > -1:
          phaseNumber = int(content[i].split()[1])
          #print phaseItemID + phase
          if phaseNumber == int(phase):
            n = i+1
            statistics = [0,0,0,0]
            vectorStatistics = [[0,0,0],[0,0,0],[0,0,0],[0,0,0]]
 
            while n < len(content):
               #Break when you come to the next phase
               if content[n].find('Phase') > -1:
                  return
               if not content[n]:
                  n += 1
                  continue
               for check in checks:
                  if content[n].split()[0] == check:
                     n += 1
                     #if check == ABSOLUTEDIFFERENCE or check == INVABSOLUTEDIFFERENCE or check == NAME_DEFABS:
                       #continue
                     if check == VECTOR_FIELD or check == INVVECTOR_FIELD:
                       for j in range(3):
                          for k in range(4):
                             if content[n].split()[0] == (values[k]+"_"+directions[j] +":"):
                                vectorStatistics[k][j] = float(content[n].split()[1])
                                n += 1

                       regHierarchyLogic.writeStatistics(phaseItemID.GetItemChildWithName(phaseItemID,
                                     check),vectorStatistics,True)      
                       n -= 1

                     else:
                        if check == ("Abs"+VECTOR_FIELD):
                          node = phaseItemID.GetItemChildWithName(phaseItemID, VECTOR_FIELD)
                        elif check == ("Abs"+INVVECTOR_FIELD):
                          node = phaseItemID.GetItemChildWithName(phaseItemID, INVVECTOR_FIELD)
                        else:
                          node = self.createChild(phaseItemID, check)
                        for k in range(4):
                           statistics[k] = float(content[n+k].split()[1])
                        regHierarchyLogic.writeStatistics(node,statistics,False)
                        n += 3
               n += 1

        i += 1
     
  
  def createChild(self,hierarchyItemID,string, UIDtext = ""):

    shNode = slicer.util.getNode('SubjectHierarchy')
    name = shNode.GenerateUniqueItemName(string)
    itemID = shNode.CreateFolderItem(hierarchyItemID,string)
    shNode.SetItemUID(itemID,'DICOM',name + UIDtext)
    
    return itemID


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

class RegistrationHierarchyHelp():
   def __init__(self, case = ""):
      self.name = ''
      self.patDirectory = ''
      self.ctDirectory  = ''
      self.doseDirectory = ''
      self.contourFile = ''
      self.fiducialsDirectory = ''
      self.vectorDirectory = ''
      self.warpDirectory = ''
      self.roiFile = ''
      self.referencePhase = ''
      
   def createFromTemplate(self,case, name = ""):
      if case == "FC":
         if name == "":
            patName = 'Lung001'
         else:
            patName = name
         
         patientDirectory = '/u/kanderle/AIXd/Data/FC/' + patName + '/'
         self.name = patName
         self.patDirectory  = patientDirectory
         self.ctDirectory   = patientDirectory + 'CTX/'
         self.doseDirectory = patientDirectory + 'Dose/'
         self.contourFile   = patientDirectory + 'Contour/4D/ByTRiPTrafo/' + patName + "_00.binfo"
         self.vectorDirectory = patientDirectory + 'Registration/4D/'
         self.warpDirectory = self.vectorDirectory
         self.fiducialsDirectory = patientDirectory + '4D/NRRD/'
         self.roiFile = patientDirectory + 'Registration/' + "R.acsv"
         self.referencePhase = "00"
      elif case == "SingleFC":
         if name == "":
            patName = 'Lung006'
         else:
            patName = name
         
         patientDirectory = '/u/kanderle/AIXd/Data/FC/' + patName + '/'
         self.name = patName
         self.patDirectory  = patientDirectory
         #self.ctDirectory   = patientDirectory + 'CTX/'
         self.ctDirectory = patientDirectory + 'RegQA/'
         self.doseDirectory = patientDirectory + 'Dose/'
         self.contourFile   = patientDirectory + 'Contour/4D/ByTRiPTrafo/' + patName + "_00.binfo"
         #self.vectorDirectory = patientDirectory + 'Registration/4D/'
         self.vectorDirectory = patientDirectory + 'RegQA/Demons/'
         self.warpDirectory = self.vectorDirectory
         self.fiducialsDirectory = patientDirectory + '4D/NRRD/'
         self.roiFile = patientDirectory + 'Registration/' + "R.acsv"
         self.referencePhase = "00"
      elif case == "MDACC":
         if name == "":
            patName = 'Patient122'
         else:
            patName = name
         patientDirectory = '/u/motion/AIXd/Data/PatientData/MDACC/' + patName + '/Pat' + patName[-3:] + 'wk0/4DSet01_original/'
         self.name = patName
         self.patDirectory  = patientDirectory
         #self.ctDirectory   = patientDirectory + 'CTX/'
         self.ctDirectory = patientDirectory + 'CTX/'
         self.doseDirectory = patientDirectory + 'Dose/'
         self.contourFile   = patientDirectory + 'Contour/4D/ByTRiPTrafo/' + patName + "_00.binfo"
         #self.vectorDirectory = patientDirectory + 'Registration/4D/'
         self.vectorDirectory = patientDirectory + 'Registration/ByPlastimatch/'
         #self.vectorDirectory = patientDirectory + 'Registration/ByPlastimatch_New/'
         self.warpDirectory = self.vectorDirectory
         self.fiducialsDirectory = patientDirectory + '4D/NRRD/'
         self.roiFile = patientDirectory + 'Registration/' + "R.acsv"
         self.referencePhase = "05"
         
      else:
         print "Unknown case" + case
         return
