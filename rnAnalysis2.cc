/*
* Author: Kevin Lin
* Date: March 2nd 2020
* Description: This macro is run in a directory with .root files with Radon concentrations taken from the SNO+
* Acrylic Vessel cover gas monitor. It integrates the radon counts and divides by the runtime in hours and graphs a
* concentration vs date graph
* Compared to rnAnalysis.cc, this one has more readable dates at the cost of dates for every value. Worth it.
* Warning: Sometimes the output has huge letters on the canvas, simply click the words to fix the issue
* Warning: Might not work on Linux
 *
*/

#include <iostream>
#include <vector>
#include <algorithm>
#include <Windows.h>
#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TList.h"
#include "TSystemDirectory.h"
#include "TString.h"
#include "TF1.h"
#include "TGraph.h"
#include "TPad.h"
#include "TROOT.h"

using namespace std;

//global variables
const bool publishFileDetails = true;
const char *pathToRootFiles = "C:/users/klin/SNOLAB/radonAnalysis/"; //change if changing files

//function prototypes
vector <string> listFiles(const char *dirname = pathToRootFiles, const char *ext = ".root");

vector <double> extractDates(vector <string> input);

double histIntegrateRN(string filename);

double runTimeInHours(string filename);

void plot(vector<double> conc, vector <double> date);

/*
 * Main function that can be called in root by using .x rnAnalysis.cc
 */
void rnAnalysis2() {
    vector <string> files = listFiles();
    vector <double> datesOnly = extractDates(files);
    cout << "1" << endl;
    cout << "Found " << files.size() << " files" << endl;
    vector<double> rnConcentrations;
    for (int x = 0; x < files.size(); x++) {
        cout << "File # " << x + 1 << ": " << files.at(x) << endl;
        double amount = histIntegrateRN(files.at(x));
        double timeInHours = runTimeInHours(files.at(x));
        rnConcentrations.push_back(amount / timeInHours);
        cout << "Concentration for " << datesOnly.at(x) << ": " << amount / timeInHours << " [counts/hour]" << endl
             << endl;
    }
    plot(rnConcentrations, datesOnly);
}

/*
 * Input: nothing (change *dirname path for usage)
 * Output: string Vector with all the .root files in the current directory
 * Description: Lists the root files found in the directory specified at the top of this file
 * Set publishFileDetails to true to see which files were found
 */
vector <string> listFiles(const char *dirname = pathToRootFiles, const char *ext = ".root") {
    cout << "Looking for .root files..." << endl;
    //store array of .root filenames
    vector <string> arr;
    TSystemDirectory dir(dirname, dirname);
    TList *files = dir.GetListOfFiles();
    if (files) {
        TSystemFile *file;
        TString fname;
        TIter next(files);
        while ((file = (TSystemFile *) next())) {
            fname = file->GetName();
            if (!file->IsDirectory() && fname.EndsWith(ext)) {
                //add .root files to arr
                arr.push_back(fname.Data());
            }
        }
    } else {
        cout << "No .root files found. Change the path in rnAnalysis.cc" << endl;
    }
    if (publishFileDetails) {
        for (auto x : arr)
            cout << x << "\n";
    }
    delete files;
    return arr;
}

/*
 * Input: string Vector of .root files of interest
 * Output: string Vector of just the dates for those .root files
 * Description: Gets the dates out of the file names. Very hacky method, removes certain letters and then cuts out stuff
 * Only works for SNO+ RN files.
 */
vector <double> extractDates(vector <string> input) {
    cout << "Extracting dates..." << endl;
    vector <double> datesOnly;
    for (auto x : input) {
        char *cstr = new char[x.length() + 1];
        strcpy(cstr, x.c_str());
        TFile *f = TFile::Open(cstr);
        TTree *t;
        f->GetObject("r", t);
        double tmin = t->GetMinimum("ftimestamp");
        datesOnly.push_back(tmin);
        delete f;
    }
    if (publishFileDetails) {
        for (auto y : datesOnly)
            cout << y << "\n";
    }
    return datesOnly;
}

/*
 * Input: Name of .root file with fadc_channel data
 * Output: Integrated gaussian fit of the Radon bump on the fadc_channel
 * Description: makes a histogram for fadc_channel of Radon monitor files, and measures the count by integrating a fit
 */
double histIntegrateRN(string filename) {
    gROOT->cd();
    //Turn string into char*
    char *cstr = new char[filename.length() + 1];
    strcpy(cstr, filename.c_str());
    //Concatenate char* so no duplicate hists are made
    char histname[100];
    strcpy(histname, "h_fadc");
    strcpy(histname, cstr);
    //manipulate histogram, fit it, integrate
    TH1D *h = new TH1D(histname, "fadc;channel;#entries", 8039., 0., 4096.);
    TFile *f = TFile::Open(cstr);
    TTree *t;
    TCanvas *c = new TCanvas("c", "titles", 600, 600);
    f->GetObject("r", t);
    gROOT->cd();
    c->cd();
    try {
        t->Draw(Form("fadc_channel>>%s", histname), "", "HIST");
    }
    catch (int e) {
        cout << "ops" << endl;
    }
    h->Fit("gaus", "Q", "", 1850, 2050);
    TF1 *func = h->GetFunction("gaus");
    double ans = 0.0;
    //Check that the fit worked
    if (func == nullptr) {
        cout << "Fit did not work, check fit bounds and histogram" << endl;
    } else {
        ans = h->GetFunction("gaus")->Integral(1850, 2050);
    }
    cout << "Counts found from integrated fit: " << ans << endl;
    //garbage collection
    delete[] cstr;
    delete c;
    delete f;
    delete func;
    return ans;
}

/*
 * Input: Name of .root file with ftimestamp data
 * Output: Time in hours where data was taken
 * Description: Uses GetMinimum and GetMaximum to find the endpoints of data collection dates,
 * finding out how many hours the run was
 */
double runTimeInHours(string filename) {
    char *cstr = new char[filename.length() + 1];
    strcpy(cstr, filename.c_str());
    TFile *f = TFile::Open(cstr);
    TTree *t;
    f->GetObject("r", t);
    double tmin = t->GetMinimum("ftimestamp");
    double tmax = t->GetMaximum("ftimestamp");
    double dt = tmax - tmin;
    std::cout << "dt = " << dt / 60 / 60 << " [hr]" << std::endl;
    //garbage collection
    delete[] cstr;
    delete f;
    return dt / 60 / 60;
}


/*
 * Input: Vector of doubles for y axis values, vector of strings for the x axis values that are strings
 * Description: Graphs and uses settimedisplay to create an x axis
 */
void plot(vector<double> conc, vector <double> date) {
    auto c = new TCanvas("c", "c", 1200, 500);
    //sanity check
    if (conc.size() != date.size()) {
        cout << "Concentration data and date data do not have the same length" << endl;
    }
    //x and y axis of graph
    Double_t x[conc.size()];
    Double_t y[date.size()];
    for (int i = 0; i < conc.size(); i++) {
        x[i] = date.at(i);
        y[i] = conc.at(i);
    }

    auto graph = new TGraph(conc.size(), x, y);
    c->SetLogy();
    graph->SetMarkerStyle(20);
    graph->Draw("ALP");
    graph->SetTitle("AV Cover Gas Radon Monitor Concentration vs Time;Date (dd/mm/yr);Concentration (counts/hour);");
    graph->GetHistogram()->GetXaxis()->SetTimeDisplay(1);
    graph->GetHistogram()->GetXaxis()->SetTimeFormat("%d\/%m\/%y%F1970-01-01 00:00:00");
    c->Print("c.pdf");
}

