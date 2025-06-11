## TODO
- [ ] Add the other descriptors to the keypoints class 
- [ ] Convert reporting to c++ (maybe)
- [x] Re-run the visual check for the homography on the bad images (may try to fix homography)
- [x] Did the re-sizing cause the homography to be off? (yes you are dumb stop being dumb)
- [x] Adjust the visualisation to not use hconcat just display two images side by side
- [x] Add graphing to report
- [x] Fix Buffer overflow issues in RGBSIFT (See TODO in RGBSIFT.cpp)
- [x] Test the other descriptors in keypoints before moving on to refactor
- [x] Add configuration class to store configuration data
- [x] move process folder to a class of static methods
- [x] move process descriptors to a class of static methods

## Test spec for program using c++ opencv and boost if needed

Goal of the program use the base images and the homo-graphical maps to calculate the mean average precision of the descriptors in order to evaluate the effectiveness of the descriptors and different t processing methods such as stacking fo descriptors and pooling descriptors.

Project structure:

/src -> main and other implementation code
/data -> data sub folder
/results -> results folder
/keypoints -> experimental descriptor implementations

Data sub folder structure and file descriptions:

/data/i_ajuntament
```
├── 1.ppm
├── 2.ppm
├── 3.ppm
├── 4.ppm
├── 5.ppm
├── 6.ppm
├── attribs.txt
├── H_1_2
├── H_1_3
├── H_1_4
├── H_1_5
└── H_1_6
```

1. 1.ppm reference image
2. 2.ppm - 6.ppm images of the same scene under differing conditions
3. H_1_2 - H_1_6 homo-graphical maps between reference images and other images

format of homo-graphical files

```
0.79208 0.010314 26.019
-0.023778 0.92337 43.513
-0.00011513 1.2161e-05 1.0003
```


```pseudo

noise levels = {easy:0, medium:1, hard:2}

for each folder in data{

        create folder in /results with the folder name to store all data for the images in that folder
	
        generate key points and descriptors for refernce image 1.ppm
        
        make results.csv with collum headers: headers (image_refernce,precision,recall,accuracy,F1)
        
        generate 2 files ref_keypoints.csv and ref_descriptors.csv{
            
                ref_keypoints.csv -> store keypoint data in csv with headers (all float values):
                header -> x,y,size,angle,response,octave,class_id
                
                ref_descriptors.csv -> store descriptor data (float or int values depending on descriptor):
                header -> index# ie 1,2,3,4.... 
        }
        
        for images 2.ppm - 6.ppm{
        
                for noise level [easy,medium,hard]{
                
                         generate keypoints for image under consideration
                         generate descriptors for image under consideration (refactor later to add processing)
                
                         generate 2 files ref_keypoints.csv and ref_descriptors.csv{
                
                                <noise level first letter,image#>.csv example e2_keypoints.csv-> 
                                store keypoint data in csv with headers (all float values):
                                header -> x,y,size,angle,response,octave,class_id
                                
                                <noise level first letter,image#>.csv example e2_descriptors.csv-> 
                                store descriptor data (float or int values depending on descriptor):
                                header -> index# ie 1,2,3,4.... 
                        }
                        
                        read in homography matrix
                        
                        precision = calculate precision of descriptors 
                        recall = calculate recall of descriptors
                        Accuracy = calculate accuracy of descriptors
                        F1 = calculate F1 score of descriptors
                        
                        append row to results.csv -> (image ref (e2,m2,h2 ect),precision,recall,accuracy,F1)
                        
                        apply noise to image
                } 
        }

}
```

### Precision

Precision measures the proportion of positive identifications that were actually correct. It's calculated as the number of true positive matches divided by the total number of positive matches (true positives plus false positives).

