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

#include "gutil_vector.h"
#include <QWidget>

namespace GKChess{
    class Square;
    class ObservableBoard;
namespace UI{

class IFactory_PieceIcon;


/** A viewer you can use with the chess board model to display
 *  a chess position in a Qt application.
 *
 *  By default the view first tries to paint the pieces using the icon factory, if given.
 *  If no factory was given then it tries to paint icons provided by the model in the "Qt::DecorationRole".
 *  If there are no icons produced by the model then the view simply paints the unicode chess characters.
*/
class BoardView :
        public QWidget
{
    Q_OBJECT
    void *d;
public:
    BoardView(QWidget * = 0);
    ~BoardView();

    /** Returns the board used by this view. */
    ObservableBoard const *GetBoard() const;

    /** Returns the board used by this view. */
    ObservableBoard *GetBoard();

    /** Sets the board for the view.  The view does not own the board, only plays with it. */
    void SetBoard(ObservableBoard *);

    /** Tells the view to use the given icon factory, which overrides
        icons returned by the model.  Pass NULL to clear the icon factory.
    */
    void SetIconFactory(IFactory_PieceIcon *);
    IFactory_PieceIcon *GetIconFactory() const;

    /** Returns the pixel size of a square. */
    float GetSquareSize() const;

    /** Sets the pixel size of a square. */
    void SetSquareSize(float);

    QColor GetDarkSquareColor() const;
    void SetDarkSquareColor(const QColor &);

    QColor GetLightSquareColor() const;
    void SetLightSquareColor(const QColor &);

    QColor GetActiveSquareHighlightColor() const;
    void SetActiveSquareHighlightColor(const QColor &);

    /** Causes the square to be highlighted. */
    void HighlightSquare(const Square &, const QColor &);

    /** Causes all the squares to be highlighted. */
    void HighlightSquares(const GUtil::Vector<Square const *> &, const QColor &);

    /** Removes all highlighting from the board. */
    void ClearSquareHighlighting();

    /** Returns true if the board is editable. */
    bool Editable() const;

    /** Controls whether the view will allow the user to edit the board, such as moving pieces. */
    void SetEditable(bool);

    /** Controls whether the view will show threat counts on each square. */
    void SetShowThreatCounts(bool);

    /** Returns true if the view is showing threat counts. */
    bool GetShowThreatCounts() const;


public slots:

    /** Rotates the board 180 degrees. */
    void FlipOrientation();

};


}}

#endif // GKCHESS_BOARDVIEW_H
