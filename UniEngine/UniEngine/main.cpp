
#include <stdio.h>
#include "ECS.h"
#include "DeltaTime.h"
#include <iostream>

struct Transform
{
	Transform(float x, float y, float z) : x(x), y(y), z(z) {};

	float x, y, z;
};

void TransformSystem(ECS& ecs, float deltaTime)
{
	for (int i = 0; i < ecs.GetAllEntities().size(); i++)
	{
		std::cout << deltaTime << "\n";
	}
}

void main()
{
	ECS ecs;
	DeltaTime time;

	for (int i = 0; i < 10000; i++)
	{
		uint64_t ent = ecs.CreateEntity();
		ecs.AttachComponents(ent, Transform(1.0f, 0.0f, 1.0f));
	}

	ecs.AddSystem(TransformSystem);

	while (true)
	{
		float delta = time.CalculateDeltaTime();
		ecs.UpdateSystems( delta );
	}
}