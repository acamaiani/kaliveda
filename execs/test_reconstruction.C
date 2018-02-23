#include <KVDataRepositoryManager.h>
#include <KVDataSetManager.h>
#include <KVEventReconstructor.h>
#include <KVGANILDataReader.h>
#include <KVIDCsIRLLine.h>
#include <KVIDZALine.h>
#include <iostream>
#include "TApplication.h"
#include "TROOT.h"
using namespace std;

int main(int argc, char* argv[])
{
   KVBase::InitEnvironment();

   TApplication myapp("myapp", &argc, argv);

   ROOT::EnableThreadSafety();

   KVDataRepositoryManager drm;
   drm.Init();
   gDataSetManager->GetDataSet("INDRA_camp5")->cd();
   unique_ptr<KVGANILDataReader> raw_file(gDataSet->OpenRunfile<KVGANILDataReader>("raw", 6800));

   KVEventReconstructor erec(gMultiDetArray, new KVReconstructedEvent);

   auto start = std::chrono::steady_clock::now();
   for (int i = 0; i < 1000; ++i) {
      //std::cout << std::endl << "==============================================EVENT " << i << std::endl << std::endl;

      if (!raw_file->GetNextEvent()) break;

      erec.ReconstructEvent(raw_file->GetFiredDataParameters());

      //erec.GetEvent()->SetNumber(i);
      //erec.GetEvent()->ls();
   }
   auto end = std::chrono::steady_clock::now();
   auto diff = end - start;
   std::cout << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;
   erec.bilan();
}
