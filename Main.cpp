// https://github.com/idk-v1/3D-City
// Currently just a basic 3D renderer with collisions and minor clipping errors

#include <fstream>

#include "City.h"


void getInput(sf::RenderWindow& pWin, sf::Vector3f& pVel, sf::Vector3f& pRot, sf::Vector2i& pMousePos, int pDelta);


void collide(City& pCity, sf::Vector3f& pPos, sf::Vector3f& pVel);


void genCube(sf::VertexArray& pVert, int pIndex, Block pBlock, sf::Vector3f pPlayerPos, sf::Vector3f pPlayerRot, sf::Vector3f pObjPos, sf::Vector2u pWinSize);


void genLights(sf::VertexArray& pVert, int pIndex, Block pBlock, sf::Vector3f pPlayerPos, sf::Vector3f pPlayerRot, sf::Vector3f pObjPos, sf::Vector2i pLightPos, sf::Vector2i pLightSize, sf::Vector2u pWinSize, bool pXLightState, bool pZLightState);


bool project(sf::Vertex& pVert, sf::Vector3f pPlayerPos, sf::Vector3f pPlayerRot, sf::Vector3f pObjPos, sf::Vector2u pWinPos);


void loadBlockTemplates(std::vector<Block>& pBlocks);


int main()
{
    sf::RenderWindow win(sf::VideoMode(800, 600), "");
    sf::RenderTexture buildTex;
    sf::Texture tex;
    sf::RenderStates renderState(&tex);
    sf::Shader shader;
    sf::View view;
    sf::Clock timer;
    sf::VertexArray vert, lVert;
    sf::Color bg(255, 127, 255);
    sf::RectangleShape overlay, buildRect;

    sf::Vector3f pos(-50.f, 100.f, -50.f), vel, rot(225.f, -45.f, 0.f);
    sf::Vector2i mousePos(win.getSize().x / 2, win.getSize().y / 2);

    sf::Vector3i size(11, 11, 11);
    City city(size);
    std::vector<Vec3Dist> reorder;
    std::vector<Block> templates;

    int delta, count, dayTime = 0, dayLength = 60000;
    unsigned long long ticks = 0;
    bool focus = true, click = false, paused = false;

    Block block;

    if (!shader.loadFromFile("shader.frag", sf::Shader::Fragment))
        return EXIT_FAILURE;
    win.setFramerateLimit(60);
    win.setMouseCursorVisible(false);
    sf::Mouse::setPosition(sf::Vector2i(win.getSize().x / 2, win.getSize().y / 2), win);
    vert.setPrimitiveType(sf::Quads);
    lVert.setPrimitiveType(sf::Quads);
    loadBlockTemplates(templates);
    city.setTemplatePtr(&templates);
    city.generate(time(0));
    tex.loadFromFile("res/blocks.png");
    overlay.setFillColor(sf::Color(0, 0, 0, 96));
    vert.resize(size.x * size.y * size.z * 4 * 3 * 2);
    reorder.resize(size.x * size.y * size.z);

    count = 0;
    for (int x = 0; x < size.x; x++)
        for (int y = 0; y < size.y; y++)
            for (int z = 0; z < size.z; z++)
            {
                block = city.getBlock(x, y, z);
                if (block.isSolid)
                    count += block.lights.size();
            }
    lVert.resize(count * 4);

    while (win.isOpen())
    {
        sf::Event evt;
        while (win.pollEvent(evt))
            switch (evt.type)
            {
            case sf::Event::Closed:
                win.close();
                break;
            case sf::Event::GainedFocus:
                focus = true;
                break;
            case sf::Event::LostFocus:
                focus = false;
                break;
            case sf::Event::KeyPressed:
                if (focus)
                {
                    if (evt.key.code == sf::Keyboard::Escape)
                    {
                        paused = !paused;
                        if (paused)
                        {
                            win.setFramerateLimit(20);
                            win.setMouseCursorVisible(true);
                            sf::Mouse::setPosition(sf::Vector2i(win.getSize().x / 2, win.getSize().y / 2), win);
                        }
                        else
                        {
                            win.setFramerateLimit(60);
                            win.setMouseCursorVisible(false);
                            sf::Mouse::setPosition(sf::Vector2i(win.getSize().x / 2, win.getSize().y / 2), win);
                            mousePos = sf::Vector2i(win.getSize().x / 2, win.getSize().y / 2);
                        }
                    }
                }
                break;
            case sf::Event::Resized:
                view.reset(sf::FloatRect(0, 0, evt.size.width, evt.size.height));
                win.setView(view);
                break;
            }

        delta = timer.restart().asMilliseconds();
        ticks += delta;
        dayTime = ticks % dayLength - dayLength / 2;
        if (focus && !paused)
            getInput(win, vel, rot, mousePos, delta);
        collide(city, pos, vel);

        for (int x = 0; x < size.x; x++)
            for (int y = 0; y < size.y; y++)
                for (int z = 0; z < size.z; z++)
                {
                    block = city.getBlock(x, y, z);
                    for (int i = 0; i < block.lights.size(); i++)
                        if (rand() % 100 == 0)
                            city.getBlockPtr(x, y, z)->toggle(i);
                }

        win.setTitle("3D City | FPS: " + (delta ? std::to_string(1000 / delta) : "Infinity"));

        city.calcVis(reorder, pos);

        count = 0;
        for (int ii = 0; ii < reorder.size(); ii++)
        {
            block = city.getBlock(sf::Vector3i(reorder[ii].pos));
            for (int i = 0; i < block.lights.size(); i += 2)
                genLights(lVert, count++ * 4 * 2, block, pos, rot, reorder[ii].pos, block.lights[i].pos, block.lights[i].size, win.getSize(), block.lights[i].state, block.lights[i + 1].state);
        }

        for (int ii = 0; ii < reorder.size(); ii++)
        {
            genCube(vert, ii * 4 * 3 * 2, city.getBlock(sf::Vector3i(reorder[ii].pos)), pos, rot, reorder[ii].pos, win.getSize());
            for (int i = 0; i < 4 * 3; i++)
            {
                vert[(ii * 2 + 1) * 4 * 3 + i] = vert[ii * 2 * 4 * 3 + i];
                vert[ii * 2 * 4 * 3 + i].color = bg;
                vert[ii * 2 * 4 * 3 + i].texCoords.y -= 60;
            }
        }

        buildTex.create(win.getSize().x, win.getSize().y);
        buildTex.clear();
        buildTex.draw(vert, &tex);
        buildTex.display();
        buildRect.setSize(sf::Vector2f(win.getSize()));
        buildRect.setTexture(&buildTex.getTexture());
        win.clear(bg);
        win.draw(lVert);
        win.draw(buildRect, &shader);

        if (paused)
        {
            overlay.setSize(sf::Vector2f(win.getSize()));
            win.draw(overlay);
        }

        win.display();

        for (int i = 0; i < vert.getVertexCount(); i++)
            vert[i] = sf::Vertex();
        for (int i = 0; i < lVert.getVertexCount(); i++)
            lVert[i] = sf::Vertex();
        for (int i = 0; i < reorder.size(); i++)
            reorder[i] = Vec3Dist();
    }
}

