#include "mangainfowidget.h"

#include <QResizeEvent>
#include <QScrollBar>

#include "cscrollbar.h"
#include "qstringlistmodel.h"
#include "ui_mangainfowidget.h"

MangaInfoWidget::MangaInfoWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::MangaInfoWidget), currentmanga()
{
    ui->setupUi(this);
    adjustSizes();
}

MangaInfoWidget::~MangaInfoWidget() { delete ui; }

void MangaInfoWidget::adjustSizes()
{
    ui->pushButtonReadContinue->setMinimumHeight(buttonsize);
    ui->pushButtonReadFirst->setMinimumHeight(buttonsize);
    ui->pushButtonReadLatest->setMinimumHeight(buttonsize);
    ui->pushButtonMangaInfoAddFavorites->setMinimumHeight(
        buttonsizeaddfavorite);
    ui->pushButtonMangaInfoAddFavorites->setMaximumWidth(buttonsizeaddfavorite);
    ui->pushButtonMangaInfoAddFavorites->setFocusPolicy(Qt::NoFocus);
    ui->pushButtonMangaInfoAddFavorites->setText("");
    ui->pushButtonMangaInfoAddFavorites->setIconSize(
        QSize(buttonsizeaddfavorite * 0.7, buttonsizeaddfavorite * 0.7));

    isfavoriteicon = QIcon(
        QPixmap(":/resources/images/icons/favourite-star-full.png")
            .scaledToHeight(buttonsizeaddfavorite, Qt::SmoothTransformation));
    isnotfavoriteicon = QIcon(
        QPixmap(":/resources/images/icons/favourite-star.png")
            .scaledToHeight(buttonsizeaddfavorite, Qt::SmoothTransformation));

    ui->listViewChapters->setVerticalScrollBar(
        new CScrollBar(Qt::Vertical, ui->listViewChapters));
    ui->listViewChapters->setHorizontalScrollBar(
        new CScrollBar(Qt::Horizontal, ui->listViewChapters));
    ui->listViewChapters->setVerticalScrollMode(
        QAbstractItemView::ScrollPerPixel);
    ui->listViewChapters->setUniformItemSizes(true);

    ui->scrollAreaMangaInfoSummary->setVerticalScrollBar(new CScrollBar(
        Qt::Vertical, ui->scrollAreaMangaInfoSummary, summaryscrollbarwidth));

    ui->labelMangaInfoTitle->setStyleSheet("QLabel{ font-size: 18pt; }");

    QString favbuttonstyle =
        "QPushButton {         "
        "    background: white;"
        "    border: none;     "
        "    color: black;     "
        "    outline: none;    "
        "}                     "
        "QPushButton:focus {   "
        "    border: none;     "
        "}                     "
        "QPushButton:pressed { "
        "    border: none;     "
        "}                     ";

    ui->pushButtonMangaInfoAddFavorites->setStyleSheet(favbuttonstyle);

#ifndef WINDOWS

    // TODO
//    activate_scroller(qobject_cast<QAbstractScrollArea
//    *>(ui->scrollAreaMangaInfoSummary));
#endif
}

inline void updateLabel(QLabel *caption, QLabel *content, const QString &text)
{
    bool hide = text.length() <= 1;
    caption->setHidden(hide);
    content->setHidden(hide);

    content->setText(text);
}

void MangaInfoWidget::setManga(QSharedPointer<MangaInfo> manga)
{
    if (currentmanga.get() != manga.get())
    {
        currentmanga.clear();
        currentmanga = manga;
    }

    QStringListModel *model = new QStringListModel(this);
    model->setStringList(currentmanga->chaperTitleListDescending);

    if (ui->listViewChapters->model() != nullptr)
        ui->listViewChapters->model()->deleteLater();

    ui->listViewChapters->setModel(model);

    ui->labelMangaInfoTitle->setText(currentmanga->title);

    updateLabel(ui->labelMangaInfoLabelAuthor,
                ui->labelMangaInfoLabelAuthorContent, currentmanga->author);
    updateLabel(ui->labelMangaInfoLabelArtist,
                ui->labelMangaInfoLabelArtistContent, currentmanga->artist);
    updateLabel(ui->labelMangaInfoLabelGenres,
                ui->labelMangaInfoLabelGenresContent, currentmanga->genres);
    updateLabel(ui->labelMangaInfoLabelStaus,
                ui->labelMangaInfoLabelStausContent, currentmanga->status);

    ui->labelMangaInfoLabelSummaryContent->setText(currentmanga->summary);

    ui->scrollAreaMangaInfoSummary->verticalScrollBar()->setValue(0);
    ui->listViewChapters->verticalScrollBar()->setValue(0);

    bool enable = currentmanga->numChapters > 0;

    ui->pushButtonReadContinue->setEnabled(enable);
    ui->pushButtonReadFirst->setEnabled(enable);
    ui->pushButtonReadLatest->setEnabled(enable);

    updateCover();

    QObject::connect(currentmanga.get(), SIGNAL(updatedSignal()), this,
                     SLOT(updateManga()));

    QObject::connect(currentmanga.get(), SIGNAL(coverLoaded()), this,
                     SLOT(updateCover()));
}

void MangaInfoWidget::updateManga()
{
    qDebug() << "update";
    ui->labelMangaInfoLabelStaus->setText(currentmanga->status);
    static_cast<QStringListModel *>(ui->listViewChapters->model())
        ->setStringList(currentmanga->chaperTitleListDescending);
}

void MangaInfoWidget::updateCover()
{
    if (!QFile::exists(currentmanga->coverPath))
    {
        ui->labelMangaInfoCover->clear();
    }
    else
    {
        QPixmap img;
        img.load(currentmanga->coverPath);
        ui->labelMangaInfoCover->setPixmap(
            img.scaled(coversize, coversize, Qt::KeepAspectRatio,
                       Qt::SmoothTransformation));
    }
}

void MangaInfoWidget::setFavoriteButtonState(bool state)
{
    if (state)
        ui->pushButtonMangaInfoAddFavorites->setIcon(isfavoriteicon);
    else
        ui->pushButtonMangaInfoAddFavorites->setIcon(isnotfavoriteicon);
}

void MangaInfoWidget::on_pushButtonMangaInfoAddFavorites_clicked()
{
    if (!currentmanga.isNull())
        emit toggleFavoriteClicked(currentmanga);
}

void MangaInfoWidget::on_listViewChapters_clicked(const QModelIndex &index)
{
    emit readMangaClicked(
        MangaIndex(currentmanga->numChapters - 1 - index.row(), 0));
}

void MangaInfoWidget::on_pushButtonReadLatest_clicked()
{
    emit readMangaClicked(MangaIndex(currentmanga->numChapters - 1, 0));
}

void MangaInfoWidget::on_pushButtonReadContinue_clicked()
{
    emit readMangaClicked(currentmanga->currentIndex);
}

void MangaInfoWidget::on_pushButtonReadFirst_clicked()
{
    emit readMangaClicked(MangaIndex(0, 0));
}
