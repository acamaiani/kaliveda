//Created by KVClassFactory on Mon Oct 19 09:33:44 2015
//Author: John Frankland,,,

#ifndef __KVEVENTRECONSTRUCTOR_H
#define __KVEVENTRECONSTRUCTOR_H

#include <vector>
#include "KVBase.h"
#include "KVDetectorEvent.h"
#include "KVMultiDetArray.h"
#include "KVReconstructedEvent.h"

class KVEventReconstructor : public KVBase {

   KVMultiDetArray*       fArray;//!       Array for which events are to be reconstructed
   KVReconstructedEvent*  fEvent;//!       The reconstructed event
   TObjArray       fGroupReconstructor;//! array of group reconstructors
   Int_t           fNGrpRecon;//!          number of group reconstructors for current event
   TList           fThreads;//!            list of threads for parallel reconstruction
   Bool_t          fThreaded;
   std::vector<int> fHitGroups;//!         group indices in current event
   KVDetectorEvent detev;//!               list of hit groups in event

protected:
   KVMultiDetArray* GetArray()
   {
      return fArray;
   }

public:
   KVEventReconstructor(KVMultiDetArray*, KVReconstructedEvent*, Bool_t = kFALSE);
   virtual ~KVEventReconstructor() {}

   void Copy(TObject& obj) const;

   void ReconstructEvent(TSeqCollection* = nullptr);
   void MergeGroupEventFragments();

   KVReconstructedEvent* GetEvent()
   {
      return fEvent;
   }
   static void ThreadedReconstructor(void* arg);

   ClassDef(KVEventReconstructor, 0) //Base class for handling event reconstruction
};

#endif
