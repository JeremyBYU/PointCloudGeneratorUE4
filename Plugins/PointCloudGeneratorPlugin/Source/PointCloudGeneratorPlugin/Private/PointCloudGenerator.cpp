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
	if (enabled)
	{
		GetWorldTimerManager().SetTimer(
			UnusedHandle, this, &APointCloudGenerator::TimerElapsed, 1, false);
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("PointCloudGenerator disabled."));
	}
}


void APointCloudGenerator::TimerElapsed()
{
	auto bounds = this->GetBounds();

	auto start_x = bounds.Origin.X - bounds.BoxExtent.X;
	auto start_y = bounds.Origin.Y - bounds.BoxExtent.Y;
	auto start_z = bounds.Origin.Z + bounds.BoxExtent.Z;

	auto range_x = bounds.BoxExtent.X * 2;
	auto range_y = bounds.BoxExtent.Y * 2;
	auto range_z = bounds.BoxExtent.Z * 2;

	FVector start(start_x, start_y, start_z);
	FVector range(range_x, range_y, range_z);
	UE_LOG(LogTemp, Display, TEXT("Resolution: %f; Trace: %d; Record Classes: %d"), resolution, showTrace, recordClasses);

	// Build Point Cloud
	PointCloud pc;
	GatherPoints(start, range, pc, resolution, showTrace, recordClasses);

	// Save data
	SaveFile(saveDirectory, pc);
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
				pc.points.emplace_back(hit_result.ImpactPoint.X);
				pc.points.emplace_back(hit_result.ImpactPoint.Y);
				pc.points.emplace_back(hit_result.ImpactPoint.Z);

				if (recordClasses)
				{
					auto display_name = hit_result.GetActor()->GetActorLabel();
					std::string display_name_str(TCHAR_TO_UTF8(*display_name));
					// Check if the key exists
					if (pc.class_mapping.find(display_name_str) != pc.class_mapping.end())
					{
						// convert integer class mapping to float and add to point vector
						pc.points.emplace_back(static_cast<float>(pc.class_mapping[display_name_str]));
					}
					else
					{
						// else create new mapping and increment mapping count
						pc.class_mapping[display_name_str] = numClasses;
						pc.points.emplace_back(static_cast<float>(numClasses));
						numClasses += 1;
					}
					//UE_LOG(LogTemp, Warning, TEXT("PointCloudGenerator: Hit Component %s, at (%f, %f)"), *display_name, x, y);
				}


				if (trace)
				{
					DrawDebugLine(GetWorld(), trace_start, hit_result.ImpactPoint, FColor::Green, true, 1, 0, 1);
				}
			}
			else
			{
				//UE_LOG(LogTemp, Warning, TEXT("PointCloudGenerator: No object hit in raycast (%f, %f)"), x, y);
				if (trace)
				{
					DrawDebugLine(GetWorld(), trace_start, trace_end, FColor::Red, true, 1, 0, 1);
				}
			}

		}

	}

}

int Map2JSON(std::string fname, std::unordered_map<std::string, int> &m) {
	int count = 0;
	if (m.empty())
		return 0;

	// Now we want to transfer our unorderem_map into an ordered map
	// unordered map was faster, but now we care about human readable output
	std::map<int, std::string> ordered_map;
	for (auto it = m.begin(); it != m.end(); it++) {
		ordered_map[it->second] = it->first;
	}

	FILE *fp = fopen(fname.c_str(), "w");
	if (!fp)
		return -errno;
	fprintf(fp, "{\n");
	fprintf(fp, "  \"classes\": [\n");
	for (auto it = ordered_map.begin(); it != --ordered_map.end(); it++) {
		fprintf(fp, "    \"%s\",\n", it->second.c_str());
		count++;
	}
	fprintf(fp, "    \"%s\"\n", (--ordered_map.end())->second.c_str());
	fprintf(fp, "  ]\n");
	fprintf(fp, "}\n");

	fclose(fp);
	return count;
}

void APointCloudGenerator::SaveFile(FString saveDir, PointCloud &pc)
{
	if (FPaths::DirectoryExists(saveDir))
	{
		const long unsigned cols = recordClasses ? 4 : 3;
		const long unsigned rows = pc.points.size() / cols;
		const long unsigned leshape[] = { rows, cols };

		auto numpyPathF = FPaths::Combine(saveDir, FString("point_cloud.npy"));
		UE_LOG(LogTemp, Display, TEXT("Saving file: %s"), *numpyPathF);
		npy::SaveArrayAsNumpy(TCHAR_TO_UTF8(*numpyPathF), false, 2, leshape, pc.points);
		if (recordClasses)
		{
			auto classesPathF = FPaths::Combine(saveDir, FString("point_cloud_classes.json"));
			UE_LOG(LogTemp, Display, TEXT("Saving file: %s"), *classesPathF);
			Map2JSON(TCHAR_TO_UTF8(*classesPathF), pc.class_mapping);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Error! Directory does not exist: %s"), *saveDir);
		
	}

}