void genCube(sf::VertexArray& pVert, int pIndex, Block pBlock, sf::Vector3f pPlayerPos, sf::Vector3f pPlayerRot, sf::Vector3f pObjPos, sf::Vector2u pWinSize)
{
    if (pObjPos.x == 0 && pObjPos.y == 0 && pObjPos.z == 0)
        return;
    else
    {
        pObjPos.x -= 0.5f;
        pObjPos.y -= 0.5f;
        pObjPos.z -= 0.5f;
    }
    if (pBlock.visXP && pPlayerPos.x > (pObjPos.x + 0.5f) * SCALE)
    {
        for (int i = 0; i < 4; i++)
            pVert[pIndex + 0 + i].color = sf::Color(255, 255, 255);
        if (/**/project(pVert[pIndex + 0], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y + 0.5f, pObjPos.z - 0.5), pWinSize) &&
                project(pVert[pIndex + 1], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y + 0.5f, pObjPos.z + 0.5), pWinSize) &&
                project(pVert[pIndex + 2], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y - 0.5f, pObjPos.z + 0.5), pWinSize) &&
                project(pVert[pIndex + 3], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y - 0.5f, pObjPos.z - 0.5), pWinSize))
        {
            pVert[pIndex + 0].texCoords = sf::Vector2f((pBlock.type + 0) * TEXSIZE + 1, 2 * TEXSIZE);
            pVert[pIndex + 1].texCoords = sf::Vector2f((pBlock.type + 1) * TEXSIZE + 1, 2 * TEXSIZE);
            pVert[pIndex + 2].texCoords = sf::Vector2f((pBlock.type + 1) * TEXSIZE + 1, 3 * TEXSIZE);
            pVert[pIndex + 3].texCoords = sf::Vector2f((pBlock.type + 0) * TEXSIZE + 1, 3 * TEXSIZE);
        }
        else
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 0 + i] = sf::Vertex();
    }
    else if (pBlock.visXN && pPlayerPos.x < (pObjPos.x - 0.5f) * SCALE)
    {
        for (int i = 0; i < 4; i++)
            pVert[pIndex + 0 + i].color = sf::Color(0, 0, 0);
        if (/**/project(pVert[pIndex +  0], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y + 0.5f, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex +  1], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y + 0.5f, pObjPos.z + 0.5f), pWinSize) &&
                project(pVert[pIndex +  2], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y - 0.5f, pObjPos.z + 0.5f), pWinSize) &&
                project(pVert[pIndex +  3], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y - 0.5f, pObjPos.z - 0.5f), pWinSize))
        {
            pVert[pIndex + 0].texCoords = sf::Vector2f((pBlock.type + 0) * TEXSIZE + 1, 2 * TEXSIZE);
            pVert[pIndex + 1].texCoords = sf::Vector2f((pBlock.type + 1) * TEXSIZE + 1, 2 * TEXSIZE);
            pVert[pIndex + 2].texCoords = sf::Vector2f((pBlock.type + 1) * TEXSIZE + 1, 3 * TEXSIZE);
            pVert[pIndex + 3].texCoords = sf::Vector2f((pBlock.type + 0) * TEXSIZE + 1, 3 * TEXSIZE);
        }
        else
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 0 + i] = sf::Vertex();
    }
    if (pBlock.visYP && pPlayerPos.y > (pObjPos.y + 0.5f) * SCALE)
    {
        for (int i = 0; i < 4; i++)
            pVert[pIndex + 4 + i].color = sf::Color(192, 192, 192);
        if (/**/project(pVert[pIndex +  4], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y + 0.5f, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex +  5], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y + 0.5f, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex +  6], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y + 0.5f, pObjPos.z + 0.5f), pWinSize) &&
                project(pVert[pIndex +  7], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y + 0.5f, pObjPos.z + 0.5f), pWinSize))
        {
            pVert[pIndex + 4].texCoords = sf::Vector2f((pBlock.type + 0) * TEXSIZE + 1, 3 * TEXSIZE);
            pVert[pIndex + 5].texCoords = sf::Vector2f((pBlock.type + 1) * TEXSIZE + 1, 3 * TEXSIZE);
            pVert[pIndex + 6].texCoords = sf::Vector2f((pBlock.type + 1) * TEXSIZE + 1, 4 * TEXSIZE);
            pVert[pIndex + 7].texCoords = sf::Vector2f((pBlock.type + 0) * TEXSIZE + 1, 4 * TEXSIZE);
        }
        else
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 4 + i] = sf::Vertex();
    }
    else if (pBlock.visYN && pPlayerPos.y < (pObjPos.y - 0.5f) * SCALE)
    {
        for (int i = 0; i < 4; i++)
            pVert[pIndex + 4 + i].color = sf::Color(64, 64, 64);
        if (/**/project(pVert[pIndex +  4], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y - 0.5f, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex +  5], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y - 0.5f, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex +  6], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y - 0.5f, pObjPos.z + 0.5f), pWinSize) &&
                project(pVert[pIndex +  7], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y - 0.5f, pObjPos.z + 0.5f), pWinSize))
        {
            pVert[pIndex + 4].texCoords = sf::Vector2f((pBlock.type + 0) * TEXSIZE + 1, 3 * TEXSIZE);
            pVert[pIndex + 5].texCoords = sf::Vector2f((pBlock.type + 1) * TEXSIZE + 1, 3 * TEXSIZE);
            pVert[pIndex + 6].texCoords = sf::Vector2f((pBlock.type + 1) * TEXSIZE + 1, 4 * TEXSIZE);
            pVert[pIndex + 7].texCoords = sf::Vector2f((pBlock.type + 0) * TEXSIZE + 1, 4 * TEXSIZE);
        }
        else
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 4 + i] = sf::Vertex();
    }
    if (pBlock.visZP && pPlayerPos.z > (pObjPos.z + 0.5f) * SCALE)
    {
        for (int i = 0; i < 4; i++)
            pVert[pIndex + 8 + i].color = sf::Color(128, 128, 128);
        if (/**/project(pVert[pIndex +  8], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y + 0.5f, pObjPos.z + 0.5f), pWinSize) &&
                project(pVert[pIndex +  9], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y + 0.5f, pObjPos.z + 0.5f), pWinSize) &&
                project(pVert[pIndex + 10], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y - 0.5f, pObjPos.z + 0.5f), pWinSize) &&
                project(pVert[pIndex + 11], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y - 0.5f, pObjPos.z + 0.5f), pWinSize))
        {
            pVert[pIndex +  8].texCoords = sf::Vector2f((pBlock.type + 0) * TEXSIZE + 1, 2 * TEXSIZE);
            pVert[pIndex +  9].texCoords = sf::Vector2f((pBlock.type + 1) * TEXSIZE + 1, 2 * TEXSIZE);
            pVert[pIndex + 10].texCoords = sf::Vector2f((pBlock.type + 1) * TEXSIZE + 1, 3 * TEXSIZE);
            pVert[pIndex + 11].texCoords = sf::Vector2f((pBlock.type + 0) * TEXSIZE + 1, 3 * TEXSIZE);
        }
        else
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 8 + i] = sf::Vertex();
    }
    else if (pBlock.visZN && pPlayerPos.z < (pObjPos.z - 0.5f) * SCALE)
    {
        for (int i = 0; i < 4; i++)
            pVert[pIndex + 8 + i].color = sf::Color(128, 128, 128);
        if (/**/project(pVert[pIndex +  8], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y + 0.5f, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex +  9], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y + 0.5f, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex + 10], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y - 0.5f, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex + 11], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y - 0.5f, pObjPos.z - 0.5f), pWinSize))
        {
            pVert[pIndex +  8].texCoords = sf::Vector2f((pBlock.type + 0) * TEXSIZE + 1, 2 * TEXSIZE);
            pVert[pIndex +  9].texCoords = sf::Vector2f((pBlock.type + 1) * TEXSIZE + 1, 2 * TEXSIZE);
            pVert[pIndex + 10].texCoords = sf::Vector2f((pBlock.type + 1) * TEXSIZE + 1, 3 * TEXSIZE);
            pVert[pIndex + 11].texCoords = sf::Vector2f((pBlock.type + 0) * TEXSIZE + 1, 3 * TEXSIZE);
        }
        else
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 8 + i] = sf::Vertex();
    }
}


