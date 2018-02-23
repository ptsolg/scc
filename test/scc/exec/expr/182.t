struct Point
{
	int x;
	int y;
	int z;
};

typedef struct Point Point3D;

int main()
{
	Point3D p;
	p.x = 0;
	p.y = 0;
	p.z = 0;

	Point3D* pp = &p;
	if (pp->x != 0)
		return 1;
	if (pp->y != 0)
		return 1;
	if (pp->z != 0)
		return 1;

	return 0;
}