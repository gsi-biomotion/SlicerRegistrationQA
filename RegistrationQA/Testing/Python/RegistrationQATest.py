import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging

class RegistrationQATest(unittest.TestCase):
  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

    #TODO: Comment out
    #logFile = open('d:/pyTestLog.txt', 'a')
    #logFile.write(repr(slicer.modules.RegistrationQATest) + '\n')
    #logFile.write(repr(slicer.modules.dicomrtimportexport) + '\n')
    #logFile.close()

  #------------------------------------------------------------------------------
  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_RegistrationQATest_FullTest1()

  #------------------------------------------------------------------------------
  def test_RegistrationQATest_FullTest1(self):
    # Check for modules
    self.assertIsNotNone( slicer.modules.registrationqa )
    #self.assertIsNotNone( slicer.modules.segmentations )
    #self.assertIsNotNone( slicer.modules.dicom )
    
    self.registrationQAWidget = slicer.modules.registrationqa.widgetRepresentation().self()
    self.assertIsNotNone( self.registrationQAWidget )
    
    self.registrationQALogic = slicer.modules.registrationqa.logic()
    self.assertIsNotNone( self.registrationQALogic )
    self.registrationQALogic.SetMRMLScene( slicer.mrmlScene )
    
    self.registrationQANode = slicer.vtkMRMLRegistrationQANode()
    self.assertIsNotNone( self.registrationQANode )
    slicer.mrmlScene.AddNode( self.registrationQANode )
    
    #Create & test backward node
    self.registrationQALogic.CreateBackwardParameters(  self.registrationQANode )
    self.assertIsNotNone( self.registrationQANode.GetBackwardRegistrationQAParameters() )
    
    #Create & test table
    self.registrationQANode.CreateDefaultRegistrationQATable()
    self.assertIsNotNone( self.registrationQANode.GetRegistrationQATableNode() );
    self.assertIsNotNone( self.registrationQANode.GetBackwardRegistrationQAParameters().GetRegistrationQATableNode() );

    self.TestSection_VolumeTests()


    logging.info("Test finished")

  #------------------------------------------------------------------------------
  def TestSection_VolumeTests(self):
    import SampleData
    sampleDataLogic = SampleData.SampleDataLogic()
    tumor1 = sampleDataLogic.downloadMRBrainTumor1()
    tumor2 = sampleDataLogic.downloadMRBrainTumor2()
    
    #Add volumes to fixed, moving and backward and forward image
    backwardNode = self.registrationQANode.GetBackwardRegistrationQAParameters()
    
    self.registrationQANode.SetAndObserveVolumeNodeID( tumor1.GetID() )
    self.registrationQANode.SetAndObserveWarpedVolumeNodeID( tumor2.GetID() )
    backwardNode.SetAndObserveVolumeNodeID( tumor2.GetID() )
    backwardNode.SetAndObserveWarpedVolumeNodeID( tumor1.GetID() )
    
    self.assertEqual( self.registrationQANode.GetVolumeNodeID(), tumor1.GetID() )
    self.assertEqual( self.registrationQANode.GetWarpedVolumeNodeID(), tumor2.GetID() )
    self.assertEqual( backwardNode.GetVolumeNodeID(), tumor2.GetID() )
    self.assertEqual( backwardNode.GetWarpedVolumeNodeID(), tumor1.GetID() )
    
    self.registrationQALogic.SetAndObserveRegistrationQANode( self.registrationQANode )
    
    #Check Checkerboard
    self.registrationQALogic.Checkerboard()
    self.assertIsNotNone( self.registrationQANode.GetCheckerboardVolumeNodeID() )
    
    #Check Absolute Difference
    self.calculateRegQA(1)
    self.assertIsNotNone( self.registrationQANode.GetAbsoluteDiffVolumeNodeID() );
