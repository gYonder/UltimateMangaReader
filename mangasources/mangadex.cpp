#include "mangadex.h"

MangaDex::MangaDex(QObject *parent, DownloadManager *dm)
    : AbstractMangaSource(parent, dm)
{
    name = "MangaDex";
    baseurl = "https://mangadex.org";

    //    downloadmanager->addCookie(".mangadex.org", "mangadex_h_toggle", "1");
    downloadManager->addCookie(".mangadex.org", "mangadex_title_mode", "2");
    downloadManager->addCookie(".mangadex.org", "mangadex_filter_langs", "1");
    //    downloadmanager->addCookie(".mangadex.org",
    //    "mangadex_rememberme_token",
    //    "ba07d6d335a2b433d4b57b396d99224cbfaf100cad243a50694161d681270c5a");

    login();
}

void MangaDex::login()
{
    QUrlQuery postData;
    postData.addQueryItem("login_username", "UMRBot2");
    postData.addQueryItem("login_password", "umrbot123");
    postData.addQueryItem("remember_me", "1");
    auto query = postData.query().toUtf8();

    QString loginurl(
        "https://mangadex.org/ajax/actions.ajax.php?function=login&nojs=1");

    auto job = downloadManager->downloadAsStringPost(loginurl, &query);

    job->await(1000);

    auto ncookies = job->getCookies();
    foreach (QNetworkCookie c, ncookies)
    {
        qDebug() << "Added cookie" << c.name() << c.value();
        downloadManager->addCookie(".mangadex.org", c.name(), c.value());
    }
}

