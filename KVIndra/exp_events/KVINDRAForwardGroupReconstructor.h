//Created by KVClassFactory on Tue Feb 27 11:35:10 2018
//Author: John Frankland,,,

#ifndef __KVINDRAFORWARDGROUPRECONSTRUCTOR_H
#define __KVINDRAFORWARDGROUPRECONSTRUCTOR_H

#include "KVINDRAGroupReconstructor.h"

class KVINDRAForwardGroupReconstructor : public KVINDRAGroupReconstructor {
   Bool_t CoherencyChIoSiCsI(KVReconstructedNucleus& PART);
   Bool_t CoherencySiCsI(KVReconstructedNucleus& PART);

public:
   KVINDRAForwardGroupReconstructor() {}
   virtual ~KVINDRAForwardGroupReconstructor() {}

   bool DoCoherencyAnalysis(KVReconstructedNucleus& PART);

   ClassDef(KVINDRAForwardGroupReconstructor, 1) //Reconstruct data in rings 1-9 of INDRA
};

#endif