// https://en.wikipedia.org/wiki/3D_projection
bool project(sf::Vertex& pVert, sf::Vector3f pPlayerPos, sf::Vector3f pPlayerRot, sf::Vector3f pObjPos, sf::Vector2u pWinSize)
{
    float fog = 500.f,
          x = pObjPos.x * SCALE - pPlayerPos.x,
          y = pObjPos.y * SCALE - pPlayerPos.y,
          z = pObjPos.z * SCALE - pPlayerPos.z;
    sf::Vector3f d,
    c(std::cos(pPlayerRot.y * 3.14f / 180.f), std::cos(pPlayerRot.x * 3.14f / 180.f), std::cos(pPlayerRot.z * 3.14f / 180.f)),
    s(std::sin(pPlayerRot.y * 3.14f / 180.f), std::sin(pPlayerRot.x * 3.14f / 180.f), std::sin(pPlayerRot.z * 3.14f / 180.f));

    d.x = c.y * (s.z * y + c.z * x) - s.y * z;
    d.y = s.x * (c.y * z + s.y * (s.z * y + c.z * x)) + c.x * (c.z * y - s.z * x);
    d.z = c.x * (c.y * z + s.y * (s.z * y + c.z * x)) - s.x * (c.z * y - s.z * x);

    pVert.position.x = pWinSize.x / 2 - (d.x * 1000.f) / -d.z;
    pVert.position.y = pWinSize.y / 2 - (d.y * 1000.f) / -d.z;
    pVert.color.a = std::fmax(0, 255 - std::fmin(255, 255.f * (std::sqrt(x * x + y * y + z * z) / fog)));

    return d.z < 0.f;
}


