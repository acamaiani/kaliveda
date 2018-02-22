//Created by KVClassFactory on Wed Feb 21 13:42:47 2018
//Author: John Frankland,,,

#ifndef __KVINDRAGROUPRECONSTRUCTOR_H
#define __KVINDRAGROUPRECONSTRUCTOR_H

#include "KVGroupReconstructor.h"

class KVINDRAGroupReconstructor : public KVGroupReconstructor {
public:
   KVINDRAGroupReconstructor();

   virtual ~KVINDRAGroupReconstructor();

   ClassDef(KVINDRAGroupReconstructor, 1) //Reconstruct particles in INDRA groups
};

#endif
