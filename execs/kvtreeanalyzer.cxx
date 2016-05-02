#include <KVTreeAnalyzer.h>
#include "TRint.h"

int main(int argc, char** argv)
{
   // kvtreeanalyzer [file.root]

   TString fname;
   if (argc == 2) {
      fname = argv[1];
      --argc;
   }
   TRint* myapp = new TRint("kvtreeanalyzer", &argc, argv, NULL, 0);
   KVTreeAnalyzer* kvt = new KVTreeAnalyzer(kFALSE);
   if (fname != "" && fname.EndsWith(".root")) {
      kvt->OpenAnyFile(argv[1]);
   }
   myapp->Run();
   delete myapp;
   return 0;
}