void loadBlockTemplates(std::vector<Block>& pBlocks)
{
    std::ifstream file("res/blocks.type");
    int count;
    std::vector<std::string> lights;
    std::string name, tempStr;
    int lightCount;
    bool solid;
    if (file.is_open())
    {
        file >> count;
        pBlocks.resize(count);
        for (int i = 0; i < count; i++)
        {
            file >> solid >> lightCount;
            lights.resize(lightCount);
            for (int ii = 0; ii < lightCount; ii++)
            {
                file >> tempStr;
                lights[ii] = tempStr;
            }
            file >> name;
            pBlocks[i].type = i;
            pBlocks[i].isSolid = solid;
            pBlocks[i].lights.resize(lightCount * 2);
            for (int ii = 0; ii < lightCount * 2; ii++)
                pBlocks[i].lights[ii] = Light(lights[ii / 2]);
            pBlocks[i].name = name;
        }
    }
}


void getInput(sf::RenderWindow& pWin, sf::Vector3f& pVel, sf::Vector3f& pRot, sf::Vector2i& pMousePos, int pDelta)
{
    sf::Vector2f sensitivity(0.25f, 0.25f);
    float speed = 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        pVel.x += pDelta * speed / 100.f * std::sin((pRot.x + 180.f) * 3.14f / 180.f) * std::sin((pRot.y +  90.f) * 3.14f / 180.f);
        pVel.z += pDelta * speed / 100.f * std::sin((pRot.x + 270.f) * 3.14f / 180.f) * std::sin((pRot.y +  90.f) * 3.14f / 180.f);
        pVel.y += pDelta * speed / 100.f * std::sin((pRot.y +   0.f) * 3.14f / 180.f);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        pVel.x += pDelta * speed / 100.f * std::sin((pRot.x +   0.f) * 3.14f / 180.f) * std::sin((pRot.y +  90.f) * 3.14f / 180.f);
        pVel.z += pDelta * speed / 100.f * std::sin((pRot.x +  90.f) * 3.14f / 180.f) * std::sin((pRot.y +  90.f) * 3.14f / 180.f);
        pVel.y += pDelta * speed / 100.f * std::sin((pRot.y + 180.f) * 3.14f / 180.f);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        pVel.x += pDelta * speed / 100.f * std::sin((pRot.x + 270.f) * 3.14f / 180.f);
        pVel.z += pDelta * speed / 100.f * std::sin((pRot.x +   0.f) * 3.14f / 180.f);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        pVel.x += pDelta * speed / 100.f * std::sin((pRot.x +  90.f) * 3.14f / 180.f);
        pVel.z += pDelta * speed / 100.f * std::sin((pRot.x + 180.f) * 3.14f / 180.f);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
        pVel.y += pDelta * speed / 100.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
        pVel.y -= pDelta * speed / 100.f;

    pRot.x -= (pMousePos.x - sf::Mouse::getPosition(pWin).x) * pDelta / 50.f * sensitivity.x;
    pRot.y += (pMousePos.y - sf::Mouse::getPosition(pWin).y) * pDelta / 50.f * sensitivity.y;
    if (pRot.y > 90.f)
        pRot.y = 90.f;
    if (pRot.y < -90.f)
        pRot.y = -90.f;
    if (pRot.x >= 360)
        pRot.x -= 360;
    if (pRot.x < 0)
        pRot.x += 360;


    // mouse leaves left of window
    if (sf::Mouse::getPosition(pWin).x < 0)
        sf::Mouse::setPosition(sf::Vector2i(pWin.getSize().x + sf::Mouse::getPosition(pWin).x, sf::Mouse::getPosition(pWin).y), pWin);
    // mouse leaves right of window
    if (sf::Mouse::getPosition(pWin).x >= pWin.getSize().x)
        sf::Mouse::setPosition(sf::Vector2i(sf::Mouse::getPosition(pWin).x - pWin.getSize().x, sf::Mouse::getPosition(pWin).y), pWin);
    // mouse leaves top of window
    if (sf::Mouse::getPosition(pWin).y < 0)
        sf::Mouse::setPosition(sf::Vector2i(sf::Mouse::getPosition(pWin).x, pWin.getSize().y + sf::Mouse::getPosition(pWin).y), pWin);
    // mouse leaves bottom of window
    if (sf::Mouse::getPosition(pWin).y >= pWin.getSize().y)
        sf::Mouse::setPosition(sf::Vector2i(sf::Mouse::getPosition(pWin).x, sf::Mouse::getPosition(pWin).y - pWin.getSize().y), pWin);

    pMousePos = sf::Mouse::getPosition(pWin);
}


