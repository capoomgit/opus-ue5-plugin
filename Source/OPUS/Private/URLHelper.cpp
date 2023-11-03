// Fill out your copyright notice in the Description page of Project Settings.

#include "URLHelper.h"
#include "CoreMinimal.h"

FString URLHelper::ApiUrl = "https://opus5.p.rapidapi.com/";
FString URLHelper::GetModels = "https://opus5.p.rapidapi.com/get_model_names";
FString URLHelper::CreateComponent = "https://opus5.p.rapidapi.com/create_opus_component";
FString URLHelper::CreateStructure = "https://opus5.p.rapidapi.com/create_opus_structure";
FString URLHelper::GetAttributesWithName = "https://opus5.p.rapidapi.com/get_attributes_with_name";
FString URLHelper::GetJobResults = "https://opus5.p.rapidapi.com/get_opus_job_result";
