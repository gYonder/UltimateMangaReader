#ifndef MANGAINFOWIDGET_H
#define MANGAINFOWIDGET_H

#include <QIcon>
#include <QWidget>

#include "defines.h"
#include "mangainfo.h"

namespace Ui
{
class MangaInfoWidget;
}

class MangaInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MangaInfoWidget(QWidget *parent = 0);
    ~MangaInfoWidget();

    void setManga(QSharedPointer<MangaInfo> manga);
    void setFavoriteButtonState(bool state);

signals:
    void toggleFavoriteClicked(QSharedPointer<MangaInfo> manga);
    void readMangaClicked(MangaIndex index);

public slots:
    void updateManga();
    void updateCover();

private slots:
    void on_pushButtonMangaInfoAddFavorites_clicked();

    void on_listViewChapters_clicked(const QModelIndex &index);

    void on_pushButtonReadLatest_clicked();
    void on_pushButtonReadContinue_clicked();
    void on_pushButtonReadFirst_clicked();

private:
    Ui::MangaInfoWidget *ui;

    QSharedPointer<MangaInfo> currentmanga;

    QIcon isfavoriteicon;
    QIcon isnotfavoriteicon;

    void adjustSizes();
};

#endif  // MANGAINFOWIDGET_H
