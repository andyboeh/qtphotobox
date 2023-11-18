#include "curlWorker.h"
#include "settings.h"
#include <curl/curl.h>
#include <QDateTime>
#include <QUuid>


static const char inline_text[] =
    "Please see the attachment for the QtPhotobox picture.\r\n";

static const char inline_html[] =
    "<html><body>\r\n"
    "<p>Please see the attachment for the QtPhotobox picture.</p>"
    "</body></html>\r\n";

curlWorker::curlWorker()
{
    pbSettings &pbs = pbSettings::getInstance();
}

curlWorker::~curlWorker()
{

}

void curlWorker::start()
{
    bool running = true;
    pbSettings &pbs = pbSettings::getInstance();

    emit started();

    while(running) {
        waitForCommand();

        if(mCommandList.isEmpty())
            continue;

        mMutex.lock();
        QString command = mCommandList.takeFirst();
        mMutex.unlock();
        if(command.startsWith("sendMail")) {
            CURL *curl;
            CURLcode res = CURLE_OK;
            curl_mime *mime;
            curl_mime *alt;
            curl_mimepart *part;
            struct curl_slist *recipients = NULL;
            struct curl_slist *headers = NULL;
            struct curl_slist *slist = NULL;
            QString path = command.split(":").at(1);

            curl = curl_easy_init();
            if(curl) {
                curl_easy_setopt(curl, CURLOPT_URL, pbs.get("upload", "mailserver").toStdString().c_str());
                if(pbs.getBool("upload", "usessl"))
                    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
                curl_easy_setopt(curl, CURLOPT_USERNAME, pbs.get("upload", "mailuser").toStdString().c_str());
                curl_easy_setopt(curl, CURLOPT_PASSWORD, pbs.get("upload", "mailpassword").toStdString().c_str());
                curl_easy_setopt(curl, CURLOPT_LOGIN_OPTIONS, "AUTH=PLAIN");
                curl_easy_setopt(curl, CURLOPT_MAIL_FROM, pbs.get("upload", "mailfrom").toStdString().c_str());
                QStringList recpts = pbs.get("upload", "mailto").split(",");
                QStringList headerRecpts;;
                foreach(QString recpt, recpts) {
                    headerRecpts.append("<" + recpt + ">");
                    recipients = curl_slist_append(recipients, recpt.toStdString().c_str());
                }
                curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

                QStringList headersList;
                headersList.append("Date: " + QDateTime::currentDateTime().toString(Qt::RFC2822Date));
                headersList.append("To: " + headerRecpts.join(", "));
                headersList.append("From: <" + pbs.get("upload", "mailfrom") + ">");
                headersList.append("Message-ID: <" + QUuid::createUuid().toString() + "@qtphotobox.aboehler.at>");
                headersList.append("Subject: Picture from QtPhotobox");

                foreach(QString header, headersList)
                    headers = curl_slist_append(headers, header.toStdString().c_str());
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

                mime = curl_mime_init(curl);
                alt = curl_mime_init(curl);

                part = curl_mime_addpart(alt);
                curl_mime_data(part, inline_html, CURL_ZERO_TERMINATED);
                curl_mime_type(part, "text/html");

                part = curl_mime_addpart(alt);
                curl_mime_data(part, inline_text, CURL_ZERO_TERMINATED);

                part = curl_mime_addpart(mime);
                curl_mime_subparts(part, alt);
                curl_mime_type(part, "multipart/alternative");
                slist = curl_slist_append(NULL, "Content-Disposition: inline");
                curl_mime_headers(part, slist, 1);

                part = curl_mime_addpart(mime);
                curl_mime_filedata(part, path.toStdString().c_str());
                curl_mime_encoder(part, "base64");
                curl_mime_type(part, "image/jpeg");
                curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

                curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
                res = curl_easy_perform(curl);

                if(res != CURLE_OK) {
                    qDebug() << "curl_easy_perform() failed" << curl_easy_strerror(res);
                    qDebug() << "Re-queuing mail";
                    sendAttachmentViaEmail(path);
                }

                curl_slist_free_all(headers);
                curl_slist_free_all(recipients);
                curl_easy_cleanup(curl);
                curl_mime_free(mime);
            }
        } else if(command == "stopThread") {
            running = false;
        }
    }

    emit finished();
}

void curlWorker::stop()
{
    qDebug() << "stopThread in curlWorker";
    mMutex.lock();
    mCommandList.append("stopThread");
    mMutex.unlock();
}

void curlWorker::sendAttachmentViaEmail(QString path)
{
    mMutex.lock();
    mCommandList.append("sendMail:"+path);
    mMutex.unlock();
}
