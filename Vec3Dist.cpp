#include "Vec3Dist.h"


Vec3Dist::Vec3Dist()
{

}


Vec3Dist::Vec3Dist(sf::Vector3f pPlayerPos, sf::Vector3f pObjPos)
{
    pos = pObjPos;
    float x = pObjPos.x * SCALE - pPlayerPos.x,
          y = pObjPos.y * SCALE - pPlayerPos.y,
          z = pObjPos.z * SCALE - pPlayerPos.z;

    dist = sqrt(x * x + y * y + z * z);
}