![Precision](https://latex.codecogs.com/png.latex?%5Cdpi%7B150%7D%20%20Precision%20%3D%20%5Cfrac%7BTrue%20Positives%7D%7BTrue%20Positives%20&plus;%20False%20Positives%7D)
```math
Precision = \frac{True Positives}{True Positives + False Positives}
```

### Recall

Recall measures the proportion of actual positive matches that were correctly identified. It's useful for understanding how many of the true matches were found.

![Recall](https://latex.codecogs.com/png.latex?%5Cdpi%7B150%7D%20%%20Recall%20%3D%20%5Cfrac%7BTrue%20Positives%7D%7BTrue%20Positives%20&plus;%20False%20Negatives%7D)
```math
Recall = \frac{True Positives}{True Positives + False Negatives}
```

### F1-Score

F1-Score is the harmonic mean of precision and recall. It provides a single metric to assess the balance between precision and recall, useful when you want to consider both false positives and false negatives.

![F1-Score](https://latex.codecogs.com/png.latex?%5Cdpi%7B150%7D%20%20F1-Score%20%3D%202%20%5Ctimes%20%5Cfrac%7BPrecision%20%5Ctimes%20Recall%7D%7BPrecision%20&plus;%20Recall%7D)

```math
F1-Score = 2 \times \frac{Precision \times Recall}{Precision + Recall}
```

### Accuracy

Accuracy measures the proportion of true results (both true positives and true negatives) among the total number of cases examined. It may not be as informative when the number of matchings (positives) and non-matchings (negatives) greatly varies.
![Accuracy](https://latex.codecogs.com/png.latex?%5Cdpi%7B150%7D%20%20Accuracy%20%3D%20%5Cfrac%7BTrue%20Positives%20&plus;%20True%20Negatives%7D%7BTotal%20Number%20of%20Samples%7D)


```math
Accuracy = \frac{True Positives + True Negatives}{Total Number of Samples}
```

### Mean Average Precision (MAP)

Mean Average Precision is a measure to evaluate the quality of object detectors and ranked retrieval systems. For each query, the Average Precision (AP) is calculated from the precision at each threshold in the ranked sequence of retrieved items. The MAP is then the mean of the AP scores for all queries.

![MAP Equation](https://latex.codecogs.com/png.latex?MAP%20%3D%20%5Cfrac%7B1%7D%7BQ%7D%20%5Csum_%7Bq%3D1%7D%5E%7BQ%7D%20AP_%7Bq%7D)

Where `Q` is the number of queries, and `AP_{q}` is the average precision for query `q`.


## Plan for locked in keypoints class

This class will make a set of keypoints for each image in the /data folder for each sub folder in data there will
be a matching folder in /reference_keypoints and each folder will contain a csv file of the keypoints for each of the images 
in the data folders.

Each subfolder in /data is structured as follows:

/data/i_ajuntament
```
├── 1.ppm
├── 2.ppm
├── 3.ppm
├── 4.ppm
├── 5.ppm
├── 6.ppm
├── attribs.txt
├── H_1_2
├── H_1_3
├── H_1_4
├── H_1_5
└── H_1_6
```

The resulting folder structure will be:

/reference_keypoints/i_ajuntament
```
1ppm.csv
2ppm.csv
3ppm.csv
4ppm.csv
5ppm.csv
6ppm.csv
```

The keypoints will be detected using the SIFT algorithm and the keypoints will meet the following criteria:

Get the keypoints from the 1.ppm image
Sorted by the response value take the top 75% of the keypoints
Using the homography matrix make sure all keypoints are present in the other images removing those that are not present in the other images

Store the keypoints for 1.ppm in 1ppm.csv 

use the homography matrix to transform the keypoints to the other images and store the keypoints in the corresponding csv files

## Plan to adjust the keypoints produced by the locked in keypoints class

We need to filter the keypoints further to remove those that are not present in the other images. This will be done by using 
the homography matrix to transform the keypoints to the other images and removing those that are not present in the other images.

So first we filter out the keypoints that are within 40px of the sides as we are going to use 65X65px
patches to generate cnn descriptors and we need to make sure that the patches are within the image.

Then we will use the homography matrix to transform the keypoints to the other images and remove 
those that are not present in the other images.

Finally 




