#include "Vec3Dist.h"


Vec3Dist::Vec3Dist()
{

}


Vec3Dist::Vec3Dist(sf::Vector3f pPlayerPos, sf::Vector3f pObjPos)
{
    pos = pObjPos;
    dist = getDist(pPlayerPos, pObjPos);
}


float Vec3Dist::getDist(sf::Vector3f pPlayerPos, sf::Vector3f pObjPos)
{
    float scale = 10.f,
          x = pObjPos.x * scale - pPlayerPos.x,
          y = pObjPos.y * scale - pPlayerPos.y,
          z = pObjPos.z * scale - pPlayerPos.z;

    return std::sqrt(x * x + y * y + z * z);
}
