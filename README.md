# radonAnalysis
A ROOT C++ macro that analyzes .root files and plots Radon concentration over time and saves it to c.pdf
Included is a few sample files from the Radon monitor. They are not to be used in any context other than understanding this macro.

## A macro that I wrote to do the following:
1. Open a .root file
2. Extract the date from the file name
3. Create a histogram using one of the leafs (fadc_channel)
4. Integrate the Radon part of the energy histogram and store the value
5. Record the total amount of time that the Radon run spanned using (ftimestamp)
6. Repeat over all the .root files and plot the concentration (integrated value/time) as a function of date

## Functions
This macro can't be copied directly, because the method I use to extract the date is specific to the length of files.

However, since I've seperated my macro into functions, many of them will work people trying to replicate. 

1. listFiles will list out files as long as the path is updated at the top of the file
2. extractDates is specific to my files as mentioned above
3. histIntegrateRN integrates the fadc_channel using a gaussian fit
4. runTimeInHours takes the start and end point and finds how many hours were in a Radon measurement run
5. plot on rnAnalysis.cc uses the extractDates and a Latex workaround to put the dates on the x axis, in rnAnalysis2.cc (preferred), I use SetTimeDisplay() to put spaced out dates on the x axis.

To run this macro, open up a terminal, clone this repo, cd into it, then run 
```root rnAnalysis2.cc```

