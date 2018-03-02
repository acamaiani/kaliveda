//Created by KVClassFactory on Mon Oct 19 14:11:26 2015
//Author: John Frankland,,,

#ifndef __KVGROUPRECONSTRUCTOR_H
#define __KVGROUPRECONSTRUCTOR_H

#include "KVBase.h"
#include "KVGroup.h"
#include "KVReconstructedEvent.h"
#include <map>
#include <string>

class KVGroupReconstructor : public KVBase {

   KVGroup*              fGroup;//!        the group where we are reconstructing
   KVReconstructedEvent* fGrpEvent;//!     event containing particles reconstructed in this group
   TString               fPartSeedCond;//! condition for seeding reconstructed particles
protected:
   mutable int nfireddets;//! number of fired detectors in group for current event
   KVIdentificationResult partID;//! identification to be applied to current particle
   KVIDTelescope* identifying_telescope;//! telescope which identified current particle
   std::map<std::string, KVIdentificationResult*> id_by_type; //! identification results by type

   virtual KVReconstructedNucleus* ReconstructTrajectory(const KVGeoDNTrajectory* traj, const KVGeoDetectorNode* node);
   void ReconstructParticle(KVReconstructedNucleus* part, const KVGeoDNTrajectory* traj, const KVGeoDetectorNode* node);
   virtual void IdentifyParticle(KVReconstructedNucleus& PART);
   virtual void CalibrateParticle(KVReconstructedNucleus* PART);

   Double_t GetTargetEnergyLossCorrection(KVReconstructedNucleus* ion);
   TString GetPartSeedCond() const
   {
      return fPartSeedCond;
   }

public:
   KVGroupReconstructor();
   virtual ~KVGroupReconstructor();

   void SetReconEventClass(TClass* c);
   void Copy(TObject& obj) const;
   int GetNFiredDets() const
   {
      return nfireddets;
   }

   KVReconstructedEvent* GetEventFragment() const
   {
      return fGrpEvent;
   }
   virtual void SetGroup(KVGroup* g);
   KVGroup* GetGroup() const
   {
      return fGroup;
   }

   static KVGroupReconstructor* Factory(const TString& plugin = "");

   void Process();
   void Reconstruct();
   virtual void Identify();
   void Calibrate();

   void AnalyseParticles();
   Int_t GetNIdentifiedInGroup()
   {
      //number of identified particles reconstructed in group
      Int_t n = 0;
      if (GetEventFragment()->GetMult()) {
         for (KVEvent::Iterator it = GetEventFragment()->begin(); it != GetEventFragment()->end(); ++it) {
            KVReconstructedNucleus& nuc = it.reference<KVReconstructedNucleus>();
            n += (Int_t) nuc.IsIdentified();
         }
      }
      return n;
   }
   Int_t GetNUnidentifiedInGroup()
   {
      //number of unidentified particles reconstructed in group
      return (GetEventFragment()->GetMult() - GetNIdentifiedInGroup());
   }

   ClassDef(KVGroupReconstructor, 0) //Base class for handling event reconstruction in detector groups
};

#endif
