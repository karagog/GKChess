/*Copyright 2014 George Karagoulis

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#ifndef GKCHESS_HTMLBOARDGENERATOR_H
#define GKCHESS_HTMLBOARDGENERATOR_H

#include "gkchess_globals.h"
#include <QString>
#include <QColor>

NAMESPACE_GKCHESS;


class AbstractBoard;

class HtmlBoardGenerator
{
public:

    /** Describes the available options for the board as an html table. */
    struct HtmlFormattingOptions
    {
        /** Controls whether the html will be formatted prettily. */
        bool HumanReadable;

        /** Controls the color of the pieces. */
        QColor PieceColor;

        /** Controls the color of the light squares. */
        QColor LightSquareColor;

        /** Controls the color of the dark squares. */
        QColor DarkSquareColor;

        /** The number of html pts for the size of a square. */
        int SquareSize;

        /** The thickness of the border between squares. */
        int BorderSize;

        /** The size of the pieces. */
        int PieceSize;

        /** The size of the index characters for the rows and columns (1-8, a-h). */
        int IndexSize;

        /** Initializes some useful defaults. */
        HtmlFormattingOptions();
    };

    /** Returns an HTML table of the chess board with the given formatting options.
     *  You can then display this in a web browser.
    */
    static QString GenerateHtml(const AbstractBoard &,
                                const HtmlFormattingOptions & = HtmlFormattingOptions());

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_HTMLBOARDGENERATOR_H
