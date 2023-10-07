#ifndef CITY_H
#define CITY_H

#include <random>
#include <vector>

#include "Block.h"


class City
{
    public:
        City(sf::Vector3i pSize);

        void generate(sf::Uint64 pSeed);

        void calcVis();

        Block getBlock(int pX, int pY, int pZ);
        Block getBlock(sf::Vector3i pos);

        void setTemplatePtr(std::vector<Block>* pTemplatePtr);

    private:
        std::vector<std::vector<std::vector<Block>>> blocks;
        std::vector<Block>* templatePtr;
        sf::Vector3i size;
        sf::Uint64 seed;
        int gridSize = 5;
};

#endif
