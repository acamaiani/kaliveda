//Created by KVClassFactory on Tue Feb 27 11:35:10 2018
//Author: John Frankland,,,

#include "KVINDRAForwardGroupReconstructor.h"
#include "KVINDRACodeMask.h"
#include "KVIDGChIoSi.h"
#include "KVIDGCsI.h"

ClassImp(KVINDRAForwardGroupReconstructor)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVINDRAForwardGroupReconstructor</h2>
<h4>Reconstruct data in rings 1-9 of INDRA</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

bool KVINDRAForwardGroupReconstructor::DoCoherencyAnalysis(KVReconstructedNucleus& PART)
{
   // Coherency analysis for forward rings 1-9 of INDRA

   bool ok = false;
   if (PART.GetStoppingDetector()->IsType("CSI")) {
      // particles stopping in CsI detectors on rings 1-9
      // check coherency of CsI-R/L and Si-CsI identifications
      ok = CoherencySiCsI(PART);
      // we check that the ChIo contribution is sane:
      // if no other particles hit this group, the Z given by the ChIoSi
      // must be <= the Z found from Si-CsI or CsI-RL identification
      fUseFullChIoEnergyForCalib = CoherencyChIoSiCsI(PART);
   } else {
      // particle stopped in Si (=> ChIo-Si) or ChIo (=> Zmin)
      ok = PART.IsIdentified() && identifying_telescope;
      fUseFullChIoEnergyForCalib = !(theChio && theChio->GetNHits() > 1);
   }

   return ok;
}

//____________________________________________________________________________________________

Bool_t KVINDRAForwardGroupReconstructor::CoherencyChIoSiCsI(KVReconstructedNucleus& PART)
{
   // Called by DoCoherency() for particles stopping in CsI detectors on rings 2-9,
   // which have a ChIo in front of them.
   //
   // we check that the ChIo contribution is sane:
   // if no other particles hit this group, the Z given by the ChIoSi
   // must be <= the Z found from Si-CsI or CsI-RL identification
   //
   // in this case the measured energy loss of the ChIo can be solely attributed to this particle
   // and we return kTRUE;

   // ChIo was hit by more than one particle in group
   if (theChio && theChio->GetNHits() > 1) {
      return kFALSE;
   }

   KVIdentificationResult* IDchiosi = PART.GetIdentificationResult(3);
   if (!IDchiosi->IDattempted) {
      // no ChIo-Si identification ? assume coherency ?
      return kTRUE;
   }

   // if we have a successful ChIo-Si id with a Z > Z given by CsI or Si-CsI id,
   // then we consider that there is a pile-up in the ChIo
   // note that we consider Z_ChIoSi = 1 to mean the ChIo is in the pedestal i.e. nothing seen
   // we also require the chio-si identification to be above the punch-through line
   if (IDchiosi->IDOK && IDchiosi->Z > 1 && IDchiosi->IDquality != KVIDGChIoSi::k_BelowPunchThrough && IDchiosi->Z > partID.Z) {
      return kFALSE;
   }

   return kTRUE;
}

