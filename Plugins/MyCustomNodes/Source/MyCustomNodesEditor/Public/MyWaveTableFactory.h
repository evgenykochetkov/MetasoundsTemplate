// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "MyWaveTableFactory.generated.h"


UCLASS()
class MYCUSTOMNODESEDITOR_API UMyWaveTableFactory : public UFactory {
  GENERATED_BODY()

public:
  UMyWaveTableFactory();

  UObject* FactoryCreateNew(UClass* Class,
                            UObject* InParent,
                            FName Name,
                            EObjectFlags Flags,
                            UObject* Context,
                            FFeedbackContext* Warn);
};
