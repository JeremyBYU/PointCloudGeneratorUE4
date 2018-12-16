// Fill out your copyright notice in the Description page of Project Settings.

// Fill out your copyright notice in the Description page of Project Settings.

#include "PointCloudGenerator.h"
#include "DrawDebugHelpers.h"


// Called when the game starts or when spawned
void APointCloudGenerator::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle UnusedHandle;
	// Timer put in because of paranio....
	GetWorldTimerManager().SetTimer(
		UnusedHandle, this, &APointCloudGenerator::TimerElapsed, 1, false);


	//for (TActorIterator<AStaticMeshActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	//{
	//	// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
	//	AStaticMeshActor *Mesh = *ActorItr;
	//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("%s, %s"), *ActorItr->GetName(), *ActorItr->GetActorLocation().ToString()));
	//	//ClientMessage(ActorItr->GetName());
	//	//ClientMessage(ActorItr->GetActorLocation().ToString());
	//}
}


void APointCloudGenerator::TimerElapsed()
{
	auto test = this->GetBounds();

	auto start_x = test.Origin.X - test.BoxExtent.X;
	auto start_y = test.Origin.Y - test.BoxExtent.Y;
	auto start_z = test.Origin.Z + test.BoxExtent.Z;

	auto range_x = test.BoxExtent.X * 2;
	auto range_y = test.BoxExtent.Y * 2;
	auto range_z = test.BoxExtent.Z * 2;

	FVector start(start_x, start_y, start_z);
	FVector range(range_x, range_y, range_z);
	UE_LOG(LogTemp, Warning, TEXT("Resolution: %f; Trace: %d"), resolution, showTrace);

	PointCloud pc;
	GatherPoints(start, range, pc, resolution, showTrace, recordClasses);

}


void APointCloudGenerator::GatherPoints(FVector start, FVector range, PointCloud &pc, float res = 25.0, bool trace = false, bool recordClasses = true)
{
	// Collision parameters and settings
	FCollisionQueryParams trace_params;
	FHitResult hit_result = FHitResult(ForceInit);
	trace_params.AddIgnoredActor(this);

	int numClasses = 0;

	for (float x = start.X; x < start.X + range.X; x += res)
	{
		for (float y = start.Y; y < start.Y + range.Y; y += res)
		{
			hit_result = FHitResult(ForceInit);
			FVector trace_start(x, y, start.Z);
			FVector trace_end(x, y, start.Z - range.Z);
			auto hit = GetWorld()->LineTraceSingleByChannel(hit_result, trace_start, trace_end, ECC_Visibility, trace_params);
			if (hit)
			{
				if (recordClasses)
				{
					auto display_name = hit_result.GetActor()->GetActorLabel();
					std::string display_name_str(TCHAR_TO_UTF8(*display_name));
					// Check if the key exists
					if (pc.class_mapping.find(display_name_str) != pc.class_mapping.end())
					{
						// If the class string key exists, then gets its integer mapping and add to class vector
						pc.classes.push_back(pc.class_mapping[display_name_str]);
					}
					else
					{
						// else create new mapping and increment mapping count
						pc.class_mapping[display_name_str] = numClasses;
						pc.classes.push_back(numClasses);
						numClasses += 1;
					}
					//auto object_name = comp->GetName();
					UE_LOG(LogTemp, Warning, TEXT("PointCloudGenerator: Hit Component %s, at (%f, %f)"), *display_name, x, y);
				}

				pc.points.emplace_back(hit_result.ImpactPoint.X);
				pc.points.emplace_back(hit_result.ImpactPoint.Y);
				pc.points.emplace_back(hit_result.ImpactPoint.Z);

				if (trace)
				{
					DrawDebugLine(GetWorld(), trace_start, hit_result.ImpactPoint, FColor::Green, true, 1, 0, 1);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("PointCloudGenerator: No object hit in raycast (%f, %f)"), x, y);
				if (trace)
				{
					DrawDebugLine(GetWorld(), trace_start, trace_end, FColor::Red, true, 1, 0, 1);
				}
			}

		}

	}

}


