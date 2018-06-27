# Slicer RegistrationQA Introduction

**A registration quality assurance module for 3D Slicer**

Dr. Kristjan Anderle
(kristjan.anderle@gmail.com)

# Introduction

Slicer RegQA extension enables quality assurance (QA) for image registration. It can perform different test, both qualitative and quantitative to estimate registration quality. It can also create output files, which can serve as documentation for specific registration QA.

# Terminology

To provide a clear and consistent description of the module, an overview of the nomenclature used is given here.

- _Fixed image_ – the image that serves as a reference position in the registration (image that is being registered to).
- _Moving image_ – the image that is matched to the reference image (image that is being registered from).
- _Reference image_ – the initial choice of fixed image. The choice is based on the registration usage (i.e. opposite registration directions are need for contour propagation and dose wrapping). In a 4D-CT registration a single phase is usually chosen as a reference phase.
- _Transformation_ – the result from registration algorithm is a transformation that transforms moving to fixed image. It can either be a matrix for rigid/affine registrations or a vector field for deformable registrations.
- _Warped image_ – the result of applying a transformation to the moving image. It should be as close to the fixed image as possible.
- _Forward registration_ – registration when reference image is the fixed image. The term &quot;forward&quot; can also be applied to corresponding items (warped image, transformation, Jacobian determinant).
- _Backward registration_ – registration when reference image is the moving image. The term &quot;backward&quot; can also be applied to corresponding items.

# Measures

The registration QA is based on the recent AAPM task report [1], which recommends verification of 8 different measures. The reader of this document is encouraged to read the document [1], since it provides detailed instructions on registration QA.

