/*
* Author: Kevin Lin
* Date: March 2nd 2020
* Description: This macro is run in a directory with .root files with Radon concentrations taken from the SNO+
* Acrylic Vessel cover gas monitor. It integrates the radon counts and divides by the runtime in hours and graphs a
* concentration vs date graph
* Compared to rnAnalysis2, this has more dates but in a less readable layout
* This script still illustrates an interesting method of putting strings on the x axis using latex
* Warning: Sometimes the output has huge letters on the canvas, simply click the words to fix the issue
* Warning: Might not work on Linux
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

vector <string> extractDates(vector <string> input);

double histIntegrateRN(string filename);

double runTimeInHours(string filename);

void plot(vector<double> conc, vector <string> date);

void GraphDoubleString(TGraph *gr, std::string c[]);

/*
 * Main function that can be called in root by using .x rnAnalysis.cc
 */
void rnAnalysis() {
    vector <string> files = listFiles();
    vector <string> datesOnly = extractDates(files);
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
vector <string> extractDates(vector <string> input) {
    cout << "Extracting dates..." << endl;
    vector <string> datesOnly;
    for (auto x : input) {
        string key(x);
        char chars[] = "UofARrun_cpy.t";
        for (unsigned int i = 0; i < strlen(chars); ++i) {
            key.erase(remove(key.begin(), key.end(), chars[i]), key.end());
        }
        int cutoff = key.find('-');
        key.replace(0, cutoff - 4, "");
        //add date to datesonly
        datesOnly.push_back(key);
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
 * Output: Uses helper function GraphDoubleString to make a graph
 * Description: Converts vectors into arrays for easy graphing
 */
void plot(vector<double> conc, vector <string> date) {
    auto c = new TCanvas("c", "c", 1400, 500);
    //sanity check
    if (conc.size() != date.size()) {
        cout << "Concentration data and date data do not have the same length" << endl;
    }
    //x and y axis of graph
    Double_t x[conc.size()];
    Double_t y[conc.size()];
    string z[date.size()];
    for (int i = 0; i < conc.size(); i++) {
        x[i] = i;
        y[i] = conc.at(i);
        z[i] = date.at(i);
    }

    auto graph = new TGraph(conc.size(), x, y);
    graph->SetMarkerStyle(20);
    GraphDoubleString(graph, z);
    c->Print("c.pdf");
}

/*
 * Input: An existing graph with x and y and an array of strings to replace the x labels (should be same length)
 * Description: Function to plot a graph gr with strings as the x axis instead of numbers
 * Written by couet of the root forum https://root-forum.cern.ch/t/graph-with-strings-as-x-axis/38274/16
 */
void GraphDoubleString(TGraph *gr, std::string c[]) {
    gr->Draw("ALP");
    gr->GetXaxis()->SetLabelSize(0);
    gr->GetXaxis()->SetTickLength(0);
    gr->GetYaxis()->SetTickLength(0);

    Int_t i, n;
    Double_t x, y;
    TLatex *t;
    TLine *tick;
    TLine *grid;
    printf("%s\n", c[0].c_str());

    double ymin = gr->GetHistogram()->GetMinimum();
    double ymax = gr->GetHistogram()->GetMaximum();
    double dy = (ymax - ymin);
    n = gr->GetN();
    gr->GetHistogram()->GetXaxis()->SetTimeDisplay(1);

    for (i = 0; i < n; i++) {
        gr->GetPoint(i, x, y);
        t = new TLatex(x, ymin - 0.01 * dy, c[i].c_str());
        t->SetTextSize(0.015);
        t->SetTextFont(0);
        t->SetTextAlign(32);
        t->SetTextAngle(45);
        t->Draw();
        tick = new TLine(x, ymin, x, ymin + 0.03 * dy);
        tick->Draw();
        grid = new TLine(x, ymin, x, y);
        grid->SetLineStyle(3);
        grid->Draw();
    }
}
