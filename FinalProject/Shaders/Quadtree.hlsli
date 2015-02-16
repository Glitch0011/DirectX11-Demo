
#define PER_QUAD_CAPACITY 16
#define DIVISION_FACTOR 2.0f
#define CHILD_COUNT 4

bool intersect(float4 PosAndSize, float4 pos)
{
	return
		(
		pos.x < PosAndSize.z &&
		pos.y > PosAndSize.w &&
		pos.z < PosAndSize.x &&
		pos.w < PosAndSize.y
		);
}

bool intersect(float4 PosAndSize, float2 pos)
{
	return
		(
		pos.x > PosAndSize.x &&
		pos.y > PosAndSize.y &&
		pos.x < PosAndSize.z &&
		pos.y < PosAndSize.w
		);
}


class Quadtree
{
	float4 PosAndSize;

	int values[PER_QUAD_CAPACITY];
	int valueCount;

	int children[CHILD_COUNT];
	bool hasChildren;

	int id;

	void subdivide();

	bool insert(int index, float2 pos);
};

AppendStructuredBuffer<Quadtree> quadAppendableData : register(t6);
RWStructuredBuffer<Quadtree> quadData : register(t6);

void Quadtree::subdivide()
{
	float2 perNewPart = (this.PosAndSize.zw - this.PosAndSize.xy) / DIVISION_FACTOR;

	int count = 0;

	for (int x = 0; x < DIVISION_FACTOR; x++)
	{
		for (int y = 0; y < DIVISION_FACTOR; y++)
		{
			float2 min = float2(
				this.PosAndSize.xy +
				float2(
				x * perNewPart[0],
				y * perNewPart[1]));

			Quadtree branch;

			branch.PosAndSize.xy = min;
			branch.PosAndSize.zw = min + float2(perNewPart[0], perNewPart[1]);

			uint numStructs;
			uint stride;
			quadAppendableData.GetDimensions(numStructs, stride);

			branch.hasChildren = false;
			branch.id = numStructs;
			
			children[count++] = branch.id;

			quadAppendableData.Append(branch);
		}
	}

	this.hasChildren = true;
}

bool Quadtree::insert(int index, float2 pos)
{
	if (intersect(PosAndSize, pos))
	{
		if (valueCount < PER_QUAD_CAPACITY)
		{
			values[valueCount++] = index;
			return true;
		}
		else
		{
			if (!this.hasChildren)
				subdivide();

			for (int i = 0; i < CHILD_COUNT; i++)
				if (quadData[children[i]].insert(index, pos))
					break;
		}
	}
	else
	{
		return false;
	}
}