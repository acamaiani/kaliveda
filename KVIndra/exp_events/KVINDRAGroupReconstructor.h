//Created by KVClassFactory on Wed Feb 21 13:42:47 2018
//Author: John Frankland,,,

#ifndef __KVINDRAGROUPRECONSTRUCTOR_H
#define __KVINDRAGROUPRECONSTRUCTOR_H

#include "KVGroupReconstructor.h"
#include "KVChIo.h"

class KVINDRAGroupReconstructor : public KVGroupReconstructor {

protected:
   KVChIo* theChio;                 // the ChIo of the group
   Bool_t fUseFullChIoEnergyForCalib;//decided by coherency analysis
   Bool_t fCoherent;//coherency of CsI & Si-CsI identifications
   Bool_t fPileup;//apparent pileup in Si, revealed by inconsistency between CsI & Si-CsI identifications

public:
   KVINDRAGroupReconstructor() {}
   virtual ~KVINDRAGroupReconstructor() {}

   void SetGroup(KVGroup* g)
   {
      KVGroupReconstructor::SetGroup(g);
      theChio = (KVChIo*)g->GetDetectorByType("CI");
   }

   KVReconstructedNucleus* ReconstructTrajectory(const KVGeoDNTrajectory* traj, const KVGeoDetectorNode* node);

   void Identify();
   void IdentifyParticle(KVReconstructedNucleus& PART);

   virtual bool DoCoherencyAnalysis(KVReconstructedNucleus& PART) = 0;

   ClassDef(KVINDRAGroupReconstructor, 1) //Reconstruct particles in INDRA groups
};

#endif
