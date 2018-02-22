//Created by KVClassFactory on Wed Feb 21 13:43:10 2018
//Author: John Frankland,,,

#ifndef __KVINDRAETALONGROUPRECONSTRUCTOR_H
#define __KVINDRAETALONGROUPRECONSTRUCTOR_H

#include "KVINDRAGroupReconstructor.h"

class KVINDRAEtalonGroupReconstructor : public KVINDRAGroupReconstructor {
public:
   KVINDRAEtalonGroupReconstructor() {}
   virtual ~KVINDRAEtalonGroupReconstructor() {}

   KVReconstructedNucleus* ReconstructTrajectory(const KVGeoDNTrajectory* traj, const KVGeoDetectorNode* node);

   ClassDef(KVINDRAEtalonGroupReconstructor, 1) //Reconstruct particles in INDRA groups with etalon telescopes
};

#endif
