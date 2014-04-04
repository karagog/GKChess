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

#ifndef GKCHESS_BOARDVIEW_H
#define GKCHESS_BOARDVIEW_H

#include "gutil_smartpointer.h"
#include "gutil_vector.h"
#include <QWidget>

namespace GKChess{
    class ISquare;
    class AbstractBoard;
namespace UI{


class BoardModel;
class IFactory_PieceIcon;


/** A viewer you can use with the chess board model to display
 *  a chess position in a Qt application.
 * 
 *  The base boardview class takes care of all the basic functions of displaying
 *  a board, but does not allow you to edit it or move pieces (if you want editing then you
 *  need the BoardEdit, and if you want to play a game you need the GameView).
 * 
 *  By default the view first tries to paint the pieces using the icon factory, if given.
 *  If no factory was given then it tries to paint icons provided by the model in the "Qt::DecorationRole".
 *  If there are no icons produced by the model then the view simply paints the unicode chess characters.
*/
class BoardView :
        public QWidget
{
    void *ptr;
public:
    BoardView(QWidget * = 0);
    ~BoardView();

    BoardModel *GetBoardModel() const;
    void SetBoardModel(BoardModel *);

    /** Tells the view to use the given icon factory, which overrides
        icons returned by the model.  Pass NULL to clear the icon factory.
    */
    void SetIconFactory(IFactory_PieceIcon *);
    IFactory_PieceIcon *GetIconFactory() const;

    float GetSquareSize() const;
    void SetSquareSize(float);

    QColor GetDarkSquareColor() const;
    void SetDarkSquareColor(const QColor &);

    QColor GetLightSquareColor() const;
    void SetLightSquareColor(const QColor &);

    QColor GetActiveSquareHighlightColor() const;
    void SetActiveSquareHighlightColor(const QColor &);

    /** Causes the square to be highlighted. */
    void HighlightSquare(const ISquare &, const QColor &);

    /** Causes all the squares to be highlighted. */
    void HighlightSquares(const GUtil::Vector<ISquare const *> &, const QColor &);

    /** Removes all highlighting from the board. */
    void ClearSquareHighlighting();

};


}}

#endif // GKCHESS_BOARDVIEW_H
