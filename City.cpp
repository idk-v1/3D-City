#include "City.h"


City::City(sf::Vector3i pSize)
{
    Block b;
    size = pSize;
    blocks.resize(size.x);
    for (auto& x : blocks)
    {
        x.resize(size.y);
        for (auto& y : x)
        {
            y.resize(size.z);
            for (auto& z : y)
                z.setAttrib(b);
        }
    }
}


void City::generate(sf::Uint64 pSeed)
{
    seed = pSeed;
    srand(seed);

    for (int x = 0; x < size.x; x++)
        for (int z = 0; z < size.z; z++)
            blocks[x][0][z].setAttrib((*templatePtr)[1]);

    for (int x = 0; x < size.x; x++)
        for (int y = 1; y < size.y; y++)
            for (int z = 0; z < size.z; z++)
            {
                if(blocks[x][y - 1][z].isSolid && rand() % 3 && x % gridSize && z % gridSize)
                    blocks[x][y][z].setAttrib((*templatePtr)[1]);
                else
                    blocks[x][y][z].setAttrib((*templatePtr)[0]);
            }
}


void City::calcVis()
{
    for (int x = 0; x < size.x; x++)
        for (int y = 0; y < size.y; y++)
            for (int z = 0; z < size.z; z++)
                if (blocks[x][y][z].isSolid)
                {
                    blocks[x][y][z].visXP = !getBlock(x + 1, y, z).isSolid;
                    blocks[x][y][z].visXN = !getBlock(x - 1, y, z).isSolid;
                    blocks[x][y][z].visYP = !getBlock(x, y + 1, z).isSolid;
                    blocks[x][y][z].visYN = !getBlock(x, y - 1, z).isSolid;
                    blocks[x][y][z].visZP = !getBlock(x, y, z + 1).isSolid;
                    blocks[x][y][z].visZN = !getBlock(x, y, z - 1).isSolid;
                }
}


Block City::getBlock(int pX, int pY, int pZ)
{
    if (pX < 0 || pY < 0 || pZ < 0 || pX >= size.x || pY >= size.y || pZ >= size.z)
        return (*templatePtr)[0];
    return blocks[pX][pY][pZ];
}

Block City::getBlock(sf::Vector3i pPos)
{
    if (pPos.x < 0 || pPos.y < 0 || pPos.z < 0 || pPos.x >= size.x || pPos.y >= size.y || pPos.z >= size.z)
        return (*templatePtr)[0];
    return blocks[pPos.x][pPos.y][pPos.z];
}


void City::setTemplatePtr(std::vector<Block>* pTemplatePtr)
{
    templatePtr = pTemplatePtr;
}
