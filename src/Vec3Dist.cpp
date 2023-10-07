#include "Vec3Dist.h"


Vec3Dist::Vec3Dist()
{

}


Vec3Dist::Vec3Dist(sf::Vector3f pPos, sf::Vector3f oPos)
{
    pos = oPos;
    dist = getDist(pPos, oPos);
}


float Vec3Dist::getDist(sf::Vector3f pPos, sf::Vector3f oPos)
{
    float scale = 10.f,
          x = oPos.x * scale - pPos.x,
          y = oPos.y * scale - pPos.y,
          z = oPos.z * scale - pPos.z;

    return std::sqrt(x * x + y * y + z * z);
}
