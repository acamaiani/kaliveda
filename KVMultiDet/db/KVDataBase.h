/***************************************************************************
$Id: KVDataBase.h,v 1.20 2009/01/22 13:55:00 franklan Exp $
                          KVDataBase.h  -  description
                             -------------------
    begin                : jeu fév 6 2003
    copyright            : (C) 2003 by Alexis Mignon
    email                : mignon@ganil.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KV_DATA_BASE_H
#define KV_DATA_BASE_H
#include "TFolder.h"
#include "TString.h"
#include "KVList.h"
#include "KVDBTable.h"
#include "KVDBRecord.h"

class TFile;
class KVNumberList;

class KVDataBase: public TFolder {

   TString fFolderName;

public:
   KVDataBase();
   KVDataBase(const Char_t* name);
   KVDataBase(const Char_t* name, const Char_t* title);
   virtual ~ KVDataBase();

   inline virtual KVDBTable* GetTable(const Char_t* table) const;
   inline virtual TList* GetTables() const;
   virtual Bool_t AddTable(KVDBTable* table);
   virtual KVDBTable* AddTable(const Char_t* name, const Char_t* title,
                               Bool_t unique = kFALSE);
   virtual KVDBRecord* GetRecord(const Char_t* table_name,
                                 const Char_t* rec_name) const;
   virtual void Print(Option_t* option = "") const;

   ClassDef(KVDataBase, 3)     // Base Class for a database of parameters
};

KVDBTable* KVDataBase::GetTable(const Char_t* table) const
{
   return (KVDBTable*) FindObject(table);
}

TList* KVDataBase::GetTables() const
{
   return (TList*) GetListOfFolders();
}

#endif
