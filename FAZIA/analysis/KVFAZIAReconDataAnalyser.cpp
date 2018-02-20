/*
$Id: KVFAZIAReconDataAnalyser.cpp,v 1.4 2007/11/15 14:59:45 franklan Exp $
$Revision: 1.4 $
$Date: 2007/11/15 14:59:45 $
*/

//Created by KVClassFactory on Wed Apr  5 23:50:04 2006
//Author: Eric Bonnet

#include "KVBase.h"
#include "KVFAZIAReconDataAnalyser.h"
#include "KVFAZIADBRun.h"
#include "KVDataAnalysisTask.h"
#include "KVDataSet.h"
#include "TChain.h"
#include "TObjString.h"
#include "TChain.h"
#include "KVAvailableRunsFile.h"
#include "KVFAZIA.h"
#include "KVEventSelector.h"

using namespace std;

ClassImp(KVFAZIAReconDataAnalyser)
////////////////////////////////////////////////////////////////////////////////
// For analysing reconstructed FAZIA data
//
////////////////////////////////////////////////////////////////////////////////
KVFAZIAReconDataAnalyser::KVFAZIAReconDataAnalyser()
{
   //Default constructor
   theTree = 0;
   fLinkRawData = kFALSE;
}

KVFAZIAReconDataAnalyser::~KVFAZIAReconDataAnalyser()
{
   //Destructor
}

//_________________________________________________________________

void KVFAZIAReconDataAnalyser::SubmitTask()
{
   //Run the interactive analysis
   if (gDataSet != GetDataSet()) GetDataSet()->cd();

   KVEventSelector es;
   es.SetOption(GetUserClassOptions());
   es.ParseOptions();
   fLinkRawData = es.IsOptGiven("ReadRawData");

   //loop over runs
   GetRunList().Begin();
   while (!GetRunList().End()) {
      fRunNumber = GetRunList().Next();
      Info("SubmitTask", "treatment of run# %d", fRunNumber);
      ProcessRun();
   }
}

void KVFAZIAReconDataAnalyser::ProcessRun()
{
   // Perform treatment of a given run
   // Before processing each run, after opening the associated file, user's InitRun() method is called.
   // After each run, user's EndRun() is called.
   // For each event of each run, user's Analysis() method is called.
   //
   // For further customisation, the pre/post-methods are called just before and just after
   // each of these methods (preInitRun(), postAnalysis(), etc. etc.)
   TString fullPathToRunfile = gDataSet->GetFullPathToRunfile(GetDataType(), fRunNumber);
   TFile* f = gDataSet->OpenRunfile<TFile>(GetDataType(), fRunNumber);
   if (!(f && !f->IsZombie())) {
      Error("ProcessRun", "file %s does not exist or is made zombie", fullPathToRunfile.Data());
      return;
   }

   theTree = (TTree*)f->Get("ReconstructedEvents");

   TFile* ffriend = 0;
   if (fLinkRawData) {
      fullPathToRunfile = gDataSet->GetFullPathToRunfile("raw", fRunNumber);
      ffriend = gDataSet->OpenRunfile<TFile>("raw", fRunNumber);
      if (!(ffriend && !ffriend->IsZombie())) {
         Warning("ProcessRun", "file %s does not exist or is made zombie\n Reading of raw data is not possible", fullPathToRunfile.Data());
      } else {
         theTree->AddFriend("FAZIA", ffriend);
      }
   }

   TString option("");
   // Add any user-defined options
   if (GetUserClassOptions() != "") {
      if (option != "")
         option += ",";
      option += GetUserClassOptions();
   }

   Info("SubmitTask", "Beginning TTree::Process... Option=%s", option.Data());
   if (GetNbEventToRead()) {
      theTree->Process(GetUserClass(), option.Data(), GetNbEventToRead());
   } else {
      theTree->Process(GetUserClass(), option.Data());
   }

   if (ffriend && ffriend->IsOpen())
      ffriend->Close();
   f->Close();

}

