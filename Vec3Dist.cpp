#include "Vec3Dist.h"


Vec3Dist::Vec3Dist()
{

}


Vec3Dist::Vec3Dist(sf::Vector3f pPlayerPos, sf::Vector3f pObjPos)
{
    pos = pObjPos;
    float scale = 10.f,
          x = pObjPos.x * scale - pPlayerPos.x,
          y = pObjPos.y * scale - pPlayerPos.y,
          z = pObjPos.z * scale - pPlayerPos.z;

    dist = std::sqrt(x * x + y * y + z * z);
}
