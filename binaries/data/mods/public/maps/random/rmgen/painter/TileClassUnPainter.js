/**
 * Removes the given tileclass from a given area.
 */
function TileClassUnPainter(tileClass)
{
	this.tileClass = tileClass;
}

TileClassUnPainter.prototype.paint = function(area)
{
	for (const point of area.getPoints())
		this.tileClass.remove(point);
};
