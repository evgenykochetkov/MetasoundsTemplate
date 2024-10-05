// Fill out your copyright notice in the Description page of Project Settings.


#include "MyWaveTableFactory.h"

#include "MyWaveTable.h"

UMyWaveTableFactory::UMyWaveTableFactory() {
  SupportedClass = UMyWaveTable::StaticClass();
  bCreateNew = true;
}

UObject * UMyWaveTableFactory::FactoryCreateNew(UClass *Class, UObject *InParent, FName Name, EObjectFlags Flags, UObject *Context, FFeedbackContext *Warn) {
  return NewObject<UMyWaveTable>(InParent, Class, Name, Flags, Context);
}