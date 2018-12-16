// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include <vector>
#include <unordered_map>
#include "CoreMinimal.h"
#include "Engine/TriggerVolume.h"
#include "EngineUtils.h"
#include "Engine.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "PointCloudGenerator.generated.h"


struct PointCloud
{
	// this map will hold the class mapping values (string label of class -> interger value)
	std::vector<float> points;
	// this holds the array of classes
	std::vector<int> classes;
	// this holds the points received from the point cloud
	std::unordered_map<std::string, int> class_mapping;
};

/**
 *
 */
UCLASS()
class POINTCLOUDGENERATORPLUGIN_API APointCloudGenerator : public ATriggerVolume
{

	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Point Cloud Generation")
		float resolution = 100.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Point Cloud Generation", META = (Name = "Show Trace"))
		bool showTrace = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Point Cloud Generation", META = (Name = "Record Classes"))
		bool recordClasses = true;



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void GatherPoints(FVector start, FVector range, PointCloud &pc, float res, bool trace, bool recordClasses);
	void TimerElapsed();


};

