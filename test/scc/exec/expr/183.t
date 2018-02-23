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
	Point3D* pp = &p;
	
	pp->y = 2;
	if (pp->y != 2)
		return 2;

	return 0;
}