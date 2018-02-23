typedef struct
{
	int x, y;
} Point2D;

void Point2D_setX(Point2D* self, int x)
{
	self->x = x;
}

int Point2D_getX(Point2D* self)
{
	return self->x;
}

void Point2D_setY(Point2D* self, int y)
{
	self->y = y;
}

int Point2D_getY(Point2D* self)
{
	return self->y;
}

void Point2D_init(Point2D* self)
{
	Point2D_setY(self, 0);
	Point2D_setX(self, 0);
}

int main()
{
	Point2D p;
	Point2D_init(&p);

	if (Point2D_getX(&p) != 0)
		return 1;
	if (Point2D_getY(&p) != 0)
		return 2;

	return 0;
}