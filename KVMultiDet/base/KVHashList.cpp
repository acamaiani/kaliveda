//Created by KVClassFactory on Mon Nov 30 15:00:00 2009
//Author: John Frankland,,,

#include "KVHashList.h"
#include "THashList.h"

ClassImp(KVHashList)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVHashList</h2>
<h4>Extended version of ROOT THashList</h4>
This collection class uses a THashList for quick look-up of objects based on
the TString::Hash() value of their name, and adds all the extra functionality
defined in KVSeqCollection. Automatic rehashing of the list is enabled by default
(with rehash level = 2), unlike THashList (disabled by default).
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVHashList::KVHashList(Int_t capacity, Int_t rehash)
	: KVSeqCollection()
{
   // Create a THashList object. Capacity is the initial hashtable capacity
   // (i.e. number of slots), by default kInitHashTableCapacity = 17, and
   // rehash is the value at which a rehash will be triggered. I.e. when the
   // average size of the linked lists at a slot becomes longer than rehash
   // then the hashtable will be resized and refilled to reduce the collision
   // rate to about 1. The higher the collision rate, i.e. the longer the
   // linked lists, the longer lookup will take. 
   // The default value of rehash = 2 (minimum allowed value) - automatic
   // rehashing is enabled by default. If rehash=0 the table will
   // NOT automatically be rehashed. Use Rehash() for manual rehashing.
   // WARNING !!!
   // If the name of an object in the HashList is modified, The hashlist
   // must be Rehashed
   
   fCollection = new THashList(capacity, rehash);
}

KVHashList::~KVHashList()
{
   // Destructor
}

Float_t KVHashList::AverageCollisions() const
{
   // Return the average collision rate. The higher the number the longer
   // the linked lists in the hashtable, the slower the lookup. If the number
   // is high, or lookup noticeably too slow, perfrom a Rehash()
   
   return ((THashList*)fCollection)->AverageCollisions();
}

void KVHashList::Rehash(Int_t newCapacity)
{
   // Rehash the hashlist. If the collision rate becomes too high (i.e.
   // the average size of the linked lists become too long) then lookup
   // efficiency decreases since relatively long lists have to be searched
   // every time. To improve performance rehash the hashtable. This resizes
   // the table to newCapacity slots and refills the table. Use
   // AverageCollisions() to check if you need to rehash.

   ((THashList*)fCollection)->Rehash(newCapacity);
} 

TList* KVHashList::GetListForObject(const char *name) const
{
   // Return the THashTable's list (bucket) in which obj can be found based on
   // its hash; see THashTable::GetListForObject().

   return  ((THashList*)fCollection)->GetListForObject(name);
}

//______________________________________________________________________________

TList *KVHashList::GetListForObject(const TObject *obj) const
{
   // Return the THashTable's list (bucket) in which obj can be found based on
   // its hash; see THashTable::GetListForObject().

   return ((THashList*)fCollection)->GetListForObject(obj);
}

