#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QStandardItemModel>
#include <QWidget>

#include "VirtualKeyboard.h"
#include "defines.h"
#include "downloadmanager.h"
#include "favoritesmanager.h"
#include "favoriteswidget.h"
#include "homewidget.h"
#include "jaiminisbox.h"
#include "mangadex.h"
#include "mangahub.h"
#include "mangakakalot.h"
#include "mangaowl.h"
#include "mangapanda.h"
#include "mangareaderwidget.h"
#include "settings.h"

namespace Ui
{
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();

public slots:

    void viewFavorite(QSharedPointer<MangaInfo> info, bool current);

    void setCurrentSource(AbstractMangaSource *source);
    void viewMangaInfo(QSharedPointer<MangaInfo> info);
    void viewMangaInfo(const QString &mangalink, const QString &mangatitle);
    void toggleFavorite(QSharedPointer<MangaInfo> manga);

    void viewMangaImage(const MangaIndex &index);
    void advanceMangaPage(bool direction);
    void readerGoBack();

    void setWidgetTab(int page);

    void setFrontLight(int light, int comflight);

    void enableVirtualKeyboard(bool enabled);

private slots:
    void on_pushButtonHome_clicked();
    void on_pushButtonClose_clicked();
    void on_pushButtonFavorites_clicked();

    void restoreFrontLight();

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *ev) override;

private:
    void setupUI();
    void setupFrontLight();
    void adjustSizes();
    void setupDirs();

    void setAddFavoriteButtonState(bool state);
    void updateReaderLabels();

    void readerShowImage(const QString &path);

    Ui::MainWidget *ui;

    DownloadManager downloadmanager;

    QList<AbstractMangaSource *> mangasources;

    AbstractMangaSource *currentsource;
    QSharedPointer<MangaInfo> currentmanga;

    FavoritesManager favoritesmanager;
    Settings settings;

    int lastTab;

    QTimer restorefrontlighttimer;
    VirtualKeyboard *virtualKeyboard;
};

#endif  // MAINWIDGET_H
