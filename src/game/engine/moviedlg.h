#pragma once

#ifndef MOVIEDLG_H
#define MOVIEDLG_H

#include "movie.h"
#include "dialog.h"

class MovieChoiceClass
{
public:
    void Draw_It(int index, int x, int y, int xmax, int ymax, BOOL selected, TextPrintType style);
    
    bool Is_Allowed()
    {
        if (IsAvailable && Length != 0) {
            return true;
        }
        return false;
    }

    static void Scan();

public:
    const char *BaseName;
    const char *Description;
    MovieType Movie;
    int Length;
    bool IsAvailable;
    bool IsMovie640;
};

class MovieControlsClass
{
public:
    int Process();//remove return, its here just so i can simply test this by replacing load menu
};

#endif //MOVIEDLG_H
