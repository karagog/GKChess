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

#ifndef CHESS960GENERATOR_H
#define CHESS960GENERATOR_H

#include <QDialog>

namespace Ui {
class Chess960Generator;
}

namespace GKChess{ namespace UI{


class Chess960Generator : public QDialog
{
    Q_OBJECT
    QString m_fen;
    bool m_ignoreSpinbox;
public:
    explicit Chess960Generator(QWidget *parent = 0);
    ~Chess960Generator();

    const QString &GetFEN() const{ return m_fen; }


public slots:

    /** Generates a random position if index is == -1.  Gets the specific position if it's >= 0 */
    void Generate(int index = -1);

    /** Copies the current FEN string to clipboard. */
    void CopyToClipboard();


private:
    Ui::Chess960Generator *ui;
};


}}

#endif // CHESS960GENERATOR_H