void collide(City& pCity, sf::Vector3f& pPos, sf::Vector3f& pVel)
{
    pVel.x *= 0.8f;
    pVel.y *= 0.8f;
    pVel.z *= 0.8f;
    pPos.x += pVel.x;
    pPos.y += pVel.y;
    pPos.z += pVel.z;

    if (pCity.getBlock(pPos.x / (float)SCALE + 0.5f + 1.f / (float)SCALE * (pVel.x > 0 ? 1 : -1), pPos.y / (float)SCALE - 0.5f, pPos.z / (float)SCALE + 0.5f).isSolid)
    {
        pPos.x -= pVel.x;
        pVel.x = 0;
    }
    if (pCity.getBlock(pPos.x / (float)SCALE + 0.5f, pPos.y / (float)SCALE - 0.5f + 1.f / (float)SCALE * (pVel.y > 0 ? 1 : -1), pPos.z / (float)SCALE + 0.5f).isSolid)
    {
        pPos.y -= pVel.y;
        pVel.y = 0;
    }
    if (pCity.getBlock(pPos.x / (float)SCALE + 0.5f, pPos.y / (float)SCALE - 0.5f, pPos.z / (float)SCALE + 0.5f + 1.f / (float)SCALE * (pVel.z > 0 ? 1 : -1)).isSolid)
    {
        pPos.z -= pVel.z;
        pVel.z = 0;
    }
}


