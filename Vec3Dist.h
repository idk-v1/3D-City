#ifndef VEC3DIST_H
#define VEC3DIST_H

#include <SFML/System/Vector3.hpp>

#include <math.h>


class Vec3Dist
{
    public:
        Vec3Dist();

        Vec3Dist(sf::Vector3f pPlayerPos, sf::Vector3f pObjPos);

        sf::Vector3f pos;
        float dist = 0;
};

#endif