//    fSelector = 0;
//
//    //if (theChain) delete theChain;
//    theChain = new TChain("ReconstructedEvents");
//    theChain->SetDirectory(0); // we handle delete
//
//    //if (theFriendChain) delete theFriendChain;
//    if (fLinkRawData) {
//       theFriendChain = new TChain("FAZIA");
//       theFriendChain->SetDirectory(0); // we handle delete
//    } else {
//       theFriendChain = 0;
//    }
//
//    fRunList.Begin();
//    Int_t run;
//
//    // open and add to TChain all required files
//    // we force the opening of the files to avoid problems with xrootd which sometimes
//    // seems to have a little difficulty
//    while (!fRunList.End()) {
//       run = fRunList.Next();
//
//       //Add run file
//       TString fullPathToRunfile = gDataSet->GetFullPathToRunfile(fDataType.Data(), run);
//       cout << "Opening file " << fullPathToRunfile << endl;
//       TFile* f = (TFile*)gDataSet->OpenRunfile(fDataType.Data(), run);
//       cout << "Adding file " << fullPathToRunfile;
//       cout << " to the TChain." << endl;
//       theChain->Add(fullPathToRunfile);
//       if (f && !f->IsZombie()) {
//          // update run infos in available runs file if necessary
//          KVAvailableRunsFile* ARF = gDataSet->GetAvailableRunsFile(fDataType.Data());
//          if (ARF->InfosNeedUpdate(run, gSystem->BaseName(fullPathToRunfile))) {
//             if (!((TTree*)f->Get("ReconstructedEvents"))) {
//                Error("SubmitTask", "No tree named ReconstructedEvents is present in the current file");
//                delete theChain;
//                return;
//             }
//             TEnv* treeInfos = (TEnv*)((TTree*)f->Get("ReconstructedEvents"))->GetUserInfo()->FindObject("TEnv");
//             if (treeInfos) {
//                TString kvversion = treeInfos->GetValue("KVBase::GetKVVersion()", "");
//                TString username = treeInfos->GetValue("gSystem->GetUserInfo()->fUser", "");
//                if (kvversion != "") ARF->UpdateInfos(run, gSystem->BaseName(fullPathToRunfile), kvversion, username);
//             } else {
//                Info("SubmitTask", "No TEnv object associated to the tree");
//             }
//          }
//       }
//
//       //Add run file
//       if (theFriendChain) {
//          fullPathToRunfile = gDataSet->GetFullPathToRunfile("raw", run);
//          cout << "Opening file " << fullPathToRunfile << endl;
//          f = (TFile*)gDataSet->OpenRunfile("raw", run);
//          cout << "Adding file " << fullPathToRunfile;
//          cout << " to the friend TChain." << endl;
//          theFriendChain->Add(fullPathToRunfile);
//       }
//    }
//
//    if (theFriendChain) theChain->AddFriend("FAZIA");
//
//    TotalEntriesToRead = theChain->GetEntries();
//    TString option("");
//    if (fDataSelector.Length()) {
//       option.Form("DataSelector=%s", fDataSelector.Data());
//       cout << "Data Selector : " << fDataSelector.Data() << endl;
//    }
//
//    // Add any user-defined options
//    if (GetUserClassOptions() != "") {
//       option += ",";
//       option += GetUserClassOptions();
//    }
//
//    fSelector = (TSelector*)GetInstanceOfUserClass();
//
//    if (!fSelector || !fSelector->InheritsFrom("TSelector")) {
//       cout << "The selector \"" << GetUserClass() << "\" is not valid." << endl;
//       cout << "Process aborted." << endl;
//    } else {
//       SafeDelete(fSelector);
//       Info("SubmitTask", "Beginning TChain::Process...%s %s", GetUserClass(), option.Data());
//       if (nbEventToRead) {
//          theChain->Process(GetUserClass(), option.Data(), nbEventToRead);
//       } else {
//          theChain->Process(GetUserClass(), option.Data());
//       }
//    }
//
//    if (theFriendChain) {
//       theChain->RemoveFriend(theFriendChain);
//       delete theFriendChain;
//       theFriendChain = 0;
//    }
//    delete theChain;
//    theChain = 0;
//    fSelector = 0; //deleted by TChain/TTreePlayer



void KVFAZIAReconDataAnalyser::preInitAnalysis()
{
   // Called by currently-processed KVFAZIASelector before user's InitAnalysis() method.
   // We build the multidetector for the current dataset in case informations on
   // detector are needed e.g. to define histograms in InitAnalysis().
   // Note that at this stage we are not analysing a given run, so the parameters
   // of the array are not set (they will be set in preInitRun()).

   Info("preInitAnalysis", "Appel");
   if (gDataSet && !gMultiDetArray) {
      Info("preInitAnalysis", "Building of FAZIA array");
      KVMultiDetArray::MakeMultiDetector(gDataSet->GetName());
   }

}


void KVFAZIAReconDataAnalyser::preInitRun()
{
   // Called by currently-processed KVFAZIASelector when a new file in the TChain is opened.
   // We call gFazia->SetParameters for the current run.

   // Infos on currently read file/tree are printed.

   Info("preInitRun", "Appel");
   gMultiDetArray->SetParameters(fRunNumber);
}

void KVFAZIAReconDataAnalyser::preAnalysis()
{
   // Called by currently-processed KVFAZIASelector after reading a new event
   // and just before calling user analysis
   // Set acquisition trigger multiplicity as minimum multiplicity for acceptable event
   gMultiDetArray->SetMinimumOKMultiplicity(fEventSelector->GetEvent());
}