Eight different inputs are required to evaluate all measures for a single registration. Measures 1 – 4 are qualitative that require user to spot any irregularities in registration, while measures 5 – 8 are quantitative, where each measure has a tolerance value. All measures, except measure 8, can be verified for both forward and backward registration.

 
![al text](https://github.com/gsi-biomotion/SlicerRegistrationQA/blob/master/Screenshots/MeasuresList.png "Measures")

Figure 1 - Schematic presentation of registration QA measures and the required inputs. I.e. for forward measure 4, contours from fixed image and forward warped image are required.

# Module Overview

The RegQA module is divided into three parts, as shown in Error! Reference source not found.. In the first part user can mark all the inputs. The possible inputs are shown in Figure 1. Additionally, a region of interest (ROI) can be created around the selected segment. The segment has to be first chosen as an input. The ROI is later used in measures 3, 7 and 8. Among two images we can have two registrations (forward and backward) and corresponding verifications. User can always switch between the two registrations, which will influence the measure verification execution.

The second part of the module is where different measures can be verified. Buttons for measures become available as soon as necessary inputs are selected in the first part. The overlay of the images has two additional options – inverse color and match window/level. The inverse color will set the overlay images in contrast colors (red and cyan) – all perfectly aligned structures will be therefore displayed in gray. Any deviations will be shown in either red or cyan for fixed or warped (moving) image, respectively. The match window/level checkbox will copy window and level values from the fixed image to the overlaying one. Inverse color and match window/level options are useful when inspecting intra-modality registration.

![overview](https://github.com/gsi-biomotion/SlicerRegistrationQA/blob/master/Screenshots/RegQAOverview.png "RegistrationQA Module overview")

Figure 2 - RegQA module. Three different parts of module are highlighted – input (1), measures (2) and table (3).

The checkerboard and absolute difference correspond to measures 1 and 3. Start movie button automatically increases the slices on the checked views above the button (red, yellow or green). Flicker rapidly switches between foreground and background image, which have to be set previously, either with overlay button or manually.

Buttons fiducial distance, contour measures, Jacobian determinant and inverse consistency are all quantitative and correspond to measures 5, 6, 7 and 8, respectively. A short overview for them:

- *Fiducial distance* calculates distance between fiducials before and after registration. Fiducials (called markups in Slicer) have to be marked in fixed and moving image in the same order – Markup 1 in fixed image has to correspond to Markup 1 in moving image.
- *Contour measures* calculate Hausdorff distance and dice metric between contour in fixed and moving image. Currently only one contour (called segment in Slicer) can be selected. It has to be selected in both, fixed and moving image.
- *Jacobian determinant* calculates Jacobian determinant on the vector field. Forward and backward Jacobian determinant can be calculated on forward and backward vector fields.
- *Inverse consistency* calculates consistency between forward and backward vector field. It has only one value.

The third part on Figure 2 is a table with all the information about the current registration QA – all the inputs with their corresponding nodes and quantitative results from corresponding measures, if they were calculated. The table can also be exported using _Save Data_ button in Slicer. An example of the table is shown in Table 1.

# Tutorial

Download the data from [here](https://github.com/gsi-biomotion/SlicerRegistrationQA/releases/download/0.0.1/RegistrationData.zip) (data downloaded and modified from [2])
and unzip it in a directory. The images in Image directory represent the two extreme breathing phases. Image1 has been chosen as the reference image and a registration in forward and backward direction has been performed (using Plastimatch module in SlicerRT). The resulting warped images and vector fields are denoted as \_warped and \_vf, respectively and are stored in RegFiles directory. There are contours and fiducials in Contours and Fiducials directory, respectively, corresponding to both images.

## Assigning roles

Load images, warped images, vector fields, contours and fiducials in Slicer. Navigate to _Registration Quality Assurance_ module. All inputs should be seen in the input section of module. Next, we have to assign all inputs to their corresponding roles in registration QA, as shown in Figure 1. We&#39;ll assign Image 1 to reference image first. Right click on Image 1, and then select _Registration QA -&gt; Assign node to: -&gt; Image -&gt; Reference Image_, see Figure 3. If you scroll down in the module, the Image 1 should be seen in the Reference Image row. Repeat the procedure for all other input, assigning them to the right roles. For contours you have to select only GTV63 for both images. If you&#39;re unsure of the right nodes, you can check the Table 1 at the end of the document.

![roles](https://github.com/gsi-biomotion/SlicerRegistrationQA/blob/master/Screenshots/SpecifyRoles.png "Assing roles to nodes")

Figure 3 - Assigning roles to specific nodes.

![overlay](https://github.com/gsi-biomotion/SlicerRegistrationQA/blob/master/Screenshots/ContrastColorsOverlay.png "Overlay images with contrast colors")

Figure 4 - Overlay with Contrast colors and matched windows and level.

## Registration QA checks

Uncheck the eyes next to the Contours\_Image1 and Contours\_Image2, seen we want to hide their display momentarily.  Next, click _Overlay Warped Image_ button. The forward warped image will be displayed on top of reference image (Image 1). This corresponds to Measure 2 from Figure 1. Since we are overlaying two CT the difference is not clear. Let&#39;s check the _Contrast Colors_ and _Match Window/Level_ checkboxes. Now click _Overlay Warped Image_ button again ( Figure 4). Forward warped and reference image are now displayed in opposite colors (cyan and red). Everything that aligns is now shown in gray and deviations are shown in the respective colors. If you click _Overlay Moving Image_ button you can see the difference from before registration.

Next we can click _Checkerboard_ button. The resulting image shows alternating tiles of forward warped and reference image. This corresponds to Measure 1 and is designed to spot errors in edges.

_Absolute difference_ button will produce an image where each voxel is an absolute difference between values in forward warped and reference image. It corresponds to Measure 3 and mean square error metric, which is commonly used in registration. Details shown in Figure 5.

![abs](https://github.com/gsi-biomotion/SlicerRegistrationQA/blob/master/Screenshots/AbsoluteDifference.png "Absolute Difference example")
Figure 5 - Absolute difference overlay on top of reference image.

_Start Movie_ and _Flicker_ button help user to focus on images. You can select checkbox in front of the red box and click _Start Movie._ The red view should start scrolling slices. You can check other two as well. When you&#39;re finished, press _Stop Movie_. Now press _Flicker._ The views will rapidly change between forward warped and reference image. Press _Flicker_ again to stop.

To inspect Measure 4 you just have to display only forward warped image and contours from image 1 (reference image). Now you can investigate if contours overlay the desired structures. This usually requires an expert, since contour delineation is not a trivial task.

Next, click _Fiducial Distance_, which corresponds to Measure 5. This calculates distance between fiducials in reference and moving image before and after registration. Results can be found in the table in the bottom part of the module.

Similarly, _Contour Measures_ will calculate Hausdorff distance and dice metric between contours before and after registration. You can click the button and investigate results in the table. It corresponds to Measure 6.

![invers](https://github.com/gsi-biomotion/SlicerRegistrationQA/blob/master/Screenshots/InverseConsistencyInsideROI.png "Inverse Consistency inside the ROI)

Figure 6 - Inverse consistency calculated inside the ROI around GTV contour.

Jacobian determinant and Inverse Consistency (Measures 7 and 8) will be calculated on the whole CT, if nothing else is specified. This does not bring the desired information of registration quality, since a large part of the CT is air. Therefore we want to create a region of interest (ROI) where Jacobian determinant and inverse consistency will be calculated. We can create ROI manually in Slicer and then assign it the right role. Or we can create it automatically by clicking the _Create ROI around segments_, which will create ROI around the contours we assigned. Click this button now and a ROI should appear. Next, click on _Jacobian Determinant_ and _Inverse Consistency_ buttons. Two images will be displayed over the reference image, one with Jacobian determinant and the other with Inverse Consistency values. The later are normalized to maximum voxel spacing, since inverse consistency error should be smaller than maximum voxel spacing. This means that all inverse consistency values should be below 1. In our case the values are indeed below one, indicating a good registration quality.

## Change registration direction

All of the checks explained so far have been for forward registration (except for inverse consistency, which applies to both). We can simply switch which registration direction we want to inspect by clicking _Change to Back. Reg._ Click it now. The button will change to _Change to Forw. Reg._ This enables user to switch between two directions. Repeat now all checks for backward registration.

The table should now be completely filled and be the same as Table 1. Beside in the bottom part of the module you can inspect it in detail in Tables module in Slicer. The table can also be saved to the hard drive and can serve as a registration QA documentation. Alongside should also be screenshots for different measures, similarly as on Figure 7.

![Example](https://github.com/gsi-biomotion/SlicerRegistrationQA/blob/master/Screenshots/MeasuresExample.png "Measures example")

Figure 7 - Examples of Measures 1-4. A checkerboard image with alternating tiles between fixed and warped image is shown on image 1. The image 2 shows overlay of fixed and warped image, where both images are display in contrast colors. An absolute difference between fixed and warped image is shown on image 3. Contour overlay on warped image is shown on image 4.

## Create Registration Hierarchy

All the input can be manually loaded; however this takes some time and is tedious work when multiple there&#39;s multiple registrations. An auxiliary module called _Create Registration Hierarchy_ can create a hierarchy in Slicer, which can be then used to automate the process. The module is written for a specific nomenclature, so you should modify it to serve your needs.

If you open _Create Registration Hierarchy_ module and select _Example_ from the drop box all the lines should fill up. Replace the text so that it will point to the location on your hard drive where you extracted zip file from previous tutorial. Next click the _Create Subject Hierarchy_ button. If you navigate now back to _Registration Quality Assurance_ module you should see a Subject hierarchy with the name _Example_. Make sure that node under _Parameter Set_ is called _ExampleregQANode_. If not, switch to it. Next right click on the _Example_ in hierarchy and click _Registration QA -&gt;Load from disk_. This will load all the files from disk directly into Slicer and assign them the right nodes. With that you can immediately start with specific checks (except for loading of contours which is yet to be implemented).

Additionally, you can right click on VectorFields folder in hierarchy view and select _Registration QA -&gt;Calculate Registration QA_, which will perform Measures 7 and 8 on the vector fields. There can also be several vector fields there, such as in 4DCT registration, significantly improving the process. The results for Measures 7 and 8 are also written in two separate tables.



Table 1 - Input and results from registration QA in from of a table.

| Column 1 | Column 2 | Column 3 | Column 4 | Column 5 |
| --- | --- | --- | --- | --- |
| Reference Image | Image1 |   |   |   |
| Moving Image | Image2 |   |   |   |
| Forward Warped Image | Forward\_warped |   |   |   |
| Backward Warped Image | Backward\_warped |   |   |   |
| Forward vector field | Forward\_vf |   |   |   |
| Backward vector field | Backward\_vf |   |   |   |
| Fixed Contour | GTV63 |   |   |   |
| Moving Contour | GTV63 |   |   |   |
| Fixed Fiducial | Fiducials\_Image1 |   |   |   |
| Moving Fiducial | Fiducials\_Image2 |   |   |   |
| ROI | GTV63 |   |   |   |
| Measure: | Max | Min | Mean | STD |
| AbsDiff (for) | 875 | 0 | 57.9797 | 83.7474 |
| AbsDiff (back) | 894 | 0 | 49.8978 | 75.8831 |
| Jacobian (for) | 1.06519 | 0.841157 | 0.940259 | 0.0435678 |
| Jacobian (back) | 1.18093 | 0.932946 | 1.06183 | 0.0501429 |
| InvConsist | 0.571222 | 0.00460278 | 0.239684 | 0.103036 |
|   | Mean Hauss. Before | Mean Hauss. After | Dice Coeff. Before | Dice Coeff. After |
| Contour (for) | 2.19623 | 1.27664 | 0.839715 | 0.901302 |
| Contour (back) | 2.19519 | 1.13237 | 0.839845 | 0.91102 |
| Fiducial | Distance Before | Distance After |   |   |
| Image1-1 | 3.80245 | 2.23676 |   |   |
| Image1-2 | 4.16145 | 1.96649 |   |   |
| Image2-1 | 3.80245 | 2.15441 |   |   |
| Image2-2 | 4.16145 | 1.90811 |   |   |

# References

[1] Brock KK, et al: Use of image registration and fusion algorithms and techniques in radiotherapy: Report of the AAPM Radiation Therapy Committee Task Group No. 132. Medical Physics 44, 2017

[2] https://www.aapm.org/pubs/MPPG/TPS/
