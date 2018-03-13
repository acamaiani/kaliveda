//Created by KVClassFactory on Wed Feb 21 13:42:47 2018
//Author: John Frankland,,,

#include "KVINDRAGroupReconstructor.h"
#include <KVIDINDRACsI.h>
#include <KVINDRADetector.h>
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

KVReconstructedNucleus* KVINDRAGroupReconstructor::ReconstructTrajectory(const KVGeoDNTrajectory* traj, const KVGeoDetectorNode* node)
{
   // Specialised event reconstruction for INDRA data
   // Triggered CsI detectors are checked: if it is a gamma, we count it (parameter "INDRA_GAMMA_MULT")
   // and add the name of the detector to the parameter "INDRA_GAMMA_DETS"
   // but do not reconstruct a particle.

   if (node->GetDetector()->IsType("CSI")) {
      if (node->GetDetector()->Fired(GetPartSeedCond())) {
         ++nfireddets;
         KVIDINDRACsI* idt = (KVIDINDRACsI*)traj->GetIDTelescopes()->FindObjectByType("CSI_R_L");
         if (idt) {
            KVIdentificationResult idr;
            if (idt->IsReadyForID()) {
               idt->Identify(&idr);
               if (idr.IDOK && idr.IDcode == kIDCode0) {
                  GetEventFragment()->GetParameters()->IncrementValue("INDRA_GAMMA_MULT", 1);
                  GetEventFragment()->GetParameters()->IncrementValue("INDRA_GAMMA_DETS", node->GetName());
                  node->GetDetector()->SetAnalysed();
                  return nullptr;
               }
            }
         }
         return GetEventFragment()->AddParticle();
      }
      return nullptr;
   }
   return KVGroupReconstructor::ReconstructTrajectory(traj, node);
}

void KVINDRAGroupReconstructor::Identify()
{
   KVGroupReconstructor::Identify();
   for (KVEvent::Iterator it = GetEventFragment()->begin(); it != GetEventFragment()->end(); ++it) {
      KVReconstructedNucleus& d = it.reference<KVReconstructedNucleus>();
      if (d.IsIdentified() && d.GetStatus() == KVReconstructedNucleus::kStatusStopFirstStage) {
         if (d.GetIdentifyingTelescope()) {
            d.GetIdentifyingTelescope()->SetIDCode(&d, kIDCode5);
         } else {
            if (d.InheritsFrom("KVINDRAReconNuc")) d.SetIDCode(kIDCode5);
            else d.SetIDCode(5);
         }
      } else if (!d.IsIdentified()) {
         if (d.InheritsFrom("KVINDRAReconNuc")) d.SetIDCode(kIDCode14);
         else d.SetIDCode(14);  // unidentifiable particle
      }
   }
}

void KVINDRAGroupReconstructor::IdentifyParticle(KVReconstructedNucleus& PART)
{
   // INDRA-specific particle identification.
   // Here we attribute the Veda6-style general identification codes depending on the
   // result of KVReconstructedNucleus::Identify and the subcodes from the different
   // identification algorithms:
   // If the particle's mass A was NOT measured, we make sure that it is calculated
   // from the measured Z using the mass formula defined by default
   //
   //IDENTIFIED PARTICLES
   //Identified particles with ID code = 2 with subcodes 4 & 5
   //(masse hors limite superieure/inferieure) are relabelled
   //with kIDCode10 (identification entre les lignes CsI)
   //
   //UNIDENTIFIED PARTICLES
   //Unidentified particles receive the general ID code for non-identified particles (kIDCode14)
   //EXCEPT if their identification in CsI R-L gave subcodes 6 or 7
   //(Zmin) then they are relabelled "Identified" with IDcode = 9 (ident. incomplete dans CsI ou Phoswich (Z.min))
   //Their "identifying" telescope is set to the CsI ID telescope

   KVGroupReconstructor::IdentifyParticle(PART);
   // if a successful identification has occured, identifying_telescope & partID are now set
   // and the particle has IsIdentified()=true

   // INDRA coherency treatment
   fUseFullChIoEnergyForCalib = kFALSE;
   Bool_t ok = DoCoherencyAnalysis(PART);
   PART.SetParameter("UseFullChIoEnergyForCalib", fUseFullChIoEnergyForCalib);
   PART.SetParameter("Coherent", fCoherent);
   PART.SetParameter("Pileup", fPileup);

   if (ok) { // identification may change here due to coherency analysis
      PART.SetIsIdentified();
      PART.SetIdentifyingTelescope(identifying_telescope);
      PART.SetIdentification(&partID, identifying_telescope);
   } // if not ok, do we need to unset any previously identified particle?

   if (PART.IsIdentified()) {

      /******* IDENTIFIED PARTICLES *******/
      if (partID.IsType("CSI_R_L")) {     /**** CSI R-L IDENTIFICATION ****/

         //Identified particles with ID code = 2 with subcodes 4 & 5
         //(masse hors limite superieure/inferieure) are relabelled
         //with kIDCode10 (identification entre les lignes CsI)

         Int_t grid_code = partID.IDquality;
         if (grid_code == KVIDGCsI::kICODE4 || grid_code == KVIDGCsI::kICODE5) {
            partID.IDcode = kIDCode10;
            PART.SetIdentification(&partID, identifying_telescope);
         }

      }

   } else {

      /******* UNIDENTIFIED PARTICLES *******/

      /*** general ID code for non-identified particles ***/
      if (PART.InheritsFrom("KVINDRAReconNuc")) PART.SetIDCode(kIDCode14);
      else PART.SetIDCode(14);

      std::map<std::string, KVIdentificationResult*>::iterator csirl = id_by_type.find("CSI_R_L");
      if (csirl != id_by_type.end()) {
         //Particles remaining unidentified are checked: if their identification in CsI R-L gave subcodes 6 or 7
         //(Zmin) then they are relabelled "Identified" with IDcode = 9 (ident. incomplete dans CsI ou Phoswich (Z.min))
         //Their "identifying" telescope is set to the CsI ID telescope
         if (csirl->second->IDattempted) {
            if (csirl->second->IDquality == KVIDGCsI::kICODE6 || csirl->second->IDquality == KVIDGCsI::kICODE7) {
               PART.SetIsIdentified();
               csirl->second->IDcode = kIDCode9;
               partID = *(csirl->second);
               identifying_telescope = (KVIDTelescope*)PART.GetReconstructionTrajectory()->GetIDTelescopes()->FindObjectByType("CSI_R_L");
               PART.SetIdentifyingTelescope(identifying_telescope);
               PART.SetIdentification(&partID, identifying_telescope);
            }
         }
      }

   }
}
