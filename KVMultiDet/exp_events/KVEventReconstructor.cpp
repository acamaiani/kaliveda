//Created by KVClassFactory on Mon Oct 19 09:33:44 2015
//Author: John Frankland,,,

#include "KVEventReconstructor.h"
#include "KVDetectorEvent.h"
#include "KVGroupReconstructor.h"
#include "KVTarget.h"

#include <TThread.h>

#include <iostream>
using namespace std;

ClassImp(KVEventReconstructor)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVEventReconstructor</h2>
<h4>Base class for handling event reconstruction</h4>
<!-- */
// --> END_HTML
// Event reconstruction begins with a KVMultiDetArray containing hit
// detectors (either following the simulated detection of a simulated event,
// or after reading some experimental data corresponding to an experimental
// event) and ends with a KVReconstructedEvent object filled with
// KVReconstructedNucleus objects corresponding to the reconstructed
// (and possibly identfied & calibrated) particles.
//
// 1. Get list of hit groups (KVDetectorEvent)
// 2. For each hit group, perform reconstruction/identification/ etc. in the group
//    using a KVGroupReconstructor-derived object (uses plugins)
// 3. Merge together the different event fragments into the output reconstructed
//    event object
////////////////////////////////////////////////////////////////////////////////

KVEventReconstructor::KVEventReconstructor(KVMultiDetArray* a, KVReconstructedEvent* e, Bool_t threaded)
   : KVBase("KVEventReconstructor", Form("Reconstruction of events in array %s", a->GetName())),
     fArray(a), fEvent(e), fGroupReconstructor(a->GetNumberOfGroups(), 1), fThreaded(threaded)
{
   // Default constructor
   // Set up group reconstructor for every group of the array.
   // It is assumed that these are all numbered uniquely & sensibly ;)
   fGroupReconstructor.SetOwner();
   unique_ptr<KVSeqCollection> gr_list(a->GetStructureTypeList("GROUP"));
   KVGroup* gr;
   TIter it(gr_list.get());
   unsigned int N = gr_list->GetEntries();
   while ((gr = (KVGroup*)it())) {
      UInt_t i = gr->GetNumber();
      if (i > N || i < 1) {
         Warning("KVEventReconstructor", "Groups in array %s are not numbered the way I like...%u", a->GetName(), i);
         gr->Print();
      } else {
         if (fGroupReconstructor[i]) {
            Warning("KVEventReconstructor", "%s array has non-unique group numbers!!!", a->GetName());
         } else {
            fGroupReconstructor[i] = a->GetReconstructorForGroup(gr);
            ((KVGroupReconstructor*)fGroupReconstructor[i])->SetReconEventClass(e->IsA());
            ((KVGroupReconstructor*)fGroupReconstructor[i])->SetGroup(gr);
         }
      }
   }
}

//________________________________________________________________

void KVEventReconstructor::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVEventReconstructor::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVBase::Copy(obj);
   //KVEventReconstructor& CastedObj = (KVEventReconstructor&)obj;
}

void KVEventReconstructor::ReconstructEvent(TSeqCollection* fired)
{
   // Reconstruct current event based on state of detectors in array
   //
   // The list pointer, if given, can be used to supply a list of fired
   // acquisition parameters to KVMultiDetArray::GetDetectorEvent

   if (GetArray()->GetTarget()) {
      // for target energy loss correction calculation
      GetArray()->GetTarget()->SetIncoming(kFALSE);
      GetArray()->GetTarget()->SetOutgoing(kTRUE);
   }

   GetEvent()->Clear("NGR");// No Group Reset (would not reset groups with no reconstructed particles)
   detev.Clear();// reset all groups, even if no particles were reconstructed in them

   GetArray()->GetDetectorEvent(&detev, fired);

   fNGrpRecon = 0;
   fHitGroups.clear();
   fHitGroups.reserve(detev.GetGroups()->GetEntries());
   TIter it(detev.GetGroups());
   KVGroup* group;
   while ((group = (KVGroup*)it())) {
      KVGroupReconstructor* grec = (KVGroupReconstructor*)fGroupReconstructor[group->GetNumber()];
      fHitGroups[fNGrpRecon] = group->GetNumber();
      if (fThreaded) {
         TThread* t = new TThread(Form("grp_rec_%d", fNGrpRecon), (TThread::VoidRtnFunc_t)&ThreadedReconstructor, (void*)grec);
         fThreads.Add(t);
         t->Run();
      } else {
         grec->Process();
      }
      ++fNGrpRecon;
   }
   if (fThreaded) {
      // wait for threads to finish
      TThread* th;
      TIter itr(&fThreads);
      while ((th = (TThread*)itr())) th->Join();
      // cleanup threads
      fThreads.Delete();
   }

   // merge resulting event fragments
   MergeGroupEventFragments();
}

void KVEventReconstructor::ThreadedReconstructor(void* arg)
{
   // Group reconstruction in separate threads

   KVGroupReconstructor* grec = (KVGroupReconstructor*)arg;
   grec->Process();
}

void KVEventReconstructor::MergeGroupEventFragments()
{
   // After processing has finished in groups, call this method to produce
   // a final merged event containing particles from all groups

   TList to_merge;

   for (int k = 0; k < fNGrpRecon; ++k) {
      int i = fHitGroups[k];
      to_merge.Add(((KVGroupReconstructor*)fGroupReconstructor[i])->GetEventFragment());
   }
   GetEvent()->MergeEventFragments(&to_merge, "NGR");// "NGR" = no group reset
}
