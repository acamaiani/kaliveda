//Created by KVClassFactory on Tue Jul 12 11:43:52 2016
//Author: bonnet,,,

#ifndef __KVEXPDB_H
#define __KVEXPDB_H

#include "KVDataBase.h"
#include "KVDBRun.h"

class KVDBSystem;

class KVExpDB : public KVDataBase {

protected:
   TString fDataSet;//the name of the dataset to which this database is associated
   TString fDataSetDir;//the directory containing the dataset files

   Int_t kFirstRun;
   Int_t kLastRun;
   KVDBTable* fRuns;            //-> table of runs
   KVDBTable* fSystems;         //-> table of systems

   Bool_t OpenCalibFile(const Char_t* type, std::ifstream& fs) const;
   virtual void ReadSystemList();
   void init();

public:

   KVExpDB();
   KVExpDB(const Char_t* name);
   KVExpDB(const Char_t* name, const Char_t* title);

   virtual ~KVExpDB();

   virtual void LinkListToRunRanges(TList* list, UInt_t rr_number,
                                    UInt_t run_ranges[][2]);
   virtual void LinkRecordToRunRanges(KVDBRecord* rec, UInt_t rr_number,
                                      UInt_t run_ranges[][2]);
   virtual void LinkRecordToRunRange(KVDBRecord* rec, UInt_t first_run,
                                     UInt_t last_run);
   virtual void LinkListToRunRange(TList* list, KVNumberList nl);
   virtual void LinkRecordToRunRange(KVDBRecord* rec,  KVNumberList nl);
   virtual void LinkRecordToRun(KVDBRecord* rec,  Int_t run);

   void AddRun(KVDBRun* r)
   {
      fRuns->AddRecord(r);
   }
   virtual KVSeqCollection* GetRuns() const
   {
      return fRuns->GetRecords();
   }
   KVDBRun* GetDBRun(Int_t number) const
   {
      return (KVDBRun*)fRuns->GetRecord(number);
   }
   Int_t GetFirstRunNumber() const
   {
      return kFirstRun;
   }
   Int_t GetLastRunNumber() const
   {
      return kLastRun;
   }

   virtual KVDBSystem* GetSystem(const Char_t* system) const
   {
      return (KVDBSystem*) fSystems->GetRecord(system);
   }
   virtual KVSeqCollection* GetSystems() const
   {
      return fSystems->GetRecords();
   }

   void AddSystem(KVDBSystem* r)
   {
      fSystems->AddRecord(r);
   }
   void RemoveSystem(KVDBSystem* s)
   {
      fSystems->RemoveRecord(s);
   }

   void WriteSystemsFile() const;
   void WriteRunListFile() const;

   virtual void Save(const Char_t*);

   virtual const Char_t* GetDBEnv(const Char_t*) const;
   const Char_t* GetCalibFileName(const Char_t* type) const
   {
      return GetDBEnv(type);
   }

   virtual void WriteObjects(TFile*)
   {
      // Abstract method. Can be overridden in child classes.
      // When the database is written to disk (by the currently active dataset, see
      // KVDataSet::WriteDBFile) any associated objects (histograms, trees, etc.)
      // can be written using this method.
      // The pointer to the file being written is passed as argument.

      AbstractMethod("WriteObjects");
   }

   virtual void ReadObjects(TFile*)
   {
      // Abstract method. Can be overridden in child classes.
      // When the database is read from disk (by the currently active dataset, see
      // KVDataSet::OpenDBFile) any associated objects (histograms, trees, etc.)
      // stored in the file can be read using this method.
      // The pointer to the file being read is passed as argument.

      AbstractMethod("ReadObjects");
   }

   virtual void PrintRuns(KVNumberList&) const;

   virtual void cd();

   const Char_t* GetDataSetDir() const
   {
      return fDataSetDir;
   }
   void SetDataSetDir(const Char_t* d)
   {
      fDataSetDir = d;
   }
   virtual void Build()
   {
      AbstractMethod("Build");
   }

   static KVExpDB* MakeDataBase(const Char_t* name, const Char_t* datasetdir);

   ClassDef(KVExpDB, 1) //base class to describe database of an experiment
};

//........ global variable
R__EXTERN KVExpDB* gExpDB;

#endif
