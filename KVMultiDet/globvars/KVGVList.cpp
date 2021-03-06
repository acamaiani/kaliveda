// Author: Daniel Cussol
//
// 17/02/2004
#include "Riostream.h"
#include "KVGVList.h"

#include <KVEvent.h>

ClassImp(KVGVList)
//////////////////////////////////////////////////////////////////////////////////
/*
<h2>KVGVList</h2>
<h4>List of global variables</h4>

This class allows to process in a single call many KVVargGlob instances.
The methods used to initialize and fill variables in such a list are:

~~~~~~~~~~~~
    void Init(void);                             // initializes the global variables (call once)
    void CalculateGlobalVariables(KVEvent *e);   // compute all global variables for the event
~~~~~~~~~~~~

By default the KVGVList does not own the objects it contains (they may be on the stack).
User's responsibility in this case to delete heap-based objects after use.

~~~~~~~~~~~~
    // Declarations and initialisations
    KVEkin *Sekin=new KVEkin("SEkin");
    KVZmean zmean;
    KVZmax  zmax;
    KVGVList gvlist;
    gvlist.Add(Sekin);
    gvlist.Add(&zmean);
    gvlist.Add(&zmax);
    gvlist.Init();

    // Treatment loop for each event called for each event
    gvlist.CalculateGlobalVariables(event); // with KVEvent* pointer to event to analyse

    cout << "Total kinetic energy : " << Sekin->GetValue() << endl;
    cout << "Mean charge          : " << zmean() << endl;
    cout << "Standard deviation   : " << zmean("RMS") << endl;
    cout << "Charge of the heaviest   : " << zmax() << endl;
    cout << "Vpar of the heaviest     : " << zmax.GetZmax(0)->GetVpar() << endl;
~~~~~~~~~~~~

*/
//////////////////////////////////////////////////////////////////////////////////

//_________________________________________________________________
void KVGVList::init_KVGVList(void)
{
   fNbBranch = 0;
   fNbIBranch = 0;
}

//_________________________________________________________________
KVGVList::KVGVList(void): KVUniqueNameList()
{
   init_KVGVList();
}


//_________________________________________________________________
KVGVList::KVGVList(const KVGVList& a) : KVUniqueNameList()
{
   init_KVGVList();
   a.Copy(*this);
}

//_________________________________________________________________
void KVGVList::Init(void)
{
   // Initialisation of all global variables in list
   // As this method may be called several times we ensure that
   // variables are only initialised once

   this->R__FOR_EACH(KVVarGlob, ListInit)();
}

//_________________________________________________________________
void KVGVList::Reset(void)
{
   // Remise a zero avant le
   // traitement d'un evenement
   this->R__FOR_EACH(KVVarGlob, Reset)();
}

//_________________________________________________________________
void KVGVList::Fill(KVNucleus* c)
{
   // Calls KVVarGlob::Fill(KVNucleus*) method of all one-body variables in the list
   // for all KVNucleus satisfying the KVParticleCondition given to
   // KVVarGlob::SetSelection() (if no selection given, all nuclei are used).

   fVG1.R__FOR_EACH(KVVarGlob, FillWithCondition)(c);
}


//_________________________________________________________________
void KVGVList::Fill2(KVNucleus* c1, KVNucleus* c2)
{
   // Calls KVVarGlob::Fill(KVNucleus*,KVNucleus*) method of all two-body variables in the list
   // for all pairs of KVNucleus (c1,c2) satisfying the KVParticleCondition given to
   // KVVarGlob::SetSelection() (if no selection given, all nuclei are used).

   fVG2.R__FOR_EACH(KVVarGlob, Fill2WithCondition)(c1, c2);
}

//_________________________________________________________________
void KVGVList::FillN(KVEvent* r)
{
   // Calls KVVarGlob::FillN(KVEvent*) method of all N-body variables in the list
   TObjLink* lnk = fVGN.FirstLink();
   while (lnk) {
      KVVarGlob* vg = (KVVarGlob*) lnk->GetObject();
      vg->FillN(r);
      lnk = lnk->Next();
   }
}

