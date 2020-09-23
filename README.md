# radonAnalysis
I wrote this macro during my COOP work term at SNOLAB, a state of the art science facility that features numerous neutrino and dark matter experiments.
This repo is a ROOT C++ macro that analyzes .root files and plots Radon concentration over time and saves it to c.pdf
Two examples are given, rnAnalysis.cc and rnAnalysis2.cc. They have differences that are discussed below.

## A macro that I wrote to do the following:
1. Open a .root file
2. Extract the date from the file data or file name
3. Create a histogram using one of the leafs (fadc_channel)
4. Integrate the Radon part of the energy histogram and store the value
5. Record the total amount of time that the Radon run spanned using (ftimestamp)
6. Repeat over all the .root files and plot the concentration (integrated value/time) as a function of date

## Functions
This macro can't be copied directly, because the method I use to extract the date is specific to the file name (rnAnalysis) or specific to the time data taken (rnAnalysis2)

However, since I've seperated my macro into functions, many of them will work for people trying to replicate. 

1. listFiles will list out .root files as long as the path is updated at the top of the file
2. extractDates extracts the dates either using epoch time for rnAnalysis2 or by removing letters from the file name in rnAnalysis
3. histIntegrateRN integrates the fadc_channel in the .root file using a gaussian fit
4. runTimeInHours takes the start and end point in the .root file and finds how many hours were in a Radon measurement run
5. plot on rnAnalysis.cc uses the extractDates and a Latex workaround to put the dates on the x axis, in rnAnalysis2.cc (preferred), I use SetTimeDisplay() to put spaced out dates on the x axis.

To run this macro, open up a terminal, clone this repo, cd into it, then run 
```root rnAnalysis2.cc```

