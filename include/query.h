/*
 * Copyright (C) 2015 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Kyle Nitzsche <kyle.nitzsche@canonical.com>
 *
 */

#ifndef QUERY_H
#define QUERY_H

#include "scope.h"

#include <unity/scopes/SearchQueryBase.h>
#include <unity/scopes/ReplyProxyFwd.h>

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QString>
#include <QByteArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QList>
#include <QFile>
#include <QLocale>


class Query : public unity::scopes::SearchQueryBase
{
public:
    Query(const unity::scopes::CannedQuery &query, const unity::scopes::SearchMetadata & metadata_, Scope &, const QString & cacheDir);
    ~Query();
    virtual void cancelled() override;
    virtual void run(unity::scopes::SearchReplyProxy const& reply) override;
private:
    Scope & scope_;
    QCoreApplication *app;
    QNetworkAccessManager manager;
    QEventLoop loop;
    QMap<QString,QString> getSuns(QFile &, QFile &);
    QString cacheDir_;
    QString mUserAgent;
    QString getLunarPhase(QFile &, QFile &);
    const unity::scopes::CannedQuery cannedQuery;
    void makeSignature();
    bool haveSignature = false;
    int secs = 84300;
    QByteArray service;
    QByteArray accessKey;
    QByteArray secretKey;
    QString signature;
    QString signatureEncoded;
    QString timestampUtc;
    QString timestampUtcEncoded;
    QString expiresUtc;
    QString lat;
    QString lng;
    QString lang;
    QString locale_qs;

    QFile lunarData_f;
    QString lunarData_filename;
    QFile sunData_f;
    QString sunData_filename;
    QFile lunarLastRefresh_f;
    QString lunarLastRefresh_filename;
    QFile sunsLastRefresh_f;
    QString sunsLastRefresh_filename;
    QString phase;
    QMap<QString,QString> suns;
    enum class data_t {suns, lunar};
    bool useNetwork(QFile &, QFile &, data_t);
    QString formatToday();
    QString parseLunarPhase(QJsonDocument &);
    void getFromNetwork(QFile &, QFile &, data_t);
    void getFromLocal(QFile &, QFile &, data_t);
};

#endif