Bool_t KVINDRAForwardGroupReconstructor::CoherencySiCsI(KVReconstructedNucleus& PART)
{
   // Called by DoCoherency() for particles stopping in CsI detectors on rings 1-9,
   // which have a Silicon detector just in front of them.
   //
   // coherent = kFALSE if CsI-R/L and Si-CsI identifications are not coherent.
   //                 this is a warning, the CsI identification is kept, either the Si signal
   //                  was not good (particle hitting dead zone), or it actually corresponds
   //                  to two particles reaching the CsI at the same time
   // pileup = kTRUE means that the particle identified in CsI-R/L is correct,
   //              and there is probably a second particle which stopped in the silicon
   //              detector at the same time, to be identified in ChIo-Si after
   //              subtraction of the Silicon contribution

   KVIdentificationResult* IDcsi = PART.GetIdentificationResult(1);
   KVIdentificationResult* IDsicsi = PART.GetIdentificationResult(2);
   KVIDTelescope* idt_csi = (KVIDTelescope*)PART.GetReconstructionTrajectory()->GetIDTelescopes()->FindObjectByType(IDcsi->GetType());
   KVIDTelescope* idt_sicsi = (KVIDTelescope*)PART.GetReconstructionTrajectory()->GetIDTelescopes()->FindObjectByType(IDsicsi->GetType());
   fCoherent = kTRUE;
   fPileup = kFALSE;

   // Unsuccessful/no CsI id attempt with successful Si-CsI id ?
   // Then use Si-CsI identification result
   if (IDsicsi->IDOK && !IDcsi->IDOK) {
      partID = *IDsicsi;
      identifying_telescope = idt_sicsi;
      return kTRUE;
   }

   // check coherency of CsI-R/L and Si-CsI identifications
   if (IDcsi->IDOK) {
      partID = *IDcsi;
      identifying_telescope = idt_csi;

      // Neutrons have no energy loss in Si detector (pedestal), have a reaction in the CsI and create a Z=1
      // or Z=2 which is identified in CsI R-L,  while they show up in Si-CsI maps as a horizontal
      // band around the Si pedestal for low energies (energies where proton dE is significantly larger than
      // the pedestal).
      if ((IDcsi->Z == 1 || IDcsi->Z == 2) && IDsicsi->IDattempted) {
         if (IDsicsi->deltaEpedestal == KVIdentificationResult::deltaEpedestal_UNKNOWN) {
            // no explicit treatment of 'neutron-like' particles with a cut in Si-CsI id grid
            // First we check that we are in the domain where proton dE can be distinguished from pedestal.
            // If so, if the measured dE is below [ped + factor*(dE_exp - ped)], then we label the particle as a neutron.
            // 'factor' depends on the Si-CsI telescope: if it has mass identification, factor=0.3; if not, factor=0.1
            // (these empirical values correspond to 5th campaign data)
            if (idt_sicsi) {
               Double_t ped = idt_sicsi->GetPedestalY();
               Int_t status;
               Double_t dE_exp = idt_sicsi->GetMeanDEFromID(status, 1, 1);
               if (status == KVIDTelescope::kMeanDE_OK) { // proton/Z=1 line exists, and we are in its energy range
                  if (dE_exp > ped + 5.) { // arbitrary choice, must have expected dE at least 5 channels above pedestal

                     // if Si-CsI has no isotopic identification, reduce factor
                     Double_t factor = (idt_sicsi->HasMassID() ? 0.3 : 0.1);
                     if (idt_sicsi->GetIDMapY() < (ped + factor * (dE_exp - ped))) {
                        partID = *IDsicsi;
                        identifying_telescope = idt_sicsi;
                        partID.IDOK = kTRUE;
                        partID.Zident = kTRUE;
                        partID.Aident = kTRUE;
                        partID.Z = 0;
                        partID.A = 1;
                        partID.IDcode = kIDCode1; // general code for neutrons
                        return kTRUE;
                     }
                  }
               }
            }
         } else if (IDsicsi->deltaEpedestal == KVIdentificationResult::deltaEpedestal_YES) {
            // explicit treatment of 'neutron-like' particles with a cut in Si-CsI id grid
            partID = *IDsicsi;
            identifying_telescope = idt_sicsi;
            partID.IDOK = kTRUE;
            partID.Zident = kTRUE;
            partID.Aident = kTRUE;
            partID.Z = 0;
            partID.A = 1;
            partID.IDcode = kIDCode1; // general code for neutrons
            return kTRUE;
         }
      }

      // We check the coherency of the mass and charge identifications
      // If a successful Si-CsI identification is available we check:
      //   if Si-CsI gives A & Z - must have same Z, A within +/-1 unit
      //                                      if Z or A smaller => incoherency : pile-up of two particles in CsI ?
      //                                               or bad signal from Si detector (dead zones) ?
      //                                      if Z or A larger, CsI identification is good,
      //                                      assume another particle stopped in Si (identify in ChIo-Si)
      //   if Si-CsI gives just Z - must have same Z
      //                                      if Z smaller => incoherency : pile-up of two particles in CsI ?
      //                                               or bad signal from Si detector (dead zones) ?
      //                                      if Z larger => CsI identification is good,
      //                                      assume another particle stopped in Si (identify in ChIo-Si)
      //
      // If CsI identification gives code KVIDGCsI::kICode1 ou KVIDGCsI::kICode3 and the
      // Si-CsI gives the same Z but A = Acsi + 1, we use the Si-CsI identification.
      //
      // If CsI identification gives code KVIDGCsI::kICode2 ou KVIDGCsI::kICode3 and the
      // Si-CsI gives the same Z but A = Acsi - 1, we use the Si-CsI identification.
      //
      // N.B. if CsI-R/L identification gives "8Be" (2 alpha particles) then there are two correct possibilities:
      //     1) Si-CsI identification gives 7Li => CsI identification is correct ("8Be")
      //     2) Si-CsI identification gives 8He => the particle is 8He (falls on same R/L line as 2*alpha)
      // Seeing the very low statistics for 8He compared to 8Be/2alpha, we assume that if Si-CsI id
      // gives something above 8He it is either incoherent (below 7Li) or 8Be + something else in ChIo-Si
      // (above 7Li).
      if (IDsicsi->IDOK) {
         Int_t Zref = IDcsi->Z;
         Int_t Aref = IDcsi->A;
         if (IDsicsi->Aident) { // Si-CsI provides mass identification

            if (IDcsi->Z == 4 && IDcsi->A == 8) {
               // traitement special 8Be
               // if sicsi => 7Li, it is 8Be (2alpha)
               // if sicsi => 8He, it is 8He
               if (IDsicsi->Z < 2 || (IDsicsi->Z == 2 && IDsicsi->A < 7)) {
                  fCoherent = kFALSE;
                  IDsicsi->SetComment("CsI-R/L & Si-CsI identifications not coherent");
                  return kTRUE;
               } else if (IDsicsi->Z == 2 && IDsicsi->A > 6 && IDsicsi->A < 10) {
                  // accept helium-7,8,9 as 8He
                  partID = *IDsicsi;
                  identifying_telescope = idt_sicsi;
                  return kTRUE;
               } else if ((IDsicsi->Z == 2 && IDsicsi->A > 9) || (IDsicsi->Z == 3 && IDsicsi->A < 6)) {
                  fCoherent = kFALSE;
                  IDsicsi->SetComment("CsI-R/L & Si-CsI identifications not coherent");
                  return kTRUE;
               } else if (IDsicsi->Z == 3 && IDsicsi->A > 5 && IDsicsi->A < 9) {
                  // accept lithium-6,7,8 as 7Li
                  return kTRUE;
               } else if ((IDsicsi->Z == 3 && IDsicsi->A > 8) || IDsicsi->Z > 3) {
                  fPileup = kTRUE;
                  IDsicsi->SetComment("Second particle stopping in Si, identification ChIo-Si required");
                  return kTRUE;
               }
            }
            // if CsI says A could be bigger and Si-CsI gives same Z and A+1, use Si-CsI
            if ((IDsicsi->Z == Zref) && (IDsicsi->A == (Aref + 1))
                  && (IDcsi->IDquality == KVIDGCsI::kICODE1 || IDcsi->IDquality == KVIDGCsI::kICODE3)) {
               partID = *IDsicsi;
               identifying_telescope = idt_sicsi;
               return kTRUE;
            }
            // if CsI says A could be smaller and Si-CsI gives same Z and A-1, use Si-CsI
            if ((IDsicsi->Z == Zref) && (IDsicsi->A == (Aref - 1))
                  && (IDcsi->IDquality == KVIDGCsI::kICODE2 || IDcsi->IDquality == KVIDGCsI::kICODE3)) {
               partID = *IDsicsi;
               identifying_telescope = idt_sicsi;
               return kTRUE;
            }
            // everything else - Z must be same, A +/- 1 unit
            if (IDsicsi->Z == Zref && TMath::Abs(IDsicsi->A - Aref) < 2) {
               return kTRUE;
            } else if (IDsicsi->Z < Zref || (IDsicsi->Z == Zref && IDsicsi->A < (Aref - 1))) {
               fCoherent = kFALSE;
               IDsicsi->SetComment("CsI-R/L & Si-CsI identifications not coherent");
               return kTRUE;
            } else  if (IDsicsi->Z > Zref || (IDsicsi->Z == Zref && IDsicsi->A > (Aref + 1))) {
               fPileup = kTRUE;
               IDsicsi->SetComment("Second particle stopping in Si, identification ChIo-Si required");
               return kTRUE;
            }
         } else { // only Z identification from Si-CsI
            if (IDcsi->Z == 4 && IDcsi->A == 8) {
               // traitement special 8Be
               // we ask for Z to be equal 3 in SiCsI, but with no mass identification
               // we do not try for 8He identification
               if (IDsicsi->Z < 3) {
                  fCoherent = kFALSE;
                  IDsicsi->SetComment("CsI-R/L & Si-CsI identifications not coherent");
                  return kTRUE;
               } else if (IDsicsi->Z == 3) {
                  return kTRUE;
               } else {
                  fPileup = kTRUE;
                  IDsicsi->SetComment("Second particle stopping in Si, identification ChIo-Si required");
                  return kTRUE;
               }
            }
            // everything else
            if (IDsicsi->Z == Zref) {
               return kTRUE;
            } else if (IDsicsi->Z < Zref) {
               fCoherent = kFALSE;
               IDsicsi->SetComment("CsI-R/L & Si-CsI identifications not coherent");
               return kTRUE;
            } else {
               fPileup = kTRUE;

               IDsicsi->SetComment("Second particle stopping in Si, identification ChIo-Si required");
               return kTRUE;
            }
         }
      }
      return kTRUE;
   }
   return kFALSE;
}
