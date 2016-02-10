/*=========================================================================

  Program:   Slicer
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

  =========================================================================*/

#include "ConvertSlicerROIToRegion.h"

#include <itkImageBase.h>
// #include <itkSlicerBoxSpatialObject.h>

itk::ImageRegion<3>
convertPointsToRegion(const itk::Point<double, 3>& p1,
                      const itk::Point<double, 3>& p2,
                      const itk::ImageBase<3>* img)
{
  // convert two points to an ITK region
  itk::Index<3> ind1, ind2;
  itk::Index<3> startind;
  itk::Index<3> upperInd = img->GetLargestPossibleRegion().GetUpperIndex();
  
  itk::Size<3>  size;

  img->TransformPhysicalPointToIndex(p1, ind1);
  img->TransformPhysicalPointToIndex(p2, ind2);

  // Find the absolute size of the bounding region
  for (int i=0;i<3;i++){
        size[i] = std::abs(ind1[i] - ind2[i]);

        startind[i] = std::min(ind1[i], ind2[i]);
          
        // Check if the boundries are ok
        if (startind[i] < 0 ) startind[i] = 0;
        if ( (startind[i] + size[i]) > upperInd[i] ) size[i] = (upperInd[i]-startind[i]) + 1;
  }

  return itk::ImageRegion<3>(startind, size);
}