void KVGVList::CalculateGlobalVariables(KVEvent* e)
{
   // This method will calculate all global variables defined in the list for the event 'e'.
   // - all 1-body variables will be calculated in a single loop over the particles;
   // - all 2-body variables will be calculated in a single loop over particle pairs;
   // - all N-body variables will be calculated

   Reset();
   if (Has1BodyVariables() || Has2BodyVariables()) {

#ifdef __WITH_TITER_BUG
      Int_t mult = e->GetMult();
      for (int it1 = 1; it1 <= mult; ++it1) {
         KVNucleus* n1 = e->GetParticle(it1);
         if (!n1->IsOK()) continue;
         if (Has1BodyVariables()) Fill(n1); // calculate 1-body variables

         if (Has2BodyVariables()) {
            for (int it2 = it1; it2 <= mult; ++it2) {
               KVNucleus* n2 = e->GetParticle(it2);
               if (!n2->IsOK()) continue;
               // calculate 2-body variables
               // we use every pair of particles (including identical pairs) in the event
               Fill2(n1, n2);
            }
         }
      }
#else
#ifdef WITH_CPP11
      for (KVEvent::Iterator it1(e, KVEvent::Iterator::Type::OK); it1 != KVEvent::Iterator::End(); ++it1) {
#else
      for (KVEvent::Iterator it1(e, KVEvent::Iterator::OK); it1 != KVEvent::Iterator::End(); ++it1) {
#endif
         if (Has1BodyVariables()) Fill(&(*it1));// calculate 1-body variables
         if (Has2BodyVariables()) {
            for (KVEvent::Iterator it2(it1); it2 != KVEvent::Iterator::End(); ++it2) {
               // calculate 2-body variables
               // we use every pair of particles (including identical pairs) in the event
               Fill2(&(*it1), &(*it2));
            }
         }
      }
#endif
   }
   // calculate N-body variables
   if (HasNBodyVariables()) FillN(e);
}

//_________________________________________________________________
KVVarGlob* KVGVList::GetGV(const Char_t* nom)
{
   //Return pointer to global variable in list with name 'nom'

   return (KVVarGlob*) FindObject(nom);
}

//_________________________________________________________________
void KVGVList::Add(TObject* obj)
{
   // Overrides KVUniqueNameList::Add(TObject*) so that global variable pointers are sorted
   // between the 3 lists used for 1-body, 2-body & N-body variables.
   //
   // We also print a warning in case the user tries to add a global variable with the
   // same name as one already in the list. In the case we retain only the first global variable,
   // any others with the same name are ignored

   KVUniqueNameList::Add(obj);   // add object to main list, check duplicates
   if (!ObjectAdded()) {
      Warning("Add", "You tried to add a global variable with the same name as one already in the list");
      Warning("Add", "Only the first variable added to the list will be used: name=%s class=%s",
              GetGV(obj->GetName())->GetName(), GetGV(obj->GetName())->ClassName());
      Warning("Add", "The following global variable (the one you tried to add) will be ignored:");
      printf("\n");
      obj->Print();
      return;
   }
   if (obj->InheritsFrom("KVVarGlob")) {
      // put global variable pointer in appropriate list
      KVVarGlob* vg = (KVVarGlob*)obj;
      if (vg->IsOneBody()) fVG1.Add(vg);
      else if (vg->IsTwoBody()) fVG2.Add(vg);
      else if (vg->IsNBody()) fVGN.Add(vg);
   }
}

//_________________________________________________________________

void KVGVList::MakeBranches(TTree* tree)
{
   // Create a branch in the TTree for each global variable in the list.
   // A leaf with the name of each global variable will be created to hold the
   // value of the variable (result of KVVarGlob::GetValue() method).
   // For multi-valued global variables we add a branch for each value with name
   //
   //    `GVname.ValueName`
   //
   // Any variable for which KVVarGlob::SetMaxNumBranches() was called with argument `0`
   // will not be added to the TTree.

   if (!tree) return;
   if (fNbIBranch >= MAX_CAP_BRANCHES && fNbBranch >= MAX_CAP_BRANCHES) return;

   // Make sure all variables are initialised before proceeding
   Init();

   TIter next(this);
   KVVarGlob* ob;
   while ((ob = (KVVarGlob*)next())) {
      if (ob->GetNumberOfBranches()) { //skip variables for which KVVarGlob::SetMaxNumBranches(0) was called
         if (ob->GetNumberOfValues() > 1) {
            // multi-valued variable
            for (int i = 0; i < ob->GetNumberOfBranches(); i++) {
               // replace any nasty mathematical symbols which could pose problems
               // in names of TTree leaves/branches
               TString sane_name(ob->GetValueName(i));
               sane_name.ReplaceAll("*", "star");
               if (ob->GetValueType(i) == 'I') {
                  if (fNbIBranch < MAX_CAP_BRANCHES)  tree->Branch(Form("%s.%s", ob->GetName(), sane_name.Data()), &fIBranchVar[ fNbIBranch++ ], Form("%s.%s/I", ob->GetName(), sane_name.Data()));
               } else {
                  if (fNbBranch < MAX_CAP_BRANCHES)  tree->Branch(Form("%s.%s", ob->GetName(), sane_name.Data()), &fBranchVar[ fNbBranch++ ], Form("%s.%s/D", ob->GetName(), sane_name.Data()));
               }
               if (fNbIBranch == MAX_CAP_BRANCHES) break;
               if (fNbBranch == MAX_CAP_BRANCHES) break;
            }
         } else {
            if (ob->GetValueType(0) == 'I') {
               if (fNbIBranch < MAX_CAP_BRANCHES)   tree->Branch(ob->GetName(), &fIBranchVar[ fNbIBranch++ ], Form("%s/I", ob->GetName()));
            } else {
               if (fNbBranch < MAX_CAP_BRANCHES)    tree->Branch(ob->GetName(), &fBranchVar[ fNbBranch++ ], Form("%s/D", ob->GetName()));
            }
         }
      }
   }
}

//_________________________________________________________________

void KVGVList::FillBranches()
{
   // Use this method ONLY if you first use MakeBranches(TTree*) in order to
   // automatically create branches for your global variables.
   // Call this method for each event in order to put the values of the variables
   // in the branches ready for TTree::Fill to be called (note that TTree::Fill is not
   // called in this method: you should do it after this).

   if (!fNbBranch && !fNbIBranch) return;  // MakeBranches has not been called

   int INT_index = 0;
   int FLT_index = 0;
   TIter next(this);
   KVVarGlob* ob;
   while ((ob = (KVVarGlob*)next())) {
      if (ob->GetNumberOfBranches()) { //skip variables for which KVVarGlob::SetMaxNumBranches(0) was called

         if (ob->GetNumberOfValues() > 1) {
            // multi-valued variable
            for (int j = 0; j < ob->GetNumberOfBranches(); j++) {
               if (ob->GetValueType(j) == 'I') fIBranchVar[ INT_index++ ] = (Int_t)ob->GetValue(j);
               else fBranchVar[ FLT_index++ ] = ob->GetValue(j);
            }
         } else {
            if (ob->GetValueType(0) == 'I') fIBranchVar[ INT_index++ ] = (Int_t)ob->GetValue();
            else fBranchVar[ FLT_index++ ] = ob->GetValue();
         }

      }
   }
}