void genLights(sf::VertexArray& pVert, int pIndex, Block pBlock, sf::Vector3f pPlayerPos, sf::Vector3f pPlayerRot, sf::Vector3f pObjPos, sf::Vector2i pLightPos, sf::Vector2i pLightSize, sf::Vector2u pWinSize, bool pXLightState, bool pZLightState)
{
    if (pObjPos.x == 0 && pObjPos.y == 0 && pObjPos.z == 0)
        return;
    else
    {
        pObjPos.x -= 0.5f;
        pObjPos.y -= 0.5f;
        pObjPos.z -= 0.5f;
    }
    if (pBlock.visXP && pPlayerPos.x > (pObjPos.x + 0.5f) * SCALE)
    {
        if (/**/project(pVert[pIndex + 0], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y + 0.5f - (pLightPos.y + pLightSize.y) / (float)TEXSIZE, pObjPos.z + 0.5f - (pLightPos.x) / (float)TEXSIZE),                pWinSize) &&
                project(pVert[pIndex + 1], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y + 0.5f - (pLightPos.y + pLightSize.y) / (float)TEXSIZE, pObjPos.z + 0.5f - (pLightPos.x + pLightSize.x) / (float)TEXSIZE), pWinSize) &&
                project(pVert[pIndex + 2], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y + 0.5f - (pLightPos.y) / (float)TEXSIZE,                pObjPos.z + 0.5f - (pLightPos.x + pLightSize.x) / (float)TEXSIZE), pWinSize) &&
                project(pVert[pIndex + 3], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y + 0.5f - (pLightPos.y) / (float)TEXSIZE,                pObjPos.z + 0.5f - (pLightPos.x) / (float)TEXSIZE),                pWinSize))
        {
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 0 + i].color = sf::Color(255 * pXLightState, 255 * pXLightState, 255 * pXLightState);
        }
        else
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 0 + i] = sf::Vertex();
    }
    else if (pBlock.visXN && pPlayerPos.x < (pObjPos.x + 0.5f) * SCALE)
    {
        if (/**/project(pVert[pIndex + 0], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y + 0.5f - (pLightPos.y + pLightSize.y) / (float)TEXSIZE, pObjPos.z + 0.5f - (pLightPos.x) / (float)TEXSIZE),                pWinSize) &&
                project(pVert[pIndex + 1], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y + 0.5f - (pLightPos.y + pLightSize.y) / (float)TEXSIZE, pObjPos.z + 0.5f - (pLightPos.x + pLightSize.x) / (float)TEXSIZE), pWinSize) &&
                project(pVert[pIndex + 2], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y + 0.5f - (pLightPos.y) / (float)TEXSIZE,                pObjPos.z + 0.5f - (pLightPos.x + pLightSize.x) / (float)TEXSIZE), pWinSize) &&
                project(pVert[pIndex + 3], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y + 0.5f - (pLightPos.y) / (float)TEXSIZE,                pObjPos.z + 0.5f - (pLightPos.x) / (float)TEXSIZE),                pWinSize))
        {
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 0 + i].color = sf::Color(255 * pXLightState, 255 * pXLightState, 255 * pXLightState);
        }
        else
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 0 + i] = sf::Vertex();
    }
    if (pBlock.visZP && pPlayerPos.z > (pObjPos.z + 0.5f) * SCALE)
    {
        if (/**/project(pVert[pIndex + 4], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f - (pLightPos.x) / (float)TEXSIZE,                pObjPos.y + 0.5f - (pLightPos.y + pLightSize.y) / (float)TEXSIZE, pObjPos.z + 0.5f), pWinSize) &&
                project(pVert[pIndex + 5], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f - (pLightPos.x + pLightSize.x) / (float)TEXSIZE, pObjPos.y + 0.5f - (pLightPos.y + pLightSize.y) / (float)TEXSIZE, pObjPos.z + 0.5f), pWinSize) &&
                project(pVert[pIndex + 6], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f - (pLightPos.x + pLightSize.x) / (float)TEXSIZE, pObjPos.y + 0.5f - (pLightPos.y) / (float)TEXSIZE,                pObjPos.z + 0.5f), pWinSize) &&
                project(pVert[pIndex + 7], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f - (pLightPos.x) / (float)TEXSIZE,                pObjPos.y + 0.5f - (pLightPos.y) / (float)TEXSIZE,                pObjPos.z + 0.5f), pWinSize))
        {
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 4 + i].color = sf::Color(255 * pXLightState, 255 * pXLightState, 255 * pXLightState);
        }
        else
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 4 + i] = sf::Vertex();
    }
    else if (pBlock.visZN && pPlayerPos.z < (pObjPos.z + 0.5f) * SCALE)
    {
        if (/**/project(pVert[pIndex + 4], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f - (pLightPos.x) / (float)TEXSIZE,                pObjPos.y + 0.5f - (pLightPos.y + pLightSize.y) / (float)TEXSIZE, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex + 5], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f - (pLightPos.x + pLightSize.x) / (float)TEXSIZE, pObjPos.y + 0.5f - (pLightPos.y + pLightSize.y) / (float)TEXSIZE, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex + 6], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f - (pLightPos.x + pLightSize.x) / (float)TEXSIZE, pObjPos.y + 0.5f - (pLightPos.y) / (float)TEXSIZE,                pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex + 7], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f - (pLightPos.x) / (float)TEXSIZE,                pObjPos.y + 0.5f - (pLightPos.y) / (float)TEXSIZE,                pObjPos.z - 0.5f), pWinSize))
        {
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 4 + i].color = sf::Color(255 * pXLightState, 255 * pXLightState, 255 * pXLightState);
        }
        else
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 4 + i] = sf::Vertex();
    }
}

