/*****************************************************************************
 * Qwt Examples - Copyright (C) 2002 Uwe Rathmann
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

#pragma once

#include <QString>

class Settings
{
  public:
    Settings()
    {
        legendItem.isEnabled = false;
        legendItem.numColumns = 0;
        legendItem.alignment = 0;
        legendItem.backgroundMode = 0;
        legendItem.size = 12;       
    }

    struct
    {
        bool isEnabled;
        int numColumns;
        int alignment;
        int backgroundMode;
        int size;
    } legendItem;
};
