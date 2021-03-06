#ifndef FAVORITESWIDGET_H
#define FAVORITESWIDGET_H

#include <QWidget>

#include "favoritesmanager.h"

namespace Ui
{
class FavoritesWidget;
}

class FavoritesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FavoritesWidget(QWidget *parent = 0);
    ~FavoritesWidget();

    void showFavoritesList();
    FavoritesManager *favoritesmanager;

signals:
    void favoriteClicked(QSharedPointer<MangaInfo> fav, bool current);
    void mangaListUpdated();

private slots:
    void on_tableWidget_cellClicked(int row, int column);
    void mangaUpdated();
    void coverLoaded();

private:
    Ui::FavoritesWidget *ui;

    void insertRow(const QSharedPointer<MangaInfo> &fav, int row);
    void adjustSizes();
    void moveFavoriteToFront(int i);

    QWidget *makeIconTextWidget(const QString &path, const QString &text,
                                const QSize &iconsize);
};

#endif  // FAVORITESWIDGET_H
