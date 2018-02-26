//Created by KVClassFactory on Wed Feb 21 13:42:47 2018
//Author: John Frankland,,,

#include "KVINDRAGroupReconstructor.h"
#include <KVIDINDRACsI.h>
#include "KVINDRACodeMask.h"

ClassImp(KVINDRAGroupReconstructor)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVINDRAGroupReconstructor</h2>
<h4>Reconstruct particles in INDRA groups</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVINDRAGroupReconstructor::KVINDRAGroupReconstructor()
   : KVGroupReconstructor()
{
   // Default constructor
}

//____________________________________________________________________________//

KVINDRAGroupReconstructor::~KVINDRAGroupReconstructor()
{
   // Destructor
}

KVReconstructedNucleus* KVINDRAGroupReconstructor::ReconstructTrajectory(const KVGeoDNTrajectory* traj, const KVGeoDetectorNode* node)
{
   // Specialised event reconstruction for INDRA data
   // Triggered CsI detectors are checked: if it is a gamma, we count it (parameter "INDRA_GAMMA_MULT")
   // but do not reconstruct a particle.

   if (node->GetDetector()->Fired(GetPartSeedCond()) && node->GetDetector()->IsType("CSI")) {
      ++nfireddets;
      KVIDINDRACsI* idt = (KVIDINDRACsI*)traj->GetIDTelescopes()->FindObjectByType("CSI_R_L");
      if (idt) {
         KVIdentificationResult idr;
         if (idt->IsReadyForID()) {
            idt->Identify(&idr);
            if (idr.IDOK && idr.IDcode == kIDCode0) {
               GetEventFragment()->GetParameters()->IncrementValue("INDRA_GAMMA_MULT", 1);
               return nullptr;
            }
         }
      }
      return GetEventFragment()->AddParticle();
   }
   return KVGroupReconstructor::ReconstructTrajectory(traj, node);
}

//____________________________________________________________________________//