MangaList MangaDex::getMangaList()
{
    QRegularExpression nummangasrx(
        R"(<p class=[^>]*>Showing .*? (\d+,\d+) titles)");

    QRegularExpression mangarx(
        R"lit(<a title=['"]([^'"]*?)['"][^<]*?href=['"]([^'"]*?)['"][^<]*?class=")lit");

    MangaList mangas;

    QString basedictlink = baseurl + "/titles/2/";

    auto job = downloadManager->downloadAsString(basedictlink + "1", -1);

    if (!job->await(5000))
    {
        emit updateError(job->errorString);
        return mangas;
    }

    emit updateProgress(10);

    QElapsedTimer timer;
    timer.start();

    auto nummangasrxmatch = nummangasrx.match(job->buffer);

    mangas.nominalSize = 0;
    if (nummangasrxmatch.hasMatch())
        mangas.nominalSize = nummangasrxmatch.captured(1).remove(',').toInt();

    int pages = (mangas.nominalSize + 99) / 100;
    //    pages = 5;
    qDebug() << "pages" << pages;

    auto lambda = [&](QSharedPointer<DownloadStringJob> job) {
        int matches = 0;
        for (auto &match : getAllRxMatches(mangarx, job->buffer))
        {
            mangas.links.append(match.captured(2) + "/chapters/");
            mangas.titles.append(
                htmlToPlainText(htmlToPlainText(match.captured(1))));
            matches++;
        }
        mangas.actualSize += matches;

        emit updateProgress(10 + 90 * mangas.actualSize / mangas.nominalSize);

        qDebug() << "matches:" << matches;
    };

    lambda(job);

    QList<QString> urls;
    for (int i = 2; i <= pages; i++)
        urls.append(basedictlink + QString::number(i));

    DownloadQueue queue(downloadManager, urls, maxparalleldownloads, lambda);

    queue.start();

    awaitSignal(&queue, {SIGNAL(allCompleted())}, 1000000);

    qDebug() << "mangas:" << mangas.actualSize << "time:" << timer.elapsed();

    emit updateProgress(100);

    return mangas;
}

void MangaDex::updateMangaInfoFinishedLoading(
    QSharedPointer<DownloadStringJob> job, QSharedPointer<MangaInfo> info)
{
    QRegularExpression titlerx(R"(class="mx-1">([^<]*)<)");

    QRegularExpression authorrx("Author:</div>[^>]*>[^>]*>([^<]*)");
    QRegularExpression artistrx("Artist:</div>[^>]*>[^>]*>([^<]*)");
    QRegularExpression statusrx("Pub. status:</div>[^>]*>([^<]*)");
    QRegularExpression yearrx;
    QRegularExpression demographicrx(
        "Demographic:</div>[^>]*>[^>]*>[^>]*>([^<]*)<");
    QRegularExpression genresrx("Genre:</div>[^>]*>[^>]*>([^<]*)<");

    QRegularExpression summaryrx("Description:</div>[^>]*>(.*?)</div>");

    QRegularExpression coverrx(
        R"lit(<img class="rounded" width="100%" src="([^\"]*)")lit");

    QRegularExpression chapterrx("<a href='(/chapter/[^']*)'[^>]*>([^<]*)</a>");

    QRegularExpression pagerx(
        R"(<p class='text-center'>Showing 1 to \d+ of ([\d,]+))");

    fillMangaInfo(info, job->buffer, titlerx, authorrx, artistrx, statusrx,
                  yearrx, genresrx, summaryrx, coverrx);

    auto demographicrxmatch = demographicrx.match(job->buffer);

    if (demographicrxmatch.hasMatch())
    {
        auto demo = htmlToPlainText(demographicrxmatch.captured(1).trimmed());

        if (info->genres != "")
            demo += ", ";
        else
            demo += " ";

        info->genres = demo + info->genres;
    }

    info->numChapters = 0;

    int pages = 1;

    auto pagerxmatch = pagerx.match(job->buffer);
    auto lambda = [&](QSharedPointer<DownloadStringJob> job) {
        for (auto &match : getAllRxMatches(chapterrx, job->buffer))
        {
            info->chapters.insert(
                0, MangaChapter(baseurl + match.captured(1), this));

            info->chaperTitleListDescending.append(match.captured(2));
            info->numChapters++;
        }

        //            qDebug() << "rx" << rxi ;
    };

    lambda(job);

    if (pagerxmatch.hasMatch())
    {
        int chapters = pagerxmatch.captured(1).remove(',').toInt();
        pages = (chapters + 99) / 100;

        QList<QString> urls;
        for (int i = 2; i <= pages; i++)
            urls.append(info->link + QString::number(i));

        DownloadQueue queue(downloadManager, urls, maxparalleldownloads,
                            lambda);

        queue.start();
        awaitSignal(&queue, {SIGNAL(allCompleted())}, 1000000);
    }
}

QStringList MangaDex::getPageList(const QString &chapterlink)
{
    QString scriptstart("<script");
    QRegularExpression baserx(R"(server\s+=\s+'([^']*)')");
    QRegularExpression datarx("var dataurl = '([^']*)'");
    QRegularExpression pagesrx("var page_array = \\[([^\\]]*)");

    auto job = downloadManager->downloadAsString(chapterlink);

    QStringList pageLinks;

    if (!job->await(3000))
        return pageLinks;

    int spos = job->buffer.indexOf(scriptstart);
    if (spos == -1)
        return pageLinks;

    auto baserxmatch = baserx.match(job->buffer);
    auto datarxmatch = datarx.match(job->buffer);
    auto pagesrxmatch = pagesrx.match(job->buffer);

    if (!baserxmatch.hasMatch() || !datarxmatch.hasMatch() ||
        !pagesrxmatch.hasMatch())
        return pageLinks;

    QString baselink =
        baserxmatch.captured(1).remove('\\') + datarxmatch.captured(1) + '/';

    for (QString s : pagesrxmatch.captured(1).split(','))
    {
        s = s.remove('\'').remove('\r').remove('\n');
        if (s != "")
            pageLinks.append(baselink + s.remove('\'').remove('\r'));
    }

    return pageLinks;
}

QString MangaDex::getImageLink(const QString &pagelink)
{
    // pagelinks are actually already imagelinks
    return pagelink;
}
