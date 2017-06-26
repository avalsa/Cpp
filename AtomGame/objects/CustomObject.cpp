//
// Created by svuatoslav on 5/14/17.
//

#include "CustomObject.h"
#include "tinyxml2.h"

CustomObject::CustomObject (int x, int y, int sizeX, int sizeY, const char *file) : PhysicalObject (x,
                                                                                                    y,
                                                                                                    sizeX,
                                                                                                    sizeY),
                                                                                    proc (nullptr)
{
    if (file == nullptr)
        return;
    tinyxml2::XMLDocument settings;
    if (int err = settings.LoadFile (file))
    {
        logger.warn ("Failed to load settings from \"%s\", errorID = %d", file, err);
    } else
    {
        logger.info ("\"%s\" parsed", file);
    }
    if (tinyxml2::XMLElement *block = settings.FirstChildElement ("Block"))
        load (block);
}

PhysicalObject::Position CustomObject::tick ()
{
    if (proc)
    {
        proc->setReg (_x, 0);
        proc->setReg (_y, 1);
        proc->setReg (_vx, 2);
        proc->setReg (_vy, 3);
        proc->setReg (_ax, 4);
        proc->setReg (_ay, 5);
        proc->setReg(_st, 6);
        proc->execute ();
        proc->reset ();
        _x = proc->getReg (0);
        _y = proc->getReg (1);
        _vx = proc->getReg (2);
        _vy = proc->getReg (3);
        _ax = proc->getReg (4);
        _ay = proc->getReg (5);
    }
    return PhysicalObject::tick ();
}

CustomObject::~CustomObject ()
{
    if (proc != NULL)
        delete proc;
}

CustomObject CustomObject::clone () const
{
    CustomObject obj (*this);
    obj.proc = new (std::nothrow) VCPU (*(this->proc));
    if (obj.proc == nullptr)
    {
        logger.error ("Out of memory, aborting");
        abort ();
    }
}

CustomObject::CustomObject (const CustomObject &that) : PhysicalObject (that._x, that._y, that._sizeX, that._sizeY),
                                                        proc (
                                                                nullptr)
{
}

CustomObject::CustomObject (int x, int y, int sizeX, int sizeY, tinyxml2::XMLElement *block) : PhysicalObject (x, y,
                                                                                                               sizeX,
                                                                                                               sizeY),
                                                                                               proc (nullptr)
{
    if (block == nullptr)
        return;
    load (block);
}

void CustomObject::load (tinyxml2::XMLElement *block)
{
    if (tinyxml2::XMLElement *script = block->FirstChildElement ("Script"))
    {
        if (const char *prog = script->Attribute ("file"))
        {
            proc = new (std::nothrow) VCPU;
            if (proc)
            {
                std::ifstream in (prog, std::ios_base::binary);
                if (proc->load (in))
                    logger.info ("Loaded program from \"%s\"", prog);
                else
                {
                    logger.warn ("Failed to load program from \"%s\"", prog);
                    delete proc;
                    proc = nullptr;
                }
            } else
            {
                logger.error ("Out of memory, aborting");
                abort ();
            }

        }
    }
    if (tinyxml2::XMLElement *coordinates = block->FirstChildElement ("Place"))
    {
        if (coordinates->Attribute ("X"))
            _x = coordinates->IntAttribute ("X");
        if (coordinates->Attribute ("Y"))
            _y = coordinates->IntAttribute ("Y");
    }
    if (tinyxml2::XMLElement *size = block->FirstChildElement ("Size"))
    {
        if (size->Attribute ("X"))
            _sizeX = size->IntAttribute ("X");
        if (size->Attribute ("Y"))
            _sizeY = size->IntAttribute ("Y");
    }
    if (tinyxml2::XMLElement *info = block->FirstChildElement ("Step"))
    {
        if (info->Attribute ("Count"))
            _st = info->IntAttribute ("Count");
    }
    if (tinyxml2::XMLElement *velocity = block->FirstChildElement ("Velocity"))
    {
        if (velocity->Attribute ("X"))
            _vx = velocity->IntAttribute ("X");
        if (velocity->Attribute ("Y"))
            _vy = velocity->IntAttribute ("Y");
    }
    if (tinyxml2::XMLElement *acceleration = block->FirstChildElement ("Acceleration"))
    {
        if (acceleration->Attribute ("X"))
            _ax = acceleration->IntAttribute ("X");
        if (acceleration->Attribute ("Y"))
            _ay = acceleration->IntAttribute ("Y");
    }
    if (const char *type = block->Attribute ("Type"))
    {
        if (strcmp (type, "Deadly") == 0)
            setType (Deadly);
        else if (strcmp (type, "Respawn") == 0)
            setType (Respawn);
        else if (strcmp (type, "Portal") == 0)
            setType (Portal);
        else if (strcmp (type, "MapChange") == 0)
            setType (MapChange);
        else if (strcmp (type, "Coin") == 0)
            setType (Coin);
        else if (strcmp (type, "Bot") == 0)
            setType (Bot);
    }
}

CustomObject::CustomObject (int x, int y, int sizeX, int sizeY, PhysicalObject::BlockType type) : PhysicalObject (x, y,
                                                                                                                  sizeX,
                                                                                                                  sizeY,
                                                                                                                  type),
                                                                                                  proc (nullptr)
{
}
