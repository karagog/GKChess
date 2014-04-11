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

#ifndef GKCHESS_BOARDMODEL_H
#define GKCHESS_BOARDMODEL_H

#include "gutil_map.h"
#include "gkchess_piece.h"
#include "gkchess_igamelogic.h"
#include <QColor>
#include <QAbstractTableModel>

namespace GKChess{ 
    class AbstractBoard;
    class ISquare;

namespace UI{


/** Describes a data model for a chess board. It supports readonly and editable
 *  experiences.
 *
 *  You supply game logic via SetGameLogic().  If you don't supply any game logic,
 *  then it behaves as a readonly model, allowing you to view positions but not edit them.
 *
 *  If you supply a game logic, then it immediately takes effect without disrupting the position
 *  on the board.
 *
 *  This can be used with Qt's model-view framework.
*/
class BoardModel :
        public QAbstractTableModel
{
    Q_OBJECT

    struct formatting_options_t{
        QColor BackgroundColor;
    };

    AbstractBoard *m_board;
    IGameLogic *i_logic;
public:

    /** You must give the model a reference to a chessboard
     *  object. It must exist at least for the lifetime of
     *  this object.
     *
     *  By default there is no game logic, meaning you cannot move any pieces,
     *  only look at the position on the board.
    */
    explicit BoardModel(AbstractBoard *, QObject *parent = 0);

    /** Returns a const reference to the board object, so you can
        access the other meta-data not held in the indices.
    */
    AbstractBoard const *GetBoard() const{ return m_board; }

    /** Sets the game logic. */
    void SetGameLogic(IGameLogic *);

    /** Returns the current game logic. */
    IGameLogic *GetGameLogic() const{ return i_logic; }

    /** Returns a reference to the square at the given index.
     *  It will return 0 on errors.
    */
    ISquare const *ConvertIndexToSquare(const QModelIndex &) const;

    /** Returns the model index corresponding to the given square. */
    QModelIndex ConvertSquareToIndex(const ISquare &) const;


    /** Defines the custom data roles implemented by the board model. */
    enum CustomDataRoleEnum
    {
        /** This role returns the Piece on the square, or a null QVariant if the square is empty. */
        PieceRole = Qt::UserRole,

        /** This role returns a list of model indexes to which the piece on the given index can move. */
        ValidMovesRole = Qt::UserRole + 1
    };


    /** Returns true if the piece moving from source to dest is a valid chess move. */
    bool ValidateMove(const QModelIndex &source, const QModelIndex &dest) const;

    /** Moves the piece at the source index to the dest index.
     *
     *  This function does nothing if there is no game logic set.
     *
     *  You must supply a player response object, to handle pawn promotions when they occur.
    */
    void MovePiece(const QModelIndex &source, const QModelIndex &dest, IGameLogic::IPlayerResponse *);

    /** \name QAbstractTableModel interface
     *  \{
    */
    virtual int rowCount(const QModelIndex & = QModelIndex()) const;
    virtual int columnCount(const QModelIndex & = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &, int) const;
    virtual bool setData(const QModelIndex &, const QVariant &, int);
    virtual QVariant headerData(int, Qt::Orientation, int) const;
    virtual Qt::ItemFlags flags(const QModelIndex &) const;

    virtual QMimeData *mimeData(const QModelIndexList &) const;
    virtual bool dropMimeData(const QMimeData *, Qt::DropAction, int, int, const QModelIndex &);
    virtual QStringList mimeTypes() const;
    /** \} */

private slots:

    void _square_updated(const GKChess::ISquare &);
    void _piece_moved(const GKChess::MoveData &);

};


}}


/** Defines the mimetype used for dragging and dropping pieces. You can use this
 *  to drop pieces onto the board.
 *
 *  The data is an ascii string with the following format:   <piece>:<column>,<row>
 *  Where the piece is a single char used in FEN notation and the column and row are
 *  0-based indices describing the source of the piece. Leave the source info blank
 *  if it describes a new piece.
*/
#define MIMETYPE_GKCHESS_PIECE "gkchess/piece"


Q_DECLARE_METATYPE(QModelIndexList)


#endif // GKCHESS_BOARDMODEL_H
