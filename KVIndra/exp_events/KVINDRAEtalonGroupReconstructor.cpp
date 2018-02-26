//Created by KVClassFactory on Wed Feb 21 13:43:10 2018
//Author: John Frankland,,,

#include "KVINDRAEtalonGroupReconstructor.h"
#include "KVINDRACodeMask.h"
#include <string>
#include <map>
#include <iostream>
using namespace std;

ClassImp(KVINDRAEtalonGroupReconstructor)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVINDRAEtalonGroupReconstructor</h2>
<h4>Reconstruct particles in INDRA groups with etalon telescopes</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVReconstructedNucleus* KVINDRAEtalonGroupReconstructor::ReconstructTrajectory(const KVGeoDNTrajectory* traj, const KVGeoDetectorNode* node)
{
   // if node is a CsI detector with more than one trajectory, we are behind the etalons
   // if not, standard treatment
   //
   // as the sili & si75 coders are opened by every firing of the csi directly behind them,
   // their acq params are present in the event whether the particle went through them
   // or passed directly from chio to csi.
   //
   // the best way to choose the right reconstruction trajectory is to see if sili-csi
   // provides an identification, and if so is it coherent with that of the csi.

   if (node->GetDetector()->Fired(GetPartSeedCond()) && node->GetDetector()->IsType("CSI") && node->GetNTrajForwards() > 1) {
      ++nfireddets;
      const KVSeqCollection* idt_list = traj->GetIDTelescopes();
      TIter next_idt(idt_list);
      KVIDTelescope* idt;
      bool with_etalon = (traj->GetN() == 4);

      std::map<std::string, KVIdentificationResult> IDR;
      while ((idt = (KVIDTelescope*)next_idt())) {
         if (idt->IsReadyForID()) { // is telescope able to identify for this run ?
            IDR[idt->GetType()].IDattempted = kTRUE;
            idt->Identify(&IDR[idt->GetType()]);
         } else
            IDR[idt->GetType()].IDattempted = kFALSE;
      }
      // cases:
      //  gamma in CsI: * if this is the short trajectory (CSI-CI), accept this identification
      //                this will set the CsI analysed state, so that when the long trajectory is
      //                treated it will be ignored unless SILI or SI75 have fired.
      //                * if this is the long trajectory, we ignore the gamma and wait for next
      //                 iteration to see what is in SILI & SI75
      // particle identified in CsI:
      //                * if this is the long trajectory, check the SILI-CSI identification
      //                  if SILI-CSI identification no good, then short trajectory should be used
      //                  so we return nullptr
      //                * if this is the short trajectory, we accept it
      KVIdentificationResult&  idcsi = IDR["CSI_R_L"];
      if (idcsi.IDattempted) {
         if (idcsi.IDOK) {
            if (idcsi.IDcode == kIDCode0) { // gamma
               //Info("ReconstructTrajectory","Gamma in CsI: with_etalon=%d",with_etalon);
               if (!with_etalon) return GetEventFragment()->AddParticle();
               else return nullptr;
            } else { // charged particle identified
               //Info("ReconstructTrajectory","Charged particle in CsI: with_etalon=%d",with_etalon);
               if (with_etalon) {
                  if (IDR["SILI_CSI"].IDattempted && IDR["SILI_CSI"].IDOK) {
                     //Info("ReconstructTrajectory","Charged particle in SILI too Zcsi=%d Zsili=%d",idcsi.Z,IDR["SILI_CSI"].Z);
                     return GetEventFragment()->AddParticle();
                  } else return nullptr;
               } else
                  return GetEventFragment()->AddParticle();
            }
         }
      }
      return nullptr;
   }
   // standard non-etalon treatment
   return KVINDRAGroupReconstructor::ReconstructTrajectory(traj, node);
}
